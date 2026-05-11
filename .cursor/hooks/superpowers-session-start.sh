#!/usr/bin/env bash
# Run Superpowers SessionStart from the repo-local superpowers checkout.
# CURSOR_PLUGIN_ROOT selects Cursor's additional_context JSON shape in session-start.
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
export CURSOR_PLUGIN_ROOT="${ROOT}/superpowers"
exec bash "${CURSOR_PLUGIN_ROOT}/hooks/session-start"
