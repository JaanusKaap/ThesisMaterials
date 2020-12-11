#pragma once
#include<Windows.h>
#include"IOCTLs.h"
#include"SharedStructures.h"

class DriverIO
{
protected:
	HANDLE handle = INVALID_HANDLE_VALUE;
	bool initInner(const  wchar_t* device);
	bool devIOctrl(DWORD code, PVOID inBuffer, DWORD inBufferSize, PVOID outBuffer, DWORD* outBufferSize);
};

class DriverIoVMBusChannels:public DriverIO
{
public:
	bool init();

	//VMBusChannels driver
	bool getChannelsCount(UINT32* count);
	bool getChannelsData(VMBusChannelData** data, UINT32* count);
};