#!/bin/bash

g++ src/main.cpp src/Area.cpp -lX11 `pkg-config --libs --cflags libusb-1.0` -g -o driver.o
