#!/bin/bash
gcc -lpthread -lX11 -lvlc src/main.c src/video.c src/window.c src/args.c src/webkit.c -o bin/iptvx $(pkg-config --cflags --libs webkit2gtk-4.0)