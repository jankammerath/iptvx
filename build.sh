#!/bin/bash
gcc -lpthread -lX11 -lvlc src/main.c -o bin/iptvx $(pkg-config --cflags --libs webkit2gtk-4.0)