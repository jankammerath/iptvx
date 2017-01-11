#!/bin/bash
gcc -lpthread -lSDL -lavutil -lavformat -lavcodec -lz -lavutil -lm -I/usr/include/ffmpeg/ src/main.c -o bin/iptvx