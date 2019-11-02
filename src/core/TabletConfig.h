#pragma once
#include <string>

struct TabletConfig {
	std::string deviceName;
	std::string vendorName;
	int numButtons;
	int virtualWidth;
	int virtualHeight;
	int physicalWidth;
	int physicalHeight;

	TabletConfig() {}

	TabletConfig(const TabletConfig& other) {
		deviceName 		= other.deviceName;
		vendorName 		= other.vendorName;
		numButtons 		= other.numButtons;
		virtualWidth 	= other.virtualWidth;
		virtualHeight 	= other.virtualHeight;
		physicalWidth 	= other.physicalWidth;
		physicalHeight 	= other.physicalHeight;
	}
};
