#include "Misc/AutomationTest.h"
#include "Core/CityGrid.h"
#include "Core/BuildingPlacer.h"
#include "Core/VehicleParams.h"
#include "Core/PedestrianParams.h"

// Tier 1 — No PIE required. Tests run headlessly with -NullRHI.
// Run: UnrealEditor OpenCity.uproject -ExecCmds="Automation RunTests OpenCity.Core" -NullRHI -Unattended

static FCityGridParams MakeDefaultGrid()
{
    return FCityGridParams{};
}

// ── CityGrid: WorldToCell ─────────────────────────────────────────────────────

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCityGridOrigin, "OpenCity.Core.CityGrid.OriginIsCell00",
    EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FCityGridOrigin::RunTest(const FString&)
{
    const FCityGridParams P = MakeDefaultGrid();
    const FIntPoint C = FCityGrid::WorldToCell(0.f, 0.f, P);
    TestEqual(TEXT("Origin X"), C.X, 0);
    TestEqual(TEXT("Origin Y"), C.Y, 0);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCityGridOneCellBoundary, "OpenCity.Core.CityGrid.OneCellSize",
    EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FCityGridOneCellBoundary::RunTest(const FString&)
{
    const FCityGridParams P = MakeDefaultGrid();
    const float CellCm = P.CellSizeM * 100.f;
    const FIntPoint C = FCityGrid::WorldToCell(CellCm, CellCm, P);
    TestEqual(TEXT("One cell X"), C.X, 1);
    TestEqual(TEXT("One cell Y"), C.Y, 1);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCityGridNegative, "OpenCity.Core.CityGrid.NegativeCoords",
    EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FCityGridNegative::RunTest(const FString&)
{
    const FCityGridParams P = MakeDefaultGrid();
    const FIntPoint C = FCityGrid::WorldToCell(-1.f, -1.f, P);
    TestEqual(TEXT("Negative X"), C.X, -1);
    TestEqual(TEXT("Negative Y"), C.Y, -1);
    return true;
}

// ── CityGrid: CellKey roundtrip ───────────────────────────────────────────────

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCityGridKeyRoundtrip, "OpenCity.Core.CityGrid.KeyRoundtrip",
    EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FCityGridKeyRoundtrip::RunTest(const FString&)
{
    for (int32 X : {-100, -1, 0, 1, 100})
    for (int32 Y : {-100, -1, 0, 1, 100})
    {
        const int64 Key       = FCityGrid::CellKey(X, Y);
        const FIntPoint Decoded = FCityGrid::CellKeyDecode(Key);
        TestEqual(FString::Printf(TEXT("Key X roundtrip at (%d,%d)"), X, Y), Decoded.X, X);
        TestEqual(FString::Printf(TEXT("Key Y roundtrip at (%d,%d)"), X, Y), Decoded.Y, Y);
    }
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCityGridKeyUnique, "OpenCity.Core.CityGrid.KeysAreUnique",
    EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FCityGridKeyUnique::RunTest(const FString&)
{
    TSet<int64> Seen;
    for (int32 X = -10; X <= 10; X++)
    for (int32 Y = -10; Y <= 10; Y++)
    {
        const int64 Key = FCityGrid::CellKey(X, Y);
        TestFalse(FString::Printf(TEXT("Duplicate key at (%d,%d)"), X, Y), Seen.Contains(Key));
        Seen.Add(Key);
    }
    return true;
}

// ── CityGrid: road widths ─────────────────────────────────────────────────────

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCityGridMajorWider, "OpenCity.Core.CityGrid.MajorRoadWiderThanMinor",
    EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FCityGridMajorWider::RunTest(const FString&)
{
    const FCityGridParams P = MakeDefaultGrid();
    const float Major = FCityGrid::RoadHalfWidthCm(0, P); // boundary 0 is major (0 % 4 == 0)
    const float Minor = FCityGrid::RoadHalfWidthCm(1, P); // boundary 1 is minor
    TestTrue(TEXT("Major road half-width > minor"), Major > Minor);
    TestTrue(TEXT("Minor road half-width > 0"), Minor > 0.f);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCityGridBuildablePositive, "OpenCity.Core.CityGrid.BuildableRangeIsPositive",
    EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FCityGridBuildablePositive::RunTest(const FString&)
{
    const FCityGridParams P = MakeDefaultGrid();
    for (int32 C : {0, 1, -1, 4})
    {
        float MinCm, MaxCm;
        FCityGrid::CellBuildableRange(C, P, MinCm, MaxCm);
        TestTrue(FString::Printf(TEXT("Cell %d: buildable max > min"), C), MaxCm > MinCm);
        TestTrue(FString::Printf(TEXT("Cell %d: buildable width >= 50m"), C), (MaxCm - MinCm) >= 5000.f);
    }
    return true;
}

// ── BuildingPlacer: determinism ───────────────────────────────────────────────

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FBuildingPlacerDeterministic, "OpenCity.Core.BuildingPlacer.SameSeedSameResult",
    EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FBuildingPlacerDeterministic::RunTest(const FString&)
{
    const FCityGridParams     Grid{};
    const FBuildingPlacerParams Bld{};

    const auto A = FBuildingPlacer::PlaceInCell(3, -7, Grid, Bld, 42u);
    const auto B = FBuildingPlacer::PlaceInCell(3, -7, Grid, Bld, 42u);

    TestEqual(TEXT("Same seed → same building count"), A.Num(), B.Num());
    if (A.Num() > 0)
    {
        TestEqual(TEXT("First building X matches"), A[0].WorldXCm, B[0].WorldXCm);
        TestEqual(TEXT("First building H matches"), A[0].HeightCm, B[0].HeightCm);
    }
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FBuildingPlacerDifferentSeeds, "OpenCity.Core.BuildingPlacer.DifferentSeedsDifferentResult",
    EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FBuildingPlacerDifferentSeeds::RunTest(const FString&)
{
    const FCityGridParams     Grid{};
    const FBuildingPlacerParams Bld{};

    const auto A = FBuildingPlacer::PlaceInCell(0, 0, Grid, Bld, 1u);
    const auto B = FBuildingPlacer::PlaceInCell(0, 0, Grid, Bld, 999u);

    if (A.Num() > 0 && B.Num() > 0)
    {
        // Heights should differ for different seeds (not a strict guarantee, but true for any reasonable LCG).
        TestFalse(TEXT("Different seeds → different first building height"),
            FMath::IsNearlyEqual(A[0].HeightCm, B[0].HeightCm, 1.f));
    }
    return true;
}

// ── BuildingPlacer: spawn area clear ─────────────────────────────────────────

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FBuildingPlacerSpawnClear, "OpenCity.Core.BuildingPlacer.SpawnAreaIsClear",
    EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FBuildingPlacerSpawnClear::RunTest(const FString&)
{
    const FCityGridParams     Grid{};
    const FBuildingPlacerParams Bld{};
    const TArray<FBuildingSpec> Buildings = FBuildingPlacer::PlaceInCell(0, 0, Grid, Bld, 42u);

    const float SpawnClearCm = Bld.SpawnClearRadiusM * 100.f;
    const bool bClear = FBuildingPlacer::IsAreaClear(Buildings, 0.f, 0.f, SpawnClearCm);
    TestTrue(TEXT("No building overlaps the player spawn radius"), bClear);
    return true;
}

// ── BuildingPlacer: height range ──────────────────────────────────────────────

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FBuildingPlacerHeights, "OpenCity.Core.BuildingPlacer.HeightsInRange",
    EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FBuildingPlacerHeights::RunTest(const FString&)
{
    const FCityGridParams     Grid{};
    const FBuildingPlacerParams Bld{};
    const TArray<FBuildingSpec> Buildings = FBuildingPlacer::PlaceInCell(0, 0, Grid, Bld, 42u);

    const float MinCm = Bld.MinHeightM * 100.f;
    const float MaxCm = Bld.MaxHeightM * 100.f;

    for (const FBuildingSpec& B : Buildings)
    {
        TestTrue(FString::Printf(TEXT("Height %.0fcm >= min %.0fcm"), B.HeightCm, MinCm), B.HeightCm >= MinCm * 0.99f);
        TestTrue(FString::Printf(TEXT("Height %.0fcm <= max %.0fcm"), B.HeightCm, MaxCm), B.HeightCm <= MaxCm * 1.01f);
    }
    return true;
}

// ── BuildingPlacer: buildings stay within cell bounds ─────────────────────────

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FBuildingPlacerWithinBounds, "OpenCity.Core.BuildingPlacer.BuildingsWithinCellBounds",
    EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FBuildingPlacerWithinBounds::RunTest(const FString&)
{
    const FCityGridParams      Grid{};
    const FBuildingPlacerParams Bld{};

    for (int32 CX : {-1, 0, 1, 4})
    for (int32 CY : {-1, 0, 1, 4})
    {
        float XMin, XMax, YMin, YMax;
        FCityGrid::CellBuildableRange(CX, Grid, XMin, XMax);
        FCityGrid::CellBuildableRange(CY, Grid, YMin, YMax);

        const TArray<FBuildingSpec> Buildings = FBuildingPlacer::PlaceInCell(CX, CY, Grid, Bld, 42u);
        for (const FBuildingSpec& B : Buildings)
        {
            TestTrue(FString::Printf(TEXT("Cell(%d,%d) building X=%.0f >= XMin=%.0f"), CX, CY, B.WorldXCm, XMin),
                B.WorldXCm >= XMin);
            TestTrue(FString::Printf(TEXT("Cell(%d,%d) building X=%.0f <= XMax=%.0f"), CX, CY, B.WorldXCm, XMax),
                B.WorldXCm <= XMax);
            TestTrue(FString::Printf(TEXT("Cell(%d,%d) building Y=%.0f >= YMin=%.0f"), CX, CY, B.WorldYCm, YMin),
                B.WorldYCm >= YMin);
            TestTrue(FString::Printf(TEXT("Cell(%d,%d) building Y=%.0f <= YMax=%.0f"), CX, CY, B.WorldYCm, YMax),
                B.WorldYCm <= YMax);
        }
    }
    return true;
}

// ── VehicleParams: defaults are sane ─────────────────────────────────────────

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FVehicleParamsValid, "OpenCity.Core.VehicleParams.DefaultsAreValid",
    EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FVehicleParamsValid::RunTest(const FString&)
{
    const FCarParams P{};
    TestTrue(TEXT("Default params are valid"), P.AreValid());
    TestTrue(TEXT("Mass > 0"),         P.MassKg > 0.f);
    TestTrue(TEXT("Wheelbase > 0"),    P.WheelbaseM > 0.f);
    TestTrue(TEXT("Wheel radius > 0"), P.WheelRadiusM > 0.f);
    TestTrue(TEXT("Max speed > 0"),    P.MaxSpeedKmH > 0.f);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FVehicleParamsWheelPositions, "OpenCity.Core.VehicleParams.WheelPositions",
    EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FVehicleParamsWheelPositions::RunTest(const FString&)
{
    const FCarParams P{};
    const TArray<FVector> Wheels = P.GetWheelPositionsCm();

    TestEqual(TEXT("Exactly 4 wheel positions"), Wheels.Num(), 4);

    // Front wheels have positive X, rear wheels have negative X.
    TestTrue(TEXT("Front-left X > 0"),  Wheels[0].X > 0.f);
    TestTrue(TEXT("Front-right X > 0"), Wheels[1].X > 0.f);
    TestTrue(TEXT("Rear-left X < 0"),   Wheels[2].X < 0.f);
    TestTrue(TEXT("Rear-right X < 0"),  Wheels[3].X < 0.f);

    // Left wheels have positive Y, right wheels have negative Y.
    TestTrue(TEXT("Front-left Y > 0"),  Wheels[0].Y > 0.f);
    TestTrue(TEXT("Front-right Y < 0"), Wheels[1].Y < 0.f);

    // All wheels are above the ground plane.
    for (const FVector& W : Wheels)
        TestTrue(FString::Printf(TEXT("Wheel Z=%.1f > 0"), W.Z), W.Z > 0.f);

    return true;
}

// ── PedestrianPlacer ──────────────────────────────────────────────────────────

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPedestrianParamsValid, "OpenCity.Core.Pedestrian.DefaultParamsAreValid",
    EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FPedestrianParamsValid::RunTest(const FString&)
{
    const FPedestrianParams P{};
    TestTrue(TEXT("Default params are valid"),       P.AreValid());
    TestTrue(TEXT("PerCell > 0"),                    P.PerCell > 0);
    TestTrue(TEXT("WalkSpeedCmS > 0"),               P.WalkSpeedCmS > 0.f);
    TestTrue(TEXT("WaypointReachRadiusCm > 0"),      P.WaypointReachRadiusCm > 0.f);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPedestrianPlacerCount, "OpenCity.Core.Pedestrian.PlaceInCellReturnsPerCell",
    EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FPedestrianPlacerCount::RunTest(const FString&)
{
    const FCityGridParams   Grid{};
    const FPedestrianParams Ped{};

    const TArray<FPedestrianWaypoints> Routes = FPedestrianPlacer::PlaceInCell(0, 0, Grid, Ped, 42u);
    TestEqual(TEXT("PlaceInCell returns exactly PerCell routes"), Routes.Num(), Ped.PerCell);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPedestrianPlacerOnSidewalk, "OpenCity.Core.Pedestrian.WaypointsAreOnSidewalk",
    EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FPedestrianPlacerOnSidewalk::RunTest(const FString&)
{
    const FCityGridParams   Grid{};
    const FPedestrianParams Ped{};

    float MinXCm, MaxXCm, MinYCm, MaxYCm;
    FCityGrid::CellBuildableRange(0, Grid, MinXCm, MaxXCm);
    FCityGrid::CellBuildableRange(0, Grid, MinYCm, MaxYCm);

    const TArray<FPedestrianWaypoints> Routes = FPedestrianPlacer::PlaceInCell(0, 0, Grid, Ped, 42u);
    for (int32 i = 0; i < Routes.Num(); ++i)
    {
        const FVector2D& S = Routes[i].Start;
        const FVector2D& E = Routes[i].End;
        // Each waypoint must be outside the buildable rectangle on at least one axis.
        const bool StartOutside = S.X < MinXCm || S.X > MaxXCm || S.Y < MinYCm || S.Y > MaxYCm;
        const bool EndOutside   = E.X < MinXCm || E.X > MaxXCm || E.Y < MinYCm || E.Y > MaxYCm;
        TestTrue(FString::Printf(TEXT("Route[%d] Start is on sidewalk (outside buildable zone)"), i), StartOutside);
        TestTrue(FString::Printf(TEXT("Route[%d] End is on sidewalk (outside buildable zone)"), i), EndOutside);
    }
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPedestrianPlacerDeterministic, "OpenCity.Core.Pedestrian.SameSeedSameResult",
    EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FPedestrianPlacerDeterministic::RunTest(const FString&)
{
    const FCityGridParams   Grid{};
    const FPedestrianParams Ped{};

    const auto A = FPedestrianPlacer::PlaceInCell(3, -7, Grid, Ped, 42u);
    const auto B = FPedestrianPlacer::PlaceInCell(3, -7, Grid, Ped, 42u);

    TestEqual(TEXT("Same seed → same route count"), A.Num(), B.Num());
    for (int32 i = 0; i < A.Num(); ++i)
    {
        TestEqual(FString::Printf(TEXT("Route[%d] Start.X matches"), i), A[i].Start.X, B[i].Start.X);
        TestEqual(FString::Printf(TEXT("Route[%d] End.Y matches"),   i), A[i].End.Y,   B[i].End.Y);
    }
    return true;
}
