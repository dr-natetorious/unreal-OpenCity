# ADR-001: Layered Architecture and Data Ownership

## Status

Accepted

## Date

2026-04-26

## Context

OpenCity is a procedural sandbox project where gameplay and world generation need deterministic behavior, testability, and clear ownership boundaries between pure logic and Unreal lifecycle concerns.

## Decision

Use a two-layer architecture with tests validating both layers:

1. Core layer (pure functions, no UObject lifecycle dependency)
2. Actor/Subsystem layer (spawning, possession, input, world ticking)

### Core Layer Decisions

- `Core/CityGrid.*` owns coordinate and road/buildable-space math.
- `Core/BuildingPlacer.*` owns deterministic building spec generation and spawn-clear checks.
- `Core/VehicleParams.h` owns vehicle parameter data and validation helpers.
- Core code remains deterministic and callable without PIE.

### Actor/Subsystem Layer Decisions

- `City/CityStreamingSubsystem.*` manages loading and unloading city cells around player position.
- `City/CityBlockActor.*` materializes one cell of generated building specs.
- `City/BuildingActor.*` renders one building from a generated spec.
- `Character/OpenCityCharacter.*` and `Vehicle/CarPawn.*` own player interaction, possession, and input mapping transitions.
- `GameMode/OpenCityGameMode.*` sets the default pawn class.

## Rationale

- Keeps procedural algorithms testable and reusable.
- Avoids duplicating generation logic in actors.
- Constrains UE-specific state to wrapper classes.
- Supports headless Tier 1 tests for core logic and higher-tier behavior tests for runtime systems.

## Consequences

- New procedural rules must be implemented in Core first, then called by wrappers.
- Runtime classes should not re-implement Core decisions.
- Regression risk is reduced by keeping deterministic code outside actor lifecycle.
