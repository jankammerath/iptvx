CC = gcc
CFLAGS = `pkg-config --cflags --libs glib-2.0 webkit2gtk-4.0 libconfig libvlc libcurl libxml-2.0 sdl SDL_image json-c`

all:
	$(CC) -g -o bin/iptvx src/*.c $(CFLAGS) 

install:
	cp bin/iptvx /usr/bin/iptvx
	chmod +x /usr/bin/iptvx
	mkdir /etc/iptvx
	cp cfg/iptvx.conf /etc/iptvx/iptvx.conf
	cp cfg/channels.conf /etc/iptvx/channels.conf
	chmod 0455 /etc/iptvx -R
	mkdir /var/iptvx/data
	mkdir /var/iptvx/data/epg
	mkdir /var/iptvx/data/logo
	chmod 4666 /var/iptvx/data -R

