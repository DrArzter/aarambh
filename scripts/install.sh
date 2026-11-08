#!/bin/bash
#
# Build Aarambh, copy it to the EFI system partition, register a firmware boot
# entry and connect the running OS through hooks/setup.sh.
#
# Usage: scripts/install.sh [--dry-run] [--reset-config] [setup.sh options]

set -euo pipefail

REPO="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BOOT_LABEL="Aarambh"
EFI_LOADER='\EFI\aarambh\aarambh.efi'
RESET_CONFIG=0
SETUP_ARGS=()

LEGACY_FILES=(
    light.ttf regular.ttf bold.ttf aarambh-hook.sh
    ubuntu-logo.png fedora-logo.png settings-logo.png restart-logo.png power-logo.png
    ubuntu.jpg fedora.jpg settings.jpg restart.jpg power.jpg
)

# shellcheck source=../hooks/lib/common.sh
. "$REPO/hooks/lib/common.sh"
# shellcheck source=../hooks/lib/detect.sh
. "$REPO/hooks/lib/detect.sh"

parse_arguments() {
    while [ "$#" -gt 0 ]; do
        case "$1" in
            --dry-run)
                DRY_RUN=1
                SETUP_ARGS+=("$1")
                ;;
            --reset-config) RESET_CONFIG=1 ;;
            *) SETUP_ARGS+=("$1") ;;
        esac
        shift
    done
}

sudo_run() {
    run sudo "$@"
}

check_uefi() {
    if [ ! -d /sys/firmware/efi ]; then
        die "this system was booted in legacy BIOS mode; Aarambh needs UEFI"
    fi
}

find_esp() {
    detect_esp || die "no EFI system partition mounted at /boot/efi, /efi or /boot"
    TARGET="$ESP_ROOT/EFI/aarambh"
    log "EFI partition: $ESP_ROOT"
}

build() {
    make -C "$REPO"

    if [ ! -d "$REPO/assets/fonts" ]; then
        "$REPO/scripts/make-assets.sh"
    fi
}

remove_legacy_files() {
    local name

    for name in "${LEGACY_FILES[@]}"; do
        if [ -e "$TARGET/$name" ]; then
            sudo_run rm -f "$TARGET/$name"
        fi
    done
}

copy_config() {
    if [ "$RESET_CONFIG" = 1 ] || [ ! -f "$TARGET/aarambh.conf" ]; then
        sudo_run cp "$REPO/config/aarambh.conf" "$TARGET/aarambh.conf"
    else
        log "keeping the existing $TARGET/aarambh.conf (use --reset-config to replace it)"
    fi
}

copy_files() {
    local folder

    sudo_run mkdir -p "$TARGET"
    sudo_run cp "$REPO/build/aarambh.efi" "$TARGET/aarambh.efi"
    sudo_run cp "$REPO/assets/distros.conf" "$TARGET/distros.conf"

    for folder in fonts logos backgrounds plymouth hooks; do
        sudo_run rm -rf "${TARGET:?}/$folder"
    done

    sudo_run cp -r "$REPO/assets/fonts" "$REPO/assets/logos" "$REPO/assets/backgrounds" "$TARGET/"
    sudo_run mkdir -p "$TARGET/plymouth"

    for folder in "$REPO"/assets/plymouth/*/; do
        local name

        name=$(basename "$folder")
        if [ "$name" != template ]; then
            sudo_run cp -r "$folder" "$TARGET/plymouth/"
            sudo_run cp "$REPO/assets/plymouth/template/aarambh.plymouth" \
                "$REPO/assets/plymouth/template/aarambh.script" "$TARGET/plymouth/$name/"
        fi
    done

    sudo_run cp -r "$REPO/hooks" "$TARGET/hooks"
    copy_config
}

register_boot_entry() {
    local source
    local disk
    local partition

    if ! has_command efibootmgr; then
        die "efibootmgr is not installed"
    fi

    if sudo efibootmgr | grep -q " $BOOT_LABEL"; then
        return 0
    fi

    source=$(findmnt -no SOURCE "$ESP_ROOT")
    disk=/dev/$(lsblk -no PKNAME "$source")
    partition=$(cat "/sys/class/block/$(basename "$source")/partition")

    sudo_run efibootmgr -q -c -d "$disk" -p "$partition" -L "$BOOT_LABEL" -l "$EFI_LOADER"
}

parse_arguments "$@"
check_uefi
find_esp
build

if [ "$DRY_RUN" = 1 ]; then
    log "dry run: nothing will be changed"
fi

remove_legacy_files
copy_files
register_boot_entry

if [ "$DRY_RUN" = 1 ]; then
    sudo bash "$REPO/hooks/setup.sh" "${SETUP_ARGS[@]}" || true
    exit 0
fi

sudo bash "$TARGET/hooks/setup.sh" "${SETUP_ARGS[@]}"

log "installed to $TARGET"
sudo efibootmgr | grep -E "BootOrder| $BOOT_LABEL" || true