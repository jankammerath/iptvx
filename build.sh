#!/bin/bash
gcc -g -lvlc -lSDL -lSDL_image src/video.c src/window.c src/args.c src/webkit.c src/main.c -o bin/iptvx $(pkg-config --cflags --libs webkit2gtk-4.0)