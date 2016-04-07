/*
 * OpenHMD - Free and Open Source API and drivers for immersive technology.
 * Copyright (C) 2013 Fredrik Hultin.
 * Copyright (C) 2013 Jakob Bornecrantz.
 * Distributed under the Boost 1.0 licence, see LICENSE for full text.
 */

/* Platform Specific Functions, Win32 Implementation */

#ifdef _WIN32

#define WIN32_LEAN_AND_MEAN
#define WIN32_EXTRA_LEAN

#include <windows.h>

double ohmd_get_tick()
{
        double high, low;
        FILETIME filetime;

        GetSystemTimeAsFileTime(&filetime);

        high = filetime.dwHighDateTime;
        low = filetime.dwLowDateTime;

        return (high * 4294967296.0 + low) / 10000000;
}

// TODO higher resolution
void ohmd_sleep(double seconds)
{
        Sleep((DWORD)(seconds * 1000));
}

static int _enable_ovr_service = 0;

void _toggle_ovr_service(int state) //State is 0 for Disable, 1 for Enable
{
	SC_HANDLE serviceDbHandle = OpenSCManager(NULL,NULL,SC_MANAGER_ALL_ACCESS);
	SC_HANDLE serviceHandle = OpenService(serviceDbHandle, 'OVRService', SC_MANAGER_ALL_ACCESS);

	SERVICE_STATUS_PROCESS status;
	DWORD bytesNeeded;
	QueryServiceStatusEx(serviceHandle, SC_STATUS_PROCESS_INFO,(LPBYTE) &status,sizeof(SERVICE_STATUS_PROCESS), &bytesNeeded);

	if (state == 0 || status.dwCurrentState == SERVICE_RUNNING)
	{
		// Stop it
		BOOL b = ControlService(serviceHandle, SERVICE_CONTROL_STOP, (LPSERVICE_STATUS) &status);
		if (b)
		{
			printf("OVRService stopped\n");
			_enable_ovr_service = 1;
		}
		else 
			printf("Error: OVRService failed to stop, please try running with Administrator rights\n");
	}
	else if (state == 1 && _enable_ovr_service)
	{
		// Start it 
		BOOL b = StartService(serviceHandle, NULL, NULL); 
		if (b) 
			printf("OVRService started\n");
		else 
			printf("Error: OVRService failed to start, please try running with Administrator rights\n");
	} 
	CloseServiceHandle(serviceHandle); 
	CloseServiceHandle(serviceDbHandle); 
}
#endif
