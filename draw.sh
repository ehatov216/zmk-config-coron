#!/bin/bash
# Regenerate keymap visualization after editing coron.keymap
set -e
mkdir -p keymap-drawer
keymap parse -z config/coron.keymap > /tmp/coron_parsed.yaml
keymap draw /tmp/coron_parsed.yaml -j config/coron.json -o keymap-drawer/coron.svg
echo "✓ keymap-drawer/coron.svg updated"
open keymap-drawer/coron.svg
