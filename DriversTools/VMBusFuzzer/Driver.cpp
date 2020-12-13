#include<ntddk.h>
#include<ntstatus.h>
#include "VMBusFuzzer.h"
#include "IOCTLs.h"

#define DEVICE_NAME L"\\Device\\VMBusFuzzer"
#define SYMBOLIC_LINK_NAME L"\\DosDevices\\VMBusFuzzer"

VMBusFuzzer vmBusObject;
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
	vmBusObject.unhook();
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
		switch (pIoStackIrp->Parameters.DeviceIoControl.IoControlCode)
		{
		case IOCTL_FUZZER_FUZZ:
		{
			if (pIoStackIrp->Parameters.DeviceIoControl.InputBufferLength < sizeof(VMBusFuzzConf))
			{
				NtStatus = STATUS_NDIS_INVALID_LENGTH;
				break;
			}
			VMBusFuzzConf* conf = (VMBusFuzzConf*)Irp->AssociatedIrp.SystemBuffer;
			if (vmBusObject.fuzz(conf))
				NtStatus = STATUS_SUCCESS;
			else
				NtStatus = STATUS_UNSUCCESSFUL;
			break;
		}
		}

	}

	Irp->IoStatus.Status = NtStatus;
	Irp->IoStatus.Information = dataLen;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return NtStatus;
}