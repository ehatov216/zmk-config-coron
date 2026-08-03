#!/bin/sh

set -eu

script_dir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
temporary_dir=$(mktemp -d "${TMPDIR:-/tmp}/coron-encoder-monitor.XXXXXX")
monitor_binary="$temporary_dir/monitor_encoder"

cleanup() {
    rm -f "$monitor_binary"
    rmdir "$temporary_dir" 2>/dev/null || true
}

trap cleanup EXIT HUP INT TERM

clang \
    -std=c11 \
    -Wall \
    -Wextra \
    -Werror \
    -framework ApplicationServices \
    "$script_dir/monitor_encoder.c" \
    -o "$monitor_binary"

"$monitor_binary" "$@"
