#!/usr/bin/env bash
set -e

PREFIX="${PREFIX:-/usr/local}"

if [ "$(id -u)" -ne 0 ]; then
    echo "Notice: System-wide installation requires root privileges."
    echo "Running: sudo $0 $@"
    exec sudo "$0" "$@"
fi

echo "Installing Nonte to ${PREFIX}..."

# Install binary
install -Dm755 src/nano "${PREFIX}/bin/nonte"

# Install desktop entry
install -Dm644 nonte.desktop /usr/share/applications/nonte.desktop

# Install icon resolutions to standard hicolor theme
install -Dm644 icon/icon.png /usr/share/icons/hicolor/512x512/apps/nonte.png
install -Dm644 icon/icon-256.png /usr/share/icons/hicolor/256x256/apps/nonte.png
install -Dm644 icon/icon-128.png /usr/share/icons/hicolor/128x128/apps/nonte.png
install -Dm644 icon/icon-64.png /usr/share/icons/hicolor/64x64/apps/nonte.png
install -Dm644 icon/icon-48.png /usr/share/icons/hicolor/48x48/apps/nonte.png
install -Dm644 icon/icon-32.png /usr/share/icons/hicolor/32x32/apps/nonte.png
install -Dm644 icon/icon-16.png /usr/share/icons/hicolor/16x16/apps/nonte.png
install -Dm644 icon/icon.png /usr/share/pixmaps/nonte.png

# Update desktop and icon caches
if command -v update-desktop-database >/dev/null 2>&1; then
    update-desktop-database /usr/share/applications 2>/dev/null || true
fi
if command -v gtk-update-icon-cache >/dev/null 2>&1; then
    gtk-update-icon-cache -f -t /usr/share/icons/hicolor 2>/dev/null || true
fi

echo "Nonte installed successfully!"
echo "Run 'nonte' in any terminal or launch from your app launcher/dash."
