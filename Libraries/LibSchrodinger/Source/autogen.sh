#!/bin/sh

if [ $(whoami) = wladimir ]; then
  confflags="--prefix=/home/wladimir --with-cuda --disable-gtk-doc"
elif [ $(whoami) = ds ]; then
  confflags="--enable-gtk-doc --disable-orc"
else
  confflags="--enable-gtk-doc"
fi

autoreconf -i -f &&
  ./configure --enable-maintainer-mode --disable-static $confflags $@
