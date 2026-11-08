# Make GRUB silent so it passes through without a visible menu. Sourced.

GRUB_DEFAULTS="$ROOT/etc/default/grub"

set_grub_option() {
    local name=$1
    local value=$2

    if grep -q "^$name=" "$GRUB_DEFAULTS"; then
        run sed -i "s|^$name=.*|$name=$value|" "$GRUB_DEFAULTS"
    elif [ "$DRY_RUN" = 1 ]; then
        echo "  would add: $name=$value to $GRUB_DEFAULTS"
    else
        echo "$name=$value" >> "$GRUB_DEFAULTS"
    fi
}

comment_grub_option() {
    run sed -i "s/^$1=/#$1=/" "$GRUB_DEFAULTS"
}

configure_grub() {
    if [ "$BOOTLOADER" != grub ]; then
        return 0
    fi

    backup_once "$GRUB_DEFAULTS"
    set_grub_option GRUB_TIMEOUT 0

    if [ "$BLS" = yes ]; then
        return 0
    fi

    comment_grub_option GRUB_THEME
    comment_grub_option GRUB_GFXMODE
    comment_grub_option GRUB_SAVEDEFAULT

    set_grub_option GRUB_TIMEOUT_STYLE hidden
    set_grub_option GRUB_TERMINAL_OUTPUT console
    set_grub_option GRUB_GFXPAYLOAD_LINUX keep
    set_grub_option GRUB_DISABLE_OS_PROBER true
    set_grub_option GRUB_RECORDFAIL_TIMEOUT 0
}

regenerate_grub() {
    if [ "$BOOTLOADER" != grub ]; then
        return 0
    fi

    if [ "$GRUB_MKCONFIG" = update-grub ]; then
        run update-grub
    else
        run "$GRUB_MKCONFIG" -o "$GRUB_CFG"
    fi
}
