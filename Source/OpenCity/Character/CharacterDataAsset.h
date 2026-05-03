#pragma once
#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Core/CharacterParams.h"
#include "CharacterDataAsset.generated.h"

// Per-character configuration asset. Create one in the editor for each of the
// ~150 planned character variants (DA_OfficerJones, DA_Mike, DA_CriminalVince…).
// AOpenCityCharacter reads this on BeginPlay to configure appearance and
// capabilities. No C++ subclass is ever needed for a new character type.
UCLASS(BlueprintType)
class OPENCITY_API UCharacterDataAsset : public UPrimaryDataAsset
{
    GENERATED_BODY()

public:
    // Display name shown in HUD and mission text.
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "OpenCity|Character")
    FText CharacterName;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "OpenCity|Character")
    FAppearanceParams Appearance;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "OpenCity|Character")
    FCapabilitySet Capabilities;
};
