/*
 * OpenHMD - Free and Open Source API and drivers for immersive technology.
 * Copyright (C) 2018 Bernd Lehmann.
 * Distributed under the Boost 1.0 licence, see LICENSE for full text.
 */
#include "RawHID.h"

int RawHID::getInterface(uint8_t* interfaceCount)
{
	*interfaceCount += 1; // uses 1
	HIDDescriptor hidInterface = {
		D_INTERFACE(pluggedInterface, 1, USB_DEVICE_CLASS_HUMAN_INTERFACE, HID_SUBCLASS_NONE, HID_PROTOCOL_NONE),
		D_HIDREPORT(sizeof(_hidReportDescriptorRawHID)),
		D_ENDPOINT(USB_ENDPOINT_IN(pluggedEndpoint), USB_ENDPOINT_TYPE_INTERRUPT, SIZE, 0x01)
	};
	return USBDevice.sendControl(&hidInterface, sizeof(hidInterface));
}

int RawHID::getDescriptor(USBSetup& setup)
{
	// Check if this is a HID Class Descriptor request
	if (setup.bmRequestType != REQUEST_DEVICETOHOST_STANDARD_INTERFACE) { return 0; }
	if (setup.wValueH != HID_REPORT_DESCRIPTOR_TYPE) { return 0; }

	// In a HID Class Descriptor wIndex cointains the interface number
	if (setup.wIndex != pluggedInterface) { return 0; }

	USBDevice.packMessages(true);
        int res = USBDevice.sendControl(_hidReportDescriptorRawHID, sizeof(_hidReportDescriptorRawHID));
        if (res == -1)
                return -1;
	USBDevice.packMessages(false);
	return res;
}

uint8_t RawHID::getShortName(char *name)
{
       name[0] = 'H';
       name[1] = 'I';
       name[2] = 'D';
       name[3] = 'A' + (sizeof(_hidReportDescriptorRawHID) & 0x0F);
       name[4] = 'A' + ((sizeof(_hidReportDescriptorRawHID) >> 4) & 0x0F);
       return 5;
}

int RawHID::SendReport(const void* data, int len)
{
	return USBDevice.send(pluggedEndpoint, data, len);
}

bool RawHID::setup(USBSetup& setup)
{
	if (pluggedInterface != setup.wIndex) {
		return false;
	}

	uint8_t request = setup.bRequest;
	uint8_t requestType = setup.bmRequestType;

	if (requestType == REQUEST_DEVICETOHOST_CLASS_INTERFACE)
	{
		if (request == HID_GET_REPORT) {
			// TODO: RawHIDGetReport();
			return true;
		}
		if (request == HID_GET_PROTOCOL) {
			// TODO: Send8(protocol);
			return true;
		}
		if (request == HID_GET_IDLE) {
			USBDevice.armSend(0, &idle, 1);
			return true;
		}
	}

	if (requestType == REQUEST_HOSTTODEVICE_CLASS_INTERFACE)
	{
		if (request == HID_SET_PROTOCOL) {
			// The USB Host tells us if we are in boot or report mode.
			// This only works with a real boot compatible device.
			protocol = setup.wValueL;
			return true;
		}
		if (request == HID_SET_IDLE) {
			idle = setup.wValueL;
			return true;
		}
		if (request == HID_SET_REPORT)
		{
			//uint8_t reportID = setup.wValueL;
			//uint16_t length = setup.wLength;
			//uint8_t data[length];
			// Make sure to not read more data than USB_EP_SIZE.
			// You can read multiple times through a loop.
			// The first byte (may!) contain the reportID on a multreport.
			//USB_RecvControl(data, length);
		}
	}

	return false;
}

RawHID::RawHID(void) : PluggableUSBModule(1, 1, epType),
                   protocol(HID_REPORT_PROTOCOL), idle(1)
{
	epType[0] = USB_ENDPOINT_TYPE_INTERRUPT | USB_ENDPOINT_IN(0);;
	PluggableUSB().plug(this);
}

int RawHID::begin(void)
{
	return 0;
}
