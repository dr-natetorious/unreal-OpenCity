#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Character/OpenCityCharacter.h"
#include "Vehicle/CarPawn.h"

// Tier 2 — PIE required (-NullRHI is still valid; no rendering needed for spawn checks).
// Run: UnrealEditor OpenCity.uproject -ExecCmds="Automation RunTests OpenCity.Spawn" -NullRHI -Unattended
//
// These tests verify actor spawn state immediately after PIE starts.
// They do NOT test movement (Tier 3) or visual quality (Tier 4).

static UWorld* GetPIEWorld()
{
    if (!GEngine) return nullptr;
    for (const FWorldContext& Ctx : GEngine->GetWorldContexts())
        if (Ctx.WorldType == EWorldType::PIE)
            return Ctx.World();
    return nullptr;
}

// ── Player character ──────────────────────────────────────────────────────────

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSpawnCharacterExists, "OpenCity.Spawn.Character.Exists",
    EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FSpawnCharacterExists::RunTest(const FString&)
{
    UWorld* W = GetPIEWorld();
    if (!TestNotNull(TEXT("PIE world is running — press Play before running Tier 2 tests"), W))
        return false;

    APawn* Pawn = UGameplayStatics::GetPlayerPawn(W, 0);
    TestNotNull(TEXT("Player pawn exists"), Pawn);
    TestTrue(TEXT("Player pawn is AOpenCityCharacter"), Cast<AOpenCityCharacter>(Pawn) != nullptr);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSpawnCharacterAboveGround, "OpenCity.Spawn.Character.AboveGround",
    EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FSpawnCharacterAboveGround::RunTest(const FString&)
{
    UWorld* W = GetPIEWorld();
    if (!TestNotNull(TEXT("PIE world"), W)) return false;

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
    UWorld* W = GetPIEWorld();
    if (!TestNotNull(TEXT("PIE world"), W)) return false;

    APlayerController* PC = UGameplayStatics::GetPlayerController(W, 0);
    TestNotNull(TEXT("PlayerController exists"), PC);
    TestNotNull(TEXT("PlayerController has a pawn"), PC ? PC->GetPawn() : nullptr);
    return true;
}
