#include "DriverIO.h"

bool DriverIO::init(const wchar_t* device)
{
	handle = CreateFile(device, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
	return (handle != INVALID_HANDLE_VALUE);
}

bool DriverIO::devIOctrl(DWORD code, PVOID inBuffer, DWORD inBufferSize, PVOID outBuffer, DWORD* outBufferSize)
{
	bool result = false;
	DWORD recv, outBufferSizeVal = 0;
	if (outBufferSize)
		outBufferSizeVal = *outBufferSize;

	if (DeviceIoControl(handle, code, inBuffer, inBufferSize, outBuffer, outBufferSizeVal, &recv, NULL))
	{
		result = true;
		if (outBufferSize)
			*outBufferSize = recv;
	}
	return result;
}

bool DriverIO::getChannelsCount(UINT32* count)
{
	if (handle == INVALID_HANDLE_VALUE)
		return false;
	DWORD size = sizeof(UINT32);
	if (devIOctrl(IOCTL_GET_CHANNELS_COUNT, NULL, 0, count, &size))
		return true;
	return false;
}

bool DriverIO::getChannelsData(VMBusChannel** data, UINT32* count)
{
	if (!getChannelsCount(count))
		return false;
	*data = new VMBusChannel[*count];
	if (*data == NULL)
		return false;
	DWORD size = sizeof(VMBusChannel) * (*count);
	if (!devIOctrl(IOCTL_GET_CHANNELS_DATA, NULL, 0, *data, &size))
		return false;
	*count = size / sizeof(VMBusChannel);
	return true;
}
