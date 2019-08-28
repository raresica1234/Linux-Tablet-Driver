#include <libusb-1.0/libusb.h>
#include <iostream>
#include <cassert>
#include <string>
#include <cstring>
#include "Area.h"
#include <X11/Xlib.h>


Area* g_displayArea = NULL;
Area* g_tabletArea = NULL;
const char* g_DeviceName = "G430S";
libusb_device_handle* tabletHandle = NULL;

libusb_context* context = NULL;
libusb_device** list = NULL;

Display* display;

void initXlib() {
	display = XOpenDisplay(NULL);
	if (display == NULL) {
		printf("Could not open display!");
		return;
	}
}

void closeXlib() {
	XCloseDisplay(display);
}

void move_to(int x, int y) {
	XEvent event;
	XQueryPointer(display, DefaultRootWindow(display), &event.xbutton.root, &event.xbutton.window, &event.xbutton.x_root, &event.xbutton.y_root,
			&event.xbutton.x, &event.xbutton.y, &event.xbutton.state);
	int lastX, lastY;
	lastX = event.xbutton.x;
	lastY = event.xbutton.y;
	
	//printf("%d, %d", lastX, lastY);

	x -= lastX;
	y -= lastY;
	XWarpPointer(display, None, None, 0, 0, 0, 0, x, y);
	XFlush(display);
}


libusb_device* pickDeviceByName(const char* name, libusb_device** list, size_t size) {
	int res = 0;
	unsigned char uname[250] = {0};
	for (size_t i = 0; i <= strlen(name); i++)
		uname[i] = static_cast<unsigned char>(name[i]);

	unsigned char productName[250] = {0};
	for (size_t i = 0; i < size; i++) {
		libusb_device_descriptor desc = {0};
		res = libusb_get_device_descriptor(list[i], &desc);
		assert(res == LIBUSB_SUCCESS);
		libusb_device_handle* deviceHandle = NULL;
		res = libusb_open(list[i], &deviceHandle);
		assert(res == LIBUSB_SUCCESS);
		res = libusb_get_string_descriptor_ascii(deviceHandle, desc.iProduct, productName, sizeof(productName));
		assert(res);
		libusb_close(deviceHandle);
		if (memcmp(productName, uname, strlen(name)) == 0)
			return list[i];
		
	}
	return NULL;
}

int main() {
	g_displayArea = new Area(0, 0, 1920, 1080);
	g_tabletArea = new Area(0, 16078, 22319, 16689);

	initXlib();
	libusb_init(&context);

	size_t size = libusb_get_device_list(context, &list); 	
	printf("Number of devices: %lu\n", size);
	libusb_device* device = NULL;
	device = pickDeviceByName(g_DeviceName, list, size);
	if (device == NULL) {
		printf("Could not find XP-PEN G430s tablet\n");
		return 0;
	}
	libusb_open(device, &tabletHandle);

	unsigned char buffer[1000] = {0};
	int transferred = 0;

	if (libusb_kernel_driver_active(tabletHandle, 0) != 0) {
		printf("Kernel driver is active, detaching...");
		int res = libusb_detach_kernel_driver(tabletHandle, 0);
		assert(res == 0);
	}

	int res = libusb_claim_interface(tabletHandle, 0);
	printf("%d\n", res);
	assert(res == 0);

	int x = 0, y = 0, pressure = 0;	
	while (true) {
		res = libusb_interrupt_transfer(tabletHandle, 0x82, buffer, sizeof(buffer), &transferred, 10);  
		if (transferred != 0) {
			//printf("Button pressed: %d ", buffer[1]);
			x = buffer[3] << 8 | buffer[2];
			y = buffer[5] << 8 | buffer[4];
			
			pressure = buffer[7] << 8 | buffer[6];

			//printf("x: %d, y: %d, pressure: %d\n", x, y, pressure);

			Area::map(x, y, g_tabletArea, g_displayArea);
			move_to(x, y);
		}
	}
	
	libusb_release_interface(tabletHandle, 0);
	libusb_close(tabletHandle);
	libusb_free_device_list(list, 1);
	libusb_exit(NULL);
	closeXlib();
	delete g_tabletArea, g_displayArea;
	return 0;
}	

