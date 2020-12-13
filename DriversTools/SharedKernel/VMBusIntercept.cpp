#include "VMBusIntercept.h"
#include "CodeFromOthers.h"
#include "conf.h"


void *VMBusIntercept::hookedChannel = NULL;
bool VMBusIntercept::logToFile = false, VMBusIntercept::logToDebug = false, VMBusIntercept::logToDebugAndBreak = false;
UINT32 VMBusIntercept::count = 0;
HANDLE VMBusIntercept::logHandle = NULL;
ChannelHandlerFunction VMBusIntercept::originalHandler = NULL;
EVT_VMB_PACKET_GET_EXTERNAL_DATA* VMBusIntercept::VmbChannelPacketGetExternalData = NULL;

void VMBusIntercept::setInt64ToChannel(void* channel, UINT32 offset, UINT64 value)
{
	*(UINT64*)(((UINT8*)channel) + offset) = value;
}

bool VMBusIntercept::hook(VMBusInteceptConf* conf)
{
	void* addr = (void*)conf->channel;
	if (addr != *(void**)addr)
		return false;
	if (hookedChannel)
		return false;

	if (VmbChannelPacketGetExternalData == NULL)
	{
		void* driver = (PUINT8)KernelGetModuleBase("vmbkmclr.sys");
		if (!driver)
			return false;
		VmbChannelPacketGetExternalData = (EVT_VMB_PACKET_GET_EXTERNAL_DATA*)KernelGetProcAddress(driver, "VmbChannelPacketGetExternalData");
		if (!VmbChannelPacketGetExternalData)
			return false;
	}

	hookedChannel = addr;
	count = conf->count;
	logToFile = (conf->logToFile & 1) > 0;
	logToDebug = (conf->logToDebug & 1) > 0;
	logToDebugAndBreak = (conf->logToDebugAndBreak & 1) > 0;
	originalHandler = (ChannelHandlerFunction)getInt64FromChannel(addr, VMBUS_CHANNEL_OFFSET_PROCESS_PACKET_CALLBACK);
	setInt64ToChannel(addr, VMBUS_CHANNEL_OFFSET_PROCESS_PACKET_CALLBACK, (UINT64)&hookHandler);
	return true;
}

bool VMBusIntercept::unhook()
{
	if (logHandle)
	{
		ZwClose(logHandle);
		logHandle = NULL;
	}
	if (!hookedChannel)
		return true;
	setInt64ToChannel(hookedChannel, VMBUS_CHANNEL_OFFSET_PROCESS_PACKET_CALLBACK, (UINT64)originalHandler);
	hookedChannel = NULL;
	originalHandler = NULL;
	return true;
}

bool VMBusIntercept::setFilename(char* filename)
{
	UNICODE_STRING uniName;
	ANSI_STRING AS;
	RtlInitAnsiString(&AS, filename);
	RtlAnsiStringToUnicodeString(&uniName, &AS, TRUE);
	OBJECT_ATTRIBUTES  objAttr;
	InitializeObjectAttributes(&objAttr, &uniName, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, NULL, NULL);
	IO_STATUS_BLOCK    ioStatusBlock;
	NTSTATUS ntstatus = ZwCreateFile(&logHandle, GENERIC_WRITE, &objAttr, &ioStatusBlock, NULL, FILE_ATTRIBUTE_NORMAL, 0, FILE_OVERWRITE_IF, FILE_SYNCHRONOUS_IO_NONALERT, NULL, 0);
	if (NT_SUCCESS(ntstatus))
	{
		ZwWriteFile(logHandle, NULL, NULL, NULL, &ioStatusBlock, "VMBS", 4, NULL, NULL);
		return true;
	}
	logHandle = NULL;
	return false;
}

void VMBusIntercept::hookHandler(void* channel, void* packet, void* buffer, UINT32 bufferLen, UINT32 flags)
{
	if (count && logToDebug)
	{
		if (flags & 0x1)
		{
			PMDL mdl = NULL;
			if (VmbChannelPacketGetExternalData(packet, 0, &mdl) != STATUS_SUCCESS)
				mdl = NULL;
			if (mdl)
			{
				void* ptr = MmGetSystemAddressForMdlSafe(mdl, NormalPagePriority);
				UINT32 len = MmGetMdlByteCount(mdl);
				DbgPrint("Intercepted packet to channel 0x%llX, with 0x%X sized buffer at 0x%llX and external data at 0x%llX with size 0x%X\n", channel, bufferLen, buffer, ptr, len);
			}
			else
			{
				DbgPrint("Intercepted packet to channel 0x%llX, with 0x%X sized buffer at 0x%llX and could not get external data\n", channel, bufferLen, buffer);
			}
		}
		else
		{
			DbgPrint("Intercepted packet to channel 0x%llX, with 0x%X sized buffer at 0x%llX and no external data\n", channel, bufferLen, buffer);
		}
		if(logToDebugAndBreak)
			DbgBreakPoint();
	}
	if (count && logToFile)
	{
		PVMBUS_CHANNEL_RECORD_DATA data = (PVMBUS_CHANNEL_RECORD_DATA)ExAllocatePoolWithTag(NonPagedPoolNx, sizeof(VMBUS_CHANNEL_RECORD_DATA), 'VMbs');
		data->WorkItem = (PWORK_QUEUE_ITEM)ExAllocatePoolWithTag(NonPagedPoolNx, sizeof(WORK_QUEUE_ITEM), 0x76687668);
		data->buffer = ExAllocatePoolWithTag(NonPagedPoolNx, bufferLen, 'VMbs');
		data->bufferLen = bufferLen;
		memcpy(data->buffer, buffer, bufferLen);
		data->external = NULL;
		data->externalLen = 0;
		if (flags & 0x1)
		{
			PMDL mdl = NULL;
			if (VmbChannelPacketGetExternalData(packet, 0, &mdl) == STATUS_SUCCESS)
			{
				data->externalLen = MmGetMdlByteCount(mdl);
				data->external = ExAllocatePoolWithTag(NonPagedPoolNx, data->externalLen, 'VMbs');
				memcpy(data->external, MmGetSystemAddressForMdlSafe(mdl, NormalPagePriority), data->externalLen);
			}
		}
		ExInitializeWorkItem(data->WorkItem, fileLogRoutine, data);
		ExQueueWorkItem(data->WorkItem, DelayedWorkQueue);
	}
	if (count)
		count--;
	return originalHandler(channel, packet, buffer, bufferLen, flags);
}

void VMBusIntercept::fileLogRoutine(PVOID Parameter)
{
	PVMBUS_CHANNEL_RECORD_DATA data = (PVMBUS_CHANNEL_RECORD_DATA)Parameter;
	if (logHandle)
	{
		IO_STATUS_BLOCK ioStatusBlock;
		ZwWriteFile(logHandle, NULL, NULL, NULL, &ioStatusBlock, &(data->bufferLen), sizeof(data->bufferLen), NULL, NULL);
		ZwWriteFile(logHandle, NULL, NULL, NULL, &ioStatusBlock, data->buffer, data->bufferLen, NULL, NULL);
		ZwWriteFile(logHandle, NULL, NULL, NULL, &ioStatusBlock, &(data->externalLen), sizeof(data->externalLen), NULL, NULL);
		if(data->externalLen)
			ZwWriteFile(logHandle, NULL, NULL, NULL, &ioStatusBlock, data->external, data->externalLen, NULL, NULL);
		ExFreePool(data->WorkItem);
		ExFreePool(data->buffer);
		if (data->externalLen)
			ExFreePool(data->external);
		ExFreePool(data);
	}
}