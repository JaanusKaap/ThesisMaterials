#pragma once
#include<ntddk.h>
#include "VMBusChannel.h"
#include "SharedStructures.h"

typedef void (*ChannelHandlerFunction)(void*, void*, void*, UINT32, UINT32);
typedef NTSTATUS EVT_VMB_PACKET_GET_EXTERNAL_DATA(VOID*, UINT32, PMDL*);

class VMBusIntercept : public VMBusChannel
{
protected:
	static EVT_VMB_PACKET_GET_EXTERNAL_DATA* VmbChannelPacketGetExternalData;
	
	static void *hookedChannel;
	static bool logToFile, logToDebug, logToDebugAndBreak;
	static UINT32 count;
	static HANDLE logHandle;
	static ChannelHandlerFunction originalHandler;

	static void hookHandler(void* channel, void* packet, void* buffer, UINT32 bufferLen, UINT32 flags);

	void setInt64ToChannel(void* channel, UINT32 offset, UINT64 value);

public:
	bool hook(VMBusInteceptConf*);
	bool unhook();
	bool setFilename(char* filename);
};