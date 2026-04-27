#pragma once
#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Core/CityGrid.h"
#include "Core/BuildingPlacer.h"
#include "CityStreamingSubsystem.generated.h"

class ACityBlockActor;
class AActor;

// Streams city cells in/out around the player at runtime.
// Only cells within LoadRadius of the player cell are kept alive;
// everything else is destroyed — memory is bounded to (2*R+1)^2 cells.
//
// Cell coordinate IS the seed: same location always produces the same city.
UCLASS()
class OPENCITY_API UCityStreamingSubsystem : public UTickableWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual void Tick(float DeltaTime) override;
    virtual TStatId GetStatId() const override;
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

    // Cells loaded in each direction. Max loaded = (2*R+1)^2 = 49.
    static constexpr int32 LoadRadius = 3;

#if WITH_AUTOMATION_TESTS
    void  Test_ForceRefresh();
    int32 Test_LoadedCellCount()              const { return LoadedBlocks.Num(); }
    bool  Test_IsCellLoaded(int32 CX, int32 CY) const;
#endif

private:
    void RefreshAroundCell(FIntPoint Center);
    void SpawnCell(int32 CX, int32 CY);
    void DespawnCell(int64 Key);

    TMap<int64, TObjectPtr<ACityBlockActor>> LoadedBlocks;
    TMap<int64, TObjectPtr<AActor>>          LoadedFloors;

    float     CheckCooldown = 0.f;
    FIntPoint LastCenter    = FIntPoint(MAX_int32, MAX_int32);

    FCityGridParams       GridParams;
    FBuildingPlacerParams BuildingParams;

    static constexpr float  CheckEveryS = 0.5f;
    static constexpr uint32 WorldSeed   = 42u;
};
