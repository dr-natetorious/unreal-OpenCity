#include "Character/OpenCityCharacter.h"
#include "Character/CharacterSwitchWidget.h"
#include "AbilitySystemComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/LocalPlayer.h"
#include "Engine/SkeletalMesh.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "Blueprint/UserWidget.h"
#include "InputAction.h"
#include "InputActionValue.h"
#include "InputCoreTypes.h"
#include "InputMappingContext.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Vehicle/CarPawn.h"

DEFINE_LOG_CATEGORY_STATIC(LogOpenCityCharacterAppearance, Log, All);

namespace
{
    UCharacterDataAsset *MakeRoleData(UObject *Outer,
                                      const TCHAR *Name,
                                      const TArray<FGameplayTag> &Capabilities,
                                      const FName HatStyle,
                                      const FName AccessoryStyle)
    {
        UCharacterDataAsset *Data = NewObject<UCharacterDataAsset>(Outer);
        Data->CharacterName = FText::FromString(Name);
        Data->Appearance.HatStyle = HatStyle;
        Data->Appearance.AccessoryStyle = AccessoryStyle;
        Data->Capabilities.Capabilities = Capabilities;
        return Data;
    }

    FString DescribeCapabilities(const FCapabilitySet &Capabilities)
    {
        TArray<FString> Names;
        Names.Reserve(Capabilities.Capabilities.Num());
        for (const FGameplayTag &Tag : Capabilities.Capabilities)
            Names.Add(Tag.ToString());
        return Names.Num() > 0 ? FString::Join(Names, TEXT(", ")) : TEXT("<none>");
    }

    FString DescribeCharacterData(const UCharacterDataAsset *Data)
    {
        if (!Data)
            return TEXT("<null CharacterData>");

        const FString CharacterName = !Data->CharacterName.IsEmpty() ? Data->CharacterName.ToString() : TEXT("<unnamed>");
        const FAppearanceParams &Appearance = Data->Appearance;
        return FString::Printf(
            TEXT("Name=%s | ShirtMaterial=%s | PantsMaterial=%s | ShoesMaterial=%s | FaceMaterial=%s | HatMesh=%s | LeftHand=%s | RightHand=%s | ShirtColor=%s | PantsColor=%s | SkinColor=%s | FaceColor=%s | ShoesColor=%s | HatStyle=%s | AccessoryStyle=%s | FaceStyle=%s | Caps=[%s]"),
            *CharacterName,
            Appearance.ShirtMaterial ? *Appearance.ShirtMaterial->GetName() : TEXT("<null>"),
            Appearance.PantsMaterial ? *Appearance.PantsMaterial->GetName() : TEXT("<null>"),
            Appearance.ShoesMaterial ? *Appearance.ShoesMaterial->GetName() : TEXT("<null>"),
            Appearance.FaceMaterial ? *Appearance.FaceMaterial->GetName() : TEXT("<null>"),
            Appearance.HatMesh ? *Appearance.HatMesh->GetName() : TEXT("<null>"),
            Appearance.LeftHandAccessoryMesh ? *Appearance.LeftHandAccessoryMesh->GetName() : TEXT("<null>"),
            Appearance.RightHandAccessoryMesh ? *Appearance.RightHandAccessoryMesh->GetName() : TEXT("<null>"),
            *Appearance.ShirtColor.ToString(),
            *Appearance.PantsColor.ToString(),
            *Appearance.SkinColor.ToString(),
            *Appearance.FaceColor.ToString(),
            *Appearance.ShoesColor.ToString(),
            *Appearance.HatStyle.ToString(),
            *Appearance.AccessoryStyle.ToString(),
            *Appearance.FaceStyle.ToString(),
            *DescribeCapabilities(Data->Capabilities));
    }
}

AOpenCityCharacter::AOpenCityCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    AbilitySystem = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystem"));

    // Capsule: 34cm radius, 90cm half-height → 1.8m total height at 1m=100UU scale.
    GetCapsuleComponent()->InitCapsuleSize(34.f, 90.f);

    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw = false;
    bUseControllerRotationRoll = false;

    GetCharacterMovement()->bOrientRotationToMovement = true;
    GetCharacterMovement()->RotationRate = FRotator(0.f, 500.f, 0.f);
    GetCharacterMovement()->MaxWalkSpeed = 600.f;
    GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
    GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
    GetCharacterMovement()->JumpZVelocity = 600.f;
    GetCharacterMovement()->AirControl = 0.35f;

    // ── Skeletal mesh ────────────────────────────────────────────────────────
    // CharacterMesh replaces the old 4-component static mesh setup.
    // The actual mesh asset is assigned via CharacterData->Appearance or the editor.
    CharacterMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh"));
    CharacterMesh->SetupAttachment(GetCapsuleComponent());
    CharacterMesh->SetRelativeLocation(FVector(0.f, 0.f, -90.f));
    CharacterMesh->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));
    CharacterMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    HatMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HatMesh"));
    HatMeshComponent->SetupAttachment(CharacterMesh);
    HatMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    LeftHandAccessoryMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LeftHandAccessoryMesh"));
    LeftHandAccessoryMeshComponent->SetupAttachment(CharacterMesh);
    LeftHandAccessoryMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    RightHandAccessoryMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RightHandAccessoryMesh"));
    RightHandAccessoryMeshComponent->SetupAttachment(CharacterMesh);
    RightHandAccessoryMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    // Try to load the shared character mesh at construction time.
    // When running headless (NullRHI) the asset won't exist yet, so this silently
    // no-ops — no crash, just an empty mesh until the asset is imported.
    if (USkeletalMesh *DefaultMesh = LoadObject<USkeletalMesh>(
            nullptr, TEXT("/Game/Props/Characters/SK_CityCharacter.SK_CityCharacter")))
    {
        CharacterMesh->SetSkeletalMesh(DefaultMesh);
    }

    // ── Camera ──────────────────────────────────────────────────────────────
    CameraArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraArm"));
    CameraArm->SetupAttachment(RootComponent);
    CameraArm->TargetArmLength = 400.f;
    CameraArm->bUsePawnControlRotation = true;

    FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
    FollowCamera->SetupAttachment(CameraArm, USpringArmComponent::SocketName);
    FollowCamera->bUsePawnControlRotation = false;
}

UAbilitySystemComponent *AOpenCityCharacter::GetAbilitySystemComponent() const
{
    return AbilitySystem;
}

void AOpenCityCharacter::BeginPlay()
{
    Super::BeginPlay();
    EnsureDefaultSwitchRoster();
    // Auto-assign Officer as default for locally-controlled players. NPCs and
    // test-spawned actors without a controller keep whatever data was assigned
    // externally (or none), so existing tests that check the null-data path are unaffected.
    if (!CharacterData && IsLocallyControlled() && SwitchableCharacters.Num() > 0)
        CharacterData = SwitchableCharacters[0];
    ApplyCharacterData();

    // Create the UMG switch overlay once for locally-controlled players.
    if (IsLocallyControlled())
    {
        if (APlayerController *PC = Cast<APlayerController>(GetController()))
        {
            SwitchWidget = CreateWidget<UCharacterSwitchWidget>(PC, UCharacterSwitchWidget::StaticClass());
            if (SwitchWidget)
                SwitchWidget->SetRoster(SwitchableCharacters);
        }
    }
}

void AOpenCityCharacter::ApplyCharacterData()
{
    if (!CharacterData)
    {
        UE_LOG(LogOpenCityCharacterAppearance,
               Warning,
               TEXT("ApplyCharacterData skipped on %s: CharacterData is null"),
               *GetName());
        return;
    }

    UE_LOG(LogOpenCityCharacterAppearance,
           Warning,
           TEXT("ApplyCharacterData begin on %s | %s"),
           *GetName(),
           *DescribeCharacterData(CharacterData));

    ApplyAppearanceData();
    ApplyCapabilityTagsToASC();

    UE_LOG(LogOpenCityCharacterAppearance,
           Warning,
           TEXT("ApplyCharacterData end on %s | ActiveCaps=[%s]"),
           *GetName(),
           *DescribeCapabilities(CharacterData->Capabilities));
}

void AOpenCityCharacter::ApplyAppearanceData()
{
    if (!CharacterData || !CharacterData->Appearance.AreValid())
    {
        UE_LOG(LogOpenCityCharacterAppearance,
               Warning,
               TEXT("ApplyAppearanceData skipped on %s | CharacterData=%s | AppearanceValid=%s"),
               *GetName(),
               CharacterData ? *CharacterData->GetName() : TEXT("<null>"),
               CharacterData && CharacterData->Appearance.AreValid() ? TEXT("true") : TEXT("false"));
        return;
    }

    const FAppearanceParams &Appearance = CharacterData->Appearance;
    const FLinearColor ShirtColor = CharacterAppearancePolicy::ResolveShirtColor(
        CharacterData->Appearance,
        CharacterData->Capabilities);

    UE_LOG(LogOpenCityCharacterAppearance,
           Warning,
           TEXT("ApplyAppearanceData on %s | ResolvedShirtColor=%s | Mesh=%s"),
           *GetName(),
           *ShirtColor.ToString(),
           CharacterMesh && CharacterMesh->GetSkeletalMeshAsset() ? *CharacterMesh->GetSkeletalMeshAsset()->GetName() : TEXT("<null>"));

    ApplySlotMaterial(CharacterMeshSlot::Skin, Appearance.SkinMaterial);
    ApplySlotMaterial(CharacterMeshSlot::Face, Appearance.FaceMaterial);
    ApplySlotMaterial(CharacterMeshSlot::Shirt, Appearance.ShirtMaterial);
    ApplySlotMaterial(CharacterMeshSlot::Pants, Appearance.PantsMaterial);
    ApplySlotMaterial(CharacterMeshSlot::Shoes, Appearance.ShoesMaterial);

    ApplySlotColor(CharacterMeshSlot::Skin, Appearance.SkinColor, TEXT("SkinTint"));
    ApplySlotColor(CharacterMeshSlot::Face, Appearance.FaceColor, TEXT("FaceTint"));
    ApplySlotColor(CharacterMeshSlot::Shirt, ShirtColor, TEXT("ShirtTint"));
    ApplySlotColor(CharacterMeshSlot::Pants, Appearance.PantsColor, TEXT("PantsTint"));
    ApplySlotColor(CharacterMeshSlot::Shoes, Appearance.ShoesColor, TEXT("ShoesTint"));

    ApplyAttachmentMesh(HatMeshComponent, Appearance.HatMesh, Appearance.HatSocket);
    ApplyAttachmentMesh(LeftHandAccessoryMeshComponent,
                        Appearance.LeftHandAccessoryMesh,
                        Appearance.LeftHandAccessorySocket);
    ApplyAttachmentMesh(RightHandAccessoryMeshComponent,
                        Appearance.RightHandAccessoryMesh,
                        Appearance.RightHandAccessorySocket);
}

bool AOpenCityCharacter::HasCapability(const FGameplayTag &Tag) const
{
    if (!Tag.IsValid() || !CharacterData)
        return false;
    return CharacterData->Capabilities.HasCapability(Tag);
}

void AOpenCityCharacter::ApplyCapabilityTagsToASC()
{
    if (!AbilitySystem)
        return;

    for (const FGameplayTag &Tag : AppliedCapabilityTags)
        AbilitySystem->RemoveLooseGameplayTag(Tag);
    AppliedCapabilityTags.Reset();

    if (!CharacterData)
        return;

    for (const FGameplayTag &Tag : CharacterData->Capabilities.Capabilities)
    {
        if (!Tag.IsValid())
            continue;
        AbilitySystem->AddLooseGameplayTag(Tag);
        AppliedCapabilityTags.AddTag(Tag);
    }
}

void AOpenCityCharacter::ApplySlotMaterial(const FName &SlotName, UMaterialInterface *Material)
{
    if (!CharacterMesh)
        return;

    if (!Material)
    {
        UE_LOG(LogOpenCityCharacterAppearance,
               Warning,
               TEXT("ApplySlotMaterial on %s | Slot=%s | Material=<null> (skipped)"),
               *GetName(),
               *SlotName.ToString());
        return;
    }

    const int32 SlotIndex = CharacterMesh->GetMaterialIndex(SlotName);
    if (SlotIndex == INDEX_NONE)
    {
        UE_LOG(LogOpenCityCharacterAppearance,
               Error,
               TEXT("ApplySlotMaterial on %s | Slot=%s not found on mesh %s | Material=%s"),
               *GetName(),
               *SlotName.ToString(),
               CharacterMesh->GetSkeletalMeshAsset() ? *CharacterMesh->GetSkeletalMeshAsset()->GetName() : TEXT("<null mesh>"),
               *Material->GetName());
        return;
    }

    CharacterMesh->SetMaterial(SlotIndex, Material);

    UE_LOG(LogOpenCityCharacterAppearance,
           Warning,
           TEXT("ApplySlotMaterial on %s | Slot=%s | SlotIndex=%d | Material=%s"),
           *GetName(),
           *SlotName.ToString(),
           SlotIndex,
           *Material->GetName());
}

void AOpenCityCharacter::ApplySlotColor(const FName &SlotName,
                                        const FLinearColor &Color,
                                        const FName &PrimaryParameterName)
{
    if (!CharacterMesh)
        return;

    const int32 SlotIndex = CharacterMesh->GetMaterialIndex(SlotName);
    if (SlotIndex == INDEX_NONE)
    {
        UE_LOG(LogOpenCityCharacterAppearance,
               Error,
               TEXT("ApplySlotColor on %s | Slot=%s not found on mesh %s | Color=%s | Param=%s"),
               *GetName(),
               *SlotName.ToString(),
               CharacterMesh->GetSkeletalMeshAsset() ? *CharacterMesh->GetSkeletalMeshAsset()->GetName() : TEXT("<null mesh>"),
               *Color.ToString(),
               *PrimaryParameterName.ToString());
        return;
    }

    // Dynamic material creation is a no-op under NullRHI in headless automation.
    if (UMaterialInstanceDynamic *Mat = CharacterMesh->CreateDynamicMaterialInstance(SlotIndex))
    {
        Mat->SetVectorParameterValue(PrimaryParameterName, Color);
        Mat->SetVectorParameterValue(TEXT("TintColor"), Color);
        Mat->SetVectorParameterValue(TEXT("Color"), Color);

        UE_LOG(LogOpenCityCharacterAppearance,
               Warning,
               TEXT("ApplySlotColor on %s | Slot=%s | SlotIndex=%d | MID=%s | Param=%s | Color=%s | AlsoSet=[TintColor,Color]"),
               *GetName(),
               *SlotName.ToString(),
               SlotIndex,
               *Mat->GetName(),
               *PrimaryParameterName.ToString(),
               *Color.ToString());
    }
    else
    {
        UE_LOG(LogOpenCityCharacterAppearance,
               Error,
               TEXT("ApplySlotColor on %s | Slot=%s | SlotIndex=%d | Failed to create MID | Color=%s | Param=%s"),
               *GetName(),
               *SlotName.ToString(),
               SlotIndex,
               *Color.ToString(),
               *PrimaryParameterName.ToString());
    }
}

void AOpenCityCharacter::ApplyAttachmentMesh(UStaticMeshComponent *Component,
                                             UStaticMesh *Mesh,
                                             const FName &SocketName)
{
    if (!Component || !CharacterMesh)
        return;

    Component->SetStaticMesh(Mesh);
    if (!Mesh)
    {
        UE_LOG(LogOpenCityCharacterAppearance,
               Warning,
               TEXT("ApplyAttachmentMesh on %s | Component=%s | Mesh=<null> | Socket=%s (cleared)"),
               *GetName(),
               *Component->GetName(),
               *SocketName.ToString());
        return;
    }

    Component->AttachToComponent(CharacterMesh,
                                 FAttachmentTransformRules::SnapToTargetNotIncludingScale,
                                 SocketName);

    UE_LOG(LogOpenCityCharacterAppearance,
           Warning,
           TEXT("ApplyAttachmentMesh on %s | Component=%s | Mesh=%s | Socket=%s"),
           *GetName(),
           *Component->GetName(),
           *Mesh->GetName(),
           *SocketName.ToString());
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

    if (auto *PC = Cast<APlayerController>(Controller))
    {
        if (auto *EIS = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
        {
            if (FootMappingContext)
                EIS->AddMappingContext(FootMappingContext, 0);
        }
    }
}

void AOpenCityCharacter::ToggleCharacterSwitchMode()
{
    EnsureDefaultSwitchRoster();
    if (SwitchableCharacters.Num() == 0)
        return;

    // Ensure a role is highlighted when opening (covers headless test callers too).
    if (!CharacterData)
        CharacterData = SwitchableCharacters[SelectedSwitchIndex];

    bCharacterSwitchModeOpen = !bCharacterSwitchModeOpen;

    UE_LOG(LogOpenCityCharacterAppearance,
           Warning,
           TEXT("ToggleCharacterSwitchMode on %s | Open=%s | SelectedIndex=%d | Selected=%s"),
           *GetName(),
           bCharacterSwitchModeOpen ? TEXT("true") : TEXT("false"),
           SelectedSwitchIndex,
           SwitchableCharacters.IsValidIndex(SelectedSwitchIndex) && SwitchableCharacters[SelectedSwitchIndex]
               ? *SwitchableCharacters[SelectedSwitchIndex]->CharacterName.ToString()
               : TEXT("<invalid>"));

    UpdateSwitchOverlay();
}

void AOpenCityCharacter::CycleSwitchSelection(int32 Delta)
{
    EnsureDefaultSwitchRoster();
    if (SwitchableCharacters.Num() == 0)
        return;

    SelectedSwitchIndex = (SelectedSwitchIndex + Delta) % SwitchableCharacters.Num();
    if (SelectedSwitchIndex < 0)
        SelectedSwitchIndex += SwitchableCharacters.Num();

    UE_LOG(LogOpenCityCharacterAppearance,
           Warning,
           TEXT("CycleSwitchSelection on %s | Delta=%d | NewIndex=%d | NewSelection=%s"),
           *GetName(),
           Delta,
           SelectedSwitchIndex,
           SwitchableCharacters[SelectedSwitchIndex] ? *SwitchableCharacters[SelectedSwitchIndex]->CharacterName.ToString() : TEXT("<null>"));

    UpdateSwitchOverlay();
}

void AOpenCityCharacter::ConfirmSwitchSelection()
{
    EnsureDefaultSwitchRoster();
    if (SwitchableCharacters.Num() == 0)
        return;

    UE_LOG(LogOpenCityCharacterAppearance,
           Warning,
           TEXT("ConfirmSwitchSelection on %s | Previous=%s | NewIndex=%d | NewSelection=%s"),
           *GetName(),
           CharacterData ? *CharacterData->CharacterName.ToString() : TEXT("<null>"),
           SelectedSwitchIndex,
           SwitchableCharacters[SelectedSwitchIndex] ? *SwitchableCharacters[SelectedSwitchIndex]->CharacterName.ToString() : TEXT("<null>"));

    CharacterData = SwitchableCharacters[SelectedSwitchIndex];
    ApplyCharacterData();
    bCharacterSwitchModeOpen = false;
    UpdateSwitchOverlay();
}

void AOpenCityCharacter::CancelSwitchSelection()
{
    bCharacterSwitchModeOpen = false;
    UpdateSwitchOverlay();
}

void AOpenCityCharacter::SetupPlayerInputComponent(UInputComponent *PlayerInputComponent)
{
    if (auto *EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent))
    {
        EIC->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AOpenCityCharacter::HandleMove);
        EIC->BindAction(LookAction, ETriggerEvent::Triggered, this, &AOpenCityCharacter::HandleLook);
        EIC->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
        EIC->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
        EIC->BindAction(InteractAction, ETriggerEvent::Started, this, &AOpenCityCharacter::HandleInteract);
    }
}

void AOpenCityCharacter::Tick(float /*DeltaSeconds*/)
{
    if (IsLocallyControlled())
    {
        if (APlayerController *PC = Cast<APlayerController>(Controller))
        {
            if (PC->WasInputKeyJustPressed(EKeys::Tab))
                ToggleCharacterSwitchMode();

            if (bCharacterSwitchModeOpen)
            {
                if (PC->WasInputKeyJustPressed(EKeys::Right) || PC->WasInputKeyJustPressed(EKeys::Down))
                    CycleSwitchSelection(+1);
                if (PC->WasInputKeyJustPressed(EKeys::Left) || PC->WasInputKeyJustPressed(EKeys::Up))
                    CycleSwitchSelection(-1);
                if (PC->WasInputKeyJustPressed(EKeys::Enter))
                    ConfirmSwitchSelection();
                if (PC->WasInputKeyJustPressed(EKeys::Escape))
                    CancelSwitchSelection();
            }
        }
    }

    if (NearbyCar && IsValid(NearbyCar))
        GEngine->AddOnScreenDebugMessage(1, 0.f, FColor::White, TEXT("Press E to enter car"));
}

void AOpenCityCharacter::HandleInteract(const FInputActionValue &)
{
    if (bCharacterSwitchModeOpen)
        return;

    if (NearbyCar && IsValid(NearbyCar))
        NearbyCar->EnterCar(this);
}

void AOpenCityCharacter::HandleMove(const FInputActionValue &Value)
{
    if (bCharacterSwitchModeOpen)
        return;

    const FVector2D V = Value.Get<FVector2D>();
    if (!Controller)
        return;

    const FRotator Yaw(0.f, Controller->GetControlRotation().Yaw, 0.f);
    AddMovementInput(FRotationMatrix(Yaw).GetUnitAxis(EAxis::X), V.Y);
    AddMovementInput(FRotationMatrix(Yaw).GetUnitAxis(EAxis::Y), V.X);
}

void AOpenCityCharacter::HandleLook(const FInputActionValue &Value)
{
    if (bCharacterSwitchModeOpen)
        return;

    const FVector2D V = Value.Get<FVector2D>();
    AddControllerYawInput(V.X);
    AddControllerPitchInput(V.Y);
}

void AOpenCityCharacter::EnsureDefaultSwitchRoster()
{
    if (SwitchableCharacters.Num() == 0)
    {
        SwitchableCharacters.Add(MakeRoleData(this,
                                              TEXT("Officer"),
                                              {TAG_Capability_CanArrest.GetTag(), TAG_Capability_HasBadge.GetTag()},
                                              TEXT("Cap"),
                                              TEXT("Badge")));
        SwitchableCharacters.Add(MakeRoleData(this,
                                              TEXT("Firefighter"),
                                              {TAG_Capability_CanFightFire.GetTag(), TAG_Capability_CanRescue.GetTag()},
                                              TEXT("Helmet"),
                                              TEXT("Hose")));
        SwitchableCharacters.Add(MakeRoleData(this,
                                              TEXT("EMT"),
                                              {TAG_Capability_CanHeal.GetTag(), TAG_Capability_CanRescue.GetTag()},
                                              TEXT("None"),
                                              TEXT("None")));

                            UE_LOG(LogOpenCityCharacterAppearance,
                                   Warning,
                                   TEXT("EnsureDefaultSwitchRoster built %d default roles on %s"),
                                   SwitchableCharacters.Num(),
                                   *GetName());
    }

    if (SelectedSwitchIndex < 0 || SelectedSwitchIndex >= SwitchableCharacters.Num())
        SelectedSwitchIndex = 0;
}

void AOpenCityCharacter::UpdateSwitchOverlay()
{
    if (!SwitchWidget)
        return;

    if (!bCharacterSwitchModeOpen)
    {
        if (SwitchWidget->IsInViewport())
            SwitchWidget->RemoveFromParent();
        return;
    }

    if (!SwitchWidget->IsInViewport())
    {
        SwitchWidget->SetRoster(SwitchableCharacters);
        SwitchWidget->AddToViewport(10);
    }
    SwitchWidget->SetSelectedIndex(SelectedSwitchIndex);
}
