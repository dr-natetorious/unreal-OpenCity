#!/usr/bin/env bash
# Run the OpenCity automated test suite headlessly.
# Tier 1 (no PIE) and Tier 2 (PIE with NullRHI) run here.
# Exit 0 = all tests passed or project not yet built (first run).
# Exit 1 = one or more tests failed.
set -euo pipefail

REPO_ROOT="$(cd "$(dirname "$0")/.." && pwd)"
UE_EDITOR="/apps/git/UnrealEngine/Engine/Binaries/Linux/UnrealEditor"
UPROJECT="$REPO_ROOT/OpenCity.uproject"
LOG_DIR="/tmp/opencity-tests"
REPORT_DIR="$LOG_DIR/results"

mkdir -p "$LOG_DIR" "$REPORT_DIR"

# Skip gracefully if the project module has not been built yet.
BUILT_MODULE="$REPO_ROOT/Binaries/Linux/libUnrealEditor-OpenCity.so"
if [ ! -f "$BUILT_MODULE" ]; then
    echo "[run_tests] Project not yet built — skipping automated tests."
    echo "  Build first with:"
    echo "    /apps/git/UnrealEngine/Engine/Build/BatchFiles/Linux/Build.sh \\"
    echo "      OpenCityEditor Linux Development $UPROJECT -waitmutex"
    exit 0
fi

echo "[run_tests] Starting Tier 1 (headless) test run..."
"$UE_EDITOR" "$UPROJECT" \
    -ExecCmds="Automation RunTests OpenCity.Core;Quit" \
    -Unattended \
    -NullRHI \
    -NoSplash \
    -log \
    -LogFile="$LOG_DIR/test-run.log" \
    -ReportOutputPath="$REPORT_DIR"

REPORT="$REPORT_DIR/index.json"
if [ ! -f "$REPORT" ]; then
    echo "[run_tests] ERROR: No report generated at $REPORT"
    echo "  See $LOG_DIR/test-run.log for details."
    exit 1
fi

# Parse results with Python (available on all supported platforms)
python3 - <<EOF
import json, sys

with open("$REPORT", encoding="utf-8-sig") as f:
    data = json.load(f)

tests   = data.get("tests", [])
passed  = [t for t in tests if t.get("state") == "Success"]
failed  = [t for t in tests if t.get("state") != "Success"]

print(f"[run_tests] {len(passed)} passed, {len(failed)} failed  ({len(tests)} total)")

for t in failed:
    print(f"  FAIL: {t['testDisplayName']}")
    for entry in t.get("entries", []):
        evt = entry.get("event", {})
        if evt.get("type") in ("Error", "Warning"):
            print(f"       {evt.get('message', '')}")

if failed:
    print(f"\n[run_tests] {len(failed)} test(s) failed. See $LOG_DIR/test-run.log")
    sys.exit(1)

print("[run_tests] All tests passed.")
EOF
