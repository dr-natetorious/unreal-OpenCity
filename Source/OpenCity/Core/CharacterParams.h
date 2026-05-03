#pragma once
#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "NativeGameplayTags.h"
#include "CharacterParams.generated.h"

class UMaterialInterface;
class UStaticMesh;

// ── Capability tags: what a character can DO ──────────────────────────────────
// Declared here, defined in CharacterParams.cpp.
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Capability_CanArrest)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Capability_CanFightFire)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Capability_CanHeal)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Capability_CanFly)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Capability_CanRepair)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Capability_CanRescue)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Capability_CanDriveEmergency)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Capability_CanHack)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Capability_CanInvestigate)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Capability_CanParachute)

// ── Capability tags: interactable NPC states ──────────────────────────────────
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Capability_IsArrestable)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Capability_IsOnFire)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Capability_NeedsRescue)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Capability_IsInjured)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Capability_IsSuspect)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Capability_IsWitness)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Capability_IsVIP)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Capability_IsRobot)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Capability_HasBadge)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Capability_HasTools)

// Runtime-selectable cosmetic params for variant characters that share one rig.
// One UCharacterDataAsset per character variant — zero C++ subclasses needed.
USTRUCT(BlueprintType)
struct OPENCITY_API FAppearanceParams
{
    GENERATED_BODY()

    // Base body materials are driven by stable shared-mesh regions.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OpenCity|Character")
    TObjectPtr<UMaterialInterface> SkinMaterial = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OpenCity|Character")
    TObjectPtr<UMaterialInterface> FaceMaterial = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OpenCity|Character")
    TObjectPtr<UMaterialInterface> ShirtMaterial = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OpenCity|Character")
    TObjectPtr<UMaterialInterface> PantsMaterial = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OpenCity|Character")
    TObjectPtr<UMaterialInterface> ShoesMaterial = nullptr;

    // Torso/top tint.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OpenCity|Character")
    FLinearColor ShirtColor = FLinearColor(0.2f, 0.2f, 0.8f, 1.f);

    // Lower body tint.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OpenCity|Character")
    FLinearColor PantsColor = FLinearColor(0.1f, 0.1f, 0.1f, 1.f);

    // Head tint.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OpenCity|Character")
    FLinearColor SkinColor = FLinearColor(1.f, 0.84f, 0.67f, 1.f);

    // Optional tint for face print/material. Defaults to white so authored materials show unchanged.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OpenCity|Character")
    FLinearColor FaceColor = FLinearColor::White;

    // Optional tint for shoes/footwear material.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OpenCity|Character")
    FLinearColor ShoesColor = FLinearColor::White;

    // Mesh-based attachments for silhouette-changing parts.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OpenCity|Character")
    TObjectPtr<UStaticMesh> HatMesh = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OpenCity|Character")
    TObjectPtr<UStaticMesh> LeftHandAccessoryMesh = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OpenCity|Character")
    TObjectPtr<UStaticMesh> RightHandAccessoryMesh = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OpenCity|Character")
    FName HatSocket = TEXT("head");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OpenCity|Character")
    FName LeftHandAccessorySocket = TEXT("hand_l");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OpenCity|Character")
    FName RightHandAccessorySocket = TEXT("hand_r");

    // Data-driven hat style. Supported in code today: None, Cap, Helmet.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OpenCity|Character")
    FName HatStyle = TEXT("None");

    // Hat tint when HatStyle != None.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OpenCity|Character")
    FLinearColor HatColor = FLinearColor::Black;

    // Accessory worn on the torso/chest. Supported: None, Badge, Hose, Wrench.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OpenCity|Character")
    FName AccessoryStyle = TEXT("None");

    // Printed face variant. Supported: Default, Smile, Frown, Angry, Scared.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OpenCity|Character")
    FName FaceStyle = TEXT("Default");

    bool AreValid() const;
};

// Material slot indices for SK_CityCharacter — must match Blender export order.
namespace CharacterMeshSlot
{
    static const FName Skin(TEXT("M_Skin"));
    static const FName Face(TEXT("M_Face"));
    static const FName Shirt(TEXT("M_Shirt"));
    static const FName Pants(TEXT("M_Pants"));
    static const FName Shoes(TEXT("M_Shoes"));
}

// The set of capability tags describing what this character can do and what can
// be done to it. Empty is valid — a plain civilian has no special capabilities.
// Stored on UCharacterDataAsset; queried at runtime to gate ability use.
USTRUCT(BlueprintType)
struct OPENCITY_API FCapabilitySet
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OpenCity|Character")
    TArray<FGameplayTag> Capabilities;

    // Returns false only if the set contains duplicate tags.
    bool AreValid() const;

    bool HasCapability(const FGameplayTag &Tag) const;
};

// Deterministic role coloring: emergency roles are always visually distinct.
namespace CharacterAppearancePolicy
{
    OPENCITY_API FLinearColor ResolveShirtColor(const FAppearanceParams &Appearance,
                                                const FCapabilitySet &Capabilities);
}
