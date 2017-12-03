CC = gcc
CFLAGS = `pkg-config --cflags --libs glib-2.0 webkit2gtk-4.0 libconfig libvlc libcurl libxml-2.0 sdl SDL_image json-c`

all:
	$(CC) -g -o bin/iptvx src/*.c $(CFLAGS) 

install:
	install -s -m 755 -o 0 -g 0 bin/iptvx /usr/bin/iptvx
	install -m 644 -o 0 -g 0 -d /etc/iptvx
	install -m 644 -o 0 -g 0 cfg/iptvx.conf /etc/iptvx/iptvx.conf
	install -m 644 -o 0 -g 0 cfg/channels.conf /etc/iptvx/channels.conf
	install -m 644 -o 0 -g 0 -d /var/iptvx
	install -m 644 -o 0 -g 0 -d /var/iptvx/app
	install -m 644 -o 0 -g 0 -d /var/iptvx/data
	install -m 644 -o 0 -g 0 -d /var/iptvx/data/epg
	install -m 644 -o 0 -g 0 -d /var/iptvx/data/logo
	install -m 644 -o 0 -g 0 -D app/*.* /var/iptvx/app
