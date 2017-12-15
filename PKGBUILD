pkgname=iptvx
pkgver=0.5
pkgrel=0
pkgdesc="IPTV player, record and streamer"
arch=('i686' 'x86_64')
url="https://iptvx.org"
license=('APL 2.0')
groups=('gnome')
depends=('glib2' 'webkit2gtk' 'libconfig' 'vlc-git' 'curl' 'libxml2' 'sdl' 'sdl_image' 'json-c')
source=(iptvx-0.5.tar.gz)
md5sums=('SKIP')

package() {
  msg "Installing build ..."
  make install
}