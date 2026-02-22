#!/usr/bin/env bash
set -euo pipefail

# Lightweight helper to build and run the OSP ISO in QEMU.
# Usage: ./sim.sh [--setup] [--test] [--headless]
#   --setup  : run ./scripts/setup_linux_env.sh (apt-based systems)
#   --test   : run `make test` (automated headless test) instead of interactive run
#   --headless : run QEMU in terminal mode (-nographic)

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

DO_SETUP=false
DO_TEST=false
HEADLESS=false
TIMEOUT=0

while [ "$#" -gt 0 ]; do
  case "$1" in
    --setup) DO_SETUP=true; shift ;;
    --test) DO_TEST=true; shift ;;
    --headless) HEADLESS=true; shift ;;
    --timeout) TIMEOUT="$2"; shift 2 ;;
    -h|--help) echo "Usage: $0 [--setup] [--test] [--headless] [--timeout <seconds>]"; exit 0 ;;
    *) echo "Unknown arg: $1" >&2; echo "Usage: $0 [--setup] [--test] [--headless] [--timeout <seconds>]"; exit 2 ;;
  esac
done

if $DO_SETUP; then
  if [ -x ./scripts/setup_linux_env.sh ]; then
    echo "Running environment setup (requires sudo for apt)..."
    ./scripts/setup_linux_env.sh
  else
    echo "No setup script found at ./scripts/setup_linux_env.sh" >&2
    exit 1
  fi
fi

MISSING=()
for cmd in make qemu-system-x86_64 grub-mkrescue; do
  if ! command -v "$cmd" >/dev/null 2>&1; then
    MISSING+=("$cmd")
  fi
done

if [ ${#MISSING[@]} -ne 0 ]; then
  echo "Missing required commands: ${MISSING[*]}" >&2
  if printf '%s\n' "${MISSING[@]}" | grep -qq "qemu-system-x86_64" 2>/dev/null || printf '%s\n' "${MISSING[@]}" | grep -q "qemu-system-x86_64"; then
    echo "qemu-system-x86_64 is required to run the ISO interactively." >&2
    echo "On Debian/Ubuntu install with: sudo apt-get install qemu-system-x86" >&2
  fi
  if printf '%s\n' "${MISSING[@]}" | grep -q "grub-mkrescue"; then
    echo "grub-mkrescue is required to build the ISO." >&2
    echo "On Debian/Ubuntu install with: sudo apt-get install grub-pc-bin xorriso" >&2
  fi
  if printf '%s\n' "${MISSING[@]}" | grep -q "make"; then
    echo "'make' is required to build; install with: sudo apt-get install make build-essential" >&2
  fi
  if $DO_TEST; then
    echo "Continuing because --test was requested (tests may still fail)."
  else
    echo "Aborting due to missing tooling." >&2
    exit 2
  fi
fi

echo "Building ISO (make all)..."
make clean && make all

if $DO_TEST; then
  echo "Running non-interactive tests (make test)..."
  make test
  TEST_EXIT=$?
  if [ $TEST_EXIT -eq 0 ]; then
    echo "Tests passed. See build/qemu.log for details.";
  else
    echo "Tests failed (exit $TEST_EXIT). See build/qemu.log" >&2;
  fi
  exit $TEST_EXIT
fi

if $HEADLESS; then
  echo "Launching QEMU in terminal mode (-nographic)."
  CMD=(qemu-system-x86_64 \
    -cdrom build/osp.iso \
    -boot d \
    -nographic \
    -no-reboot)
  
  if [ "$TIMEOUT" -gt 0 ]; then
     echo "Running with timeout: $TIMEOUT seconds"
     timeout "${TIMEOUT}s" "${CMD[@]}" || true
  else
     "${CMD[@]}"
  fi
else
  echo "Launching QEMU in a separate window (GTK display)."
  CMD=(qemu-system-x86_64 \
    -cdrom build/osp.iso \
    -boot d \
    -display gtk \
    -serial stdio \
    -no-reboot)
  
  if [ "$TIMEOUT" -gt 0 ]; then
     echo "Running with timeout: $TIMEOUT seconds"
     timeout "${TIMEOUT}s" "${CMD[@]}" || true
  else
     "${CMD[@]}"
  fi
fi
