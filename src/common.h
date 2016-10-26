/*
 * OpenHMD - Free and Open Source API and drivers for immersive technology.
 */

/* Common functions internal interace*/

#ifndef COMMON_H
#define COMMON_H

void dump_info_string(int (*fun)(hid_device*, wchar_t*, size_t), const char* what, hid_device* device);
char* _hid_to_unix_path(char* path);

#endif