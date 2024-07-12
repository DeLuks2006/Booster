#include <ntifs.h>
#include <ntddk.h>
#include "BoosterCommon.h"

/*---------[ Prototypes ]---------*/
void BoosterUnload(PDRIVER_OBJECT DriverObject);
NTSTATUS BoosterCreateClose(PDEVICE_OBJECT DeviceObject, PIRP Irp);
NTSTATUS BoosterDeviceControl(PDEVICE_OBJECT DeviceObject, PIRP Irp);

/*---------[ Driver Entry ]---------*/
extern "C" NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath) {
    UNREFERENCED_PARAMETER(RegistryPath);

    KdPrint(("Booster DriverEntry started\n"));

    DriverObject->DriverUnload = BoosterUnload;
    DriverObject->MajorFunction[IRP_MJ_CREATE] = BoosterCreateClose;
    DriverObject->MajorFunction[IRP_MJ_CLOSE] = BoosterCreateClose;
    DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = BoosterDeviceControl;

    UNICODE_STRING devName = RTL_CONSTANT_STRING(L"\\Device\\Booster");
    PDEVICE_OBJECT DeviceObject;
    NTSTATUS status = IoCreateDevice(
        DriverObject,        // driver object
        0,                   // no extra bytes
        &devName,            // device name 
        FILE_DEVICE_UNKNOWN, // device type
        0,                   // characteristics flags
        FALSE,               // not exclusive
        &DeviceObject        // resulting pointer
    );
    if (!NT_SUCCESS(status)) {
        KdPrint(("[x] Failed to create device object (0x%08X)\n", status));
        return status;
    }
    KdPrint(("[#] Device object created successfully\n"));

    UNICODE_STRING symLink = RTL_CONSTANT_STRING(L"\\??\\Booster");
    status = IoCreateSymbolicLink(&symLink, &devName);
    if (!NT_SUCCESS(status)) {
        KdPrint(("[x] Failed to create symbolic link (0x%08X)\n", status));
        IoDeleteDevice(DeviceObject);    // IMPORTANT BECAUSE OF THE CLEANUP
        return status;
    }
    KdPrint(("[#] Symbolic link created successfully\n"));

    return STATUS_SUCCESS;
}

/*---------[ Unload Routine ]---------*/
void BoosterUnload(PDRIVER_OBJECT DriverObject) {
    UNICODE_STRING symLink = RTL_CONSTANT_STRING(L"\\??\\Booster");
    // delete symbolic link
    IoDeleteSymbolicLink(&symLink);
    // delete device object
    IoDeleteDevice(DriverObject->DeviceObject);
    KdPrint(("Booster unloaded\n"));
}

NTSTATUS BoosterCreateClose(PDEVICE_OBJECT DeviceObject, PIRP Irp) {
    UNREFERENCED_PARAMETER(DeviceObject);

    Irp->IoStatus.Status = STATUS_SUCCESS;
    Irp->IoStatus.Information = 0;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return STATUS_SUCCESS;
}

NTSTATUS BoosterDeviceControl(PDEVICE_OBJECT DeviceObject, PIRP Irp) {
    UNREFERENCED_PARAMETER(DeviceObject);
    auto status = STATUS_SUCCESS;
    ULONG_PTR information = 0; // track used bytes

    // irpSp is of type PIO_STACK_LOCATION
    auto irpSp = IoGetCurrentIrpStackLocation(Irp);

    switch (irpSp->Parameters.DeviceIoControl.IoControlCode) {
    case IOCTL_PRIORITY_BOOSTER_SET_PRIORITY: {
        if (irpSp->Parameters.DeviceIoControl.InputBufferLength < sizeof(ThreadData)) {
            status = STATUS_BUFFER_TOO_SMALL;
            break;
        }
        auto data = static_cast<ThreadData*>(irpSp->Parameters.DeviceIoControl.Type3InputBuffer);
        if (data == nullptr || data->Priority < 1 || data->Priority > 31) {
            status = STATUS_INVALID_PARAMETER;
            break;
        }

        PETHREAD thread;
        status = PsLookupThreadByThreadId((HANDLE)(data->ThreadId), &thread);
        if (!NT_SUCCESS(status)) {
            break;
        }

        auto oldPriority = KeSetPriorityThread(thread, data->Priority);
        KdPrint(("[#] Priority change for thread %u from %d to %d succeeded!\n", data->ThreadId, oldPriority, data->Priority));

        ObDereferenceObject(thread);
        information = sizeof(ThreadData);
        break;
    }
    default:
        status = STATUS_INVALID_DEVICE_REQUEST;
        break;
    }

    Irp->IoStatus.Status = status;
    Irp->IoStatus.Information = information;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return status;
}
