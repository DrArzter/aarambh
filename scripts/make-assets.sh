#!/bin/bash
#
# Generate every image and font Aarambh needs: logos, wallpapers and the
# per-OS Plymouth themes.
#
# Logos: Ubuntu, Fedora and the Firmware / Restart / Power Off icons are drawn
# here. Any other distribution gets its assets once you put
# assets/custom/<id>-logo.svg (or .png) in place, where <id> is its id in
# assets/distros.conf. A custom file also replaces a built-in logo.
#
# Plymouth folders get only images. aarambh.plymouth and aarambh.script live
# once in assets/plymouth/template/; scripts/install.sh copies them into every
# OS folder on the ESP.
#
# Usage: scripts/make-assets.sh [output-dir]
# Needs: imagemagick, fonts-dejavu-core, librsvg2-bin (only for SVG logos)

set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
OUT="$(mkdir -p "${1:-$ROOT/assets}" && cd "${1:-$ROOT/assets}" && pwd)"
CUSTOM_DIR="$ROOT/assets/custom"
DISTROS="$ROOT/assets/distros.conf"
FONT_DIR=/usr/share/fonts/truetype/dejavu
FONT_BOLD="$FONT_DIR/DejaVuSans-Bold.ttf"
FONT_REGULAR="$FONT_DIR/DejaVuSans.ttf"
BUILTIN_DISTROS=(ubuntu fedora)
ACTIONS=(settings restart power)
TMP="$(mktemp -d)"

trap 'rm -rf "$TMP"' EXIT

draw_builtin_masks() {
    convert -size 256x256 xc:black \
        -fill none -stroke white -strokewidth 22 -draw 'circle 128,128 128,62' \
        -stroke none -fill black \
        -draw 'circle 210,128 210,96' -draw 'circle 87,57 87,25' -draw 'circle 87,199 87,167' \
        -fill white \
        -draw 'circle 210,128 210,106' -draw 'circle 87,57 87,35' -draw 'circle 87,199 87,177' \
        "$TMP/ubuntu.mask.png"

    convert -size 256x256 xc:black \
        -fill white -draw 'circle 128,128 128,48' \
        -fill black -font "$FONT_BOLD" -pointsize 130 -gravity center -annotate +6+6 'f' \
        "$TMP/fedora.mask.png"

    convert -size 256x256 xc:black \
        -fill white -font "$FONT_REGULAR" -pointsize 170 -gravity center -annotate +0+4 '⚙' \
        "$TMP/settings.mask.png"

    convert -size 1024x1024 xc:black \
        -fill none -stroke white -strokewidth 112 -draw 'arc 182,182 842,842 -28,262' \
        -stroke none -fill white \
        -draw 'circle 803.4,357.1 859.4,357.1' \
        -draw 'polygon 605.0,165.7 425.1,71.8 458.0,305.5' \
        -resize 256x256 "$TMP/restart.mask.png"

    convert -size 256x256 xc:black \
        -fill none -stroke white -strokewidth 20 \
        -draw 'arc 63,68 193,198 -50,230' -draw 'line 128,48 128,128' \
        "$TMP/power.mask.png"
}

rasterize_custom_logo() {
    local name=$1
    local source=$2

    if [ "${source##*.}" = svg ]; then
        if ! command -v rsvg-convert >/dev/null; then
            echo "SVG logos need rsvg-convert: sudo apt install librsvg2-bin" >&2
            exit 1
        fi
        rsvg-convert -w 256 -h 256 -a "$source" -o "$TMP/$name.raw.png"
    else
        convert -background none -density 384 "$source" -resize 256x256 "PNG32:$TMP/$name.raw.png"
    fi

    convert -background none "$TMP/$name.raw.png" -gravity center -extent 256x256 \
        "PNG32:$TMP/$name.src.png"

    convert "$TMP/$name.src.png" \
        \( +clone -alpha extract \) \
        \( "$TMP/$name.src.png" -alpha off -colorspace gray -level 25%,95% -negate \) \
        -delete 0 -compose multiply -composite "$TMP/$name.mask.png"
}

custom_logo() {
    local source

    for source in "$CUSTOM_DIR/$1-logo.svg" "$CUSTOM_DIR/$1-logo.png"; do
        if [ -f "$source" ]; then
            echo "$source"
            return 0
        fi
    done

    return 1
}

distro_ids() {
    grep -v '^[[:space:]]*#' "$DISTROS" | grep -v '^[[:space:]]*$' | cut -d'|' -f1
}

distro_field() {
    awk -F'|' -v id="$1" -v n="$2" '$1 == id { print $n; exit }' "$DISTROS"
}

# Scale a #rrggbb color by a percentage.
scale_color() {
    local hex=${1#\#}
    local percent=$2

    printf '#%02x%02x%02x' \
        $(( 0x${hex:0:2} * percent / 100 )) \
        $(( 0x${hex:2:2} * percent / 100 )) \
        $(( 0x${hex:4:2} * percent / 100 ))
}

# Every name that has a logo: built-in masks, actions and custom files.
apply_custom_logos() {
    local name
    local source

    LOGO_NAMES=("${BUILTIN_DISTROS[@]}" "${ACTIONS[@]}")

    for name in $(distro_ids) "${ACTIONS[@]}"; do
        source=$(custom_logo "$name" || true)
        if [ -z "$source" ]; then
            continue
        fi

        rasterize_custom_logo "$name" "$source"
        echo "Using custom logo: $source"

        if [[ ! " ${LOGO_NAMES[*]} " == *" $name "* ]]; then
            LOGO_NAMES+=("$name")
        fi
    done
}

render_distro_wallpaper() {
    local name=$1
    local accent
    local top
    local bottom

    accent=$(distro_field "$name" 3)
    top=$(distro_field "$name" 4)
    bottom=$(distro_field "$name" 5)

    if [ -z "$top" ]; then
        top=$(scale_color "$accent" 32)
    fi
    if [ -z "$bottom" ]; then
        bottom=$(scale_color "$accent" 10)
    fi

    render_wallpaper "$name" "$top" "$bottom" "$accent"
}

render_logo() {
    local name=$1

    convert "$TMP/$name.mask.png" -background white -alpha shape \
        -bordercolor none -border 1 -trim +repage -resize 230x230 \
        -background none -gravity center -extent 256x256 "PNG32:$TMP/$name.logo.png"

    cp "$TMP/$name.logo.png" "$OUT/logos/$name.png"
}

render_wallpaper() {
    local name=$1
    local top=$2
    local bottom=$3
    local glow=$4
    local logo="$TMP/$name.logo.png"

    convert -size 1920x1080 gradient:"$top-$bottom" \
        \( -size 1800x1800 radial-gradient:"$glow-black" -evaluate multiply 0.45 \) \
        -geometry +700-700 -compose screen -composite \
        -compose over -gravity center \
        \( "$logo" -trim +repage -resize 1350x1350 -channel A -evaluate multiply 0.086 +channel \
           -bordercolor none -border 160 -channel RGBA -blur 0x36 \) \
        -geometry +691+22 -composite \
        \( "$logo" -trim +repage -resize 1350x1350 -channel A -evaluate multiply 0.065 +channel \) \
        -geometry +691+22 -composite \
        -quality 92 "$OUT/backgrounds/$name.jpg"
}

render_vignette() {
    convert -size 1x1080 xc:black -alpha set -channel A \
        -fx "j/h<0.16 ? (0.16-j/h)/0.16*0.35 : (j/h>0.55 ? (j/h-0.55)/0.45*0.6 : 0)" \
        +channel -scale '1920x1080!' "$TMP/vignette.png"
}

render_centered_text() {
    local text=$1
    local output=$2
    local font=$3
    local size=$4
    local canvas=$5
    local baseline=$6
    local shadow=$7
    local color=$8
    local width
    local x

    width=$(convert -font "$font" -pointsize "$size" label:"$text" -format %w info:)
    x=$(( (1920 - width) / 2 ))

    convert -size "$canvas" xc:none -font "$font" -pointsize "$size" \
        -fill '#00000059' -annotate +$((x + shadow))+$((baseline + shadow)) "$text" \
        -fill "$color" -annotate +$x+$baseline "$text" "PNG32:$output"
}

render_plymouth_theme() {
    local name=$1
    local title=$2
    local dir="$OUT/plymouth/$name"

    mkdir -p "$dir"

    convert "$OUT/backgrounds/$name.jpg" "$TMP/vignette.png" -composite \
        -fill black -colorize 39% "PNG24:$dir/bg.png"
    cp "$OUT/logos/$name.png" "$dir/logo.png"

    render_centered_text "Starting $title" "$dir/title.png" "$FONT_BOLD" 55.7 1920x140 100 3 white
    render_centered_text "Restarting" "$dir/title-reboot.png" "$FONT_BOLD" 55.7 1920x140 100 3 white
    render_centered_text "Shutting down" "$dir/title-shutdown.png" "$FONT_BOLD" 55.7 1920x140 100 3 white
    render_centered_text "Enter disk password" "$dir/prompt.png" "$FONT_REGULAR" 25 1920x60 40 2 '#e8e4ee'

    convert -size 64x64 xc:none -fill white -draw 'circle 32,32 32,1' "PNG32:$dir/dot.png"
}

copy_fonts() {
    cp "$FONT_DIR/DejaVuSans-ExtraLight.ttf" "$OUT/fonts/light.ttf"
    cp "$FONT_REGULAR" "$OUT/fonts/regular.ttf"
    cp "$FONT_BOLD" "$OUT/fonts/bold.ttf"
    cp "$FONT_DIR/DejaVuSansMono-Bold.ttf" "$OUT/fonts/mono.ttf"
}

mkdir -p "$OUT/logos" "$OUT/backgrounds" "$OUT/fonts" "$OUT/plymouth"

draw_builtin_masks
apply_custom_logos

for name in "${LOGO_NAMES[@]}"; do
    render_logo "$name"
done

render_wallpaper settings '#3a3d4a' '#101116' '#8a90a8'
render_wallpaper restart  '#0f4a46' '#04120f' '#2a9d8f'
render_wallpaper power    '#4a1414' '#120404' '#c0392b'

render_vignette

for name in "${LOGO_NAMES[@]}"; do
    if [ -n "$(distro_field "$name" 1)" ]; then
        render_distro_wallpaper "$name"
        if [ "$name" != windows ]; then
            render_plymouth_theme "$name" "$(distro_field "$name" 2)"
        fi
        echo "Generated: $name"
    fi
done

copy_fonts

echo "Assets written to $OUT"