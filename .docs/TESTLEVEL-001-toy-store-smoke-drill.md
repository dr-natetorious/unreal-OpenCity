# Test Level 001: Toy Store Smoke Drill

## Purpose
Prototype a small, replayable mission room that combines Police and Firefighter personas in a Lego-style cooperative puzzle flow suitable for a 7-year-old.

This level should validate:
- Multi-role objective sequencing in one contained space
- Clear visual guidance and low reading load
- Positive, non-scary emergency fantasy
- "3 objectives -> unlock" mission rhythm from GameDesign

## Fantasy and Tone
A bright toy-store back room has a harmless smoke incident from a toppled popcorn machine. A mascot plush is trapped behind a jammed service door. The player swaps between Police and Firefighter to make the room safe and complete the rescue.

Tone rules:
- No horror cues, no screaming, no dark threat framing
- Friendly radio language: "Let's make it safe" and "Great teamwork"
- Rewarding slapstick Lego feedback (snaps, pops, confetti)

## Room Scope (Single Test Cell)
Target dimensions:
- Room footprint: 16m x 12m
- Height: 5m
- Entry safe zone: 3m deep strip at spawn side

Scale convention:
- 1m in DCC = 1m in gameplay design scale
- UE uses cm internally; authored dimensions remain meter-true by conversion

Key landmarks:
- Entry door and briefing marker
- Hazard zone around popcorn machine (center-left)
- Jammed service door (back wall)
- Rescue zone outside service door
- Exit gate with celebration trigger

## Persona Requirements (4 Objective Gates)
Mission is complete when all 4 gates are complete.

1) Police: Secure the scene
- Task: Place 3 barricades/cones on glowing floor sockets
- Validation: All sockets occupied by valid police placeables
- Feedback: Socket lights turn red -> green; "Scene secured" callout

2) Firefighter: Clear the jammed door
- Task: Axe-hit 3 highlighted weak points on service door
- Validation: Weak point hit count reaches 3
- Feedback: Chunk break VFX, door opens with Lego burst animation

3) Police + Firefighter: Open safe traversal path
- Police task: Push rolling toy cart to parking marker
- Firefighter task: Foam 2 hotspot decals to zero heat
- Validation: Cart parked AND hotspotA clear AND hotspotB clear
- Feedback: Path arrows illuminate to rescue target

4) Firefighter (or EMT variant later): Rescue mascot
- Task: Carry mascot actor to rescue zone
- Validation: Mascot overlap with rescue trigger while carried
- Feedback: Mascot cheers, confetti pop, mission complete stinger

## GAS Alignment (ADR-004)
This room should act as the first gameplay-level validation of the GAS character division system in
[.docs/ADR-004-gas-character-division-system.md](.docs/ADR-004-gas-character-division-system.md).

Hard rules for this test:
- Player pawn uses one `UAbilitySystemComponent` on `AOpenCityCharacter`
- Role checks use Gameplay Tags and capability tags only (no class checks)
- Objective state changes are driven by gameplay events and tags
- Character role comes from assigned `UCharacterDataAsset`

Required role tags:
- `Character.Division.Police`
- `Character.Division.Firefighter`

Proposed capability tags for this room:
- `Capability.SecureScene` (place barricades)
- `Capability.BreachDoor` (axe weak points)
- `Capability.ClearHotspot` (foam suppression)
- `Capability.RescueCarry` (carry mascot)

Proposed objective event tags:
- `Event.Objective.SceneSecured`
- `Event.Objective.DoorBreached`
- `Event.Objective.SafePathOpened`
- `Event.Objective.MascotRescued`

Objective-to-GAS mapping:
- Objective 1 requires `Character.Division.Police` + `Capability.SecureScene`
- Objective 2 requires `Character.Division.Firefighter` + `Capability.BreachDoor`
- Objective 3A (cart) requires police role or approved police capability tag
- Objective 3B (foam) requires `Character.Division.Firefighter` + `Capability.ClearHotspot`
- Objective 4 requires firefighter role (or EMT variant later) + `Capability.RescueCarry`

Vertical-slice implementation note:
- First pass can use lightweight placeholder `UGameplayAbility` classes that only validate tags,
  invoke interaction, and emit objective events.
- Keep puzzle logic in world actors and mission state actor; keep role/permission logic in GAS.

## Puzzle Readability for Age 7
Design constraints:
- Max 1 active instruction at a time in HUD
- Every objective has one color identity and one icon
- Never require hidden interactions; all targets glow when active
- Failure is soft only (no fail state), just hints and retries

Hint system:
- 10s idle: subtle waypoint pulse
- 20s idle: voice hint line
- 30s idle: temporary "ghost path" ribbon to next interactable

## Mission Flow (State Machine)
- State 0: Intro + objective 1 unlock
- State 1: Secure scene complete -> objective 2 unlock
- State 2: Door breach complete -> objective 3 unlock
- State 3: Safe path complete -> objective 4 unlock
- State 4: Rescue complete -> victory + reward unlock

Reset behavior:
- Full room reset on replay start
- Optional checkpoint after each completed state for rapid iteration

## Reward for Completion
Test reward (placeholder):
- Unlock "Joint Response Kiosk" prop in hub
- Award 1 Police badge tick + 1 Firefighter badge tick

## Implementation Notes (Vertical Slice)
Required interactive actors:
- 3 BarricadeSocket actors
- 1 BreachableDoor actor (3 weak points)
- 1 PushableToyCart actor + cart parking trigger
- 2 FoamHotspot actors
- 1 CarryableMascot actor + rescue trigger
- 1 ExitGate actor

Required GAS-facing hooks:
- Mission state actor subscribes to objective event tags and advances state machine
- Interactable actors expose required capability tag(s) for interaction
- HUD objective widget reads active objective tag and completion status

Telemetry to capture:
- Time-to-complete per objective
- Number of hint escalations used
- Role-switch count
- Retry interactions per objective
- Failed interaction attempts due to missing capability tag
- Time spent in each division role during mission

## Approval Checklist
Approve when all are true:
- 4 objectives are understandable without reading long text
- Playthrough duration is 4-7 minutes for a first-time player
- No scary framing in visuals, VO, or audio
- Both Police and Firefighter feel required and fun
- Mission reset supports fast repeat testing
- All objective gates are enforceable through Gameplay Tags and abilities only

## Future Extensions
- EMT branch objective variant for gate 4
- Co-op mode: one player Police, one player Firefighter
- Dynamic incident remix with the same room geometry
