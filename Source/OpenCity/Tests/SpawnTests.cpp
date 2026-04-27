#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "Tests/TestWorldHelper.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Character/OpenCityCharacter.h"
#include "City/CityBlockActor.h"
#include "City/BuildingActor.h"
#include "City/CityStreamingSubsystem.h"
#include "Components/StaticMeshComponent.h"

// Tier 2 — Spawn checks. Runs headlessly; no rendering required.
// Auto-discovered by: Automation RunTests OpenCity
//
// Tests verify actor spawn state once the game world is active.
// Car-in-level checks are omitted — level content is not guaranteed in CI.
// Car functionality is covered by the self-contained OpenCity.Movement tests.

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSpawnCharacterExists, "OpenCity.Spawn.Character.Exists",
    EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FSpawnCharacterExists::RunTest(const FString&)
{
    UWorld* W = GetTestWorld();
    if (!TestNotNull(TEXT("Game world exists"), W)) return false;

    APawn* Pawn = UGameplayStatics::GetPlayerPawn(W, 0);
    TestNotNull(TEXT("Player pawn exists"), Pawn);
    TestTrue(TEXT("Player pawn is AOpenCityCharacter"), Cast<AOpenCityCharacter>(Pawn) != nullptr);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSpawnCharacterAboveGround, "OpenCity.Spawn.Character.AboveGround",
    EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FSpawnCharacterAboveGround::RunTest(const FString&)
{
    UWorld* W = GetTestWorld();
    if (!TestNotNull(TEXT("Game world exists"), W)) return false;

    APawn* Pawn = UGameplayStatics::GetPlayerPawn(W, 0);
    if (!TestNotNull(TEXT("Player pawn"), Pawn)) return false;

    const float Z = Pawn->GetActorLocation().Z;
    TestTrue(FString::Printf(TEXT("Character Z=%.1fcm > -50cm (not through floor)"), Z), Z > -50.f);
    TestTrue(FString::Printf(TEXT("Character Z=%.1fcm < 500cm (not floating)"), Z),      Z < 500.f);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSpawnCharacterHasController, "OpenCity.Spawn.Character.HasPlayerController",
    EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FSpawnCharacterHasController::RunTest(const FString&)
{
    UWorld* W = GetTestWorld();
    if (!TestNotNull(TEXT("Game world exists"), W)) return false;

    APlayerController* PC = UGameplayStatics::GetPlayerController(W, 0);
    TestNotNull(TEXT("PlayerController exists"), PC);
    TestNotNull(TEXT("PlayerController has a pawn"), PC ? PC->GetPawn() : nullptr);
    return true;
}

// ── Buildings ─────────────────────────────────────────────────────────────────

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSpawnBuildingsBlockGenerates, "OpenCity.Spawn.Buildings.BlockGeneratesBuildings",
    EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FSpawnBuildingsBlockGenerates::RunTest(const FString&)
{
    UWorld* W = GetTestWorld();
    if (!TestNotNull(TEXT("Game world exists"), W)) return false;

    FActorSpawnParameters P;
    P.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    ACityBlockActor* Block = W->SpawnActor<ACityBlockActor>(ACityBlockActor::StaticClass(),
        FVector(50000.f, 50000.f, 0.f), FRotator::ZeroRotator, P);
    if (!TestNotNull(TEXT("CityBlockActor spawned"), Block)) return false;

    Block->CellX = 5;
    Block->CellY = 5;
    Block->Seed  = 12345;
    Block->GenerateBuildings();

    const int32 Count = Block->GetBuildings().Num();
    TestTrue(FString::Printf(TEXT("Block generated %d buildings (expected > 0)"), Count), Count > 0);

    Block->Destroy();
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSpawnBuildingsHasMesh, "OpenCity.Spawn.Buildings.HasMesh",
    EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FSpawnBuildingsHasMesh::RunTest(const FString&)
{
    UWorld* W = GetTestWorld();
    if (!TestNotNull(TEXT("Game world exists"), W)) return false;

    FActorSpawnParameters P;
    P.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    ACityBlockActor* Block = W->SpawnActor<ACityBlockActor>(ACityBlockActor::StaticClass(),
        FVector(50000.f, 60000.f, 0.f), FRotator::ZeroRotator, P);
    if (!TestNotNull(TEXT("CityBlockActor spawned"), Block)) return false;

    Block->CellX = 5;
    Block->CellY = 6;
    Block->Seed  = 99;
    Block->GenerateBuildings();

    const TArray<TObjectPtr<ABuildingActor>>& Buildings = Block->GetBuildings();
    if (!TestTrue(TEXT("At least one building spawned"), Buildings.Num() > 0))
    {
        Block->Destroy();
        return false;
    }

    for (const TObjectPtr<ABuildingActor>& B : Buildings)
    {
        if (!B) continue;
        // Access the mesh via the component — GetComponentByClass is the public API
        UStaticMeshComponent* Mesh = B->FindComponentByClass<UStaticMeshComponent>();
        TestNotNull(TEXT("BuildingActor has a StaticMeshComponent"), Mesh);
        if (Mesh)
            TestNotNull(TEXT("BuildingActor mesh is assigned (not null)"), Mesh->GetStaticMesh().Get());
    }

    Block->Destroy();
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSpawnBuildingsAboveGround, "OpenCity.Spawn.Buildings.AboveGround",
    EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FSpawnBuildingsAboveGround::RunTest(const FString&)
{
    UWorld* W = GetTestWorld();
    if (!TestNotNull(TEXT("Game world exists"), W)) return false;

    FActorSpawnParameters P;
    P.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    ACityBlockActor* Block = W->SpawnActor<ACityBlockActor>(ACityBlockActor::StaticClass(),
        FVector(50000.f, 70000.f, 0.f), FRotator::ZeroRotator, P);
    if (!TestNotNull(TEXT("CityBlockActor spawned"), Block)) return false;

    Block->CellX = 5;
    Block->CellY = 7;
    Block->Seed  = 777;
    Block->GenerateBuildings();

    for (const TObjectPtr<ABuildingActor>& B : Block->GetBuildings())
    {
        if (!B) continue;
        const float Z = B->GetActorLocation().Z;
        TestTrue(FString::Printf(TEXT("Building Z=%.1fcm >= 0 (not inside floor)"), Z), Z >= 0.f);
    }

    Block->Destroy();
    return true;
}

// ── Streaming ─────────────────────────────────────────────────────────────────

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FStreamingLoadsNearbyCell, "OpenCity.Streaming.LoadsNearbyCell",
    EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FStreamingLoadsNearbyCell::RunTest(const FString&)
{
    UWorld* W = GetTestWorld();
    if (!TestNotNull(TEXT("Game world exists"), W)) return false;

    UCityStreamingSubsystem* Sub = W->GetSubsystem<UCityStreamingSubsystem>();
    if (!TestNotNull(TEXT("CityStreamingSubsystem exists"), Sub)) return false;

    Sub->Test_ForceRefresh();

    const APlayerController* PC = W->GetFirstPlayerController();
    if (!TestNotNull(TEXT("PlayerController exists"), PC)) return false;
    if (!TestNotNull(TEXT("PlayerController has pawn"), PC->GetPawn())) return false;

    // Player is near origin; cell (0,0) must be loaded after refresh
    TestTrue(TEXT("Cell (0,0) is loaded near player spawn"), Sub->Test_IsCellLoaded(0, 0));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FStreamingUnloadsFarCell, "OpenCity.Streaming.UnloadsFarCell",
    EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FStreamingUnloadsFarCell::RunTest(const FString&)
{
    UWorld* W = GetTestWorld();
    if (!TestNotNull(TEXT("Game world exists"), W)) return false;

    UCityStreamingSubsystem* Sub = W->GetSubsystem<UCityStreamingSubsystem>();
    if (!TestNotNull(TEXT("CityStreamingSubsystem exists"), Sub)) return false;

    Sub->Test_ForceRefresh();

    // A cell far beyond LoadRadius (3) from origin must not be loaded
    const int32 FarCell = UCityStreamingSubsystem::LoadRadius + 5;
    TestFalse(FString::Printf(TEXT("Cell (%d,%d) far from player is NOT loaded"), FarCell, FarCell),
        Sub->Test_IsCellLoaded(FarCell, FarCell));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FStreamingCellCountIsBounded, "OpenCity.Streaming.CellCountIsBounded",
    EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FStreamingCellCountIsBounded::RunTest(const FString&)
{
    UWorld* W = GetTestWorld();
    if (!TestNotNull(TEXT("Game world exists"), W)) return false;

    UCityStreamingSubsystem* Sub = W->GetSubsystem<UCityStreamingSubsystem>();
    if (!TestNotNull(TEXT("CityStreamingSubsystem exists"), Sub)) return false;

    Sub->Test_ForceRefresh();

    const int32 MaxCells = (2 * UCityStreamingSubsystem::LoadRadius + 1) *
                           (2 * UCityStreamingSubsystem::LoadRadius + 1);
    const int32 Loaded   = Sub->Test_LoadedCellCount();
    TestTrue(FString::Printf(TEXT("Loaded cells %d <= max %d"), Loaded, MaxCells), Loaded <= MaxCells);
    TestTrue(TEXT("At least 1 cell loaded"), Loaded > 0);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FStreamingDeterministicReload, "OpenCity.Streaming.DeterministicReload",
    EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FStreamingDeterministicReload::RunTest(const FString&)
{
    UWorld* W = GetTestWorld();
    if (!TestNotNull(TEXT("Game world exists"), W)) return false;

    // Spawn a block at a known cell, record building count, destroy, respawn, compare
    FActorSpawnParameters SP;
    SP.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    auto SpawnAndCount = [&]() -> int32
    {
        ACityBlockActor* B = W->SpawnActor<ACityBlockActor>(ACityBlockActor::StaticClass(),
            FVector(80000.f, 80000.f, 0.f), FRotator::ZeroRotator, SP);
        if (!B) return -1;
        B->CellX = 8; B->CellY = 8; B->Seed = 42;
        B->GenerateBuildings();
        const int32 N = B->GetBuildings().Num();
        B->Destroy();
        return N;
    };

    const int32 First  = SpawnAndCount();
    const int32 Second = SpawnAndCount();

    if (!TestTrue(TEXT("First spawn produced buildings"), First > 0))  return false;
    TestEqual(TEXT("Same cell/seed produces identical building count on reload"), Second, First);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FStreamingUnloadDestroysBuildings, "OpenCity.Streaming.UnloadDestroysBuildings",
    EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FStreamingUnloadDestroysBuildings::RunTest(const FString&)
{
    UWorld* W = GetTestWorld();
    if (!TestNotNull(TEXT("Game world exists"), W)) return false;

    FActorSpawnParameters SP;
    SP.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    ACityBlockActor* Block = W->SpawnActor<ACityBlockActor>(ACityBlockActor::StaticClass(),
        FVector(90000.f, 90000.f, 0.f), FRotator::ZeroRotator, SP);
    if (!TestNotNull(TEXT("CityBlockActor spawned"), Block)) return false;

    Block->CellX = 9; Block->CellY = 9; Block->Seed = 55;
    Block->GenerateBuildings();

    const int32 BeforeCount = Block->GetBuildings().Num();
    if (!TestTrue(TEXT("Block has buildings before destroy"), BeforeCount > 0)) { Block->Destroy(); return false; }

    // Collect raw pointers to validate they are gone after destroy
    TArray<ABuildingActor*> RawBuildings;
    for (const TObjectPtr<ABuildingActor>& B : Block->GetBuildings())
        if (B) RawBuildings.Add(B.Get());

    Block->Destroy(); // triggers EndPlay → ClearBuildings

    for (ABuildingActor* B : RawBuildings)
        TestTrue(TEXT("BuildingActor is pending kill after block destroy"), !IsValid(B));

    return true;
}
