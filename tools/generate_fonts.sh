#!/bin/sh
set -eu
# Run after CMake fetched LVGL; install lv_font_conv@1.5.3 separately.
# Override FONT_CONVERTER to point to that executable.
cd "$(dirname "$0")/.."
font='build/simulator/_deps/lvgl-src/scripts/built_in_font/Montserrat-Medium.ttf'
for size in 10 12 14 16 18 24 28 36; do
  "${FONT_CONVERTER:-lv_font_conv}" --font "$font" --size "$size" --bpp 4 \
    --format lvgl --range '0x20-0x7E,0xA0-0xFF,0x20AC' --no-compress \
    --lv-font-name "meshcator_font_$size" -o "src/ui/fonts/meshcator_font_$size.c"
done

# Add the project-owned emoji fallback after regenerating the text fonts.
"${PYTHON:-python3}" tools/build_emoji_font.py
