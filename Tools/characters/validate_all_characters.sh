#!/usr/bin/env bash
set -euo pipefail

ROOT="/apps/git/unreal-OpenCity"
SCRIPT="$ROOT/Tools/characters/validate_character_blend.py"

BLENDER_CMD="${BLENDER_CMD:-blender}"

echo "Running base character validation..."
"$BLENDER_CMD" --background "$ROOT/Content/Props/BaseCharacter.blend" --python "$SCRIPT" -- --role base

echo "Running firefighter validation..."
"$BLENDER_CMD" --background "$ROOT/Content/Props/Firefighter.blend" --python "$SCRIPT" -- --role firefighter

echo "Running police validation..."
"$BLENDER_CMD" --background "$ROOT/Content/Props/Police.blend" --python "$SCRIPT" -- --role police

echo "All character blend validations passed."
