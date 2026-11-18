#!/usr/bin/env bash
# Generate the icon files of this folder from csImgTracker.svg (Linux, run after changing the SVG):
#   csImgTracker-<size>.png  the window icon (Qt resource icon.qrc), all the platforms
#   csImgTracker.ico         the icon of the Windows executable (csImgTracker.rc)
#
#   ./generate.sh [<Qt 6 folder>]
#
# The SVG is rendered by Qt (QtSvg, the renderer of the application: SVG Tiny, no clip paths), which also writes the
# ICO (32-bit bitmaps, see render_svg.cpp). The Qt folder defaults to the one downloaded by ./csImgTracker_project.py
# (gcc).
set -euo pipefail

here="$(cd "$(dirname "$0")" && pwd)"
root="$(cd "${here}/../.." && pwd)"
qt="${1:-${root}/workspace/libs/csv1-linux-ubuntu22.04-x64-generic-gcc-default-ninjamulticonfig/qt6}"
sizes=(16 24 32 48 64 128 256)

tmp="$(mktemp -d)"
trap 'rm -rf "${tmp}"' EXIT

g++ -std=c++17 -fPIC "${here}/render_svg.cpp" -o "${tmp}/render_svg" \
  -I"${qt}/include" -I"${qt}/include/QtSvg" -I"${qt}/include/QtGui" -I"${qt}/include/QtCore" \
  -L"${qt}/lib" -lQt6Svg -lQt6Gui -lQt6Core -Wl,-rpath,"${qt}/lib"

QT_QPA_PLATFORM=offscreen "${tmp}/render_svg" "${here}/csImgTracker.svg" "${tmp}" \
  --ico "${here}/csImgTracker.ico" "${sizes[@]}"

for s in "${sizes[@]}"; do
  cp "${tmp}/${s}.png" "${here}/csImgTracker-${s}.png"
done

echo "generated: ${sizes[*]} px PNG, csImgTracker.ico"
