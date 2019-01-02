// Copyright 2018, Joey Ferwerda.
// SPDX-License-Identifier: BSL-1.0
/*
 * OpenHMD - Free and Open Source API and drivers for immersive technology.
 */

/* LG R100 (360VR) Driver */


#ifndef LGR100_H
#define LGR100_H

#include <stdint.h>
#include <stdbool.h>

#include "../openhmdi.h"

#define FEATURE_BUFFER_SIZE	256

typedef enum
{
	LGR100_BUTTON_OK_ON    = 1,
	LGR100_BUTTON_BACK_ON  = 2,
	LGR100_BUTTON_BACK_OFF = 3,
	LGR100_BUTTON_OK_OFF   = 4,
} lgr100_button;

typedef struct
{
	float accel[3];
	float gyro[3];
	uint32_t tick;
} lgr100_sensor_sample;

typedef enum {
	LGR100_IRQ_NULL			= 0,
	LGR100_IRQ_UNKNOWN1		= 1,
	LGR100_IRQ_BUTTONS		= 2,
	LGR100_IRQ_DEBUG1		= 3,
	LGR100_IRQ_DEBUG2		= 4,
	LGR100_IRQ_SENSORS 		= 5,
	LGR100_IRQ_DEBUG_SEQ1 	= 32,
	LGR100_IRQ_DEBUG_SEQ2	= 33,
	LGR100_IRQ_UNKNOWN3		= 101,
	LGR100_IRQ_UNKNOWN2		= 255
} lgr100_usb_cmd;

/* All known commands as found in a firmware dump */
static const unsigned char start_device[14] = {0x03, 0x0C,'V','R',' ','A','p','p',' ','S','t','a','r','t'};
static const unsigned char start_accel[10] = {0x03, 0x0C,'A','c','c','e','l',' ','O','n'};
static const unsigned char start_gyro[9] = {0x03, 0x0C,'G','y','r','o',' ','O','n'};
static const unsigned char keep_alive[15] = {0x03, 0x0C,'S','l','e','e','p',' ','D','i','s','a','b','l','e'};
//static const unsigned char get_debug_info[14] = {0x03, 0x0C,'g','e','t','D','e','b','u','g','I','n','f','o'};
//static const unsigned char get_result[14] = {0x03, 0x0C,'g','e','t','A','A','T','R','e','s','u','l','t'};
//static const unsigned char enable[11] = {0x03, 0x0C,'A','c','c','e','l',' ','O','f','f'};
//static const unsigned char enable[16] = {0x03, 0x0C,'A','c','c','e','l',' ','S','e','l','f','t','e','s','t'};
//static const unsigned char accel_get[15] = {0x03, 0x0C,'A','c','c','e','l',' ','G','e','t',' ','X','Y','Z'};
//static const unsigned char enable[10] = {0x03, 0x0C,'G','y','r','o',' ','O','f','f'};
//static const unsigned char enable[15] = {0x03, 0x0C,'G','y','r','o',' ','S','e','l','f','t','e','s','t'};
//static const unsigned char enable[14] = {0x03, 0x0C,'G','y','r','o',' ','G','e','t',' ','X','Y','Z'};
//static const unsigned char enable[12] = {0x03, 0x0C,'C','o','m','p','a','s','s',' ','O','n'};
//static const unsigned char enable[13] = {0x03, 0x0C,'C','o','m','p','a','s','s',' ','O','f','f'};
//static const unsigned char enable[17] = {0x03, 0x0C,'C','o','m','p','a','s','s',' ','G','e','t',' ','X','Y','Z'};
//static const unsigned char enable[14] = {0x03, 0x0C,'P','r','o','x','i','m','i','t','y',' ','O','n'};
//static const unsigned char enable[15] = {0x03, 0x0C,'P','r','o','x','i','m','i','t','y',' ','O','f','f'};
//static const unsigned char enable[16] = {0x03, 0x0C,'P','r','o','x','i','m','i','t','y',' ','C','a','l'};
//static const unsigned char enable[20] = {0x03, 0x0C,'P','r','o','x','i','m','i','t','y',' ','G','e','t',' ','D','a','t','a'};
//static const unsigned char enable[25] = {0x03, 0x0C,'P','r','o','x','i','m','i','t','y',' ','G','e','t',' ','C','r','o','s','s','t','a','l','k'};
//static const unsigned char enable[20] = {0x03, 0x0C,'P','r','o','x','i','m','i','t','y',' ','S','e','t',' ','R','e','g','i'};
//static const unsigned char enable[20] = {0x03, 0x0C,'P','r','o','x','i','m','i','t','y',' ','G','e','t',' ','R','e','g','i'};
//static const unsigned char enable[16] = {0x03, 0x0C,'i','s','D','i','s','p','l','a','y','R','e','a','d','y'};
//static const unsigned char enable[8] = {0x03, 0x0C,'R','e','b','o','o','t'};
//static const unsigned char enable[16] = {0x03, 0x0C,'S','e','t',' ','B','r','i','g','h','t','n','e','s','s'};
//static const unsigned char enable[10] = {0x03, 0x0C,'S','e','t',' ','M','o','d','e'};
//static const unsigned char enable[23] = {0x03, 0x0C,'S','e','t',' ','B','a','c','k','l','i','g','h','t',' ','C','o','n','t','r','o','l'};
//static const unsigned char enable[22] = {0x03, 0x0C,'S','e','t',' ','L','C','D',' ','P','a','t','t','e','r','n',' ','T','e','s','t'};
//static const unsigned char enable[13] = {0x03, 0x0C,'R','1',' ','S','h','u','t','d','o','w','n'};
//static const unsigned char read_minios[20] = {0x03, 0x0C,'R','e','a','d',' ','M','i','n','i','O','S',' ','R','e','s','u','l','t'};

/* These values will actually change the calibration of your internal sensors, use with caution and only when it is intended */

//static const unsigned char calibrate_gyro[10] = {0x03, 0x0C,'G','y','r','o',' ','C','a','l'};
//static const unsigned char calibrate_accel[11] = {0x03, 0x0C,'A','c','c','e','l',' ','C','a','l'};

/* The write commands seem to require additional information, or a different send command */

//static const unsigned char enable[21] = {0x03, 0x0C,'W','r','i','t','e',' ','M','o','t','i','o','n',' ','R','e','s','u','l','t'};
//static const unsigned char enable[24] = {0x03, 0x0C,'W','r','i','t','e',' ','P','r','o','x','i','m','i','t','y',' ','R','e','s','u','l','t'};
//static const unsigned char enable[18] = {0x03, 0x0C,'W','r','i','t','e',' ','K','e','y',' ','R','e','s','u','l','t'};
//static const unsigned char enable[23] = {0x03, 0x0C,'W','r','i','t','e',' ','E','a','r','p','h','o','n','e',' ','R','e','s','u','l','t'};
//static const unsigned char enable[19] = {0x03, 0x0C,'W','r','i','t','e',' ','H','D','M','I',' ','R','e','s','u','l','t'};
//static const unsigned char enable[18] = {0x03, 0x0C,'W','r','i','t','e',' ','L','C','D',' ','R','e','s','u','l','t'};
//static const unsigned char enable[20] = {0x03, 0x0C,'W','r','i','t','e',' ','R','e','s','e','t',' ','R','e','s','u','l','t'};

bool decode_lgr100_imu_msg(lgr100_sensor_sample* smp, const unsigned char* buffer, int size);

#endif
