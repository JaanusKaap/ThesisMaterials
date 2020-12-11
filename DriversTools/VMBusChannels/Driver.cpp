#include<ntddk.h>
#include<ntstatus.h>
#include "VMBusChannel.h"
#include "IOCTLs.h"

#define DEVICE_NAME L"\\Device\\VMBusIntercept"
#define SYMBOLIC_LINK_NAME L"\\DosDevices\\VMBusIntercept"

VMBusChannel vmBusObject;
NTSTATUS DriverIoControl(PDEVICE_OBJECT DeviceObject, PIRP Irp);
NTSTATUS TracedrvDispatchOpenClose(IN PDEVICE_OBJECT pDO, IN PIRP Irp);
VOID DriverUnload(PDRIVER_OBJECT  DriverObject);

extern "C" NTSTATUS DriverEntry(PDRIVER_OBJECT pDriverObject, PUNICODE_STRING pRegistryPath)
{
	UNREFERENCED_PARAMETER(pRegistryPath);

	NTSTATUS NtStatus = STATUS_SUCCESS;
	PDEVICE_OBJECT pDeviceObject = NULL;
	UNICODE_STRING usDriverName, usDosDeviceName;

	pDriverObject->MajorFunction[IRP_MJ_CLOSE] = TracedrvDispatchOpenClose;
	pDriverObject->MajorFunction[IRP_MJ_CREATE] = TracedrvDispatchOpenClose;
	pDriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = DriverIoControl;

	RtlInitUnicodeString(&usDriverName, DEVICE_NAME);
	RtlInitUnicodeString(&usDosDeviceName, SYMBOLIC_LINK_NAME);
	NtStatus = IoCreateDevice(pDriverObject, 0, &usDriverName, FILE_DEVICE_UNKNOWN, FILE_DEVICE_SECURE_OPEN, FALSE, &pDeviceObject);
	if (NtStatus == STATUS_SUCCESS)
	{
		IoCreateSymbolicLink(&usDosDeviceName, &usDriverName);

		pDriverObject->DriverUnload = DriverUnload;
	}
	return NtStatus;
}

NTSTATUS TracedrvDispatchOpenClose(IN PDEVICE_OBJECT pDO, IN PIRP Irp)
{
	UNREFERENCED_PARAMETER(pDO);

	Irp->IoStatus.Status = STATUS_SUCCESS;
	Irp->IoStatus.Information = 0;

	PAGED_CODE();

	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return STATUS_SUCCESS;
}

VOID DriverUnload(PDRIVER_OBJECT  DriverObject)
{
	UNICODE_STRING usDosDeviceName;
	RtlInitUnicodeString(&usDosDeviceName, SYMBOLIC_LINK_NAME);
	IoDeleteSymbolicLink(&usDosDeviceName);
	IoDeleteDevice(DriverObject->DeviceObject);
}

NTSTATUS DriverIoControl(PDEVICE_OBJECT DeviceObject, PIRP Irp)
{
	UNREFERENCED_PARAMETER(DeviceObject);
	NTSTATUS NtStatus = STATUS_NOT_SUPPORTED;
	PIO_STACK_LOCATION pIoStackIrp = IoGetCurrentIrpStackLocation(Irp);
	ULONGLONG dataLen = 0;

	if (pIoStackIrp)
	{
#ifdef _DEBUG
		DbgPrint("DriverIoControl called with control code 0x%X\n", pIoStackIrp->Parameters.DeviceIoControl.IoControlCode);
#endif
		switch (pIoStackIrp->Parameters.DeviceIoControl.IoControlCode)
		{
		case IOCTL_GET_CHANNELS_COUNT:
			if (pIoStackIrp->Parameters.DeviceIoControl.OutputBufferLength < sizeof(UINT32))
			{
				NtStatus = STATUS_BUFFER_OVERFLOW;
				break;
			}
			*(UINT32*)Irp->AssociatedIrp.SystemBuffer = vmBusObject.getChannelCount();
			NtStatus = STATUS_SUCCESS;
			dataLen = sizeof(UINT32);
			break;
		case IOCTL_GET_CHANNELS_DATA:
			UINT32 count = vmBusObject.getChannelCount();
			if (pIoStackIrp->Parameters.DeviceIoControl.OutputBufferLength < sizeof(VMBusChannel) * count)
			{
				NtStatus = STATUS_BUFFER_OVERFLOW;
				break;
			}
			memset(Irp->AssociatedIrp.SystemBuffer, 0, pIoStackIrp->Parameters.DeviceIoControl.OutputBufferLength);
			VMBusChannelData* data = (VMBusChannelData*)Irp->AssociatedIrp.SystemBuffer;
			void* ptr = vmBusObject.getFirstChannel();
			UINT32 x;
			for (x = 0; x < count; x++)
			{
				if (ptr == NULL)
					break;
				vmBusObject.getChannelData(ptr, data + x);
				ptr = vmBusObject.getNextChannel(ptr);
			}
			NtStatus = STATUS_SUCCESS;
			dataLen = sizeof(VMBusChannel) * x;
			break;
		}
	}

	Irp->IoStatus.Status = NtStatus;
	Irp->IoStatus.Information = dataLen;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);

	return NtStatus;
}