#include <libusb-1.0/libusb.h>
#include <X11/Xlib.h>

#include <iostream>
#include <cassert>
#include <string>
#include <cstring>
#include <chrono>

#include "Area.h"
#include "CursorHelper.h"
#include "TabletDriver.h"
#include "TabletPacket.h"


Area* g_displayArea = NULL;
Area* g_tabletArea = NULL;


int main() {
	TabletDriver* driver = new TabletDriver("/config/");
	CursorHelper* cursor = new CursorHelper();
	g_displayArea = new Area(0, 0, 1920, 1080);
	g_tabletArea = new Area(0, 16078, 22319, 16689);

	
	auto start = std::chrono::system_clock::now();
	int frames = 0;
	auto current = start;

	std::chrono::duration<double> elapsed;

	int previous = 0;
	TabletPacket packet;

	while (driver->foundTablet()) {
		frames++;
		packet = driver->getData();
		if(packet.isValid()){
			Area::map(packet.x, packet.y, g_tabletArea, g_displayArea);
			cursor->MoveTo(packet.x, packet.y);
			if (packet.button == MouseButton::MouseButton1 && previous == 0) {
				cursor->Click(MouseButton::MouseButton1);
				previous = 1;
			} else if (packet.button == MouseButton::NoButton && previous == 1) {
				cursor->Release(MouseButton::MouseButton1);
				previous = 0;
			}
		}

		//res = libusb_interrupt_transfer(tabletHandle, 0x82, buffer, sizeof(buffer), &transferred, 10);  
		//if (transferred != 0) {
		//	button = (int)buffer[1];
		//	x = buffer[3] << 8 | buffer[2];
		//	y = buffer[5] << 8 | buffer[4];
		//	
		//	pressure = buffer[7] << 8 | buffer[6];
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

