# Maintainer: daddy-daiwik
pkgname=nonte
pkgver=0.1.0
pkgrel=1
pkgdesc="Nonte - Modern Enhanced Terminal Text Editor (based on GNU nano)"
arch=('x86_64' 'aarch64')
url="https://github.com/daddy-daiwik/nonte"
license=('GPL3')
depends=('ncurses' 'file' 'zlib')
makedepends=('autoconf' 'automake' 'make' 'imagemagick')
provides=('nonte')
conflicts=('nonte')

prepare() {
  cd "${srcdir}/.."
  ./autogen.sh || true
}

build() {
  cd "${srcdir}/.."
  ./configure --prefix=/usr --enable-utf8
  make -j$(nproc)
}

package() {
  cd "${srcdir}/.."
  install -Dm755 src/nano "${pkgdir}/usr/bin/nonte"
  install -Dm644 nonte.desktop "${pkgdir}/usr/share/applications/nonte.desktop"
  install -Dm644 icon/icon.png "${pkgdir}/usr/share/icons/hicolor/512x512/apps/nonte.png"
  install -Dm644 icon/icon-256.png "${pkgdir}/usr/share/icons/hicolor/256x256/apps/nonte.png"
  install -Dm644 icon/icon-128.png "${pkgdir}/usr/share/icons/hicolor/128x128/apps/nonte.png"
  install -Dm644 icon/icon-64.png "${pkgdir}/usr/share/icons/hicolor/64x64/apps/nonte.png"
  install -Dm644 icon/icon-48.png "${pkgdir}/usr/share/icons/hicolor/48x48/apps/nonte.png"
  install -Dm644 icon/icon-32.png "${pkgdir}/usr/share/icons/hicolor/32x32/apps/nonte.png"
  install -Dm644 icon/icon-16.png "${pkgdir}/usr/share/icons/hicolor/16x16/apps/nonte.png"
  install -Dm644 icon/icon.png "${pkgdir}/usr/share/pixmaps/nonte.png"
}
