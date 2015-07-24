/*
 * OpenHMD - Free and Open Source API and drivers for immersive technology.
 * Copyright (C) 2013 Fredrik Hultin.
 * Copyright (C) 2013 Jakob Bornecrantz.
 * Copyright (C) 2015 Joey Ferwerda
 * Distributed under the Boost 1.0 licence, see LICENSE for full text.
 */

/* Android Driver */

#ifndef ANDROID_H
#define ANDROID_H

#include "../openhmdi.h"

typedef enum {
    DROID_DUROVIS_OPEN_DIVE   = 1;
    DROID_DUROVIS_DIVE_5      = 2;
    DROID_DUROVIS_DIVE_7      = 3;
    DROID_CARL_ZEISS_VRONE    = 4;
    DROID_GOOGLE_CARDBOARD    = 5;

    DROID_NONE                = 0;
} android_hmd_profile;

#endif // ANDROID_H
