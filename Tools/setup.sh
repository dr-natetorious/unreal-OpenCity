#!/usr/bin/env bash
# One-time project setup. Run after cloning.
set -euo pipefail

REPO_ROOT="$(cd "$(dirname "$0")/.." && pwd)"

echo "[setup] Configuring git hooks path..."
git -C "$REPO_ROOT" config core.hooksPath .githooks
chmod +x "$REPO_ROOT/.githooks/pre-commit"
echo "[setup] Git hooks active: $REPO_ROOT/.githooks"

echo "[setup] Done. Build the project next:"
echo "  /apps/git/UnrealEngine/Engine/Build/BatchFiles/Linux/Build.sh \\"
echo "    OpenCityEditor Linux Development $REPO_ROOT/OpenCity.uproject -waitmutex"
