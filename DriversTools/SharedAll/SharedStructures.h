#pragma once
#ifdef _DRIVER
#include<ntddk.h>
#else
#include<windows.h>
#endif

typedef struct VMBusChannelData
{
	UINT64 baseAddress;
	UINT32 maxPacketCount;
	UINT32 maxPacketSize;
	UINT32 clientContextSize;
	GUID interfaceType;
	GUID interfaceInstance;
	UINT8 isPipe;
	GUID vmId;
	UINT8 vtl;
	UINT64 processPacketCallback;
	UINT64 processingCompleteCallback;
	UINT64 callbackContext;
	UINT64 channelOpenedCallback;
	UINT64 channelClosedCallback;
	UINT64 channelSuspendCallback;
	UINT64 channelStartedCallback;
	UINT64 channelPostStartedCallback;
	wchar_t name[64];
	UINT64 pointer;
}VMBusChannelData;

typedef struct VMBusInteceptConf
{
	UINT64 channel;
	UINT32 count;
	UINT8 logToDebug;
	UINT8 logToDebugAndBreak;
	UINT8 logToFile;
}VMBusInteceptConf;

typedef struct VMBusFuzzConf
{
	UINT64 channel;
	UINT32 count, minChanges, maxChanges;
	UINT8 main, mdl;
}VMBusFuzzConf;