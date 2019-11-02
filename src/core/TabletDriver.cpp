#include "TabletDriver.h"

TabletConfig* TabletDriver::s_Configs = nullptr;
size_t TabletDriver::s_ConfigsSize = 0;

TabletDriver::TabletDriver(const char* configFolder) {
	libusb_init(&m_Context);
	readConfigs(configFolder);

	printf("Getting the device list...\n");
	m_ListSize = libusb_get_device_list(m_Context, &m_List); 	
	printf("Number of devices: %lu\n", m_ListSize);

	m_FoundTablet = findTablet();
	if (!m_FoundTablet) {
		printf("Could not find tablet.\n");
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

	printf("Creating thread for event pulling\n");
	eventThread = new std::thread(event_thread, this);
	eventThread->detach();
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
	int res = 0;
	unsigned char uname[250];
	for (size_t i = 0; i < s_ConfigsSize; i++) {
		memset(uname, 0, 250);
		const char* name = s_Configs[i].deviceName.c_str();
		printf("Trying to find tablet %s\n", name);


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
			printf("Found device %s, checking if it's the correct name...\n", productName);
			if (memcmp(productName, uname, s_Configs[i].deviceName.size() + 1) == 0) {
				m_TabletHandle = deviceHandle;
				m_TabletDevice = m_List[i];
				return true;
			}
			libusb_close(deviceHandle);
		}
	}

	return false;
}

void TabletDriver::kernelClaim() {
	if (libusb_kernel_driver_active(m_TabletHandle, 0) != 0) {
		printf("Kernel driver is active, detaching...\n");
		int res = libusb_detach_kernel_driver(m_TabletHandle, 0);
		if (res != 0) {
			printf("Could not deatch kernel driver\n");
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

void TabletDriver::readConfigs(const char* configFolder) {
	using json = nlohmann::json;
	std::string tabletConfig = configFolder;
	tabletConfig.append("/tablets.json");
	std::ifstream input(tabletConfig.c_str());
	if(!input.is_open()) {
		printf("Could not find tablet configs\n");
		return;
	}
	
	json j;
	input >> j;
	input.close();
	s_Configs = new TabletConfig[j.size()];
	s_ConfigsSize = j.size();
	int pos = 0;
	for (json::iterator it = j.begin(); it != j.end(); it++) {
		TabletConfig currentConfig;
		currentConfig.deviceName = it.key();
		auto current_iteration = it.value();
		currentConfig.vendorName = current_iteration["manufacturer"].get<std::string>();
		currentConfig.numButtons = current_iteration["numButtons"].get<int>();
		currentConfig.virtualWidth = current_iteration["virtualWidth"].get<int>();
		currentConfig.virtualHeight = current_iteration["virtualHeight"].get<int>();
		currentConfig.physicalWidth = current_iteration["physicalWidth"].get<int>();
		currentConfig.physicalHeight = current_iteration["physicalHeight"].get<int>();
		s_Configs[pos] = currentConfig;
		pos++;
	}
}