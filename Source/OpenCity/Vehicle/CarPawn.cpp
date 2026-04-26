#include "Vehicle/CarPawn.h"
#include "Camera/CameraComponent.h"
#include "ChaosWheeledVehicleMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/LocalPlayer.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/SpringArmComponent.h"
#include "InputActionValue.h"

ACarPawn::ACarPawn()
{
    PrimaryActorTick.bCanEverTick = true;

    GetMesh()->SetSimulatePhysics(true);
    GetMesh()->SetCollisionProfileName(FName("Vehicle"));

    VehicleMovement = CastChecked<UChaosWheeledVehicleMovementComponent>(GetVehicleMovement());

    CameraArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraArm"));
    CameraArm->SetupAttachment(GetMesh());
    CameraArm->TargetArmLength         = 650.f;
    CameraArm->SocketOffset.Z          = 150.f;
    CameraArm->bDoCollisionTest        = false;
    CameraArm->bInheritPitch           = false;
    CameraArm->bInheritRoll            = false;
    CameraArm->bEnableCameraRotationLag = true;
    CameraArm->CameraRotationLagSpeed  = 2.f;

    FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
    FollowCamera->SetupAttachment(CameraArm);
}

void ACarPawn::NotifyControllerChanged()
{
    Super::NotifyControllerChanged();

    if (auto* PC = Cast<APlayerController>(Controller))
    {
        if (auto* EIS = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
        {
            EIS->AddMappingContext(VehicleMappingContext, 0);
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
        EIC->BindAction(BrakeAction,     ETriggerEvent::Triggered, this, &ACarPawn::HandleBrake);
        EIC->BindAction(BrakeAction,     ETriggerEvent::Completed, this, &ACarPawn::HandleBrake);
        EIC->BindAction(HandbrakeAction, ETriggerEvent::Started,   this, &ACarPawn::HandleHandbrake);
        EIC->BindAction(HandbrakeAction, ETriggerEvent::Completed, this, &ACarPawn::HandleHandbrake);
    }
}

void ACarPawn::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    // Increase angular damping while airborne to prevent uncontrolled spinning.
    GetMesh()->SetAngularDamping(VehicleMovement->IsMovingOnGround() ? 0.f : 3.f);

    // Gently realign the chase camera to face the car's forward direction.
    float CamYaw = CameraArm->GetRelativeRotation().Yaw;
    CameraArm->SetRelativeRotation(FRotator(0.f, FMath::FInterpTo(CamYaw, 0.f, DeltaSeconds, 1.f), 0.f));
}

void ACarPawn::HandleThrottle(const FInputActionValue& Value)
{
    VehicleMovement->SetThrottleInput(Value.Get<float>());
}

void ACarPawn::HandleSteering(const FInputActionValue& Value)
{
    VehicleMovement->SetSteeringInput(Value.Get<float>());
}

void ACarPawn::HandleBrake(const FInputActionValue& Value)
{
    VehicleMovement->SetBrakeInput(Value.Get<float>());
}

void ACarPawn::HandleHandbrake(const FInputActionValue& Value)
{
    VehicleMovement->SetHandbrakeInput(Value.Get<bool>());
}
