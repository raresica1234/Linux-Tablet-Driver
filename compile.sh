#!/bin/bash

if [ ! -d tools ]; then
	echo "Creating tools directory"
	$(mkdir tools)
fi

if [ ! -f tools/FuckMake ]; then
	echo "Downloading FuckMake"
	$(curl -L -o "tools/FuckMake" "https://github.com/JeppeSRC/FuckMake/releases/download/V1.1/FuckMake")
	$(chmod +x tools/FuckMake)
fi

if [ -f driver ]; then
	rm driver
fi

./tools/FuckMake
