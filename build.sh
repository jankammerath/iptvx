#!/bin/bash
gcc -g -lX11 -lvlc -lpthread src/main.c src/video.c src/window.c src/args.c src/webkit.c -o bin/iptvx $(pkg-config --cflags --libs webkit2gtk-4.0)