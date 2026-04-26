#pragma once
#include "CoreMinimal.h"
#include "Core/CityGrid.h"
#include "BuildingPlacer.generated.h"

// Axis-aligned building footprint and height, world position is the center at ground level.
USTRUCT(BlueprintType)
struct OPENCITY_API FBuildingSpec
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category="OpenCity|City")
    float WorldXCm = 0.f;

    UPROPERTY(BlueprintReadOnly, Category="OpenCity|City")
    float WorldYCm = 0.f;

    UPROPERTY(BlueprintReadOnly, Category="OpenCity|City")
    float WidthCm = 0.f;   // X extent

    UPROPERTY(BlueprintReadOnly, Category="OpenCity|City")
    float DepthCm = 0.f;   // Y extent

    UPROPERTY(BlueprintReadOnly, Category="OpenCity|City")
    float HeightCm = 0.f;  // Z extent
};

USTRUCT(BlueprintType)
struct OPENCITY_API FBuildingPlacerParams
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="OpenCity|City")
    float MinWidthM = 8.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="OpenCity|City")
    float MaxWidthM = 20.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="OpenCity|City")
    float MinDepthM = 8.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="OpenCity|City")
    float MaxDepthM = 20.f;

    // Shortest building — one floor.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="OpenCity|City")
    float MinHeightM = 4.f;

    // Tallest allowed building.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="OpenCity|City")
    float MaxHeightM = 80.f;

    // Gap between adjacent buildings, meters.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="OpenCity|City")
    float GapM = 1.f;

    // Radius around world origin kept building-free for player spawn, meters.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="OpenCity|City")
    float SpawnClearRadiusM = 12.f;
};

// Pure-function building placement. No UObject base class — testable without PIE.
struct OPENCITY_API FBuildingPlacer
{
    // Return a deterministic set of buildings for one city cell.
    // Seed is mixed with CellX/CellY so adjacent cells differ.
    static TArray<FBuildingSpec> PlaceInCell(
        int32 CellX, int32 CellY,
        const FCityGridParams& Grid,
        const FBuildingPlacerParams& Bld,
        uint32 BaseSeed);

    // True if no building in the array overlaps the circle at (CenterXCm, CenterYCm, RadiusCm).
    static bool IsAreaClear(
        const TArrayView<const FBuildingSpec>& Buildings,
        float CenterXCm, float CenterYCm,
        float RadiusCm);

private:
    static uint32 MixSeed(int32 CellX, int32 CellY, uint32 BaseSeed);
    static float RandRangeF(uint32& Seed, float Min, float Max);
};
