#!/bin/bash

g++ src/CursorHelper.cpp src/Area.cpp src/main.cpp -lX11 -lXtst `pkg-config --libs --cflags libusb-1.0` -g -o driver.o
