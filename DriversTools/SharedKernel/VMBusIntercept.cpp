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
	DbgPrint("conf->logToFile = 0x%X\n", conf->logToFile);
	DbgPrint("conf->logToDebug = 0x%X\n", conf->logToDebug);
	DbgPrint("conf->logToDebugAndBreak = 0x%X\n", conf->logToDebugAndBreak);
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
	if (count)
		count--;
	return originalHandler(channel, packet, buffer, bufferLen, flags);
}