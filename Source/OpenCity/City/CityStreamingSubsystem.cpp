#include "City/CityStreamingSubsystem.h"
#include "City/CityBlockActor.h"
#include "Core/BuildingPlacer.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"

// ── Subsystem lifecycle ───────────────────────────────────────────────────────

bool UCityStreamingSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    const UWorld* World = Cast<UWorld>(Outer);
    return World &&
        (World->WorldType == EWorldType::PIE || World->WorldType == EWorldType::Game);
}

TStatId UCityStreamingSubsystem::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(UCityStreamingSubsystem, STATGROUP_Tickables);
}

// ── Tick ─────────────────────────────────────────────────────────────────────

void UCityStreamingSubsystem::Tick(float DeltaTime)
{
    CheckCooldown -= DeltaTime;
    if (CheckCooldown > 0.f) return;
    CheckCooldown = CheckEveryS;

    const APlayerController* PC = GetWorld()->GetFirstPlayerController();
    if (!PC || !PC->GetPawn()) return;

    const FVector    Pos         = PC->GetPawn()->GetActorLocation();
    const FIntPoint  CurrentCell = FCityGrid::WorldToCell(Pos.X, Pos.Y, GridParams);

    if (CurrentCell == LastCenter) return;
    LastCenter = CurrentCell;
    RefreshAroundCell(CurrentCell);
}

// ── Streaming ─────────────────────────────────────────────────────────────────

void UCityStreamingSubsystem::RefreshAroundCell(FIntPoint Center)
{
    // Build the desired set of cell keys
    TSet<int64> Desired;
    Desired.Reserve((2 * LoadRadius + 1) * (2 * LoadRadius + 1));
    for (int32 DX = -LoadRadius; DX <= LoadRadius; DX++)
    for (int32 DY = -LoadRadius; DY <= LoadRadius; DY++)
        Desired.Add(FCityGrid::CellKey(Center.X + DX, Center.Y + DY));

    // Collect cells to unload
    TArray<int64> ToUnload;
    for (const auto& KV : LoadedBlocks)
        if (!Desired.Contains(KV.Key))
            ToUnload.Add(KV.Key);

    for (int64 Key : ToUnload)
        DespawnCell(Key);

    // Load cells not yet present
    for (int64 Key : Desired)
        if (!LoadedBlocks.Contains(Key))
        {
            const FIntPoint Cell = FCityGrid::CellKeyDecode(Key);
            SpawnCell(Cell.X, Cell.Y);
        }
}

void UCityStreamingSubsystem::SpawnCell(int32 CX, int32 CY)
{
    const int64 Key = FCityGrid::CellKey(CX, CY);
    if (LoadedBlocks.Contains(Key)) return;

    UWorld* World = GetWorld();
    const float CellCm = GridParams.CellSizeM * 100.f;

    // ── Floor tile ────────────────────────────────────────────────────────────
    UStaticMesh* CubeMesh = LoadObject<UStaticMesh>(
        nullptr, TEXT("/Engine/BasicShapes/Cube"));
    if (CubeMesh)
    {
        // Centre the tile within the cell; sit 50cm below Z=0 (1cm thick cube)
        const FVector FloorPos(
            CX * CellCm + CellCm * 0.5f,
            CY * CellCm + CellCm * 0.5f,
            -50.f);

        FActorSpawnParameters FP;
        FP.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
        AStaticMeshActor* Floor = World->SpawnActor<AStaticMeshActor>(
            AStaticMeshActor::StaticClass(), FTransform(FloorPos), FP);
        if (Floor)
        {
            Floor->GetStaticMeshComponent()->SetStaticMesh(CubeMesh);
            Floor->GetStaticMeshComponent()->SetMobility(EComponentMobility::Movable);
            Floor->SetActorScale3D(FVector(CellCm / 100.f, CellCm / 100.f, 1.f));
            LoadedFloors.Add(Key, Floor);
        }
    }

    // ── City block — deferred so CellX/CellY/Seed are set before BeginPlay ───
    const FTransform CellTransform(FVector(CX * CellCm, CY * CellCm, 0.f));
    ACityBlockActor* Block = World->SpawnActorDeferred<ACityBlockActor>(
        ACityBlockActor::StaticClass(), CellTransform);
    if (Block)
    {
        Block->CellX = CX;
        Block->CellY = CY;
        Block->Seed  = static_cast<int32>(FBuildingPlacer::MixSeed(CX, CY, WorldSeed));
        Block->FinishSpawning(CellTransform);
        LoadedBlocks.Add(Key, Block);
    }
}

void UCityStreamingSubsystem::DespawnCell(int64 Key)
{
    if (TObjectPtr<ACityBlockActor>* Block = LoadedBlocks.Find(Key))
    {
        if (*Block) (*Block)->Destroy();
        LoadedBlocks.Remove(Key);
    }
    if (TObjectPtr<AActor>* Floor = LoadedFloors.Find(Key))
    {
        if (*Floor) (*Floor)->Destroy();
        LoadedFloors.Remove(Key);
    }
}

// ── Test helpers ──────────────────────────────────────────────────────────────

#if WITH_AUTOMATION_TESTS

void UCityStreamingSubsystem::Test_ForceRefresh()
{
    CheckCooldown = 0.f;
    LastCenter    = FIntPoint(MAX_int32, MAX_int32); // invalidate cache

    const APlayerController* PC = GetWorld()->GetFirstPlayerController();
    if (!PC || !PC->GetPawn()) return;

    const FVector   Pos  = PC->GetPawn()->GetActorLocation();
    const FIntPoint Cell = FCityGrid::WorldToCell(Pos.X, Pos.Y, GridParams);
    LastCenter = Cell;
    RefreshAroundCell(Cell);
}

bool UCityStreamingSubsystem::Test_IsCellLoaded(int32 CX, int32 CY) const
{
    return LoadedBlocks.Contains(FCityGrid::CellKey(CX, CY));
}

#endif
