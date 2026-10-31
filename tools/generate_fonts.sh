#!/bin/sh
set -eu
# Run after CMake fetched LVGL; install lv_font_conv@1.5.3 separately.
# Override FONT_CONVERTER to point to that executable.
cd "$(dirname "$0")/.."
font='build/simulator/_deps/lvgl-src/scripts/built_in_font/Montserrat-Medium.ttf'
for size in 10 12 14 16 18 24 28 36; do
  # Large splash/numeric font: omit pair spacing tables to fit the preserved app slot.
  kern_option=
  if [ "$size" = 36 ]; then kern_option=--no-kerning; fi
  compression=--no-compress
  if [ "$size" = 12 ] || [ "$size" = 14 ] || [ "$size" = 16 ] || [ "$size" = 18 ] || [ "$size" = 24 ] || [ "$size" = 28 ] || [ "$size" = 36 ]; then compression=; fi
  "${FONT_CONVERTER:-lv_font_conv}" --font "$font" --size "$size" --bpp 4 \
    ${kern_option} --format lvgl --range '0x20-0x7E,0xA0-0xFF,0x152-0x153,0x178,0x1E9E,0x2019,0x20AC' ${compression} \
    --lv-font-name "meshcator_font_$size" -o "src/ui/fonts/meshcator_font_$size.c"
done

# Add the emoji fallback after regenerating the text fonts.
"${PYTHON:-python3}" tools/build_emoji_font.py --link-only
