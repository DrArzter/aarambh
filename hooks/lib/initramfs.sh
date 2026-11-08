# Initramfs tweaks and rebuilds for every supported initramfs generator. Sourced.

INITRAMFS_CONF="$ROOT/etc/initramfs-tools/initramfs.conf"
INITRAMFS_MODULES="$ROOT/etc/initramfs-tools/modules"

has_intel_graphics() {
    [ -d "$ROOT/sys/module/i915" ]
}

# initramfs-tools only: a smaller image fits on the ESP, and loading i915
# early lets Plymouth take over the screen without a mode switch.
tune_initramfs() {
    if [ "$INITRAMFS_TOOL" != initramfs-tools ]; then
        return 1
    fi

    backup_once "$INITRAMFS_CONF"
    run sed -i 's/^MODULES=most/MODULES=dep/' "$INITRAMFS_CONF"

    if has_intel_graphics && ! grep -qx i915 "$INITRAMFS_MODULES"; then
        backup_once "$INITRAMFS_MODULES"
        if [ "$DRY_RUN" = 1 ]; then
            echo "  would add: i915 to $INITRAMFS_MODULES"
        else
            echo i915 >> "$INITRAMFS_MODULES"
        fi
    fi

    return 0
}

rebuild_initramfs() {
    log "rebuilding the initramfs with $INITRAMFS_TOOL, this can take a minute"

    case "$INITRAMFS_TOOL" in
        initramfs-tools) run update-initramfs -u ;;
        mkinitcpio) run mkinitcpio -P ;;
        dracut) run dracut -f ;;
        booster)
            if [ -x "$ROOT/usr/lib/booster/regenerate_images" ]; then
                run "$ROOT/usr/lib/booster/regenerate_images"
            else
                warn "rebuild your booster images yourself"
            fi
            ;;
        *) warn "no known initramfs generator found, rebuild it yourself" ;;
    esac
}
