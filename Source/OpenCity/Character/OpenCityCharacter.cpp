#include "Character/OpenCityCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Engine/LocalPlayer.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "InputAction.h"
#include "InputActionValue.h"
#include "InputMappingContext.h"

AOpenCityCharacter::AOpenCityCharacter()
{
    PrimaryActorTick.bCanEverTick = false;

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

    CameraArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraArm"));
    CameraArm->SetupAttachment(RootComponent);
    CameraArm->TargetArmLength        = 400.f;
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
        EIC->BindAction(JumpAction, ETriggerEvent::Started,   this, &ACharacter::Jump);
        EIC->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
    }
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
