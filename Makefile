CC = gcc
CFLAGS = `pkg-config --cflags --libs glib-2.0 webkit2gtk-4.0 libconfig libvlc libcurl libxml-2.0 sdl SDL_image json-c`

all:
	$(CC) -g -o bin/iptvx src/*.c $(CFLAGS) 

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
