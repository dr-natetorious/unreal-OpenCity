#!/usr/bin/env bash
set -euo pipefail

REPO_ROOT="/apps/git/unreal-OpenCity"
UE_EDITOR="/apps/git/UnrealEngine/Engine/Binaries/Linux/UnrealEditor"
UPROJECT="$REPO_ROOT/OpenCity.uproject"
LOG_DIR="$REPO_ROOT/Saved/Logs/agent/base_character_import"
REPORT_DIR="$LOG_DIR/report"

mkdir -p "$LOG_DIR" "$REPORT_DIR"

echo "[base-character] Running Unreal import contract test..."
"$UE_EDITOR" "$UPROJECT" \
  -Game \
  -ExecCmds="Automation RunTests OpenCity.Core.CharacterAssets.ImportContract;Quit" \
  -Unattended \
  -NullRHI \
  -NoSplash \
  -log \
  -LogFile="$LOG_DIR/import-contract.log" \
  -ReportOutputPath="$REPORT_DIR"

REPORT="$REPORT_DIR/index.json"
if [ ! -f "$REPORT" ]; then
  echo "HARD BLOCK: import contract report was not generated"
  exit 1
fi

python3 - <<EOF
import json, sys

with open("$REPORT", encoding="utf-8-sig") as f:
    data = json.load(f)

tests = data.get("tests", [])
target = None
for test in tests:
    if test.get("fullTestPath") == "OpenCity.Core.CharacterAssets.ImportContract":
        target = test
        break

if target is None:
    print("HARD BLOCK: import contract test result missing from report")
    sys.exit(1)

if target.get("state") != "Success":
    print("HARD BLOCK: Unreal import contract failed")
    for entry in target.get("entries", []):
        evt = entry.get("event", {})
        message = evt.get("message", "")
        if message:
            print(f"  {message}")
    sys.exit(1)

print("[base-character] Unreal import contract passed.")
EOF