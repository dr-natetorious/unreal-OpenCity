#!/usr/bin/env bash
set -euo pipefail

ROOT="/apps/git/unreal-OpenCity"
BLENDER_CMD="${BLENDER_CMD:-blender}"
CONTRACT="$ROOT/Tools/characters/standard_contract.json"
SOURCE_BLEND="$ROOT/Content/Props/BaseCharacter.blend"
EXPORTED_FBX="$ROOT/Content/Props/Characters/SK_CityCharacter.fbx"

echo "[base-character] Validating Blender source contract..."
"$BLENDER_CMD" --background "$SOURCE_BLEND" \
  --python "$ROOT/Tools/characters/validate_character_blend.py" -- --role base --contract "$CONTRACT"

echo "[base-character] Exporting shared base character FBX..."
bash "$ROOT/Tools/characters/export_base_character.sh"

echo "[base-character] Validating exported FBX contract..."
"$BLENDER_CMD" --background --factory-startup \
  --python "$ROOT/Tools/characters/validate_base_character_fbx.py" -- --fbx "$EXPORTED_FBX" --contract "$CONTRACT"

echo "[base-character] Source and export contracts passed."