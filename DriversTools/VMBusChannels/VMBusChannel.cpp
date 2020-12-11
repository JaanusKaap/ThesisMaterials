#include "VMBusChannel.h"
#include "CodeFromOthers.h"
#include "conf.h"

void* getKmclChannelListLocation()
{
	char* ptr = (char*)KernelGetModuleBase("vmbkmclr.sys");
	ptr += VMBUS_OFFSET_FROM_VMBKMCLR_BASE;
	return ptr;
}

void* getFirstChannel()
{
	void* ptr = getKmclChannelListLocation();
	if (ptr == *((void**)ptr))
		return NULL;
	ptr = (*((char**)ptr));
	return getChannelFromLinkListPtr(ptr);
}

void* getNextChannel(void* ptr)
{
	ptr = *(void**)(((char*)ptr) + VMBUS_CHANNEL_OFFSET_LINKED_LIST);
	if (ptr == getKmclChannelListLocation())
		return NULL;
	return getChannelFromLinkListPtr(ptr);
}

void* getChannelFromLinkListPtr(void* ptr)
{
	return (((char*)ptr) - VMBUS_CHANNEL_OFFSET_LINKED_LIST);
}


UINT8 getInt8FromChannel(void* channel, UINT32 offset)
{
	return *(((UINT8*)channel) + offset);
}

UINT32 getInt32FromChannel(void* channel, UINT32 offset)
{
	return *(UINT32*)(((UINT8*)channel) + offset);
}

UINT64 getInt64FromChannel(void* channel, UINT32 offset)
{
	return *(UINT64*)(((UINT8*)channel) + offset);
}

void getBufferFromChannel(void* channel, UINT32 offset, void* buffer, UINT32 size)
{
	memcpy(buffer, ((UINT8*)channel) + offset, size);
}

UINT32 getChannelCount()
{
	UINT32 count = 0;
	for (void* ptr = getFirstChannel(); ptr; ptr = getNextChannel(ptr))
		count++;
	return count;

}

bool getChannelData(void* channel, VMBusChannel* data)
{
	data->baseAddress = (UINT64)channel;
	data->maxPacketCount = getInt32FromChannel(channel, VMBUS_CHANNEL_OFFSET_MAX_PACKET_COUNT);
	data->maxPacketSize = getInt32FromChannel(channel, VMBUS_CHANNEL_OFFSET_MAX_PACKET_SIZE);
	data->clientContextSize = getInt32FromChannel(channel, VMBUS_CHANNEL_OFFSET_CLIENT_CONTEXT_SIZE);
	getBufferFromChannel(channel, VMBUS_CHANNEL_OFFSET_INTERFACE_TYPE, &data->interfaceType, sizeof(GUID));
	getBufferFromChannel(channel, VMBUS_CHANNEL_OFFSET_INTERFACE_INSTANCE, &data->interfaceInstance, sizeof(GUID));
	data->isPipe = getInt8FromChannel(channel, VMBUS_CHANNEL_OFFSET_PIPE_FLAG);
	getBufferFromChannel(channel, VMBUS_CHANNEL_OFFSET_VM_ID, &data->vmId, sizeof(GUID));
	data->vtl = getInt8FromChannel(channel, VMBUS_CHANNEL_OFFSET_VTL);
	data->processPacketCallback = getInt64FromChannel(channel, VMBUS_CHANNEL_OFFSET_PROCESS_PACKET_CALLBACK);
	data->processingCompleteCallback = getInt64FromChannel(channel, VMBUS_CHANNEL_OFFSET_PROCESSING_COMPLETE_CALLBACK);
	data->callbackContext = getInt64FromChannel(channel, VMBUS_CHANNEL_OFFSET_CALLBACK_CONTEXT);
	data->channelOpenedCallback = getInt64FromChannel(channel, VMBUS_CHANNEL_OFFSET_CHANNEL_OPENED_CALLBACK);
	data->channelClosedCallback = getInt64FromChannel(channel, VMBUS_CHANNEL_OFFSET_CHANNEL_CLOSED_CALLBACK);
	data->channelSuspendCallback = getInt64FromChannel(channel, VMBUS_CHANNEL_OFFSET_CHANNEL_SUSPENDED_CALLBACK);
	data->channelStartedCallback = getInt64FromChannel(channel, VMBUS_CHANNEL_OFFSET_CHANNEL_STARTED_CALLBACK);
	data->channelPostStartedCallback = getInt64FromChannel(channel, VMBUS_CHANNEL_OFFSET_CHANNEL_POS_STARTED_CALLBACK);
	data->pointer = getInt64FromChannel(channel, VMBUS_CHANNEL_OFFSET_POINTER);
	return true;
}