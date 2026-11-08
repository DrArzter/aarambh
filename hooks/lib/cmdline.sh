# Kernel command line for direct boot on systems without BLS entries. Sourced.
#
# /etc/aarambh/cmdline, when present, wins. Otherwise the running kernel's
# command line is reused without the loader-specific and recovery-only parts.

RECOVERY_TOKENS="recovery single nomodeset dis_ucode_ldr systemd.unit=rescue.target"

is_recovery_cmdline() {
    case " $1 " in
        *" recovery "*|*" single "*|*" systemd.unit=rescue.target "*) return 0 ;;
    esac
    return 1
}

clean_cmdline() {
    local cmdline=$1
    local recovery=0
    local token
    local result=""
    local skip

    if is_recovery_cmdline "$cmdline"; then
        recovery=1
    fi

    for token in $cmdline; do
        skip=0

        case "$token" in
            BOOT_IMAGE=*|initrd=*) skip=1 ;;
        esac

        if [ "$recovery" = 1 ] && [[ " $RECOVERY_TOKENS " == *" $token "* ]]; then
            skip=1
        fi

        if [ "$skip" = 0 ]; then
            result="$result $token"
        fi
    done

    echo "${result# }"
}

direct_boot_cmdline() {
    local override="$ROOT/etc/aarambh/cmdline"

    if [ -f "$override" ]; then
        tr '\n' ' ' < "$override" | sed 's/[[:space:]]*$//'
        return 0
    fi

    clean_cmdline "$(cat "$ROOT/proc/cmdline")"
}
