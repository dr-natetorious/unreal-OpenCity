#!/usr/bin/env bash
set -euo pipefail

ROOT="/apps/git/unreal-OpenCity"
BLENDER_CMD="${BLENDER_CMD:-blender}"

"$BLENDER_CMD" --background "$ROOT/Content/Props/BaseCharacter.blend" \
  --python "$ROOT/Tools/characters/export_base_character_fbx.py"