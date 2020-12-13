#pragma once
#include<ntddk.h>
#include "VMBusChannel.h"
#include "SharedStructures.h"

typedef void (*ChannelHandlerFunction)(void*, void*, void*, UINT32, UINT32);
typedef NTSTATUS EVT_VMB_PACKET_GET_EXTERNAL_DATA(VOID*, UINT32, PMDL*);

typedef struct VMBUS_CHANNEL_RECORD_DATA
{
	PWORK_QUEUE_ITEM WorkItem;
	PVOID buffer, external;
	UINT32 bufferLen, externalLen;
}VMBUS_CHANNEL_RECORD_DATA, * PVMBUS_CHANNEL_RECORD_DATA;

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
	static void fileLogRoutine(PVOID Parameter);

	void setInt64ToChannel(void* channel, UINT32 offset, UINT64 value);
	virtual UINT64 getHookHandler();

public:
	bool hook(VMBusInteceptConf*);
	bool unhook();
	bool setFilename(char* filename);
};