#pragma once
#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "OpenCityGameInstance.generated.h"

// Minimal game instance — exists to call UAbilitySystemGlobals::InitGlobalData()
// before any actor spawns. GAS requires this once per process before any
// UAbilitySystemComponent registers; omitting it causes a crash on pawn spawn.
UCLASS()
class OPENCITY_API UOpenCityGameInstance : public UGameInstance
{
    GENERATED_BODY()

public:
    virtual void Init() override;
};
