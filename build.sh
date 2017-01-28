#!/bin/bash
gcc -g -lvlc -lconfig -lcurl -lSDL -lSDL_image -lxml2 -ljson-c src/*.c -o bin/iptvx $(pkg-config --cflags --libs webkit2gtk-4.0)