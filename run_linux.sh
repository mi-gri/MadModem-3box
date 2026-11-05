#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
if [[ -x "$ROOT/build-linux/MultiModeModem" ]]; then
    exec "$ROOT/build-linux/MultiModeModem" "$@"
fi
if [[ -x "$ROOT/dist/linux/bin/MultiModeModem" ]]; then
    exec "$ROOT/dist/linux/bin/MultiModeModem" "$@"
fi
if [[ -x "$ROOT/dist/linux/MultiModeModem" ]]; then
    exec "$ROOT/dist/linux/MultiModeModem" "$@"
fi
echo "MultiModeModem Linux executable not found. Build first with ./build_all.sh or cmake --build build-linux." >&2
exit 1
