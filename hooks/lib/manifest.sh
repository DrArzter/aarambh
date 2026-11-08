# Record every file setup.sh creates or changes, so uninstall.sh can undo it.
# Sourced.

MANIFEST_DIR="$ROOT/etc/aarambh"
MANIFEST="$MANIFEST_DIR/manifest"
BACKUP_SUFFIX=".aarambh-backup"

manifest_add() {
    if [ "$DRY_RUN" = 1 ]; then
        return 0
    fi

    mkdir -p "$MANIFEST_DIR"
    touch "$MANIFEST"

    if ! grep -qxF "$1 $2" "$MANIFEST"; then
        echo "$1 $2" >> "$MANIFEST"
    fi
}

# install_file <mode> <source> <target>
install_file() {
    run install -Dm"$1" "$2" "$3"
    manifest_add file "$3"
}

# Keep the original of a system file before Aarambh edits it the first time.
backup_once() {
    local file=$1

    if [ ! -f "$file" ] || [ -f "$file$BACKUP_SUFFIX" ]; then
        return 0
    fi

    run cp -p "$file" "$file$BACKUP_SUFFIX"
    manifest_add backup "$file"
}
