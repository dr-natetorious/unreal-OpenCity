# The Agency — Game Design Document

---

## The Divisions

The game is built around **The Agency** — a city-wide organization with divisions covering every heroic role. The player earns credentials across divisions, each unlocking new abilities, vehicles, and areas of the world.

Each character in the game is a data asset with a set of capability tags. There are no unique ability sets per character — just ~20 capability flags combined differently across 150+ characters. A cop and a firefighter might both have `Capability.Strength` but only the cop has `Capability.Arrest`.

---

### 🚔 Metro Division — Police

**What it does:** Vehicle pursuits, arresting criminals, traffic enforcement, securing incident perimeters.

**Key abilities:** Siren (speed boost + world reacts), Pursuit Mode (ram without damage), Arrest, Speed Trap, Roadblock, Radio Call (summon AI backup), Wanted Scan.

**Why Eli loves it:** This is his core fantasy. Being the cool cop watching for speeders, giving chase, catching the bad guy. The siren makes the whole world react to him — civilians scatter, criminals flee. The arrest moment is the payoff. Hot pursuit with escalating wanted levels is the heartbeat of this division.

---

### 🚒 Rescue Division — Firefighter

**What it does:** Extinguishing fires, breaking into locked structures, rooftop access, large-area suppression from vehicle-mounted cannon.

**Key abilities:** Hose, Axe (breaks doors + silver objects), Rescue Carry, Ladder Deploy, Foam Cannon.

**Why Eli loves it:** Big truck, big tools, hero entrance. The axe breaking things is inherently satisfying. Rescue Carry completing a save gives a clear mission-complete moment a 7-year-old feels.

---

### 🚑 EMT Division

**What it does:** Healing and reviving civilians, transporting injured to hospital, scanning for casualties nearby.

**Key abilities:** Medkit, Defibrillator (dramatic revive), Stretcher, Triage Scan, Adrenaline (self speed burst).

**Why Eli loves it:** The helper hero. Pairs naturally with firefighter on multi-division incidents. Defibrillator revive is a cinematic moment.

---

### ✈️ Aerospace Division — Pilot

**What it does:** Flight traversal, aerial surveillance, supply drops, coordinating ground divisions from above.

**Key abilities:** Flight (jetpack/wingsuit), Air Scan (full map view), Payload Drop, Eject, Beacon (mark landing zones).

**Why Eli loves it:** Flying around the city freely is a core draw. Air Scan rewards exploration — he can survey new islands from above before landing. Eject from a vehicle is a pure action movie moment.

---

### 🚀 Space Division — Astronaut

**What it does:** Surviving hostile environments (underwater, toxic zones), accessing the space island and underground areas, exotic physics abilities.

**Key abilities:** Low Gravity (moon walking, high jumps), Oxygen Pack (breathe underwater/toxic), Grapple, Satellite Hack (reveal hidden map objects), Zero-G Throw (launch objects huge distances).

**Why Eli loves it:** The exotic division. Unlocks areas no other division can reach. Low gravity movement feels completely different from everything else. The space island is the big late-game discovery.

---

### 🚂 Transit Division — Engineer

**What it does:** Repairing broken infrastructure, building outposts and bridges, operating heavy machinery, activating the train network.

**Key abilities:** Repair, Build (construct outposts/ramps), Weld, Crane Control, Track Switch (redirect trains).

**Why Eli loves it:** Track Switch is the direct payoff for his train obsession — he can physically redirect where a train goes. Build ability ties directly to unlocking new areas of the world. The engineer is the division that literally expands the map.

---

### 🦸 Special Ops — Superhero

**What it does:** Moving massive objects, accessing strength pads, breaking cracked floors, slow-fall traversal, seeing hidden objects through walls.

**Key abilities:** Super Strength, Shield, Ground Pound, Glide, Detect.

**Why Eli loves it:** The power fantasy layer on top of everything else. Strength pads block progress until this division is available — but never in a way that halts fun, only opens new things.

---

## The World

### Overarching Concept

**The Agency was built by a previous generation of heroes who worked together and kept everything running. Then they disappeared, the Agency fell apart, and the city started breaking down — incidents got more frequent, islands went dark, nobody was coordinating.**

Eli shows up as a new recruit and rebuilds the Agency from scratch. Every outpost he activates, every division he joins, every island he unlocks — he's restoring something that used to be great.

- **Age 7 read:** You're the new hero fixing the city. Cool vehicles, cool missions.
- **Age 10 read:** Where did the original heroes go? Old photos in stations, dusty retired vehicles, a veteran NPC who hints at what happened.
- **Age 13 read:** The Agency didn't fall apart by accident. The escalating incidents weren't random. Something has been quietly dismantling the systems that keep the city safe — and the original heroes knew.

The villain isn't a person. It's **entropy** — things falling apart when nobody's paying attention. Showing up and caring is how you win.

---

### World Structure — Islands and Regions

Players unlock new islands by activating enough outposts on the current one. Islands are visible but greyed out from the start — Eli can always see what's coming.

| Region | Vibe | Key Features |
|---|---|---|
| Metro City | Home base, urban | Downtown, residential, industrial district, police HQ, hospital, train central |
| Coastal Island | Harbor, beaches | Coast guard, lighthouse, ferry terminal, underwater dive base |
| Mountain Region | Wilderness, altitude | Forest ranger, ski patrol, canyon bridge, search & rescue base |
| Airfield District | Open roads, sky | Control tower, stunt hangar, blimp dock, skydiving platform |
| Space Island | Sci-fi, exotic | Rocket launchpad, zero-G facility, satellite array, alien crash site |

Each island has its own road network, incident types, division presence, and a set of super build outposts to activate.

---

### The Super Build System (~65 unlocks)

Each outpost/super build takes ~60 seconds to construct, plays a Lego-style snap-together animation, then something new visibly activates in the world. Not just a collectible — something *does* something.

Examples across regions:

**Metro City:** Police helicopter pad, fire station, train depot, hospital rooftop landing pad, underground subway, drag strip, coast guard dock, donut shop (obviously).

**Coastal:** Lighthouse (activates night navigation), ferry terminal (opens coastal island), underwater dive base (unlocks dive missions), harbor crane (engineer ability required).

**Mountains:** Forest ranger station, canyon suspension bridge (opens new road), mountain rail line (engineer repairs it), avalanche warning system.

**Airfield:** Control tower (unlocks flight missions), stunt plane hangar, blimp dock, skydiving platform.

**Space:** Rocket launchpad, zero-G training ring, deep-space satellite dish, alien crash site investigation post.

---

### The Incident System

Random incidents fire across the city continuously. Each one is *someone's job*. If Eli has that division credential, his radio lights up. If not, AI responders handle it — which is its own entertainment. He can watch a police pursuit he's not involved in.

Completing incidents earns division badges, unlocks vehicles and gadgets, and advances the story layer.

**Cross-division incidents** are the highest tier:

```
Train derailment on the mountain line:
  → EMT tends to injured passengers
  → Engineer repairs the track
  → Cop clears the suspicious vehicle that caused it
  → Unlocks: mountain rail line now active
```

---

### Mission Structure — 3 Objectives → Unlock

Each mission gives 3 objectives, each using a *different* ability or division. Teaches the system, feels designed, never feels like busywork. The unlock at the end is always something visible and tangible in the world.

The objectives feel like incidents that escalated — not "go here, do this" — the radio crackles and steps unfold as he responds.

---

### Fast Travel and the Train Network

- **World Partition** streams islands in/out automatically — no loading screens flying between islands.
- **Fast travel nodes** are the outposts. Build it, unlock fast travel there.
- **The train network is slow fast travel** — board a train, watch the world go by, arrive at destination. The engineer division repairs broken lines, expanding the network. His train obsession directly expands the map.
- **Ferry** connects coastal island. **Aircraft** connects airfield and space island once unlocked.

---

## World Generation — From Deterministic Random to Constrained Procedural

### The Problem with Pure Random

A seeded random city technically produces consistent output but not a *felt* city. Police stations cluster, harbors have no road access, districts have no coherent identity. The car threads between buildings instead of following roads.

### The Target Architecture

```
Seed
  ↓
Island Layout        — district graph, guaranteed landmarks
  ↓
Road Network         — arterials first, grid/organic fill, waterfront roads
  ↓
Building Placement   — PCG rules per district type
  ↓
Landmark Placement   — exact positions within zones, always guaranteed
  ↓
Prop Scatter         — PCG detail layer (streetlights, parked cars, benches)
  ↓
Incident Registry    — valid spawn points per incident type
```

### Layer 1 — District Placement (Macro)

Define districts with adjacency rules before placing anything. The seed decides *where* within constraints, not *whether*.

```cpp
struct FDistrictRules {
    FGameplayTag DistrictType;
    TArray<FGameplayTag> MustBeAdjacentTo;
    TArray<FGameplayTag> CannotBeAdjacentTo;
    float MinCoverage;
    float MaxCoverage;
    int32 MinCount;
    int32 MaxCount;
};
```

Rules example: Industrial must be adjacent to water or edge. Hospital must be adjacent to residential. Police district adjacent to downtown.

### Layer 2 — Road Network (Mid)

Roads before buildings. Everything hangs off the road graph.

1. Seed determines arterial angles and curves
2. Grid or organic fill between arterials (per district character)
3. Dead ends capped with cul-de-sacs or loops
4. Waterfront roads follow coastline spline
5. PCG places buildings along road edges
6. Hard rule: every building must be road-accessible — check before placing, not after

**Near-term retrofit:** Eli's current spacing gaps are already proto-roads. Extract road splines from existing gaps, tag them by type (`Road.Arterial`, `Road.Street`, `Road.Alley`), and PCG places road surface + sidewalks + props along each spline. Existing buildings automatically face roads.

### Layer 3 — Landmark Guarantees

Some things must always exist regardless of seed:

```cpp
struct FLandmarkRequirements {
    // Metro district always has:
    bool bPoliceHQ;           // one, downtown adjacent
    bool bPolicePrecinct;     // two, residential
    bool bDonutShop;          // near precinct
    
    // Every island always has:
    bool bAgencyOutpost;      // the super build anchor
    bool bFastTravelNode;     // accessible from main road
    bool bTrainStation;       // on the rail line
};
```

### Layer 4 — Wave Function Collapse (Future)

For coherent city fabric at the block level — each cell looks at its neighbors and picks a compatible tile. Guarantees consistent streetscapes without hand-authoring every combination. ~200 lines of C++. PCG can approximate this with enough rules in the near term.

### Incident-Aware Generation

City cells are tagged by zone. Incident spawner queries valid locations at runtime:

```cpp
TArray<FVector> ValidLocations = CityGrid->GetCells(
    TAG_Zone_Road,
    TAG_Road_Arterial,
    TAG_Not_RecentIncident
);
```

No incident spawns somewhere it can't resolve correctly.

---

## Unreal Engine 5.5 — Built-in Systems

These are the engine systems most relevant to this game. Recommendation is to adopt them rather than build equivalents from scratch — each one represents months of engineering that would otherwise fall on a solo developer.

---

### Gameplay Ability System (GAS)

**What it is:** Epic's built-in framework for abilities, character stats, and gameplay state.

**Why use it:** The entire division/ability/character system maps directly onto GAS primitives. Building a custom equivalent would produce an inferior version of something that already exists, is network-replicated, and is battle-tested in shipped games.

**Key primitives:**
- `UAbilitySystemComponent` — lives on the character, hub for everything
- `UGameplayAbility` — each division ability (Arrest, Repair, Hose) is a subclass
- `UGameplayEffect` — how abilities change state (Siren active, division equipped, criminal arrested)
- `FGameplayTag` — the language everything speaks (`Character.Division.Cop`, `Incident.Complete`, `NPC.State.Fleeing`)
- `UAttributeSet` — health, stamina, oxygen, division credits

Character swapping, mission objective tracking, criminal reactions, incident resolution — all expressed as tag queries and effect application. No hard references between systems.

**Recommendation:** Set up GAS before the character system gets deep. Retrofitting is painful.

---

### Enhanced Input System

**What it is:** Modern input mapping framework replacing the legacy input system.

**Why use it:** PS5 controller support, context-sensitive input (on foot vs in vehicle vs swimming), input actions as first-class assets. Division switching, vehicle entry/exit, and ability activation all need context-aware input — Enhanced Input handles this cleanly.

---

### World Partition

**What it is:** Automatic world streaming for large open worlds.

**Why use it:** The multi-island structure requires streaming. World Partition makes this automatic — each island just exists in one persistent world, and the engine streams cells in/out as the player moves. No manual level loading, no loading screens flying between islands. HLODs render distant islands as simplified meshes for free performance.

**Recommendation:** Enable before the city gets any larger. Much harder to retrofit onto an existing world.

---

### Procedural Content Generation (PCG)

**What it is:** Graph-based system for rule-driven content placement.

**Why use it:** City dressing (streetlights, parked cars, benches, district props) would be thousands of hours of manual placement. PCG does it in minutes from road splines and district zone tags. Different rule graphs per district — industrial gets dumpsters and forklifts, suburbs get mailboxes and gardens.

Respects the deterministic seed — feed it the city seed and it generates identically every time.

**Immediate win:** Start using PCG for prop scatter now. Biggest visual-to-effort ratio of any system on this list.

---

### Mass Entity + Mass Traffic

**What it is:** ECS (Entity Component System) for high-count simulated objects. Mass Traffic is built on top specifically for vehicles.

**Why use it:** 500 pedestrians and 200 traffic vehicles as regular AActors would destroy performance. As Mass Entity fragments processed in bulk, it's essentially free. The city feels alive without a performance cost.

Mass Traffic handles lane following, intersections, and traffic light stops automatically. Player's Chaos vehicle interacts with traffic naturally.

**Recommendation:** Mass Traffic once road splines are solid. Pedestrian Mass Entity after traffic is working.

---

### Chaos Vehicles

**What it is:** UE5's physics vehicle system.

**Why use it:** Supports cars, boats (via Buoyancy Component), and aircraft. The buoyancy pontoon system makes boat feel completely configurable — jet ski vs cargo ship from pontoon placement alone. Already the right system for the vehicle roster this game needs.

---

### Water Plugin

**What it is:** First-party water system with ocean, river, lake, and buoyancy support.

**Why use it:** Islands need ocean. The underwater cave system, harbor districts, and coast guard incidents all require water bodies. Lumen handles water lighting automatically. Character swimming is built into `ACharacter` (`CharacterMovementComponent`) and activates automatically when the Water plugin is present.

Oxygen attribute (Space division's core ability) only makes sense once water exists — add water before building that ability.

---

### StateTree

**What it is:** Modern replacement for Behavior Trees, for NPC AI logic.

**Why use it:** Criminal AI (patrol → flee → cornered → arrested), civilian reactions (idle → witness incident → panic/cheer), and AI responders all need readable, debuggable state machines. StateTree is cleaner than Behavior Trees for this pattern and integrates with Gameplay Tags natively.

---

### Environmental Query System (EQS)

**What it is:** AI spatial query system — finds best positions in the world at runtime.

**Why use it:** Criminal escape routing. The fleeing driver queries valid escape routes (prefers highways and alleys, avoids dead ends and water), which makes chases feel dynamic rather than scripted. Pair with StateTree for the full criminal AI stack.

---

### MetaSounds

**What it is:** Procedural audio graph system.

**Why use it:** The siren should respond to speed. Engine sounds should layer dynamically. Static audio files can't do this — MetaSounds can. The whole game's audio character (which is significant for a 7-year-old) lives here.

---

### CommonUI

**What it is:** Controller-friendly UI framework handling gamepad focus and navigation.

**Why use it:** Eli plays on a PS5 controller. Every menu — the world map, the character select, the fast travel screen, mission objectives — needs full thumbstick navigation. CommonUI handles focus management automatically. Building this manually is a significant amount of work for zero gameplay value.

---

*Document generated from design session. Next step: road network retrofit and GAS initial setup.*