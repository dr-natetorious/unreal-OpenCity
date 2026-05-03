# ADR-004: GAS Foundation — Character Division System

## Status

Accepted

## Date

2026-05-02

## Context

OpenCity needs ~150 characters across 7 divisions (Police, Firefighter, EMT, Pilot, Astronaut,
Engineer, Superhero). Each character shares ~20 capability flags combined differently. The division
system drives ability gating, NPC reactions, incident eligibility, and UI state.

A C++ subclass per character type would not scale. The system needs to be data-driven from day one.

## Decision

### 1. GAS as the ability backbone

Unreal's Gameplay Ability System (GAS) is used rather than a custom equivalent. It provides:

- `UAbilitySystemComponent` — hub for abilities, effects, and tags on the character
- `FGameplayTag` — the language all systems speak (`Character.Division.Police`, `Capability.Arrest`)
- `UGameplayAbility` — each division ability (Arrest, Hose, Repair) is a future subclass
- `UAttributeSet` — health, stamina, oxygen, division credits (future)

**One `UAbilitySystemComponent` lives on `AOpenCityCharacter`.** There are no per-division subclasses.

### 2. Data-asset-driven characters

Every character is a `UCharacterDataAsset` instance created in the editor — not a C++ subclass.
`AOpenCityCharacter` reads the assigned asset in `BeginPlay` via `ApplyCharacterData()`.

```
DA_Police.uasset         → Division: Character.Division.Police, BodyColor: Blue
DA_Firefighter.uasset    → Division: Character.Division.Firefighter, BodyColor: Red
DA_Astronaut.uasset      → Division: Character.Division.Astronaut, BodyColor: White
...
```

Adding character #87 is creating one new asset in the editor. Zero C++ required.

### 3. Core layer owns the data shapes

`FDivisionParams` and `FCapabilitySet` live in `Core/CharacterParams.h` as pure `USTRUCT`s.
`UCharacterDataAsset` (actor layer) holds these structs as properties. This follows the
existing Core/Actor split and keeps the data shapes headlessly testable.

```
Core/CharacterParams.h
  FDivisionParams        — division tag + body color, AreValid(), factory methods
  FCapabilitySet         — TArray<FGameplayTag>, AreValid() (no dupes), HasCapability()

Character/CharacterDataAsset.h
  UCharacterDataAsset    — UPrimaryDataAsset holding FDivisionParams + FCapabilitySet

Character/OpenCityCharacter.h
  AOpenCityCharacter     — IAbilitySystemInterface, UAbilitySystemComponent,
                           TObjectPtr<UCharacterDataAsset> CharacterData
```

### 4. GAS initialization

`UAbilitySystemGlobals::InitGlobalData()` must be called once before any
`UAbilitySystemComponent` registers. This lives in `UOpenCityGameInstance::Init()`.

Without this call, the first actor spawn with an ASC crashes with `FileNotFound: GameplayAbilities`.

## How to add a new character

1. **Content Browser** → right-click → Miscellaneous → Data Asset → `CharacterDataAsset`
2. Name it `DA_<CharacterName>`
3. Set `Division.DivisionTag` (e.g. `Character.Division.Police`)
4. Set `Division.BodyColor`
5. Add capability tags to `Capabilities` (e.g. `Capability.Arrest`, `Capability.Strength`)
6. Assign the asset to a character actor via the `CharacterData` property in the Details panel

## What is deferred

| System | Where it lands |
|--------|---------------|
| `UGameplayAbility` subclasses (Arrest, Hose, etc.) | Per-division ability sprint |
| `UAttributeSet` (health, stamina, oxygen) | Incident system sprint |
| GAS ability granting from data asset | After first ability is implemented |
| Division switching at runtime | Character swap system sprint |
| Criminal/NPC reactions via GameplayTags | AI sprint |

## Native gameplay tags

Defined in `Core/CharacterParams.cpp` via `UE_DEFINE_GAMEPLAY_TAG`:

| Tag | Meaning |
|-----|---------|
| `Character.Division.Police` | Metro Division — police role |
| `Character.Division.Firefighter` | Rescue Division — firefighter role |

Additional division and capability tags will be added here as each division is implemented.

## Testing

| Tier | What is tested |
|------|---------------|
| 1 | `FDivisionParams::MakePolice/Firefighter` valid, distinct, correct tags and colors |
| 1 | `FCapabilitySet` rejects empty sets and duplicates, `HasCapability` works |
| 2 | Spawned character with police data asset reports correct `GetDivisionTag()` |
| 2 | Spawned character with firefighter data asset reports correct `GetDivisionTag()` |
| 2 | Spawned character with no data asset returns invalid tag (no crash) |

## Consequences

- New division abilities must be implemented as `UGameplayAbility` subclasses, not custom code.
- Capability gating must query `FCapabilitySet::HasCapability()` — never hardcode division checks.
- `Core/CharacterParams.h` must not import GAS actor types — only `FGameplayTag` (a USTRUCT).
- The `GameplayAbilities` plugin must remain enabled in `OpenCity.uproject`.
