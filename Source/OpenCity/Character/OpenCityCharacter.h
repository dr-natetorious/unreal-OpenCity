#pragma once
#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/Character.h"
#include "GameplayTagContainer.h"
#include "InputActionValue.h"
#include "Character/CharacterDataAsset.h"
#include "OpenCityCharacter.generated.h"

class UAbilitySystemComponent;
class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
class USkeletalMeshComponent;
class UStaticMeshComponent;
class UMaterialInterface;
class UStaticMesh;
class ACarPawn;
class UCharacterSwitchWidget;

UCLASS()
class OPENCITY_API AOpenCityCharacter : public ACharacter, public IAbilitySystemInterface
{
    GENERATED_BODY()

public:
    AOpenCityCharacter();

    USkeletalMeshComponent *GetCharacterMeshComponent() const { return CharacterMesh; }

    // IAbilitySystemInterface — makes GAS internals (effects, cues) find our ASC.
    virtual UAbilitySystemComponent *GetAbilitySystemComponent() const override;

    /** Called by ACarPawn's proximity sphere — tells us a car is enterable. */
    void SetNearbyCar(ACarPawn *Car) { NearbyCar = Car; }

    // Reads CharacterData and applies capability tags + visuals.
    // Called automatically in BeginPlay; also callable directly (e.g., in tests).
    void ApplyCharacterData();

    // Returns true if this character has the requested configured capability tag.
    bool HasCapability(const FGameplayTag &Tag) const;

    // Opens/closes local role switch mode (Tab fallback in code).
    void ToggleCharacterSwitchMode();

    // Moves switch selection by +/-1 and wraps around available roster entries.
    void CycleSwitchSelection(int32 Delta);

    // Applies currently selected roster entry and closes switch mode.
    void ConfirmSwitchSelection();

    // Closes switch mode without changing the active role.
    void CancelSwitchSelection();

    bool IsCharacterSwitchModeOpen() const { return bCharacterSwitchModeOpen; }

    // The data asset driving this character's division, capabilities, and visuals.
    // Assign in editor (or via NewObject<> in tests) before BeginPlay, or call
    // ApplyCharacterData() manually after assigning at runtime.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OpenCity|Character")
    TObjectPtr<UCharacterDataAsset> CharacterData;

protected:
    virtual void BeginPlay() override;
    virtual void NotifyControllerChanged() override;
    virtual void SetupPlayerInputComponent(UInputComponent *Input) override;
    virtual void Tick(float DeltaSeconds) override;

private:
    void HandleMove(const FInputActionValue &Value);
    void HandleLook(const FInputActionValue &Value);
    void HandleInteract(const FInputActionValue &Value);

    UPROPERTY(VisibleAnywhere, Category = "OpenCity|Abilities")
    TObjectPtr<UAbilitySystemComponent> AbilitySystem;

    UPROPERTY(VisibleAnywhere, Category = "OpenCity|Mesh")
    TObjectPtr<USkeletalMeshComponent> CharacterMesh;

    UPROPERTY(VisibleAnywhere, Category = "OpenCity|Mesh")
    TObjectPtr<UStaticMeshComponent> HatMeshComponent;

    UPROPERTY(VisibleAnywhere, Category = "OpenCity|Mesh")
    TObjectPtr<UStaticMeshComponent> LeftHandAccessoryMeshComponent;

    UPROPERTY(VisibleAnywhere, Category = "OpenCity|Mesh")
    TObjectPtr<UStaticMeshComponent> RightHandAccessoryMeshComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "OpenCity|Camera", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<USpringArmComponent> CameraArm;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "OpenCity|Camera", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UCameraComponent> FollowCamera;

    UPROPERTY(EditAnywhere, Category = "OpenCity|Input")
    TObjectPtr<UInputMappingContext> FootMappingContext;

    UPROPERTY(EditAnywhere, Category = "OpenCity|Input")
    TObjectPtr<UInputAction> MoveAction;

    UPROPERTY(EditAnywhere, Category = "OpenCity|Input")
    TObjectPtr<UInputAction> LookAction;

    UPROPERTY(EditAnywhere, Category = "OpenCity|Input")
    TObjectPtr<UInputAction> JumpAction;

    UPROPERTY(EditAnywhere, Category = "OpenCity|Input")
    TObjectPtr<UInputAction> InteractAction;

    // Role roster for local character switching (Officer / Firefighter / EMT...).
    UPROPERTY(EditAnywhere, Category = "OpenCity|Character")
    TArray<TObjectPtr<UCharacterDataAsset>> SwitchableCharacters;

    // Nearest car within proximity — set by ACarPawn overlap events
    TObjectPtr<ACarPawn> NearbyCar;

    // Tracks the tags currently pushed into ASC as loose tags.
    FGameplayTagContainer AppliedCapabilityTags;

    bool bCharacterSwitchModeOpen = false;
    int32 SelectedSwitchIndex = 0;

    UPROPERTY()
    TObjectPtr<UCharacterSwitchWidget> SwitchWidget;

    void ApplyAppearanceData();
    void ApplyCapabilityTagsToASC();
    void ApplySlotMaterial(const FName &SlotName, UMaterialInterface *Material);
    void ApplySlotColor(const FName &SlotName, const FLinearColor &Color, const FName &PrimaryParameterName);
    void ApplyAttachmentMesh(UStaticMeshComponent *Component, UStaticMesh *Mesh, const FName &SocketName);
    void EnsureDefaultSwitchRoster();
    void UpdateSwitchOverlay();
};
