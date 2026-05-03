#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "Tests/TestWorldHelper.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Character/OpenCityCharacter.h"
#include "Character/PedestrianCharacter.h"
#include "Character/CharacterDataAsset.h"
#include "Core/CharacterParams.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
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

bool FSpawnCharacterExists::RunTest(const FString &)
{
    UWorld *W = GetTestWorld();
    if (!TestNotNull(TEXT("Game world exists"), W))
        return false;

    APawn *Pawn = UGameplayStatics::GetPlayerPawn(W, 0);
    TestNotNull(TEXT("Player pawn exists"), Pawn);
    TestTrue(TEXT("Player pawn is AOpenCityCharacter"), Cast<AOpenCityCharacter>(Pawn) != nullptr);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSpawnCharacterAboveGround, "OpenCity.Spawn.Character.AboveGround",
                                 EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FSpawnCharacterAboveGround::RunTest(const FString &)
{
    UWorld *W = GetTestWorld();
    if (!TestNotNull(TEXT("Game world exists"), W))
        return false;

    APawn *Pawn = UGameplayStatics::GetPlayerPawn(W, 0);
    if (!TestNotNull(TEXT("Player pawn"), Pawn))
        return false;

    const float Z = Pawn->GetActorLocation().Z;
    TestTrue(FString::Printf(TEXT("Character Z=%.1fcm > -50cm (not through floor)"), Z), Z > -50.f);
    TestTrue(FString::Printf(TEXT("Character Z=%.1fcm < 500cm (not floating)"), Z), Z < 500.f);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSpawnCharacterHasController, "OpenCity.Spawn.Character.HasPlayerController",
                                 EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FSpawnCharacterHasController::RunTest(const FString &)
{
    UWorld *W = GetTestWorld();
    if (!TestNotNull(TEXT("Game world exists"), W))
        return false;

    APlayerController *PC = UGameplayStatics::GetPlayerController(W, 0);
    TestNotNull(TEXT("PlayerController exists"), PC);
    TestNotNull(TEXT("PlayerController has a pawn"), PC ? PC->GetPawn() : nullptr);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSpawnCharacterApplyDataAddsCapabilityTags,
                                 "OpenCity.Spawn.Character.ApplyDataAddsCapabilityTags",
                                 EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FSpawnCharacterApplyDataAddsCapabilityTags::RunTest(const FString &)
{
    UWorld *W = GetTestWorld();
    if (!TestNotNull(TEXT("Game world exists"), W))
        return false;

    AOpenCityCharacter *Character = Cast<AOpenCityCharacter>(UGameplayStatics::GetPlayerPawn(W, 0));
    if (!TestNotNull(TEXT("Player pawn is AOpenCityCharacter"), Character))
        return false;

    UCharacterDataAsset *Data = NewObject<UCharacterDataAsset>(Character);
    Data->CharacterName = FText::FromString(TEXT("Officer Jones"));
    Data->Appearance.HatStyle = TEXT("Cap");
    Data->Appearance.AccessoryStyle = TEXT("Badge");
    Data->Capabilities.Capabilities = {TAG_Capability_CanArrest.GetTag()};

    Character->CharacterData = Data;
    Character->ApplyCharacterData();

    UAbilitySystemComponent *ASC = Character->GetAbilitySystemComponent();
    if (!TestNotNull(TEXT("AbilitySystemComponent exists"), ASC))
        return false;

    TestTrue(TEXT("ASC contains CanArrest as loose tag"),
             ASC->HasMatchingGameplayTag(TAG_Capability_CanArrest.GetTag()));
    TestTrue(TEXT("Character capability helper returns true for CanArrest"),
             Character->HasCapability(TAG_Capability_CanArrest.GetTag()));

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSpawnNPCIsArrestableTag,
                                 "OpenCity.Spawn.Character.NPCIsArrestableTagApplied",
                                 EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FSpawnNPCIsArrestableTag::RunTest(const FString &)
{
    UWorld *W = GetTestWorld();
    if (!TestNotNull(TEXT("Game world exists"), W))
        return false;

    FActorSpawnParameters P;
    P.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    AOpenCityCharacter *NPC = W->SpawnActor<AOpenCityCharacter>(
        AOpenCityCharacter::StaticClass(), FVector(200.f, 200.f, 100.f), FRotator::ZeroRotator, P);
    if (!TestNotNull(TEXT("NPC spawned"), NPC))
        return false;

    UCharacterDataAsset *Data = NewObject<UCharacterDataAsset>(NPC);
    Data->CharacterName = FText::FromString(TEXT("Vince the Criminal"));
    Data->Appearance.FaceStyle = TEXT("Angry");
    Data->Capabilities.Capabilities = {TAG_Capability_IsArrestable.GetTag()};

    NPC->CharacterData = Data;
    NPC->ApplyCharacterData();

    UAbilitySystemComponent *ASC = NPC->GetAbilitySystemComponent();
    if (!TestNotNull(TEXT("NPC AbilitySystemComponent exists"), ASC))
        return false;

    TestTrue(TEXT("NPC ASC contains IsArrestable as loose tag"),
             ASC->HasMatchingGameplayTag(TAG_Capability_IsArrestable.GetTag()));
    TestTrue(TEXT("NPC HasCapability returns true for IsArrestable"),
             NPC->HasCapability(TAG_Capability_IsArrestable.GetTag()));

    NPC->Destroy();
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSpawnCivilianEmptyCapabilities,
                                 "OpenCity.Spawn.Character.CivilianEmptyCapabilitiesSpawnsClean",
                                 EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FSpawnCivilianEmptyCapabilities::RunTest(const FString &)
{
    UWorld *W = GetTestWorld();
    if (!TestNotNull(TEXT("Game world exists"), W))
        return false;

    FActorSpawnParameters P;
    P.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    AOpenCityCharacter *Civilian = W->SpawnActor<AOpenCityCharacter>(
        AOpenCityCharacter::StaticClass(), FVector(400.f, 0.f, 100.f), FRotator::ZeroRotator, P);
    if (!TestNotNull(TEXT("Civilian spawned"), Civilian))
        return false;

    UCharacterDataAsset *Data = NewObject<UCharacterDataAsset>(Civilian);
    Data->CharacterName = FText::FromString(TEXT("Bystander"));
    // Capabilities intentionally empty — plain civilian.

    Civilian->CharacterData = Data;
    Civilian->ApplyCharacterData(); // Must not crash with empty capabilities.

    UAbilitySystemComponent *ASC = Civilian->GetAbilitySystemComponent();
    if (!TestNotNull(TEXT("Civilian ASC exists"), ASC))
        return false;

    TestFalse(TEXT("Civilian ASC has no CanArrest tag"),
              ASC->HasMatchingGameplayTag(TAG_Capability_CanArrest.GetTag()));
    TestFalse(TEXT("Civilian ASC has no IsArrestable tag"),
              ASC->HasMatchingGameplayTag(TAG_Capability_IsArrestable.GetTag()));

    Civilian->Destroy();
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSpawnCharacterApplyDataIdempotent,
                                 "OpenCity.Spawn.Character.ApplyCharacterDataIsIdempotent",
                                 EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FSpawnCharacterApplyDataIdempotent::RunTest(const FString &)
{
    UWorld *W = GetTestWorld();
    if (!TestNotNull(TEXT("Game world exists"), W))
        return false;

    AOpenCityCharacter *Character = Cast<AOpenCityCharacter>(UGameplayStatics::GetPlayerPawn(W, 0));
    if (!TestNotNull(TEXT("Player pawn is AOpenCityCharacter"), Character))
        return false;

    UCharacterDataAsset *Data = NewObject<UCharacterDataAsset>(Character);
    Data->Capabilities.Capabilities = {TAG_Capability_CanArrest.GetTag()};

    Character->CharacterData = Data;
    Character->ApplyCharacterData();
    Character->ApplyCharacterData(); // Call twice — must not double-add tags.
    Character->ApplyCharacterData();

    UAbilitySystemComponent *ASC = Character->GetAbilitySystemComponent();
    if (!TestNotNull(TEXT("ASC exists"), ASC))
        return false;

    // GetGameplayTagCount returns the count of matching loose tags.
    const int32 Count = ASC->GetGameplayTagCount(TAG_Capability_CanArrest.GetTag());
    TestEqual(TEXT("CanArrest tag count is exactly 1 after 3 ApplyCharacterData calls"), Count, 1);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSpawnCharacterSwapDataAsset,
                                 "OpenCity.Spawn.Character.SwapDataAssetRemovesOldTagsAddsNew",
                                 EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FSpawnCharacterSwapDataAsset::RunTest(const FString &)
{
    UWorld *W = GetTestWorld();
    if (!TestNotNull(TEXT("Game world exists"), W))
        return false;

    FActorSpawnParameters P;
    P.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    AOpenCityCharacter *Character = W->SpawnActor<AOpenCityCharacter>(
        AOpenCityCharacter::StaticClass(), FVector(600.f, 0.f, 100.f), FRotator::ZeroRotator, P);
    if (!TestNotNull(TEXT("Character spawned"), Character))
        return false;

    // First data asset: firefighter.
    UCharacterDataAsset *FireData = NewObject<UCharacterDataAsset>(Character);
    FireData->Capabilities.Capabilities = {TAG_Capability_CanFightFire.GetTag()};
    Character->CharacterData = FireData;
    Character->ApplyCharacterData();

    UAbilitySystemComponent *ASC = Character->GetAbilitySystemComponent();
    if (!TestNotNull(TEXT("ASC exists"), ASC))
        return false;
    TestTrue(TEXT("CanFightFire present after first assignment"),
             ASC->HasMatchingGameplayTag(TAG_Capability_CanFightFire.GetTag()));

    // Swap to police officer data asset.
    UCharacterDataAsset *PoliceData = NewObject<UCharacterDataAsset>(Character);
    PoliceData->Capabilities.Capabilities = {TAG_Capability_CanArrest.GetTag()};
    Character->CharacterData = PoliceData;
    Character->ApplyCharacterData();

    TestFalse(TEXT("CanFightFire removed after swap"),
              ASC->HasMatchingGameplayTag(TAG_Capability_CanFightFire.GetTag()));
    TestTrue(TEXT("CanArrest present after swap"),
             ASC->HasMatchingGameplayTag(TAG_Capability_CanArrest.GetTag()));

    Character->Destroy();
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSpawnCharacterSwitchModeCyclesRoles,
                                 "OpenCity.Spawn.Character.SwitchModeCyclesRoles",
                                 EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FSpawnCharacterSwitchModeCyclesRoles::RunTest(const FString &)
{
    UWorld *W = GetTestWorld();
    if (!TestNotNull(TEXT("Game world exists"), W))
        return false;

    FActorSpawnParameters P;
    P.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    AOpenCityCharacter *Character = W->SpawnActor<AOpenCityCharacter>(
        AOpenCityCharacter::StaticClass(), FVector(800.f, 0.f, 100.f), FRotator::ZeroRotator, P);
    if (!TestNotNull(TEXT("Character spawned"), Character))
        return false;

    // Opening and cancelling the switch menu auto-assigns the default Officer
    // role, even for headless test actors that have no PlayerController.
    Character->ToggleCharacterSwitchMode();
    Character->CancelSwitchSelection();
    TestTrue(TEXT("Default role starts as officer (CanArrest)"),
             Character->HasCapability(TAG_Capability_CanArrest.GetTag()));

    Character->ToggleCharacterSwitchMode();
    Character->CycleSwitchSelection(+1);
    Character->ConfirmSwitchSelection();

    TestTrue(TEXT("After one step, role switches to firefighter (CanFightFire)"),
             Character->HasCapability(TAG_Capability_CanFightFire.GetTag()));
    TestFalse(TEXT("After firefighter switch, CanArrest is removed"),
              Character->HasCapability(TAG_Capability_CanArrest.GetTag()));

    Character->ToggleCharacterSwitchMode();
    Character->CycleSwitchSelection(+1);
    Character->ConfirmSwitchSelection();

    TestTrue(TEXT("After second step, role switches to EMT (CanHeal)"),
             Character->HasCapability(TAG_Capability_CanHeal.GetTag()));
    TestFalse(TEXT("EMT does not have CanFightFire"),
              Character->HasCapability(TAG_Capability_CanFightFire.GetTag()));

    Character->Destroy();
    return true;
}

// ── Buildings ─────────────────────────────────────────────────────────────────

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSpawnBuildingsBlockGenerates, "OpenCity.Spawn.Buildings.BlockGeneratesBuildings",
                                 EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FSpawnBuildingsBlockGenerates::RunTest(const FString &)
{
    UWorld *W = GetTestWorld();
    if (!TestNotNull(TEXT("Game world exists"), W))
        return false;

    FActorSpawnParameters P;
    P.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    ACityBlockActor *Block = W->SpawnActor<ACityBlockActor>(ACityBlockActor::StaticClass(),
                                                            FVector(50000.f, 50000.f, 0.f), FRotator::ZeroRotator, P);
    if (!TestNotNull(TEXT("CityBlockActor spawned"), Block))
        return false;

    Block->CellX = 5;
    Block->CellY = 5;
    Block->Seed = 12345;
    Block->GenerateBuildings();

    const int32 Count = Block->GetBuildings().Num();
    TestTrue(FString::Printf(TEXT("Block generated %d buildings (expected > 0)"), Count), Count > 0);

    Block->Destroy();
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSpawnBuildingsHasMesh, "OpenCity.Spawn.Buildings.HasMesh",
                                 EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FSpawnBuildingsHasMesh::RunTest(const FString &)
{
    UWorld *W = GetTestWorld();
    if (!TestNotNull(TEXT("Game world exists"), W))
        return false;

    FActorSpawnParameters P;
    P.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    ACityBlockActor *Block = W->SpawnActor<ACityBlockActor>(ACityBlockActor::StaticClass(),
                                                            FVector(50000.f, 60000.f, 0.f), FRotator::ZeroRotator, P);
    if (!TestNotNull(TEXT("CityBlockActor spawned"), Block))
        return false;

    Block->CellX = 5;
    Block->CellY = 6;
    Block->Seed = 99;
    Block->GenerateBuildings();

    const TArray<TObjectPtr<ABuildingActor>> &Buildings = Block->GetBuildings();
    if (!TestTrue(TEXT("At least one building spawned"), Buildings.Num() > 0))
    {
        Block->Destroy();
        return false;
    }

    for (const TObjectPtr<ABuildingActor> &B : Buildings)
    {
        if (!B)
            continue;
        // Access the mesh via the component — GetComponentByClass is the public API
        UStaticMeshComponent *Mesh = B->FindComponentByClass<UStaticMeshComponent>();
        TestNotNull(TEXT("BuildingActor has a StaticMeshComponent"), Mesh);
        if (Mesh)
            TestNotNull(TEXT("BuildingActor mesh is assigned (not null)"), Mesh->GetStaticMesh().Get());
    }

    Block->Destroy();
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSpawnBuildingsAboveGround, "OpenCity.Spawn.Buildings.AboveGround",
                                 EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FSpawnBuildingsAboveGround::RunTest(const FString &)
{
    UWorld *W = GetTestWorld();
    if (!TestNotNull(TEXT("Game world exists"), W))
        return false;

    FActorSpawnParameters P;
    P.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    ACityBlockActor *Block = W->SpawnActor<ACityBlockActor>(ACityBlockActor::StaticClass(),
                                                            FVector(50000.f, 70000.f, 0.f), FRotator::ZeroRotator, P);
    if (!TestNotNull(TEXT("CityBlockActor spawned"), Block))
        return false;

    Block->CellX = 5;
    Block->CellY = 7;
    Block->Seed = 777;
    Block->GenerateBuildings();

    for (const TObjectPtr<ABuildingActor> &B : Block->GetBuildings())
    {
        if (!B)
            continue;
        const float Z = B->GetActorLocation().Z;
        TestTrue(FString::Printf(TEXT("Building Z=%.1fcm >= 0 (not inside floor)"), Z), Z >= 0.f);
    }

    Block->Destroy();
    return true;
}

// ── Streaming ─────────────────────────────────────────────────────────────────

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FStreamingLoadsNearbyCell, "OpenCity.Streaming.LoadsNearbyCell",
                                 EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FStreamingLoadsNearbyCell::RunTest(const FString &)
{
    UWorld *W = GetTestWorld();
    if (!TestNotNull(TEXT("Game world exists"), W))
        return false;

    UCityStreamingSubsystem *Sub = W->GetSubsystem<UCityStreamingSubsystem>();
    if (!TestNotNull(TEXT("CityStreamingSubsystem exists"), Sub))
        return false;

    Sub->Test_ForceRefresh();

    const APlayerController *PC = W->GetFirstPlayerController();
    if (!TestNotNull(TEXT("PlayerController exists"), PC))
        return false;
    if (!TestNotNull(TEXT("PlayerController has pawn"), PC->GetPawn()))
        return false;

    // Player is near origin; cell (0,0) must be loaded after refresh
    TestTrue(TEXT("Cell (0,0) is loaded near player spawn"), Sub->Test_IsCellLoaded(0, 0));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FStreamingUnloadsFarCell, "OpenCity.Streaming.UnloadsFarCell",
                                 EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FStreamingUnloadsFarCell::RunTest(const FString &)
{
    UWorld *W = GetTestWorld();
    if (!TestNotNull(TEXT("Game world exists"), W))
        return false;

    UCityStreamingSubsystem *Sub = W->GetSubsystem<UCityStreamingSubsystem>();
    if (!TestNotNull(TEXT("CityStreamingSubsystem exists"), Sub))
        return false;

    Sub->Test_ForceRefresh();

    // A cell far beyond LoadRadius (3) from origin must not be loaded
    const int32 FarCell = UCityStreamingSubsystem::LoadRadius + 5;
    TestFalse(FString::Printf(TEXT("Cell (%d,%d) far from player is NOT loaded"), FarCell, FarCell),
              Sub->Test_IsCellLoaded(FarCell, FarCell));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FStreamingCellCountIsBounded, "OpenCity.Streaming.CellCountIsBounded",
                                 EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FStreamingCellCountIsBounded::RunTest(const FString &)
{
    UWorld *W = GetTestWorld();
    if (!TestNotNull(TEXT("Game world exists"), W))
        return false;

    UCityStreamingSubsystem *Sub = W->GetSubsystem<UCityStreamingSubsystem>();
    if (!TestNotNull(TEXT("CityStreamingSubsystem exists"), Sub))
        return false;

    Sub->Test_ForceRefresh();

    const int32 MaxCells = (2 * UCityStreamingSubsystem::LoadRadius + 1) *
                           (2 * UCityStreamingSubsystem::LoadRadius + 1);
    const int32 Loaded = Sub->Test_LoadedCellCount();
    TestTrue(FString::Printf(TEXT("Loaded cells %d <= max %d"), Loaded, MaxCells), Loaded <= MaxCells);
    TestTrue(TEXT("At least 1 cell loaded"), Loaded > 0);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FStreamingDeterministicReload, "OpenCity.Streaming.DeterministicReload",
                                 EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FStreamingDeterministicReload::RunTest(const FString &)
{
    UWorld *W = GetTestWorld();
    if (!TestNotNull(TEXT("Game world exists"), W))
        return false;

    // Spawn a block at a known cell, record building count, destroy, respawn, compare
    FActorSpawnParameters SP;
    SP.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    auto SpawnAndCount = [&]() -> int32
    {
        ACityBlockActor *B = W->SpawnActor<ACityBlockActor>(ACityBlockActor::StaticClass(),
                                                            FVector(80000.f, 80000.f, 0.f), FRotator::ZeroRotator, SP);
        if (!B)
            return -1;
        B->CellX = 8;
        B->CellY = 8;
        B->Seed = 42;
        B->GenerateBuildings();
        const int32 N = B->GetBuildings().Num();
        B->Destroy();
        return N;
    };

    const int32 First = SpawnAndCount();
    const int32 Second = SpawnAndCount();

    if (!TestTrue(TEXT("First spawn produced buildings"), First > 0))
        return false;
    TestEqual(TEXT("Same cell/seed produces identical building count on reload"), Second, First);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FStreamingUnloadDestroysBuildings, "OpenCity.Streaming.UnloadDestroysBuildings",
                                 EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FStreamingUnloadDestroysBuildings::RunTest(const FString &)
{
    UWorld *W = GetTestWorld();
    if (!TestNotNull(TEXT("Game world exists"), W))
        return false;

    FActorSpawnParameters SP;
    SP.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    ACityBlockActor *Block = W->SpawnActor<ACityBlockActor>(ACityBlockActor::StaticClass(),
                                                            FVector(90000.f, 90000.f, 0.f), FRotator::ZeroRotator, SP);
    if (!TestNotNull(TEXT("CityBlockActor spawned"), Block))
        return false;

    Block->CellX = 9;
    Block->CellY = 9;
    Block->Seed = 55;
    Block->GenerateBuildings();

    const int32 BeforeCount = Block->GetBuildings().Num();
    if (!TestTrue(TEXT("Block has buildings before destroy"), BeforeCount > 0))
    {
        Block->Destroy();
        return false;
    }

    // Collect raw pointers to validate they are gone after destroy
    TArray<ABuildingActor *> RawBuildings;
    for (const TObjectPtr<ABuildingActor> &B : Block->GetBuildings())
        if (B)
            RawBuildings.Add(B.Get());

    Block->Destroy(); // triggers EndPlay → ClearBuildings

    for (ABuildingActor *B : RawBuildings)
        TestTrue(TEXT("BuildingActor is pending kill after block destroy"), !IsValid(B));

    return true;
}

// ── Pedestrians ───────────────────────────────────────────────────────────────

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSpawnPedestrianCount, "OpenCity.Spawn.Pedestrian.CountMatchesPerCell",
                                 EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FSpawnPedestrianCount::RunTest(const FString &)
{
    UWorld *W = GetTestWorld();
    if (!TestNotNull(TEXT("Game world exists"), W))
        return false;

    FActorSpawnParameters P;
    P.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    ACityBlockActor *Block = W->SpawnActor<ACityBlockActor>(ACityBlockActor::StaticClass(),
                                                            FVector(110000.f, 110000.f, 0.f), FRotator::ZeroRotator, P);
    if (!TestNotNull(TEXT("CityBlockActor spawned"), Block))
        return false;

    Block->CellX = 11;
    Block->CellY = 11;
    Block->Seed = 42;
    Block->GenerateBuildings();

    const int32 Expected = Block->PedestrianParams.PerCell;
    const int32 Actual = Block->GetPedestrians().Num();
    TestEqual(FString::Printf(TEXT("Pedestrian count %d == PerCell %d"), Actual, Expected), Actual, Expected);

    Block->Destroy();
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSpawnPedestrianAboveGround, "OpenCity.Spawn.Pedestrian.AboveGround",
                                 EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FSpawnPedestrianAboveGround::RunTest(const FString &)
{
    UWorld *W = GetTestWorld();
    if (!TestNotNull(TEXT("Game world exists"), W))
        return false;

    FActorSpawnParameters P;
    P.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    ACityBlockActor *Block = W->SpawnActor<ACityBlockActor>(ACityBlockActor::StaticClass(),
                                                            FVector(120000.f, 120000.f, 0.f), FRotator::ZeroRotator, P);
    if (!TestNotNull(TEXT("CityBlockActor spawned"), Block))
        return false;

    Block->CellX = 12;
    Block->CellY = 12;
    Block->Seed = 99;
    Block->GenerateBuildings();

    for (const TObjectPtr<APedestrianCharacter> &Ped : Block->GetPedestrians())
    {
        if (!Ped)
            continue;
        const float Z = Ped->GetActorLocation().Z;
        TestTrue(FString::Printf(TEXT("Pedestrian Z=%.1fcm > -10cm (not through floor)"), Z), Z > -10.f);
    }

    Block->Destroy();
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSpawnPedestrianDestroyedWithBlock, "OpenCity.Spawn.Pedestrian.DestroyedWithBlock",
                                 EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FSpawnPedestrianDestroyedWithBlock::RunTest(const FString &)
{
    UWorld *W = GetTestWorld();
    if (!TestNotNull(TEXT("Game world exists"), W))
        return false;

    FActorSpawnParameters P;
    P.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    ACityBlockActor *Block = W->SpawnActor<ACityBlockActor>(ACityBlockActor::StaticClass(),
                                                            FVector(130000.f, 130000.f, 0.f), FRotator::ZeroRotator, P);
    if (!TestNotNull(TEXT("CityBlockActor spawned"), Block))
        return false;

    Block->CellX = 13;
    Block->CellY = 13;
    Block->Seed = 77;
    Block->GenerateBuildings();

    if (!TestTrue(TEXT("Block has pedestrians before destroy"), Block->GetPedestrians().Num() > 0))
    {
        Block->Destroy();
        return false;
    }

    TArray<APedestrianCharacter *> RawPedestrians;
    for (const TObjectPtr<APedestrianCharacter> &Ped : Block->GetPedestrians())
        if (Ped)
            RawPedestrians.Add(Ped.Get());

    Block->Destroy(); // triggers EndPlay → ClearBuildings → ClearPedestrians

    for (APedestrianCharacter *Ped : RawPedestrians)
        TestTrue(TEXT("PedestrianCharacter is pending kill after block destroy"), !IsValid(Ped));

    return true;
}

// ── Character Data Asset ──────────────────────────────────────────────────────

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSpawnCharacterOfficerDataApplied, "OpenCity.Spawn.Character.OfficerDataApplied",
                                 EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FSpawnCharacterOfficerDataApplied::RunTest(const FString &)
{
    UWorld *W = GetTestWorld();
    if (!TestNotNull(TEXT("Game world exists"), W))
        return false;

    FActorSpawnParameters P;
    P.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    AOpenCityCharacter *Char = W->SpawnActor<AOpenCityCharacter>(
        AOpenCityCharacter::StaticClass(), FVector(0.f, 5000.f, 500.f), FRotator::ZeroRotator, P);
    if (!TestNotNull(TEXT("Character spawned"), Char))
        return false;

    UCharacterDataAsset *Data = NewObject<UCharacterDataAsset>(W);
    Data->CharacterName = FText::FromString(TEXT("Officer"));
    Data->Appearance.HatStyle = TEXT("Cap");
    Data->Appearance.AccessoryStyle = TEXT("Badge");
    Data->Capabilities.Capabilities = {TAG_Capability_CanArrest.GetTag()};
    Char->CharacterData = Data;
    Char->ApplyCharacterData();

    TestTrue(TEXT("CanArrest tag applied"), Char->HasCapability(TAG_Capability_CanArrest.GetTag()));
    TestTrue(TEXT("GAS component exists"), Char->GetAbilitySystemComponent() != nullptr);

    Char->Destroy();
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSpawnCharacterFirefighterDataApplied, "OpenCity.Spawn.Character.FirefighterDataApplied",
                                 EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FSpawnCharacterFirefighterDataApplied::RunTest(const FString &)
{
    UWorld *W = GetTestWorld();
    if (!TestNotNull(TEXT("Game world exists"), W))
        return false;

    FActorSpawnParameters P;
    P.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    AOpenCityCharacter *Char = W->SpawnActor<AOpenCityCharacter>(
        AOpenCityCharacter::StaticClass(), FVector(0.f, 6000.f, 500.f), FRotator::ZeroRotator, P);
    if (!TestNotNull(TEXT("Character spawned"), Char))
        return false;

    UCharacterDataAsset *Data = NewObject<UCharacterDataAsset>(W);
    Data->CharacterName = FText::FromString(TEXT("Firefighter"));
    Data->Appearance.ShirtColor = FLinearColor(0.9f, 0.1f, 0.1f, 1.f);
    Data->Appearance.HatStyle = TEXT("Helmet");
    Data->Capabilities.Capabilities = {TAG_Capability_CanFightFire.GetTag(), TAG_Capability_CanRescue.GetTag()};
    Char->CharacterData = Data;
    Char->ApplyCharacterData();

    TestTrue(TEXT("CanFightFire tag applied"), Char->HasCapability(TAG_Capability_CanFightFire.GetTag()));
    TestTrue(TEXT("CanRescue tag applied"), Char->HasCapability(TAG_Capability_CanRescue.GetTag()));
    TestTrue(TEXT("GAS component exists"), Char->GetAbilitySystemComponent() != nullptr);

    Char->Destroy();
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSpawnCharacterNoDataHasNoCapabilities, "OpenCity.Spawn.Character.NoDataHasNoCapabilities",
                                 EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FSpawnCharacterNoDataHasNoCapabilities::RunTest(const FString &)
{
    UWorld *W = GetTestWorld();
    if (!TestNotNull(TEXT("Game world exists"), W))
        return false;

    FActorSpawnParameters P;
    P.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    AOpenCityCharacter *Char = W->SpawnActor<AOpenCityCharacter>(
        AOpenCityCharacter::StaticClass(), FVector(0.f, 7000.f, 500.f), FRotator::ZeroRotator, P);
    if (!TestNotNull(TEXT("Character spawned"), Char))
        return false;

    // No CharacterData assigned — must not crash, must have no capability tags.
    TestFalse(TEXT("No data → no CanArrest capability"), Char->HasCapability(TAG_Capability_CanArrest.GetTag()));
    TestFalse(TEXT("No data → no IsArrestable capability"), Char->HasCapability(TAG_Capability_IsArrestable.GetTag()));

    Char->Destroy();
    return true;
}

// ── Character Switch Mode ─────────────────────────────────────────────────────

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSpawnCharacterSwitchDefaultRosterNotEmpty,
                                 "OpenCity.Spawn.Character.SwitchDefaultRosterNotEmpty",
                                 EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FSpawnCharacterSwitchDefaultRosterNotEmpty::RunTest(const FString &)
{
    UWorld *W = GetTestWorld();
    if (!TestNotNull(TEXT("Game world exists"), W))
        return false;

    FActorSpawnParameters P;
    P.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    AOpenCityCharacter *Char = W->SpawnActor<AOpenCityCharacter>(
        AOpenCityCharacter::StaticClass(), FVector(0.f, 8000.f, 500.f), FRotator::ZeroRotator, P);
    if (!TestNotNull(TEXT("Character spawned"), Char))
        return false;

    // Opening switch mode triggers EnsureDefaultSwitchRoster internally.
    Char->ToggleCharacterSwitchMode();
    TestTrue(TEXT("Switch mode is open after Toggle"), Char->IsCharacterSwitchModeOpen());

    // CharacterData must now be set to the first roster entry (Officer).
    TestNotNull(TEXT("CharacterData set from default roster"), Char->CharacterData.Get());

    Char->CancelSwitchSelection();
    TestFalse(TEXT("Switch mode closed after Cancel"), Char->IsCharacterSwitchModeOpen());

    Char->Destroy();
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSpawnCharacterSwitchCycleWraps,
                                 "OpenCity.Spawn.Character.SwitchCycleWraps",
                                 EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FSpawnCharacterSwitchCycleWraps::RunTest(const FString &)
{
    UWorld *W = GetTestWorld();
    if (!TestNotNull(TEXT("Game world exists"), W))
        return false;

    FActorSpawnParameters P;
    P.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    AOpenCityCharacter *Char = W->SpawnActor<AOpenCityCharacter>(
        AOpenCityCharacter::StaticClass(), FVector(0.f, 9000.f, 500.f), FRotator::ZeroRotator, P);
    if (!TestNotNull(TEXT("Character spawned"), Char))
        return false;

    Char->ToggleCharacterSwitchMode();

    // Cycle forward through the entire roster (3 entries: Officer, Firefighter, EMT).
    Char->CycleSwitchSelection(+1); // → Firefighter
    Char->CycleSwitchSelection(+1); // → EMT
    Char->CycleSwitchSelection(+1); // → wrap back to Officer

    // Confirm selects whoever is highlighted — wrapping means we land back on Officer.
    Char->ConfirmSwitchSelection();
    TestFalse(TEXT("Switch mode closed after Confirm"), Char->IsCharacterSwitchModeOpen());
    if (TestNotNull(TEXT("CharacterData set after confirm"), Char->CharacterData.Get()))
        TestTrue(TEXT("Role after full wrap is Officer"),
                 Char->HasCapability(TAG_Capability_CanArrest.GetTag()));

    Char->Destroy();
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSpawnCharacterSwitchFirefighterRole,
                                 "OpenCity.Spawn.Character.SwitchToFirefighterRole",
                                 EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FSpawnCharacterSwitchFirefighterRole::RunTest(const FString &)
{
    UWorld *W = GetTestWorld();
    if (!TestNotNull(TEXT("Game world exists"), W))
        return false;

    FActorSpawnParameters P;
    P.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    AOpenCityCharacter *Char = W->SpawnActor<AOpenCityCharacter>(
        AOpenCityCharacter::StaticClass(), FVector(0.f, 10000.f, 500.f), FRotator::ZeroRotator, P);
    if (!TestNotNull(TEXT("Character spawned"), Char))
        return false;

    Char->ToggleCharacterSwitchMode();
    Char->CycleSwitchSelection(+1); // Officer → Firefighter
    Char->ConfirmSwitchSelection();

    TestTrue(TEXT("CanFightFire set after switching to Firefighter"),
             Char->HasCapability(TAG_Capability_CanFightFire.GetTag()));
    TestFalse(TEXT("CanArrest cleared after switching away from Officer"),
              Char->HasCapability(TAG_Capability_CanArrest.GetTag()));

    Char->Destroy();
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSpawnCharacterFirefighterRedShirtTint,
                                 "OpenCity.Spawn.Character.FirefighterRoleAppliesRedShirtTint",
                                 EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FSpawnCharacterFirefighterRedShirtTint::RunTest(const FString &)
{
    UWorld *W = GetTestWorld();
    if (!TestNotNull(TEXT("Game world exists"), W))
        return false;

    FActorSpawnParameters P;
    P.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    AOpenCityCharacter *Char = W->SpawnActor<AOpenCityCharacter>(
        AOpenCityCharacter::StaticClass(), FVector(0.f, 11000.f, 500.f), FRotator::ZeroRotator, P);
    if (!TestNotNull(TEXT("Character spawned"), Char))
        return false;

    Char->ToggleCharacterSwitchMode();
    Char->CycleSwitchSelection(+1);
    Char->ConfirmSwitchSelection();

    TestTrue(TEXT("CanFightFire set after switching to Firefighter"),
             Char->HasCapability(TAG_Capability_CanFightFire.GetTag()));

    USkeletalMeshComponent *CharacterMesh = Char->GetCharacterMeshComponent();
    if (!TestNotNull(TEXT("Character mesh component exists"), CharacterMesh))
    {
        Char->Destroy();
        return false;
    }

    const int32 ShirtSlotIndex = CharacterMesh->GetMaterialIndex(CharacterMeshSlot::Shirt);
    if (!TestTrue(TEXT("Shirt slot exists on character mesh component"), ShirtSlotIndex != INDEX_NONE))
    {
        Char->Destroy();
        return false;
    }

    UMaterialInterface *AppliedShirtMaterial = CharacterMesh->GetMaterial(ShirtSlotIndex);
    if (!TestNotNull(TEXT("Firefighter shirt slot has assigned material"), AppliedShirtMaterial))
    {
        Char->Destroy();
        return false;
    }

    UMaterialInstanceDynamic *ShirtMID = Cast<UMaterialInstanceDynamic>(AppliedShirtMaterial);
    if (!TestNotNull(TEXT("Firefighter shirt slot is a dynamic material instance"), ShirtMID))
    {
        Char->Destroy();
        return false;
    }

    FLinearColor ActualShirtTint;
    if (!TestTrue(TEXT("Firefighter shirt MID exposes ShirtTint"),
                  ShirtMID->GetVectorParameterValue(FHashedMaterialParameterInfo(TEXT("ShirtTint")),
                                                    ActualShirtTint)))
    {
        Char->Destroy();
        return false;
    }

    const FLinearColor ExpectedShirtTint = FLinearColor(0.85f, 0.12f, 0.12f, 1.f);
    const float Tolerance = KINDA_SMALL_NUMBER;
    TestTrue(TEXT("Firefighter shirt tint is red"),
             ActualShirtTint.Equals(ExpectedShirtTint, Tolerance));

    Char->Destroy();
    return true;
}
