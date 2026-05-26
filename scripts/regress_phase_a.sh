#!/usr/bin/env bash
cd "$(dirname "$0")/../build" || exit 1
for c in PRESENT SIMON LBlock Rectangle TWINE; do
  echo "=== $c ==="
  case "$c" in
    SIMON)        timeout 180 ./EasyBC -div "$c" 1 17 L63R63 2>&1 | tail -6 ;;
    LBlock|TWINE) timeout 180 ./EasyBC -div "$c" 1 16 63     2>&1 | tail -6 ;;
    *)            timeout 180 ./EasyBC -div "$c" 1 9 60      2>&1 | tail -6 ;;
  esac
done
