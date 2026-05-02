#include "Core/PedestrianParams.h"

bool FPedestrianParams::AreValid() const
{
    return PerCell > 0 && WalkSpeedCmS > 0.f && WaypointReachRadiusCm > 0.f;
}

uint32 FPedestrianPlacer::MixSeed(int32 CellX, int32 CellY, uint32 BaseSeed)
{
    uint32 H = BaseSeed;
    H ^= static_cast<uint32>(CellX) * 2654435761u;
    H ^= static_cast<uint32>(CellY) * 2246822519u;
    H ^= H >> 16;
    H *= 0x45d9f3bu;
    H ^= H >> 16;
    return H;
}

TArray<FPedestrianWaypoints> FPedestrianPlacer::PlaceInCell(
    int32 CellX, int32 CellY,
    const FCityGridParams& Grid,
    const FPedestrianParams& Ped,
    uint32 BaseSeed)
{
    TArray<FPedestrianWaypoints> Result;
    Result.Reserve(Ped.PerCell);

    float MinXCm, MaxXCm, MinYCm, MaxYCm;
    FCityGrid::CellBuildableRange(CellX, Grid, MinXCm, MaxXCm);
    FCityGrid::CellBuildableRange(CellY, Grid, MinYCm, MaxYCm);

    // Sidewalk center is half the sidewalk width outside the buildable edge.
    const float SidewalkHalfCm = Grid.SidewalkM * 50.f;

    // 4 sides: 0=West, 1=East, 2=South, 3=North.
    // Each side defines a fixed-axis position and two endpoints along the perpendicular.
    struct FSide { FVector2D A; FVector2D B; };
    const FSide Sides[4] = {
        // West: X fixed left of buildable zone, patrol N↔S
        { FVector2D(MinXCm - SidewalkHalfCm, MinYCm), FVector2D(MinXCm - SidewalkHalfCm, MaxYCm) },
        // East: X fixed right of buildable zone, patrol N↔S
        { FVector2D(MaxXCm + SidewalkHalfCm, MinYCm), FVector2D(MaxXCm + SidewalkHalfCm, MaxYCm) },
        // South: Y fixed below buildable zone, patrol E↔W
        { FVector2D(MinXCm, MinYCm - SidewalkHalfCm), FVector2D(MaxXCm, MinYCm - SidewalkHalfCm) },
        // North: Y fixed above buildable zone, patrol E↔W
        { FVector2D(MinXCm, MaxYCm + SidewalkHalfCm), FVector2D(MaxXCm, MaxYCm + SidewalkHalfCm) },
    };

    for (int32 i = 0; i < Ped.PerCell; ++i)
    {
        const FSide& S = Sides[i % 4];
        FPedestrianWaypoints W;
        W.Start = S.A;
        W.End   = S.B;
        Result.Add(W);
    }

    return Result;
}
