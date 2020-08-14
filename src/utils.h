// Copyright 2013, Fredrik Hultin.
// Copyright 2013, Jakob Bornecrantz.
// SPDX-License-Identifier: BSL-1.0
/*
 * OpenHMD - Free and Open Source API and drivers for immersive technology.
 */

/* Utility functions */

#ifndef UTILS_H
#define UTILS_H

#include <wchar.h>

static inline int ohmd_wstring_match(const wchar_t *a, const wchar_t *b)
{
	if(!a || !b)
		return 0;
	return wcscmp(a, b) == 0;
}

#endif /* UTILS_H */
