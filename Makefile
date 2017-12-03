CC = gcc
CFLAGS = `pkg-config --cflags --libs glib-2.0 webkit2gtk-4.0 libconfig libvlc libcurl libxml-2.0 sdl SDL_image json-c`

all:
	$(CC) -g -o bin/iptvx src/*.c $(CFLAGS) 

install:
	mkdir -p $(DESTDIR)/usr/bin/
	cp bin/iptvx $(DESTDIR)/usr/bin/iptvx
	mkdir -p $(DESTDIR)/etc/iptvx
	cp cfg/iptvx.conf $(DESTDIR)/etc/iptvx/iptvx.conf
	cp cfg/channels.conf $(DESTDIR)/etc/iptvx/channels.conf
	mkdir -p $(DESTDIR)/var/iptvx/data/epg
	mkdir -p $(DESTDIR)/var/iptvx/data/logo
	cp app $(DESTDIR)/var/iptvx/ -R
	chmod 755 $(DESTDIR)/usr/bin/iptvx
	chmod 644 $(DESTDIR)/var/iptvx -R
	chmod 644 $(DESTDIR)/etc/iptvx -R
