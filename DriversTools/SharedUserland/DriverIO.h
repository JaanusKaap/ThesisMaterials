#pragma once
#include<Windows.h>
#include"IOCTLs.h"
#include"SharedStructures.h"

class DriverIO
{
private:
	HANDLE handle = INVALID_HANDLE_VALUE;

public:
	bool init(const  wchar_t* device);
	bool devIOctrl(DWORD code, PVOID inBuffer, DWORD inBufferSize, PVOID outBuffer, DWORD* outBufferSize);

	//VMBusChannels driver
	bool getChannelsCount(UINT32* count);
	bool getChannelsData(VMBusChannel** data, UINT32* count);
};