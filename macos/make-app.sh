#!/bin/bash
# Construit Tetris.app : le jeu console de 2008 dans une fenêtre macOS native.
#
# Deux étages : CMake produit le binaire du jeu, SwiftPM produit l'enveloppe
# SwiftTerm, et ce script les assemble en un bundle double-cliquable.
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
APP="$ROOT/macos/Tetris.app"

export DEVELOPER_DIR="${DEVELOPER_DIR:-/Applications/Xcode.app/Contents/Developer}"

echo "==> Jeu (CMake)"
cmake -S "$ROOT" -B "$ROOT/build" -DCMAKE_BUILD_TYPE=Release >/dev/null
cmake --build "$ROOT/build" >/dev/null

echo "==> Enveloppe (SwiftPM)"
swift build -c release --package-path "$ROOT/macos" >/dev/null

echo "==> Assemblage du bundle"
rm -rf "$APP"
mkdir -p "$APP/Contents/MacOS" "$APP/Contents/Resources"

cp "$ROOT/macos/.build/release/TetrisApp" "$APP/Contents/MacOS/Tetris"
cp "$ROOT/build/Tetris"                   "$APP/Contents/Resources/Tetris"
cp "$ROOT/a.xm"                           "$APP/Contents/Resources/"
cp "$ROOT/score.data" "$ROOT/splus.data"  "$APP/Contents/Resources/"

cat > "$APP/Contents/Info.plist" <<PLIST
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
    <key>CFBundleName</key>            <string>Tetris</string>
    <key>CFBundleDisplayName</key>     <string>Tetris-classic</string>
    <key>CFBundleIdentifier</key>      <string>dev.hellow.tetris-classic</string>
    <key>CFBundleExecutable</key>      <string>Tetris</string>
    <key>CFBundlePackageType</key>     <string>APPL</string>
    <key>CFBundleShortVersionString</key> <string>1.4</string>
    <key>CFBundleVersion</key>         <string>1.4</string>
    <key>LSMinimumSystemVersion</key>  <string>13.0</string>
    <key>NSHighResolutionCapable</key> <true/>
</dict>
</plist>
PLIST

codesign --force --deep --sign - "$APP" 2>/dev/null || true

echo "==> $APP"
