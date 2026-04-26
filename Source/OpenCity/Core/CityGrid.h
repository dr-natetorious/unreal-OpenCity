#pragma once
#include "CoreMinimal.h"
#include "CityGrid.generated.h"

// Grid layout parameters. All distances in meters; the grid converts to UE units (cm) internally.
USTRUCT(BlueprintType)
struct OPENCITY_API FCityGridParams
{
    GENERATED_BODY()

    // World size of one city cell (buildable block + surrounding roads), meters.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="OpenCity|Grid")
    float CellSizeM = 100.f;

    // Width of a single traffic lane, meters.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="OpenCity|Grid")
    float LaneWidthM = 3.5f;

    // Total lanes on a minor road (one lane each direction).
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="OpenCity|Grid")
    int32 LanesMinor = 2;

    // Total lanes on a major road (two lanes each direction).
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="OpenCity|Grid")
    int32 LanesMajor = 4;

    // Sidewalk added to each side of every road, meters.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="OpenCity|Grid")
    float SidewalkM = 3.f;

    // Every Nth grid boundary is a major road (0 = no major roads).
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="OpenCity|Grid")
    int32 MajorEveryN = 4;
};

// Pure-function city grid math. No UObject base class — all functions testable without PIE.
// All world positions are in UE units (cm). Cell coordinates are dimensionless integers.
struct OPENCITY_API FCityGrid
{
    // World position (cm) → cell coordinates.
    static FIntPoint WorldToCell(float WorldXCm, float WorldYCm, const FCityGridParams& P);

    // Lossless int64 key for a cell coordinate, safe for use in TMap.
    static int64 CellKey(int32 X, int32 Y);

    // Decode a cell key back to its coordinates.
    static FIntPoint CellKeyDecode(int64 Key);

    // True if the boundary at BoundaryIndex is a major (wider) road.
    // Boundary N separates cell N-1 and cell N.
    static bool IsMajorBoundary(int32 BoundaryIndex, const FCityGridParams& P);

    // Road half-width in cm at boundary BoundaryIndex (includes sidewalk).
    static float RoadHalfWidthCm(int32 BoundaryIndex, const FCityGridParams& P);

    // Buildable world-space range in cm along one axis for a cell at CellCoord.
    // Both X and Y axes use the same formula; call once per axis.
    static void CellBuildableRange(int32 CellCoord, const FCityGridParams& P, float& OutMinCm, float& OutMaxCm);
};
