# OpenCity

Lego City-style sandbox exploration game. Player walks and drives through a procedurally generated city. UE 5.5.4 built from source on Ubuntu Linux.

Engine: `/apps/git/UnrealEngine` — binary: `/apps/git/UnrealEngine/Engine/Binaries/Linux/UnrealEditor`

## Commands

```bash
# One-time setup: install git hooks
bash Tools/setup.sh

# Build editor module
/apps/git/UnrealEngine/Engine/Build/BatchFiles/Linux/Build.sh \
  OpenCityEditor Linux Development \
  /apps/git/unreal-OpenCity/OpenCity.uproject -waitmutex

# Run full automated test suite (Tier 1 + Tier 2 headless)
bash Tools/run_tests.sh

# Open editor
/apps/git/UnrealEngine/Engine/Binaries/Linux/UnrealEditor \
  /apps/git/unreal-OpenCity/OpenCity.uproject
```

## Architecture

### Core/ — No UObject base classes

`Source/OpenCity/Core/` contains pure game math and data structs. Rules:
- No `UObject`, `AActor`, `UActorComponent` as base classes.
- `USTRUCT` is allowed — it carries no UObject runtime cost and enables editor editability.
- Static methods only — no mutable global state.
- Every public function has a Tier 1 automation test before merging.

### Actor subdirectories — Thin wrappers over Core

`GameMode/`, `Character/`, `Vehicle/`, `City/` call Core functions. They own UObject lifecycle (spawn, destroy, tick) but contain no logic that can live in Core.

**Never duplicate logic from Core. Always call through.**

## Testing Requirements

No feature is complete without a corresponding test. This is enforced by the pre-commit hook.

| Tier | File | PIE? | Covers |
|------|------|------|--------|
| 1 | `Tests/CoreMathTests.cpp` | No | All `Core/` functions — grid math, building placement, vehicle params |
| 2 | `Tests/SpawnTests.cpp` | Yes | Actors spawn, are above ground, not embedded in geometry |
| 3 | `Tests/MovementTests.cpp` | Yes | Character walks, car drives, enter/exit vehicle works |
| 4 | `Tests/TESTPLAN.md` | Manual | Movement feel, camera, visual quality |

**Failure protocol:** Tier N failures must be fixed before starting Tier N+1 work.

## UE 5.5 Coding Standards

These apply to all new code in this project:

- `TObjectPtr<T>` for all UPROPERTY UObject members — never raw `T*`.
- `NotifyControllerChanged()` for input context setup, not `BeginPlay`.
- `bCanEverTick = false` in every constructor unless the class explicitly ticks.
- Enhanced Input only — no `BindAxis` / `BindAction` from the legacy input system.
- `BuildSettingsVersion::V5` and `IncludeOrderVersion::Latest` in all `.Target.cs` files.
- `PCHUsage = UseExplicitOrSharedPCHs` in `Build.cs`.
- `if (auto* X = Cast<T>(Y))` — never discard a Cast result.
- UPROPERTY categories follow `"OpenCity|Subsystem"` (e.g. `"OpenCity|Camera"`, `"OpenCity|Input"`, `"OpenCity|City"`).
- No `GEngine`, `GWorld`, or `UGameplayStatics` in `Core/` — those are actor-layer concerns.

## Scale Convention

1 meter = 100 UE units (UE uses cm internally). Design in meters; multiply by 100 for UE units.

| Object | Meters | UE units |
|--------|--------|----------|
| Player capsule height | 1.8 m | 180 |
| Car body (L×W×H) | 4.5×1.8×1.4 m | 450×180×140 |
| Road lane width | 3.5 m | 350 |
| City cell (block + roads) | 100×100 m | 10,000×10,000 |
| Standard building floor | 4 m | 400 |

All numeric constants live in `FCityGridParams`, `FCarParams`, `FBuildingPlacerParams`, or named `constexpr` values — never inline in logic.

## No Singletons

Use UE subsystems for global state (`UGameInstanceSubsystem`, `UWorldSubsystem`). Do not use the Singleton pattern or mutable global variables.
