#pragma once
#ifdef _DRIVER
#include<ntddk.h>
#else
#include<windows.h>
#endif

#define IOCTL_GET_CHANNELS_COUNT CTL_CODE(FILE_DEVICE_UNKNOWN, 0x800, METHOD_BUFFERED, FILE_READ_DATA | FILE_WRITE_DATA)
#define IOCTL_GET_CHANNELS_DATA CTL_CODE(FILE_DEVICE_UNKNOWN, 0x801, METHOD_BUFFERED, FILE_READ_DATA | FILE_WRITE_DATA)