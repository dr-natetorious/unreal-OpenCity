#include "City/CityBlockActor.h"
#include "City/BuildingActor.h"
#include "Core/BuildingPlacer.h"

ACityBlockActor::ACityBlockActor()
{
    PrimaryActorTick.bCanEverTick = false;
}

void ACityBlockActor::BeginPlay()
{
    Super::BeginPlay();
    GenerateBuildings();
}

void ACityBlockActor::ClearBuildings()
{
    for (TObjectPtr<ABuildingActor>& B : SpawnedBuildings)
    {
        if (B) B->Destroy();
    }
    SpawnedBuildings.Empty();
}

void ACityBlockActor::GenerateBuildings()
{
    ClearBuildings();

    UWorld* World = GetWorld();
    if (!World) return;

    const TArray<FBuildingSpec> Specs = FBuildingPlacer::PlaceInCell(
        CellX, CellY, GridParams, BuildingParams, static_cast<uint32>(Seed));

    SpawnedBuildings.Reserve(Specs.Num());
    for (const FBuildingSpec& Spec : Specs)
    {
        FActorSpawnParameters Params;
        Params.Owner = this;
        if (ABuildingActor* B = World->SpawnActor<ABuildingActor>(ABuildingActor::StaticClass(), FTransform::Identity, Params))
        {
            B->SetSpec(Spec);
            SpawnedBuildings.Add(B);
        }
    }
}
