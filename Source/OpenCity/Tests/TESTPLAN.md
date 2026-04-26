# OpenCity — Test Plan

Run automated tiers first. Fix failures before advancing to the next tier.
Automated tiers (1 & 2) are enforced by the pre-commit hook via `Tools/run_tests.sh`.

---

## Tier 1 — Core Math (automated, no PIE)

```
UnrealEditor OpenCity.uproject -ExecCmds="Automation RunTests OpenCity.Core;Quit" -NullRHI -Unattended
```

| Test | Covers |
|------|--------|
| `OpenCity.Core.CityGrid.OriginIsCell00` | WorldToCell at origin |
| `OpenCity.Core.CityGrid.OneCellSize` | WorldToCell at one cell boundary |
| `OpenCity.Core.CityGrid.NegativeCoords` | WorldToCell with negative input |
| `OpenCity.Core.CityGrid.KeyRoundtrip` | CellKey encode/decode |
| `OpenCity.Core.CityGrid.KeysAreUnique` | CellKey uniqueness in 21×21 grid |
| `OpenCity.Core.CityGrid.MajorRoadWiderThanMinor` | Road width hierarchy |
| `OpenCity.Core.CityGrid.BuildableRangeIsPositive` | Buildable area > 50m wide |
| `OpenCity.Core.BuildingPlacer.SameSeedSameResult` | Deterministic placement |
| `OpenCity.Core.BuildingPlacer.DifferentSeedsDifferentResult` | Seed variation |
| `OpenCity.Core.BuildingPlacer.SpawnAreaIsClear` | No building at player spawn |
| `OpenCity.Core.BuildingPlacer.HeightsInRange` | Heights within min/max params |
| `OpenCity.Core.VehicleParams.DefaultsAreValid` | FCarParams defaults pass AreValid() |
| `OpenCity.Core.VehicleParams.WheelPositions` | 4 wheels, correct F/R and L/R signs |

**All must pass before any commit.**

---

## Tier 2 — Spawn (automated, PIE with NullRHI)

```
UnrealEditor OpenCity.uproject -ExecCmds="Automation RunTests OpenCity.Spawn;Quit" -NullRHI -Unattended
```

| Test | Covers |
|------|--------|
| `OpenCity.Spawn.Character.Exists` | Player is AOpenCityCharacter |
| `OpenCity.Spawn.Character.AboveGround` | Z in [-50cm, 500cm] |
| `OpenCity.Spawn.Character.HasPlayerController` | Controller assigned |

**All must pass before Tier 3 work begins.**

---

## Tier 3 — Movement (automated, PIE)

_Tests to be added as each system is implemented._

| # | Test | Pass |
|---|------|------|
| 3.1 | Press W — character moves forward | ☐ |
| 3.2 | Character does not fall through ground while walking | ☐ |
| 3.3 | Approach a car — interact prompt appears | ☐ |
| 3.4 | Enter car — pawn switches to ACarPawn | ☐ |
| 3.5 | Throttle input — car accelerates forward | ☐ |
| 3.6 | Car does not fall through ground while driving | ☐ |
| 3.7 | Exit car — pawn switches back to AOpenCityCharacter | ☐ |

---

## Tier 4 — Feel & Visuals (manual)

Run after all automated tiers pass.

| # | Test | Pass |
|---|------|------|
| 4.1 | Character movement feels responsive (no input lag) | ☐ |
| 4.2 | Camera follows character smoothly, no jitter | ☐ |
| 4.3 | Approach car — "Press E to enter car" tooltip appears | ☐ |
| 4.4 | Press E — camera switches to car, character disappears | ☐ |
| 4.5 | W/S drives forward/backward, A/D steers | ☐ |
| 4.6 | Wheels visually spin while moving | ☐ |
| 4.7 | Press E in car — character reappears beside car, on foot again | ☐ |
| 4.8 | Buildings are visible and correctly sized | ☐ |
| 4.9 | Play for 60 seconds — no crash or freeze | ☐ |

---

## Failure Protocol

- **Tier 1 fail** → fix immediately, do not commit
- **Tier 2 fail** → fix before any new actor work
- **Tier 3 fail** → fix before adding new gameplay features
- Never skip tiers to chase a higher-tier goal
