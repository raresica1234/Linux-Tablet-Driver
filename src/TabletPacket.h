#pragma once

#include "Enums.h"

struct TabletPacket {
	MouseButton button;
	int x;
	int y;
	int pressure;

	TabletPacket() {
		button = MouseButton::NoButton;
		x = y = pressure = -1;
	}

	TabletPacket(unsigned char* data, int offset) {
		button 		= static_cast<MouseButton>(data[offset + 1]);
		x 			= static_cast<int>(data[offset + 3] << 8 | data[offset + 2]);
		y 			= static_cast<int>(data[offset + 5] << 8 | data[offset + 4]);
		pressure 	= static_cast<int>(data[offset + 7] << 8 | data[offset + 6]);
	}

	void operator==(const TabletPacket& other) {
		this->button 	= other.button;
		this->x 		= other.x;
		this->y 		= other.y;
		this->pressure 	= other.pressure;
	}

	bool isValid() {
		return (x != -1) && (y != -1) && (pressure != -1);
	}

};
