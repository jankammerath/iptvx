#!/bin/bash
gcc -g -lvlc -lconfig -lSDL -lSDL_image src/*.c -o bin/iptvx $(pkg-config --cflags --libs webkit2gtk-4.0)