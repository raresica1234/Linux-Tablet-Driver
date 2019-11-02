#include <libusb-1.0/libusb.h>
#include <X11/Xlib.h>

#include <iostream>
#include <cassert>
#include <string>
#include <cstring>
#include <chrono>

#include "util/Area.h"
#include "util/CursorHelper.h"
#include "core/TabletDriver.h"
#include "core/TabletPacket.h"

Area* g_displayArea = NULL;
Area* g_tabletArea = NULL;


int main() {
	TabletDriver* driver = new TabletDriver("config");
	CursorHelper* cursor = new CursorHelper();
	g_displayArea = new Area(0, 0, 1920, 1080);
	g_tabletArea = new Area(0, 37.3875f, 69.0f, 38.8125f);

	
	auto start = std::chrono::system_clock::now();
	int frames = 0;
	auto current = start;

	std::chrono::duration<double> elapsed;

	int previous = 0;
	TabletPacket packet;

	while (!driver->hasCrashed()) {
		frames++;
		if (driver->foundTablet()) {
			packet = driver->getData();
			if(packet.isValid()){
				float x = packet.x;
				float y = packet.y;
				driver->mapVirtualtoPhysical(x, y);
				Area::map(x, y, g_tabletArea, g_displayArea);
				cursor->MoveTo((int)x, (int)y);
				if (packet.button == MouseButton::MouseButton1 && previous == 0) {
					cursor->Click(MouseButton::MouseButton1);
					previous = 1;
				} else if (packet.button == MouseButton::NoButton && previous == 1) {
					cursor->Release(MouseButton::MouseButton1);
					previous = 0;
				}
			}
		}
		elapsed = current - start;
		if(elapsed.count() >= 1.0) {
			//printf("Polling rate: %dHz\n", frames);
			frames = 0;
			start = current;
		}
		current = std::chrono::system_clock::now();
	}
	
	
	delete g_tabletArea, g_displayArea, cursor;
	return 0;
}	

