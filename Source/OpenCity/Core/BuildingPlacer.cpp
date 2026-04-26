#include "Core/BuildingPlacer.h"

TArray<FBuildingSpec> FBuildingPlacer::PlaceInCell(
    int32 CellX, int32 CellY,
    const FCityGridParams& Grid,
    const FBuildingPlacerParams& Bld,
    uint32 BaseSeed)
{
    TArray<FBuildingSpec> Result;
    uint32 Seed = MixSeed(CellX, CellY, BaseSeed);

    float XMin, XMax, YMin, YMax;
    FCityGrid::CellBuildableRange(CellX, Grid, XMin, XMax);
    FCityGrid::CellBuildableRange(CellY, Grid, YMin, YMax);

    const float LotCm      = (Bld.MaxWidthM + Bld.GapM) * 100.f;
    const float SpawnClear = Bld.SpawnClearRadiusM * 100.f;

    for (float LotX = XMin + LotCm * 0.5f; LotX < XMax; LotX += LotCm)
    {
        for (float LotY = YMin + LotCm * 0.5f; LotY < YMax; LotY += LotCm)
        {
            // Keep the world-origin area clear for the player start.
            if (LotX * LotX + LotY * LotY < SpawnClear * SpawnClear)
                continue;

            FBuildingSpec Spec;
            Spec.WorldXCm = LotX;
            Spec.WorldYCm = LotY;
            Spec.WidthCm  = RandRangeF(Seed, Bld.MinWidthM,  Bld.MaxWidthM)  * 100.f;
            Spec.DepthCm  = RandRangeF(Seed, Bld.MinDepthM,  Bld.MaxDepthM)  * 100.f;
            Spec.HeightCm = RandRangeF(Seed, Bld.MinHeightM, Bld.MaxHeightM) * 100.f;
            Result.Add(Spec);
        }
    }

    return Result;
}

bool FBuildingPlacer::IsAreaClear(
    const TArrayView<const FBuildingSpec>& Buildings,
    float CenterXCm, float CenterYCm,
    float RadiusCm)
{
    for (const FBuildingSpec& B : Buildings)
    {
        // Closest point on the building AABB to the circle center.
        float ClosestX = FMath::Clamp(CenterXCm, B.WorldXCm - B.WidthCm * 0.5f, B.WorldXCm + B.WidthCm * 0.5f);
        float ClosestY = FMath::Clamp(CenterYCm, B.WorldYCm - B.DepthCm * 0.5f, B.WorldYCm + B.DepthCm * 0.5f);
        float Dx = CenterXCm - ClosestX;
        float Dy = CenterYCm - ClosestY;
        if (Dx * Dx + Dy * Dy < RadiusCm * RadiusCm)
            return false;
    }
    return true;
}

uint32 FBuildingPlacer::MixSeed(int32 CellX, int32 CellY, uint32 BaseSeed)
{
    uint32 H = BaseSeed;
    H ^= static_cast<uint32>(CellX) * 2654435761u;
    H ^= static_cast<uint32>(CellY) * 2246822519u;
    H ^= H >> 16;
    H *= 0x45d9f3bu;
    H ^= H >> 16;
    return H;
}

float FBuildingPlacer::RandRangeF(uint32& Seed, float Min, float Max)
{
    Seed = Seed * 1664525u + 1013904223u;
    const float T = static_cast<float>(Seed) / static_cast<float>(0xFFFFFFFFu);
    return Min + T * (Max - Min);
}
