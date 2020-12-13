#include "DriverIO.h"


bool DriverIO::initInner(const wchar_t* device)
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

////////////////////////
////DriverIoVMBusChannels
////////////////////////
bool DriverIoVMBusChannels::init()
{
	return initInner(L"\\\\.\\VMBusChannels");
}

bool DriverIoVMBusChannels::getChannelsCount(UINT32* count)
{
	if (handle == INVALID_HANDLE_VALUE)
		return false;
	DWORD size = sizeof(UINT32);
	if (devIOctrl(IOCTL_GET_CHANNELS_COUNT, NULL, 0, count, &size))
		return true;
	return false;
}

bool DriverIoVMBusChannels::getChannelsData(VMBusChannelData** data, UINT32* count)
{
	if (!getChannelsCount(count))
		return false;
	*data = new VMBusChannelData[*count];
	if (*data == NULL)
		return false;
	DWORD size = sizeof(VMBusChannelData) * (*count);
	if (!devIOctrl(IOCTL_GET_CHANNELS_DATA, NULL, 0, *data, &size))
		return false;
	*count = size / sizeof(VMBusChannelData);
	return true;
}

////////////////////////
////DriverIoVMBusIntercept
////////////////////////
bool DriverIoVMBusIntercept::init()
{
	return initInner(L"\\\\.\\VMBusIntercept");
}

bool DriverIoVMBusIntercept::hookChannel(VMBusInteceptConf* conf)
{
	if (handle == INVALID_HANDLE_VALUE)
		return false;
	if (devIOctrl(IOCTL_INTERCEPT_CHANNELS_HOOK, conf, sizeof(VMBusInteceptConf), NULL, NULL))
		return true;
	return false;
}

bool DriverIoVMBusIntercept::unhookChannel()
{
	if (handle == INVALID_HANDLE_VALUE)
		return false;
	if (devIOctrl(IOCTL_INTERCEPT_CHANNELS_UNHOOK, NULL, 0, NULL, NULL))
		return true;
	return false;
}

bool DriverIoVMBusIntercept::setFilename(std::string filename)
{
	if (handle == INVALID_HANDLE_VALUE)
		return false;
	std::string filenameWithDD = "\\DosDevices\\" + filename;
	if (devIOctrl(IOCTL_INTERCEPT_SET_FILENAME, (void*)filenameWithDD.c_str(), (DWORD)filenameWithDD.length()+1, NULL, NULL))
		return true;
	return false;
}



////////////////////////
////DriverIoVMBusFuzzer
////////////////////////
bool DriverIoVMBusFuzzer::init()
{
	return initInner(L"\\\\.\\VMBusFuzzer");
}

bool DriverIoVMBusFuzzer::fuzz(VMBusFuzzConf* conf)
{
	if (handle == INVALID_HANDLE_VALUE)
		return false;
	if (devIOctrl(IOCTL_FUZZER_FUZZ, conf, sizeof(VMBusFuzzConf), NULL, NULL))
		return true;
	return false;
}

bool DriverIoVMBusFuzzer::stopFuzz()
{
	if (handle == INVALID_HANDLE_VALUE)
		return false;
	if (devIOctrl(IOCTL_INTERCEPT_CHANNELS_UNHOOK, NULL, 0, NULL, NULL))
		return true;
	return false;
}