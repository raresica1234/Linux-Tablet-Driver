#include "TabletDriver.h"


TabletDriver::TabletDriver(const char* configFolder) {
    libusb_init(&m_Context);
    printf("Getting the device list...\n");
	m_ListSize = libusb_get_device_list(m_Context, &m_List); 	
	printf("Number of devices: %lu\n", m_ListSize);

    m_FoundTablet = findTablet();
    if (!m_FoundTablet) {
        printf("Could not find tablet.");
        return;
    }
	else {
		printf("Found tablet!\n");
	}

    if (libusb_kernel_driver_active(m_TabletHandle, 0) != 0) {
		printf("Kernel driver is active, detaching...\n");
		int res = libusb_detach_kernel_driver(m_TabletHandle, 0);
		if (res != 0)
            return;
		printf("Kernel driver detached!\n");
	}
}

TabletDriver::~TabletDriver() {
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
			return true;
		}
		libusb_close(deviceHandle);
		
	}
	return false;
}