#include "GameMode/OpenCityGameMode.h"
#include "Character/OpenCityCharacter.h"

AOpenCityGameMode::AOpenCityGameMode()
{
    DefaultPawnClass = AOpenCityCharacter::StaticClass();
}
