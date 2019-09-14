#pragma once

#include "Enums.h"

struct TabletPacket {
	MouseButton button;
	short x;
	short y;
	short pressure;

	TabletPacket(unsigned char* data, int offset) {
		button 		= static_cast<MouseButton>(data[offset + 1]);
		x 			= static_cast<short>(data[offset + 3] << 8 | data[offset + 2]);
		y 			= static_cast<short>(data[offset + 5] << 8 | data[offset + 4]);
		pressure 	= static_cast<short>(data[offset + 7] << 8 | data[offset + 6]);
	}

};
