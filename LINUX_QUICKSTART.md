# Linux Quickstart (Live USB Friendly)

This repo now supports a Linux-first workflow. You do **not** need Windows batch scripts.

## 1) Clone
```bash
git clone <your-repo-url> osp
cd osp
```

## 2) Install dependencies (fresh machine / live USB)
```bash
./scripts/setup_linux_env.sh
```

This installs:
- gcc/binutils/make
- nasm
- qemu-system-x86
- grub-pc-bin
- xorriso

## 3) Build bootable x86_64 ISO
```bash
make clean && make all
```

Output ISO:
- `build/osp.iso`

## 4) Test boot in QEMU
```bash
make test
```

## 5) Run interactively
```bash
make run
```

You can type shell commands like:
- `help`
- `echo hello`
- `mmu`
- `cache`
- `reboot`

---

## Live USB workflow every reboot
Because live USB sessions are ephemeral, repeat only:
```bash
git clone <your-repo-url> osp
cd osp
./scripts/setup_linux_env.sh
make clean && make all
make test
```

Then commit/push normally.
