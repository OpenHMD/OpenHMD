/*
 * OpenHMD - Free and Open Source API and drivers for immersive technology.
 * Copyright (C) 2018 Bernd Lehmann.
 * Distributed under the Boost 1.0 licence, see LICENSE for full text.
 */
#ifndef __RAWHID_H__
#define __RAWHID_H__

#include <Arduino.h>
#include <HID.h>
#include "USB/USBAPI.h"
#include "USB/PluggableUSB.h"

#define SIZE (sizeof(float) * 4)

#define RAWHID_USAGE_PAGE (0xFF42) 
#define RAWHID_USAGE      (0x4242) 

// This report descriptor is an adapted version from the HID-Porject
// see: https://github.com/NicoHood/HID/blob/master/src/SingleReport/RawHID.cpp
static const uint8_t  _hidReportDescriptorRawHID[] PROGMEM = {
	/*    RAW HID */
    0x06, lowByte(RAWHID_USAGE_PAGE), highByte(RAWHID_USAGE_PAGE),      /* 30 */
    0x0A, lowByte(RAWHID_USAGE), highByte(RAWHID_USAGE),

    0xA1, 0x01,                  /* Collection 0x01 */
    0x75, 0x08,                  /* report size = 8 bits */
    0x15, 0x00,                  /* logical minimum = 0 */
    0x26, 0xFF, 0x00,            /* logical maximum = 255 */

    0x95, SIZE,           /* report count TX */
    0x09, 0x01,                  /* usage */
    0x81, 0x02,                  /* Input (array) */

    0x95, SIZE,           /* report count RX */
    0x09, 0x02,                  /* usage */
    0x91, 0x02,                  /* Output (array) */
    0xC0                         /* end collection */ 
};

class RawHID: public PluggableUSBModule {
public:
  RawHID(void);
  int begin(void);
  int SendReport(const void* data, int len);

protected:
  int getInterface(uint8_t* interfaceCount);
  int getDescriptor(USBSetup& setup);
  bool setup(USBSetup& setup);
  uint8_t getShortName(char* name);

private:
  uint32_t epType[1];

  uint8_t protocol;
  uint8_t idle;
};

#endif
