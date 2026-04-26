#include "Vehicle/CarPawn.h"
#include "Camera/CameraComponent.h"
#include "Character/OpenCityCharacter.h"
#include "Components/BoxComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/LocalPlayer.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/SpringArmComponent.h"
#include "InputAction.h"
#include "InputMappingContext.h"

// ── Scale convention: 1 m = 100 UU ──────────────────────────────────────────
// Car body  : 450 × 180 × 140 UU  (4.5 m × 1.8 m × 1.4 m)
// Wheels    : cylinder 70cm dia, 30cm wide
// Root sits at floor level; body centre is half-height (70 UU) above root.

static UStaticMeshComponent* MakeWheelMesh(ACarPawn* Owner, const TCHAR* Name,
    UStaticMesh* CylinderMesh, USceneComponent* Parent, FVector LocalPos)
{
    auto* W = Owner->CreateDefaultSubobject<UStaticMeshComponent>(Name);
    W->SetupAttachment(Parent);
    W->SetRelativeLocation(LocalPos);
    // Cylinder default: 100cm tall (Z), 100cm dia. Rotate 90° pitch → axle along Y.
    W->SetRelativeRotation(FRotator(90.f, 0.f, 0.f));
    W->SetRelativeScale3D(FVector(0.7f, 0.3f, 0.7f));
    W->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    if (CylinderMesh) W->SetStaticMesh(CylinderMesh);
    return W;
}

ACarPawn::ACarPawn()
{
    PrimaryActorTick.bCanEverTick = true;

    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeAsset(
        TEXT("/Engine/BasicShapes/Cube"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> CylAsset(
        TEXT("/Engine/BasicShapes/Cylinder"));

    // Root collision box matches car body footprint
    CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
    CollisionBox->SetBoxExtent(FVector(225.f, 90.f, 70.f));
    CollisionBox->SetCollisionProfileName(TEXT("Pawn"));
    SetRootComponent(CollisionBox);

    // Body mesh: cube scaled to car dims, centre at half-height above root
    BodyMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BodyMesh"));
    BodyMesh->SetupAttachment(CollisionBox);
    BodyMesh->SetRelativeLocation(FVector(0.f, 0.f, 70.f));
    BodyMesh->SetRelativeScale3D(FVector(4.5f, 1.8f, 1.4f));
    BodyMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    if (CubeAsset.Succeeded()) BodyMesh->SetStaticMesh(CubeAsset.Object);

    // Wheel positions in root space: (±wheelbase/2, ±track/2, wheel_radius)
    // Wheelbase 280cm, track 190cm, wheel radius 35cm
    UStaticMesh* Cyl = CylAsset.Succeeded() ? CylAsset.Object : nullptr;
    WheelFL = MakeWheelMesh(this, TEXT("WheelFL"), Cyl, CollisionBox, FVector( 140.f, -95.f, 35.f));
    WheelFR = MakeWheelMesh(this, TEXT("WheelFR"), Cyl, CollisionBox, FVector( 140.f,  95.f, 35.f));
    WheelRL = MakeWheelMesh(this, TEXT("WheelRL"), Cyl, CollisionBox, FVector(-140.f, -95.f, 35.f));
    WheelRR = MakeWheelMesh(this, TEXT("WheelRR"), Cyl, CollisionBox, FVector(-140.f,  95.f, 35.f));

    // Proximity sphere for enter/exit prompt
    ProximitySphere = CreateDefaultSubobject<USphereComponent>(TEXT("ProximitySphere"));
    ProximitySphere->SetupAttachment(CollisionBox);
    ProximitySphere->SetSphereRadius(250.f);
    ProximitySphere->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
    ProximitySphere->OnComponentBeginOverlap.AddDynamic(this, &ACarPawn::OnProximityBegin);
    ProximitySphere->OnComponentEndOverlap.AddDynamic(this, &ACarPawn::OnProximityEnd);

    // Chase camera — higher and further back than character cam
    CameraArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraArm"));
    CameraArm->SetupAttachment(CollisionBox);
    CameraArm->SetRelativeLocation(FVector(0.f, 0.f, 70.f));
    CameraArm->TargetArmLength          = 650.f;
    CameraArm->SocketOffset             = FVector(0.f, 0.f, 100.f);
    CameraArm->bUsePawnControlRotation  = true;
    CameraArm->bDoCollisionTest         = false;
    CameraArm->bInheritPitch            = false;
    CameraArm->bInheritRoll             = false;
    CameraArm->bEnableCameraRotationLag = true;
    CameraArm->CameraRotationLagSpeed   = 4.f;

    FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
    FollowCamera->SetupAttachment(CameraArm, USpringArmComponent::SocketName);
    FollowCamera->bUsePawnControlRotation = false;
}

// ── Overlap callbacks ────────────────────────────────────────────────────────

void ACarPawn::OnProximityBegin(UPrimitiveComponent*, AActor* Other,
    UPrimitiveComponent*, int32, bool, const FHitResult&)
{
    if (auto* Char = Cast<AOpenCityCharacter>(Other))
    {
        NearbyCharacter = Char;
        Char->SetNearbyCar(this);
    }
}

void ACarPawn::OnProximityEnd(UPrimitiveComponent*, AActor* Other,
    UPrimitiveComponent*, int32)
{
    if (auto* Char = Cast<AOpenCityCharacter>(Other))
    {
        NearbyCharacter = nullptr;
        Char->SetNearbyCar(nullptr);
    }
}

// ── Enter / Exit ─────────────────────────────────────────────────────────────

void ACarPawn::EnterCar(AOpenCityCharacter* Driver)
{
    if (OccupyingDriver || !Driver) return;

    OccupyingDriver = Driver;
    NearbyCharacter = nullptr;

    Driver->SetActorHiddenInGame(true);
    Driver->SetActorEnableCollision(false);

    if (auto* PC = Cast<APlayerController>(Driver->GetController()))
    {
        PC->Possess(this);
    }
}

static void ExitCarToSide(ACarPawn* Car, AOpenCityCharacter* Driver)
{
    // Place driver beside the car (right side) slightly above floor
    const FVector Right    = Car->GetActorRightVector();
    const FVector ExitPos  = Car->GetActorLocation() + Right * 150.f + FVector(0.f, 0.f, 100.f);

    Driver->SetActorLocation(ExitPos, false, nullptr, ETeleportType::TeleportPhysics);
    Driver->SetActorHiddenInGame(false);
    Driver->SetActorEnableCollision(true);
}

// ── Controller / Input ───────────────────────────────────────────────────────

void ACarPawn::NotifyControllerChanged()
{
    Super::NotifyControllerChanged();

    if (auto* PC = Cast<APlayerController>(Controller))
    {
        // Load input assets here so they are ready before SetupPlayerInputComponent runs
        if (!VehicleMappingContext)
            VehicleMappingContext = LoadObject<UInputMappingContext>(
                nullptr, TEXT("/Game/Input/IMC_Vehicle.IMC_Vehicle"));
        if (!ThrottleAction)
            ThrottleAction = LoadObject<UInputAction>(
                nullptr, TEXT("/Game/Input/IA_Vehicle_Throttle.IA_Vehicle_Throttle"));
        if (!SteeringAction)
            SteeringAction = LoadObject<UInputAction>(
                nullptr, TEXT("/Game/Input/IA_Vehicle_Steer.IA_Vehicle_Steer"));
        if (!InteractAction)
            InteractAction = LoadObject<UInputAction>(
                nullptr, TEXT("/Game/Input/IA_Interact.IA_Interact"));

        if (auto* EIS = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(
                PC->GetLocalPlayer()))
        {
            if (VehicleMappingContext)
                EIS->AddMappingContext(VehicleMappingContext, 1);
        }
    }
}

void ACarPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    if (auto* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent))
    {
        EIC->BindAction(ThrottleAction,  ETriggerEvent::Triggered, this, &ACarPawn::HandleThrottle);
        EIC->BindAction(ThrottleAction,  ETriggerEvent::Completed, this, &ACarPawn::HandleThrottle);
        EIC->BindAction(SteeringAction,  ETriggerEvent::Triggered, this, &ACarPawn::HandleSteering);
        EIC->BindAction(SteeringAction,  ETriggerEvent::Completed, this, &ACarPawn::HandleSteering);
        EIC->BindAction(InteractAction,  ETriggerEvent::Started,   this, &ACarPawn::HandleInteract);
    }
}

// ── Input handlers ───────────────────────────────────────────────────────────

void ACarPawn::HandleThrottle(const FInputActionValue& Value)
{
    CurrentThrottle = Value.Get<float>();
}

void ACarPawn::HandleSteering(const FInputActionValue& Value)
{
    CurrentSteering = Value.Get<float>();
}

void ACarPawn::HandleInteract(const FInputActionValue&)
{
    if (!OccupyingDriver) return;

    AOpenCityCharacter* Driver = OccupyingDriver;
    OccupyingDriver = nullptr;
    CurrentThrottle = 0.f;
    CurrentSteering = 0.f;

    if (auto* PC = Cast<APlayerController>(Controller))
    {
        // Remove vehicle mapping context before re-possessing character
        if (auto* EIS = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(
                PC->GetLocalPlayer()))
        {
            if (VehicleMappingContext)
                EIS->RemoveMappingContext(VehicleMappingContext);
        }

        ExitCarToSide(this, Driver);
        PC->Possess(Driver);
    }
}

// ── Tick ─────────────────────────────────────────────────────────────────────

void ACarPawn::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    // Show prompt when a character is nearby and car is unoccupied
    if (NearbyCharacter && !OccupyingDriver)
    {
        GEngine->AddOnScreenDebugMessage(
            1, 0.f, FColor::White, TEXT("Press E to enter car"));
    }

    if (!OccupyingDriver) return;

    // ── Movement ─────────────────────────────────────────────────────────────
    const float SpeedCmS    = CurrentThrottle * MaxSpeedCmS;
    const FVector Delta     = GetActorForwardVector() * SpeedCmS * DeltaSeconds;
    const float   SteerDeg  = CurrentSteering * TurnRateDegS * DeltaSeconds
                              * FMath::Abs(CurrentThrottle); // no spin in place

    AddActorWorldOffset(Delta, false);
    AddActorWorldRotation(FRotator(0.f, SteerDeg, 0.f));

    // ── Wheel spin ───────────────────────────────────────────────────────────
    const float Circumference = 2.f * PI * WheelRadiusCm;
    const float RotDeg = (SpeedCmS * DeltaSeconds / Circumference) * 360.f;
    WheelAngle = FMath::Fmod(WheelAngle + RotDeg, 360.f);

    // Wheels are rotated 90° pitch at spawn; spin them around their local Z (axle)
    const FRotator WheelRot(90.f, 0.f, WheelAngle);
    WheelFL->SetRelativeRotation(WheelRot);
    WheelFR->SetRelativeRotation(WheelRot);
    WheelRL->SetRelativeRotation(WheelRot);
    WheelRR->SetRelativeRotation(WheelRot);
}
