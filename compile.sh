#!/bin/bash

if [ ! -d "tools" ]; then
	echo "Creating tools directory"
	$(mkdir tools)
fi

if [ ! -f "tools/FuckMake" ]; then
	echo "Downloading FuckMake"
	$(curl -L -o "tools/FuckMake" "https://github.com/JeppeSRC/FuckMake/releases/download/V1.1/FuckMake")
	$(chmod +x tools/FuckMake)
fi

if [ ! -d "src/json" ]; then
	echo "Creating json directory"
	$(mkdir src/json)
fi

if [ ! -f "src/json/json.hpp" ]; then
	echo "Downloading json.hpp"
	$(curl -L -o "src/json/json.hpp" "https://github.com/nlohmann/json/releases/download/v3.7.0/json.hpp")
fi

if [ -f driver ]; then
	rm driver
fi

./tools/FuckMake
