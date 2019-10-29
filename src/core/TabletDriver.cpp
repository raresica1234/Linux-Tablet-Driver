#include "TabletDriver.h"


TabletDriver::TabletDriver(const char* configFolder) {
	libusb_init(&m_Context);
	printf("Getting the device list...\n");
	m_ListSize = libusb_get_device_list(m_Context, &m_List); 	
	printf("Number of devices: %lu\n", m_ListSize);

	m_FoundTablet = findTablet();
	if (!m_FoundTablet) {
		printf("Could not find tablet.");
		m_DriverCrashed = true;
		return;
	}
	else {
		printf("Found tablet!\n");
	}

	kernelClaim();
	
	if(libusb_has_capability(LIBUSB_CAP_HAS_HOTPLUG)) {
		printf("Registering hotplug callback\n");
		libusb_device_descriptor desc = {0};
		int res = libusb_get_device_descriptor(m_TabletDevice, &desc);
		assert(res == LIBUSB_SUCCESS);
		res = libusb_hotplug_register_callback(m_Context, (libusb_hotplug_event)(LIBUSB_HOTPLUG_EVENT_DEVICE_LEFT | LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED), LIBUSB_HOTPLUG_NO_FLAGS, desc.idVendor, desc.idProduct, LIBUSB_HOTPLUG_MATCH_ANY, hotplug_callback, (void*)this, &m_CallbackHandle);
		assert(res == LIBUSB_SUCCESS);
	}

	printf("Creating thread for event pulling");
	eventThread = new std::thread(event_thread, this);
}

TabletDriver::~TabletDriver() {
	eventThread->join();
	delete eventThread;
	libusb_release_interface(m_TabletHandle, 0);
	libusb_close(m_TabletHandle);
	libusb_free_device_list(m_List, 1);
	libusb_exit(NULL);
}

TabletPacket TabletDriver::getData() {
	int transferred;
	int res = libusb_interrupt_transfer(m_TabletHandle, 0x82, buffer, sizeof(buffer), &transferred, 10);
	if (transferred)
		return TabletPacket(buffer, 0);
	return TabletPacket();
}

void TabletDriver::pullEvents() {
	libusb_handle_events_completed(m_Context, NULL);
}

bool TabletDriver::findTablet() {
	// hardcoded name for now
	const char* name = "G430S";

	int res = 0;
	unsigned char uname[250] = {0};
	for (size_t i = 0; i <= strlen(name); i++)
		uname[i] = static_cast<unsigned char>(name[i]);

	unsigned char productName[250] = {0};
	for (size_t i = 0; i < m_ListSize; i++) {
		libusb_device_descriptor desc = {0};
		res = libusb_get_device_descriptor(m_List[i], &desc);
		assert(res == LIBUSB_SUCCESS);
		libusb_device_handle* deviceHandle = NULL;
		res = libusb_open(m_List[i], &deviceHandle);
		assert(res == LIBUSB_SUCCESS);
		res = libusb_get_string_descriptor_ascii(deviceHandle, desc.iProduct, productName, sizeof(productName));
		printf("Found device, checking if it's the correct name...\n");
		if (memcmp(productName, uname, strlen(name)) == 0) {
			m_TabletHandle = deviceHandle;
			m_TabletDevice = m_List[i];
			return true;
		}
		libusb_close(deviceHandle);
		
	}
	return false;
}

void TabletDriver::kernelClaim() {
	if (libusb_kernel_driver_active(m_TabletHandle, 0) != 0) {
		printf("Kernel driver is active, detaching...\n");
		int res = libusb_detach_kernel_driver(m_TabletHandle, 0);
		if (res != 0) {
			printf("Could not deatch kernel driver");
			m_DriverCrashed = true;
			return;
		}
		printf("Kernel driver detached!\n");
	}
}

int TabletDriver::hotplug_callback(libusb_context* context, libusb_device* device, libusb_hotplug_event event, void* user_data) {
	TabletDriver* instance = (TabletDriver*)user_data;
	if (event == LIBUSB_HOTPLUG_EVENT_DEVICE_LEFT) {
		printf("Hotplug device left\n");
		if (instance->m_TabletDevice) {
			libusb_close(instance->m_TabletHandle);
			instance->m_TabletDevice = nullptr;
			instance->m_FoundTablet = false;
		}
	}
	else if (event == LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED) {
		printf("Hotplug device arrived\n");
		instance->m_TabletDevice = device;
		int res = libusb_open(instance->m_TabletDevice, &(instance->m_TabletHandle));
		if(res) {
			printf("Could not open device, error: %d\n", res);
			instance->m_DriverCrashed = true;
		} else {
			instance->m_FoundTablet = true;
			instance->kernelClaim();
		}	
	}

	return 0;
}


void TabletDriver::event_thread(TabletDriver* instance) {
	while(!instance->hasCrashed()) {
		instance->pullEvents();
	}
}