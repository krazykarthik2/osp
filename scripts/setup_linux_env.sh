#!/usr/bin/env bash
set -euo pipefail

# Quick setup for fresh/live Linux sessions.
# Supports Ubuntu/Debian apt based systems.

if ! command -v apt-get >/dev/null 2>&1; then
  echo "This setup helper currently supports apt-based Linux distributions." >&2
  echo "Install manually: make gcc ld nasm qemu-system-x86 grub-pc-bin xorriso" >&2
  exit 1
fi

SUDO=""
if [ "${EUID}" -ne 0 ]; then
  SUDO="sudo"
fi

$SUDO apt-get update
$SUDO apt-get install -y \
  build-essential \
  gcc \
  binutils \
  make \
  nasm \
  qemu-system-x86 \
  grub-pc-bin \
  grub-common \
  xorriso \
  mtools

echo "Environment setup complete."
echo "Next steps:"
echo "  make clean && make all"
echo "  make test"
