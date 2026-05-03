#include "GameMode/OpenCityGameInstance.h"
#include "AbilitySystemGlobals.h"

void UOpenCityGameInstance::Init()
{
    Super::Init();
    UAbilitySystemGlobals::Get().InitGlobalData();
}
