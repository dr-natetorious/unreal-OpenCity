# ADR-003: UML Class Structure and Relationships

## Status

Accepted

## Date

2026-04-26

## Context

Team needs a stable, implementation-aligned UML view of key runtime and core relationships.

## Decision

Adopt the following class-level structure and dependencies.

## UML (Text)

- `AOpenCityGameMode` -> creates default player pawn type `AOpenCityCharacter`
- `UCityStreamingSubsystem` -> uses `FCityGrid`
- `UCityStreamingSubsystem` -> creates `ACityBlockActor`
- `ACityBlockActor` -> uses `FBuildingPlacer`
- `ACityBlockActor` -> creates/owns multiple `ABuildingActor`
- `ABuildingActor` -> consumes `FBuildingSpec`
- `AOpenCityCharacter` <-> `ACarPawn` (proximity + interact + possession handoff)
- `ACarPawn` -> uses `FCarParams` as editable vehicle parameter state

## Layering Rule

- Core structs/functions define logic and deterministic outputs.
- Actor/subsystem classes handle Unreal lifecycle, ownership, spawn/despawn, possession, and input.

## Notes

- `FCarParams` exists as parameter model, but movement update in `ACarPawn` currently uses internal constants for speed/turn in tick path.
- If vehicle behavior is fully parameterized later, update this ADR to reflect the new dependency flow.

## Consequences

- UML should be updated whenever ownership or dependency direction changes.
- New gameplay features should keep dependency direction from wrappers to Core, not vice versa.
