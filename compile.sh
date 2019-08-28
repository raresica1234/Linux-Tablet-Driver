#!/bin/bash

g++ src/main.cpp `pkg-config --libs --cflags libusb-1.0` -g -o driver.o
