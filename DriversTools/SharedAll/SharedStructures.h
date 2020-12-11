#pragma once
#ifdef _DRIVER
#include<ntddk.h>
#else
#include<windows.h>
#endif

typedef struct VMBusChannel
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
}VMBusChannel;