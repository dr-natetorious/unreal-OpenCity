#include "Character/OpenCityCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/LocalPlayer.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "InputAction.h"
#include "InputActionValue.h"
#include "InputMappingContext.h"
#include "Vehicle/CarPawn.h"

AOpenCityCharacter::AOpenCityCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    // Capsule: 34cm radius, 90cm half-height → 1.8m total height at 1m=100UU scale.
    GetCapsuleComponent()->InitCapsuleSize(34.f, 90.f);

    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw   = false;
    bUseControllerRotationRoll  = false;

    GetCharacterMovement()->bOrientRotationToMovement = true;
    GetCharacterMovement()->RotationRate              = FRotator(0.f, 500.f, 0.f);
    GetCharacterMovement()->MaxWalkSpeed              = 600.f;
    GetCharacterMovement()->MinAnalogWalkSpeed        = 20.f;
    GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
    GetCharacterMovement()->JumpZVelocity             = 600.f;
    GetCharacterMovement()->AirControl                = 0.35f;

    // ── Simple geometry body ────────────────────────────────────────────────
    // Cylinder: default mesh is 100cm diameter × 100cm tall.
    // Scale (0.5, 0.5, 1.4) → 50cm wide, 140cm tall.
    static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderAsset(
        TEXT("/Engine/BasicShapes/Cylinder"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereAsset(
        TEXT("/Engine/BasicShapes/Sphere"));

    BodyMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BodyMesh"));
    BodyMesh->SetupAttachment(GetCapsuleComponent());
    BodyMesh->SetRelativeLocation(FVector(0.f, 0.f, -15.f));
    BodyMesh->SetRelativeScale3D(FVector(0.5f, 0.5f, 1.4f));
    BodyMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    if (CylinderAsset.Succeeded())
        BodyMesh->SetStaticMesh(CylinderAsset.Object);

    // Sphere: default mesh is 100cm diameter.
    // Scale (0.3, 0.3, 0.3) → 30cm head at top of body.
    HeadMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HeadMesh"));
    HeadMesh->SetupAttachment(GetCapsuleComponent());
    HeadMesh->SetRelativeLocation(FVector(0.f, 0.f, 75.f));
    HeadMesh->SetRelativeScale3D(FVector(0.3f, 0.3f, 0.3f));
    HeadMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    if (SphereAsset.Succeeded())
        HeadMesh->SetStaticMesh(SphereAsset.Object);

    // ── Camera ──────────────────────────────────────────────────────────────
    CameraArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraArm"));
    CameraArm->SetupAttachment(RootComponent);
    CameraArm->TargetArmLength         = 400.f;
    CameraArm->bUsePawnControlRotation = true;

    FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
    FollowCamera->SetupAttachment(CameraArm, USpringArmComponent::SocketName);
    FollowCamera->bUsePawnControlRotation = false;
}

void AOpenCityCharacter::NotifyControllerChanged()
{
    Super::NotifyControllerChanged();

    // Auto-load from known content paths if not assigned in the editor/Blueprint.
    // This means the character works as soon as Tools/setup_content.py has been run,
    // with no Blueprint subclass required.
    if (!FootMappingContext)
        FootMappingContext = LoadObject<UInputMappingContext>(nullptr, TEXT("/Game/Input/IMC_Foot.IMC_Foot"));
    if (!MoveAction)
        MoveAction = LoadObject<UInputAction>(nullptr, TEXT("/Game/Input/IA_Foot_Move.IA_Foot_Move"));
    if (!LookAction)
        LookAction = LoadObject<UInputAction>(nullptr, TEXT("/Game/Input/IA_Foot_Look.IA_Foot_Look"));
    if (!JumpAction)
        JumpAction = LoadObject<UInputAction>(nullptr, TEXT("/Game/Input/IA_Foot_Jump.IA_Foot_Jump"));

    if (!InteractAction)
        InteractAction = LoadObject<UInputAction>(nullptr, TEXT("/Game/Input/IA_Interact.IA_Interact"));

    if (auto* PC = Cast<APlayerController>(Controller))
    {
        if (auto* EIS = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
        {
            if (FootMappingContext)
                EIS->AddMappingContext(FootMappingContext, 0);
        }
    }
}

void AOpenCityCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    if (auto* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent))
    {
        EIC->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AOpenCityCharacter::HandleMove);
        EIC->BindAction(LookAction, ETriggerEvent::Triggered, this, &AOpenCityCharacter::HandleLook);
        EIC->BindAction(JumpAction,     ETriggerEvent::Started,   this, &ACharacter::Jump);
        EIC->BindAction(JumpAction,     ETriggerEvent::Completed, this, &ACharacter::StopJumping);
        EIC->BindAction(InteractAction, ETriggerEvent::Started,   this, &AOpenCityCharacter::HandleInteract);
    }
}

void AOpenCityCharacter::Tick(float /*DeltaSeconds*/)
{
    if (NearbyCar && IsValid(NearbyCar))
        GEngine->AddOnScreenDebugMessage(1, 0.f, FColor::White, TEXT("Press E to enter car"));
}

void AOpenCityCharacter::HandleInteract(const FInputActionValue&)
{
    if (NearbyCar && IsValid(NearbyCar))
        NearbyCar->EnterCar(this);
}

void AOpenCityCharacter::HandleMove(const FInputActionValue& Value)
{
    const FVector2D V = Value.Get<FVector2D>();
    if (!Controller) return;

    const FRotator Yaw(0.f, Controller->GetControlRotation().Yaw, 0.f);
    AddMovementInput(FRotationMatrix(Yaw).GetUnitAxis(EAxis::X), V.Y);
    AddMovementInput(FRotationMatrix(Yaw).GetUnitAxis(EAxis::Y), V.X);
}

void AOpenCityCharacter::HandleLook(const FInputActionValue& Value)
{
    const FVector2D V = Value.Get<FVector2D>();
    AddControllerYawInput(V.X);
    AddControllerPitchInput(V.Y);
}
