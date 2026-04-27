#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "Tests/TestWorldHelper.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Character/OpenCityCharacter.h"
#include "Vehicle/CarPawn.h"

// Tier 3 — Movement tests. Runs headlessly; no rendering required.
// Auto-discovered by: Automation RunTests OpenCity
//
// All tests spawn their own temporary actors — no level content required.

static ACarPawn* SpawnTestCar(UWorld* W)
{
    FActorSpawnParameters P;
    P.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    return W->SpawnActor<ACarPawn>(ACarPawn::StaticClass(),
        FVector(5000.f, 0.f, 100.f), FRotator::ZeroRotator, P);
}

// ── Enter car ────────────────────────────────────────────────────────────────

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCarEnterTest, "OpenCity.Movement.Car.EnterCar",
    EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FCarEnterTest::RunTest(const FString&)
{
    UWorld* W = GetTestWorld();
    if (!TestNotNull(TEXT("Game world exists"), W)) return false;

    APlayerController* PC = UGameplayStatics::GetPlayerController(W, 0);
    if (!TestNotNull(TEXT("PlayerController"), PC)) return false;

    AOpenCityCharacter* Char = Cast<AOpenCityCharacter>(PC->GetPawn());
    if (!TestNotNull(TEXT("Player is AOpenCityCharacter"), Char)) return false;

    ACarPawn* Car = SpawnTestCar(W);
    if (!TestNotNull(TEXT("Test car spawned"), Car)) return false;

    Car->EnterCar(Char);

    TestTrue(TEXT("Car reports occupied after EnterCar"), Car->Test_IsOccupied());
    TestEqual(TEXT("PC now possesses car"), PC->GetPawn(), static_cast<APawn*>(Car));

    // Restore world state
    PC->Possess(Char);
    Char->SetActorHiddenInGame(false);
    Char->SetActorEnableCollision(true);
    Car->Destroy();

    return true;
}

// ── Exit car ─────────────────────────────────────────────────────────────────

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCarExitTest, "OpenCity.Movement.Car.ExitCar",
    EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FCarExitTest::RunTest(const FString&)
{
    UWorld* W = GetTestWorld();
    if (!TestNotNull(TEXT("Game world exists"), W)) return false;

    APlayerController* PC = UGameplayStatics::GetPlayerController(W, 0);
    if (!TestNotNull(TEXT("PlayerController"), PC)) return false;

    AOpenCityCharacter* Char = Cast<AOpenCityCharacter>(PC->GetPawn());
    if (!TestNotNull(TEXT("Player is AOpenCityCharacter"), Char)) return false;

    ACarPawn* Car = SpawnTestCar(W);
    if (!TestNotNull(TEXT("Test car spawned"), Car)) return false;

    Car->EnterCar(Char);
    Car->Test_TriggerExit();

    TestFalse(TEXT("Car is unoccupied after exit"), Car->Test_IsOccupied());
    TestTrue(TEXT("PC re-possesses character after exit"),
        Cast<AOpenCityCharacter>(PC->GetPawn()) != nullptr);

    Car->Destroy();
    return true;
}

// ── Car moves on throttle ─────────────────────────────────────────────────────

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCarMovesOnThrottle, "OpenCity.Movement.Car.MovesOnThrottle",
    EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FCarMovesOnThrottle::RunTest(const FString&)
{
    UWorld* W = GetTestWorld();
    if (!TestNotNull(TEXT("Game world exists"), W)) return false;

    APlayerController* PC = UGameplayStatics::GetPlayerController(W, 0);
    if (!TestNotNull(TEXT("PlayerController"), PC)) return false;

    AOpenCityCharacter* Char = Cast<AOpenCityCharacter>(PC->GetPawn());
    if (!TestNotNull(TEXT("Player is AOpenCityCharacter"), Char)) return false;

    ACarPawn* Car = SpawnTestCar(W);
    if (!TestNotNull(TEXT("Test car spawned"), Car)) return false;

    Car->EnterCar(Char);

    const FVector Before = Car->GetActorLocation();
    Car->Test_SetThrottle(1.0f);
    Car->Test_Tick(0.1f);  // 0.1 s at 1500 cm/s → ~150 cm forward
    const FVector After = Car->GetActorLocation();

    const float Moved = FVector::Dist(Before, After);
    TestTrue(FString::Printf(TEXT("Car moved %.1f cm (expected ~150 cm)"), Moved), Moved > 10.f);

    Car->Test_TriggerExit();
    Car->Destroy();
    return true;
}
