#!/usr/bin/env bash
set -euo pipefail

REPO_ROOT="/apps/git/unreal-OpenCity"
UE_EDITOR="/apps/git/UnrealEngine/Engine/Binaries/Linux/UnrealEditor"
UPROJECT="$REPO_ROOT/OpenCity.uproject"
SCRIPT="$REPO_ROOT/Tools/import_base_character_unreal.py"
LOG_DIR="$REPO_ROOT/Saved/Logs/agent/base_character_import"
SUCCESS_MARKER="$LOG_DIR/import-success.json"

mkdir -p "$LOG_DIR"
rm -f "$SUCCESS_MARKER"

set +e
"$UE_EDITOR" "$UPROJECT" \
  -ExecutePythonScript="$SCRIPT" \
  -Unattended \
  -NoSplash \
  -log \
  -LogFile="$LOG_DIR/import-base-character.log"
STATUS=$?
set -e

if [ ! -f "$SUCCESS_MARKER" ]; then
  echo "HARD BLOCK: Unreal import did not produce success marker (exit=$STATUS)"
  exit 1
fi