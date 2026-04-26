#include "Core/CityGrid.h"

FIntPoint FCityGrid::WorldToCell(float WorldXCm, float WorldYCm, const FCityGridParams& P)
{
    const float CellCm = P.CellSizeM * 100.f;
    return FIntPoint(
        FMath::FloorToInt(WorldXCm / CellCm),
        FMath::FloorToInt(WorldYCm / CellCm)
    );
}

int64 FCityGrid::CellKey(int32 X, int32 Y)
{
    return static_cast<int64>(static_cast<uint32>(X))
         | (static_cast<int64>(static_cast<uint32>(Y)) << 32);
}

FIntPoint FCityGrid::CellKeyDecode(int64 Key)
{
    return FIntPoint(
        static_cast<int32>(static_cast<uint32>(Key & 0xFFFFFFFF)),
        static_cast<int32>(static_cast<uint32>((Key >> 32) & 0xFFFFFFFF))
    );
}

bool FCityGrid::IsMajorBoundary(int32 BoundaryIndex, const FCityGridParams& P)
{
    return P.MajorEveryN > 0 && (BoundaryIndex % P.MajorEveryN == 0);
}

float FCityGrid::RoadHalfWidthCm(int32 BoundaryIndex, const FCityGridParams& P)
{
    const int32 Lanes = IsMajorBoundary(BoundaryIndex, P) ? P.LanesMajor : P.LanesMinor;
    return (P.LaneWidthM * Lanes * 0.5f + P.SidewalkM) * 100.f;
}

void FCityGrid::CellBuildableRange(int32 CellCoord, const FCityGridParams& P, float& OutMinCm, float& OutMaxCm)
{
    const float CellCm   = P.CellSizeM * 100.f;
    const float OriginCm = CellCoord * CellCm;
    OutMinCm = OriginCm + RoadHalfWidthCm(CellCoord, P);
    OutMaxCm = OriginCm + CellCm - RoadHalfWidthCm(CellCoord + 1, P);
}
