#!/bin/bash
#
# Boot Aarambh in QEMU from a throwaway ESP image.
#
# The image contains fake loaders (copies of aarambh.efi) for Ubuntu and
# Fedora, which are listed in fixtures/aarambh.conf 
# which Aarambh finds on its own through autodetect.
#
# Usage: tests/qemu/run.sh [--no-kvm] [--theme <name>] [--default <os>]
#
#   --theme    Theme to show, e.g. cinema (default: the one in the fixture)
#   --default  Entry selected at start: ubuntu, fedora a title or an index

set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
TEST_DIR="$ROOT/tests/qemu"
WORK_DIR="$ROOT/build/qemu"
IMAGE="$WORK_DIR/esp.img"
VARS="$WORK_DIR/vars.fd"
OVMF_DIR="${OVMF_DIR:-/usr/share/OVMF}"
ESP_DIR="::/EFI/aarambh"
CONFIG="$WORK_DIR/aarambh.conf"
THEMES="accordion minimal glass-sidebar editorial editorial-dark cinema honeycomb constellation picker-wheel dial hud"

export MTOOLS_SKIP_CHECK=1

ACCEL=(-enable-kvm -cpu host)
THEME=""
DEFAULT=""

usage() {
    sed -n '3,14p' "${BASH_SOURCE[0]}" | sed 's/^# \{0,1\}//'
    exit "${1:-0}"
}

while [ "$#" -gt 0 ]; do
    case "$1" in
        --no-kvm) ACCEL=() ;;
        --theme)
            THEME="${2:-}"
            shift
            ;;
        --default)
            DEFAULT="${2:-}"
            shift
            ;;
        -h|--help) usage 0 ;;
        *) usage 1 ;;
    esac
    shift
done

if [ -n "$THEME" ] && [[ " $THEMES " != *" $THEME "* ]]; then
    echo "Unknown theme '$THEME'. Available: $THEMES" >&2
    exit 1
fi

require() {
    if ! command -v "$1" >/dev/null; then
        echo "Missing '$1'. Install it with: sudo apt install $2" >&2
        exit 1
    fi
}

build() {
    make -C "$ROOT"

    if [ ! -d "$ROOT/assets/fonts" ]; then
        "$ROOT/scripts/make-assets.sh"
    fi
}

create_image() {
    mkdir -p "$WORK_DIR"
    rm -f "$IMAGE"

    dd if=/dev/zero of="$IMAGE" bs=1M count=64 status=none
    mkfs.vfat -F 32 "$IMAGE" >/dev/null

    mmd -i "$IMAGE" ::/EFI ::/EFI/BOOT ::/EFI/ubuntu ::/EFI/fedora "$ESP_DIR"
}

write_config() {
    cp "$TEST_DIR/fixtures/aarambh.conf" "$CONFIG"

    if [ -n "$THEME" ]; then
        sed -i "s/^theme .*/theme $THEME/" "$CONFIG"
    fi

    if [ -n "$DEFAULT" ]; then
        sed -i "s/^default .*/default $DEFAULT/" "$CONFIG"
    fi

    grep -E '^(theme|default) ' "$CONFIG"
}

copy_files() {
    local efi="$ROOT/build/aarambh.efi"
    local folder

    mcopy -i "$IMAGE" "$efi" ::/EFI/BOOT/BOOTX64.EFI
    mcopy -i "$IMAGE" "$efi" ::/EFI/ubuntu/shimx64.efi
    mcopy -i "$IMAGE" "$efi" ::/EFI/fedora/shimx64.efi
    # mcopy -i "$IMAGE" "$efi" ::/EFI/debian/grubx64.efi
    # mcopy -i "$IMAGE" "$efi" ::/EFI/Microsoft/Boot/bootmgfw.efi

    mcopy -i "$IMAGE" "$CONFIG" "$ESP_DIR/"
    mcopy -i "$IMAGE" "$TEST_DIR/fixtures/ubuntu.kernels" "$ESP_DIR/"

    for folder in fonts logos backgrounds; do
        mcopy -s -i "$IMAGE" "$ROOT/assets/$folder" "$ESP_DIR/"
    done
}

run_qemu() {
    cp "$OVMF_DIR/OVMF_VARS_4M.fd" "$VARS"

    qemu-system-x86_64 "${ACCEL[@]}" \
        -machine q35 \
        -m 1024 \
        -drive if=pflash,format=raw,readonly=on,file="$OVMF_DIR/OVMF_CODE_4M.fd" \
        -drive if=pflash,format=raw,file="$VARS" \
        -drive format=raw,file="$IMAGE" \
        -net none \
        ${QEMU_DISPLAY:--display gtk}
}

require qemu-system-x86_64 qemu-system-x86
require mcopy mtools
require mkfs.vfat dosfstools

build
create_image
write_config
copy_files
run_qemu