#pragma once

struct ThreadData {
	ULONG ThreadId; // we use a ULONG because DWORD is not defined in the driver headers
	int	Priority;
};

#define PRIORITY_BOOSTER_SERVICE 0x8000
#define IOCTL_PRIORITY_BOOSTER_SET_PRIORITY CTL_CODE(PRIORITY_BOOSTER_SERVICE, 0x800, METHOD_NEITHER, FILE_ANY_ACCESS)