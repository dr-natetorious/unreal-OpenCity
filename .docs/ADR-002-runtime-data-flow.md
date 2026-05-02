# ADR-002: Runtime Data Flow (Spawn, Streaming, Interaction)

## Status

Accepted

## Date

2026-04-26

## Context

OpenCity runtime behavior depends on predictable flow across startup, world streaming, and character-to-vehicle interaction.

## Decision

Standardize runtime flow as follows.

## Data Flow

### 1) Startup and Player Spawn

1. Game module loads.
2. `AOpenCityGameMode` sets `DefaultPawnClass = AOpenCityCharacter`.
3. Character input mappings/actions are attached in `NotifyControllerChanged`.

### 2) City Streaming Loop

1. `UCityStreamingSubsystem::Tick` samples player pawn world location.
2. Position maps to grid coordinates through `FCityGrid::WorldToCell`.
3. `RefreshAroundCell` computes desired key set within `LoadRadius`.
4. Cells outside set are despawned; missing desired cells are spawned.
5. Spawned cell creates:
   - floor tile actor
   - deferred `ACityBlockActor` with `CellX`, `CellY`, `Seed`

### 3) Building Generation

1. `ACityBlockActor::BeginPlay` calls `GenerateBuildings`.
2. `GenerateBuildings` calls `FBuildingPlacer::PlaceInCell`.
3. Each returned `FBuildingSpec` is materialized into an `ABuildingActor` via `SetSpec`.

### 4) Character <-> Car Interaction

1. Car proximity overlap sets/clears nearby car pointer on character.
2. Character interact calls `ACarPawn::EnterCar`.
3. Car possession hides character and transfers controller possession to car.
4. Car tick applies throttle/steer movement and wheel spin while occupied.
5. Interact in car exits, removes vehicle mapping context, and re-possesses character.

## Seed and Determinism Path

- World seed + cell coords are mixed into per-cell seed.
- Same cell coordinates and seed produce the same building specs.
- Procedural city is location-deterministic.

## Consequences

- Streaming behavior depends on correct world-to-cell mapping.
- Possession and mapping-context handoff must remain symmetrical on enter/exit.
- Any change in seed mixing or placement rules alters world determinism and should be explicitly documented.
