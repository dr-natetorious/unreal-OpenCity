# AGENT.md

OpenCity character production guide for agent-driven edits.

## Mission
Build scalable, data-driven character content for 150+ playable/NPC variants with a shared animation pipeline.

## Character Standard
- One shared humanoid skeleton for all standard Lego characters.
- Required bone set (17):
  - root
  - pelvis, spine, head
  - upperarm_l, forearm_l, hand_l
  - upperarm_r, forearm_r, hand_r
  - prop_r
  - thigh_l, calf_l, foot_l
  - thigh_r, calf_r, foot_r
- Character root armature at world origin unless explicitly requested otherwise.

## Binding Standard
- Do not use mesh-to-bone parenting as the final rig method.
- Final mesh binding must be:
  - Armature modifier on each mesh part
  - Vertex group assignment to target bone (rigid 1.0 weights for Lego parts)
- Preserve world transforms when converting older files.

## Animation Standard
- Maintain a shared core action set for all compatible characters.
- Transition rule for current prototype files:
  - Firefighter locomotion uses AN_FF_Walk_Basic
  - Police locomotion uses AN_PO_Walk_Basic
- Target rule for 150+ character pipeline:
  - Introduce AN_Base_Walk_Basic and migrate both roles to the base locomotion action.
- Division-specific actions are additive to shared set:
  - Firefighter: AN_FF_Chop_Door
  - Police: AN_PO_Arrest
  - EMT: AN_EMT_Revive (planned)

## GAS/Gameplay Standard
- Character differentiation is data-driven:
  - Visual style (materials/parts)
  - GAS tags
  - Ability set assignment
- Do not create a custom rig or class for every character variant.
- Prefer one base character implementation plus division/ability data assets.

## Naming and Export
- Character FBX output folder: Content/Props/
- Current prototype exports:
  - CHR_Firefighter_Lego.fbx
  - CHR_Police_Lego.fbx
- Shared base character export:
  - Content/Props/Characters/SK_CityCharacter.fbx
- Export settings:
  - axis_forward=-Y
  - axis_up=Z
  - apply_unit_scale=True
  - add_leaf_bones=False
  - bake_anim=True
  - use_armature_deform_only=False for shared character exports so helper bones are preserved

## Contract Gates
- BaseCharacter source validation is a hard gate.
- BaseCharacter FBX export validation is a hard gate.
- Unreal import validation is a hard gate.
- If helper bones, required material slots, or required imported assets are missing, stop immediately.

## Agent Workflow
1. Validate armature bone names and action list.
2. Validate mesh binding method (modifiers + vertex groups).
3. Fix broken actions or assign default walk action for preview.
4. Keep police at (0,0,0) when user requests save-ready state.
5. Run character validator tooling before export.
6. Re-export FBX only after validation passes.

## Logging Policy
- Write any agent-created diagnostic logs, command captures, or temporary investigation output inside this repo.
- Default location for future logs: `Saved/Logs/agent/` unless the user asks for a different in-repo path.
- Avoid writing ad hoc logs to home-directory, editor-session, or other external filesystem locations when a project-local path is available.

## Out of Scope
- Do not introduce new skeleton families without explicit approval.
- Do not hand-author one-off pipelines that cannot scale to 150 characters.
