#pragma once
#include<Windows.h>
#include<string>
#include"IOCTLs.h"
#include"SharedStructures.h"

class DriverIO
{
protected:
	HANDLE handle = INVALID_HANDLE_VALUE;
	bool initInner(const  wchar_t* device);
	bool devIOctrl(DWORD code, PVOID inBuffer, DWORD inBufferSize, PVOID outBuffer, DWORD* outBufferSize);
};


//VMBusChannels driver
class DriverIoVMBusChannels :public DriverIO
{
public:
	bool init();

	bool getChannelsCount(UINT32* count);
	bool getChannelsData(VMBusChannelData** data, UINT32* count);
};


//VMBusIntercept driver
class DriverIoVMBusIntercept :public DriverIO
{
public:
	bool init();

	bool hookChannel(VMBusInteceptConf*);
	bool unhookChannel();
	bool setFilename(std::string filename);
};

//VMBusFuzzer driver
class DriverIoVMBusFuzzer :public DriverIO
{
public:
	bool init();

	bool fuzz(VMBusFuzzConf*);
	bool stopFuzz();
};