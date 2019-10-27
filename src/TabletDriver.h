#pragma once

#include <libusb-1.0/libusb.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <thread>

#include "TabletPacket.h"

class TabletDriver {
private:
	libusb_context* m_Context = nullptr;
	libusb_device** m_List = nullptr;
	size_t m_ListSize;
	
	libusb_device* m_TabletDevice = nullptr;
	libusb_device_handle* m_TabletHandle = nullptr;

	libusb_hotplug_callback_handle m_CallbackHandle;

	const char* m_ConfigFolder;

	bool m_FoundTablet = false;

	unsigned char buffer[100] = {0};

	bool m_DriverCrashed = false;

	std::thread* eventThread = nullptr;

public:
	TabletDriver(const char* configFolder);
	~TabletDriver();

	TabletPacket getData();

	inline bool foundTablet() { return m_FoundTablet; }
	inline bool hasCrashed() { return m_DriverCrashed; }

private:
	void pullEvents();

	bool findTablet();
	void kernelClaim();

	static int hotplug_callback(libusb_context* context, libusb_device* device, libusb_hotplug_event event, void* user_data);
	static void event_thread(TabletDriver* instance);
};
