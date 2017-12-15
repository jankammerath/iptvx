CC = gcc
CFLAGS = `pkg-config --cflags --libs glib-2.0 webkit2gtk-4.0 libconfig libvlc libcurl libxml-2.0 sdl SDL_image json-c`

# this makes the binary
all:
	mkdir -p bin
	$(CC) -g -std=c99 -o bin/iptvx src/*.c $(CFLAGS) 

# use this for standalone install
install:
	mkdir -p /usr/bin/
	cp bin/iptvx /usr/bin/iptvx
	mkdir -p /etc/iptvx
	cp cfg/iptvx.conf /etc/iptvx/iptvx.conf
	cp cfg/channels.conf /etc/iptvx/channels.conf
	mkdir -p /var/iptvx/data/epg
	mkdir -p /var/iptvx/data/logo
	cp app /var/iptvx/ -R
	chmod 755 /usr/bin/iptvx
	chmod 644 /var/iptvx -R
	chmod 644 /etc/iptvx -R

# this is for debian package build
packinstall:
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
