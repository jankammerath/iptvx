CC = gcc
CFLAGS = `pkg-config --cflags --libs glib-2.0 webkit2gtk-4.0 libconfig libvlc libcurl libxml-2.0 sdl SDL_image json-c`

all:
	$(CC) -g $(CFLAGS) src/*.c -o bin/iptvx