#include "City/CityBlockActor.h"
#include "City/BuildingActor.h"
#include "Character/PedestrianCharacter.h"
#include "Core/BuildingPlacer.h"
#include "Core/PedestrianParams.h"

ACityBlockActor::ACityBlockActor()
{
    PrimaryActorTick.bCanEverTick = false;
}

void ACityBlockActor::BeginPlay()
{
    Super::BeginPlay();
    GenerateBuildings();
}

void ACityBlockActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    ClearBuildings();
    Super::EndPlay(EndPlayReason);
}

void ACityBlockActor::ClearBuildings()
{
    for (TObjectPtr<ABuildingActor>& B : SpawnedBuildings)
    {
        if (B) B->Destroy();
    }
    SpawnedBuildings.Empty();
    ClearPedestrians();
}

void ACityBlockActor::ClearPedestrians()
{
    for (TObjectPtr<APedestrianCharacter>& P : SpawnedPedestrians)
    {
        if (P) P->Destroy();
    }
    SpawnedPedestrians.Empty();
}

void ACityBlockActor::GenerateBuildings()
{
    ClearBuildings();   // also clears pedestrians

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

    SpawnPedestrians();
}

void ACityBlockActor::SpawnPedestrians()
{
    UWorld* World = GetWorld();
    if (!World) return;

    const TArray<FPedestrianWaypoints> Routes = FPedestrianPlacer::PlaceInCell(
        CellX, CellY, GridParams, PedestrianParams, static_cast<uint32>(Seed));

    SpawnedPedestrians.Reserve(Routes.Num());
    for (const FPedestrianWaypoints& Route : Routes)
    {
        const FVector SpawnLoc(Route.Start.X, Route.Start.Y, 0.f);
        FActorSpawnParameters Params;
        Params.Owner = this;
        Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
        if (APedestrianCharacter* P = World->SpawnActor<APedestrianCharacter>(
            APedestrianCharacter::StaticClass(), FTransform(SpawnLoc), Params))
        {
            const FVector A(Route.Start.X, Route.Start.Y, P->GetActorLocation().Z);
            const FVector B(Route.End.X,   Route.End.Y,   P->GetActorLocation().Z);
            P->InitPatrol(A, B, PedestrianParams.WalkSpeedCmS);
            SpawnedPedestrians.Add(P);
        }
    }
}
