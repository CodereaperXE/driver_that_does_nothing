#include <ntifs.h>
#include <ntddk.h>
#include <prioritybooster.h>

control code-----------
#define PRIORITY_BOOSTER_DEVICE 0x8000
#define FUNCTION_DEVICE_ONE 0x800
#define IOCTL_PRIORITY_BOOSTER_SET_PRIORITY CTL_CODE(PRIORITY_BOOSTER_DEVICE, FUNCTION_DEVICE_ONE,METHOD_NEITHER,FILE_ANY_ACCESS)

unloader--------------------
void PriorityBoosterUnload(_In_ PDRIVER_OBJECT DriverObject) {
	UNICODE_STRING symlink = RTL_CONSTANT_STRING(L"\\Device\\PrioritBooster");
	IoDeleteSymlink(&symlink);
	IodeleteDevice(DriverOBject->DeviceObject);
}

NTSTATUS PriorityBoosterCreateClose(PDEVICE_OBJECT DeviceObject, PIRP Irp) {
	UNREFERENCED_PARAMETER(DeviceObject);
	Irp->IoStatus,Status=STATUS_SUCCESS;
	Irp->IoStatus.Information=0;
	IocompleteRequest(Irp, IO_NO_INCREMENT);
	return STATUS_SUCCESS;
}

NTSTATUS PriorityBoosterDeviceControl(PDEVICE_OBJECT, DeviceObject, PIRP Irp) {
	auto stack= IoGetCurrentStackLocation(Irp);
	auto status=STATUS_SUCCESS;

	switch (stack->Parameters.DeviceIoControl.IoControlCode) {
	case IOCTL_PRIORITY_BOOSTER_SET_PRIORITY: {
	if (stack->Parameters.DeviceIoControl.InutBufferLength < sizeof(ThreadData)) {
		status=STATUS_BUFFER_TOO_SMALL;
		break;
	}
	auto data = (ThreadData*)stack->Parameters.DeviceIoControl.Type3InputBuffer;
	
	if (data->Priority <1 || data->Priority >31) {
		status = STATUS_INVALID_PARAMETER;
		break;
	}
	PETHREAD Thread;
	status = PsLookupThreadByThreadId(ULongToHandle(data->ThreadId), &Thread);
	if (!NT_SUCCESS(status))
		break;

	KeSetPriorityThread((PKTHREAD)Thread, data->Priority);
	ObDereferernceObject(Thread);
	Kdprint(("Thread priority change for %d to %d succeded!\n", data->TheadId, data->Priority));
	}
	default:
		status=STATUS_INVALID_DEVICE_REQUEST;
		KdPrint(("Wrong IOCTL CODE!\n"));
		break;
}

extern "C" NTSTATUS
DriverEntry(_In_ PDRIVER_OBJECT DriverObject, _In_ PUNICODE_STRING Registry) {
	UNREFERENCED_PARAMETER(DriverObject);
	UNREFERENCED_PARAMETER(RegistryPath);
	DriverObject->DriverUnload;
	DriverObject->MajorFunction[IRP_MJ_CREATE] = PriorityBoosterCreateClose;
	DriverObject->MajorFunction[IRP_MJ_CREATE] = PriorityBoosterCreateClose;
	DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = PriorityBoosterDeviceControl;
	UNICODE devName = RTL_CONSTANT_STRING(L"\\Device\\PriorityBooster");
	PDEVICE_OBJECT DeviceObject;

	NTSTATUS status = IoCreateDevice(
	DriverObject,
	0,
	&devName,
	FILE_DEVICE_UNKNOWN,
	0,
	FALSE,
	&DeviceObject
	);

	if (!NT_SUCCESS(status)) {
		KdPrint(("Failed to create device object (0x%08X)\n", status));
		return status;
	}

	UNICODE_STRING symlink = RTL_CONSTANT_STRING(L"\\??\\PriorityBooster");
	status = IoCreateSymbolicLink(&symlink, &devName);
	if (!NT_SUCCESS(status)) {
		KdPrint(("Failed to create symbolic link (0x%08X)\n", status));
		IoDeleteDevice(DeviceObject);
		return status;

	}
	return STATUS_SUCCESS;
}

