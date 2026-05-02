#pragma once
#include "CoreMinimal.h"
#include "Core/CityGrid.h"
#include "PedestrianParams.generated.h"

// Tuning parameters for pedestrian NPCs.
USTRUCT(BlueprintType)
struct OPENCITY_API FPedestrianParams
{
    GENERATED_BODY()

    // How many pedestrians to spawn per city cell (one per sidewalk side).
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="OpenCity|NPC")
    int32 PerCell = 4;

    // Walk speed in cm/s (~1.4 m/s ≈ normal walking pace).
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="OpenCity|NPC")
    float WalkSpeedCmS = 140.f;

    // Distance from waypoint at which the pedestrian turns around.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="OpenCity|NPC")
    float WaypointReachRadiusCm = 60.f;

    bool AreValid() const;
};

// XY patrol endpoints for one pedestrian. Z is supplied by the spawning actor.
struct OPENCITY_API FPedestrianWaypoints
{
    FVector2D Start;
    FVector2D End;
};

// Pure-function pedestrian placement. No UObject base class — testable without PIE.
struct OPENCITY_API FPedestrianPlacer
{
    // Return PerCell sidewalk patrol routes for one city cell.
    // Routes are deterministic: same inputs always produce the same output.
    static TArray<FPedestrianWaypoints> PlaceInCell(
        int32 CellX, int32 CellY,
        const FCityGridParams& Grid,
        const FPedestrianParams& Ped,
        uint32 BaseSeed);

    // Same hash as FBuildingPlacer::MixSeed — kept consistent across Core.
    static uint32 MixSeed(int32 CellX, int32 CellY, uint32 BaseSeed);
};
