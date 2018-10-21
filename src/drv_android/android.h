// Copyright 2013, Fredrik Hultin.
// Copyright 2013, Jakob Bornecrantz.
// Copyright 2015, Joey Ferwerda.
// SPDX-License-Identifier: BSL-1.0
/*
 * OpenHMD - Free and Open Source API and drivers for immersive technology.
 */

/* Android Driver */

#ifndef ANDROID_H
#define ANDROID_H

#include "../openhmdi.h"

typedef enum {
    DROID_DUROVIS_OPEN_DIVE   = 1,
    DROID_DUROVIS_DIVE_5      = 2,
    DROID_DUROVIS_DIVE_7      = 3,
    DROID_CARL_ZEISS_VRONE    = 4,
    DROID_GOOGLE_CARDBOARD    = 5,

    DROID_NONE                = 0,
} android_hmd_profile;

//Android copy-paste from android_native_app_glue to be able to cast data to something useful
#include <poll.h>
#include <pthread.h>
#include <sched.h>

#include <android/configuration.h>
#include <android/looper.h>
#include <android/native_activity.h>

struct android_app;

#endif // ANDROID_H
