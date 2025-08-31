#include <ntifs.h>

#include "Driver.h"


extern "C" {
	NTKERNELAPI NTSTATUS IoCreateDriver(
		PUNICODE_STRING DriverName,
		PDRIVER_INITIALIZE InitializationFunction
	);

	// Copies virtual memory between processes.
	NTKERNELAPI NTSTATUS MmCopyVirtualMemory(
		PEPROCESS SourceProcess,
		PVOID SourceAddress,
		PEPROCESS TargetProcess,
		PVOID TargetAddress,
		SIZE_T BufferSize,
		KPROCESSOR_MODE PreviousMode,
		PSIZE_T ReturnSize
	);
}


void DebugPrint(PCSTR Text) {

#ifndef DEBUG
	UNREFERENCED_PARAMETER(Text);
#endif // DEBUG

	KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, Text));
}


// Forward declaration for suppressing code analysis warnings.
DRIVER_INITIALIZE DriverEntry;


// IRP Major Functions
NTSTATUS Create(PDEVICE_OBJECT DeviceObject, PIRP Irp) {
	UNREFERENCED_PARAMETER(DeviceObject);

	Irp->IoStatus.Status = STATUS_SUCCESS;
	Irp->IoStatus.Information = 0;

	IoCompleteRequest(Irp, IO_NO_INCREMENT);

	return Irp->IoStatus.Status;
}


NTSTATUS Close(PDEVICE_OBJECT DeviceObject, PIRP Irp) {
	UNREFERENCED_PARAMETER(DeviceObject);

	Irp->IoStatus.Status = STATUS_SUCCESS;
	Irp->IoStatus.Information = 0;

	IoCompleteRequest(Irp, IO_NO_INCREMENT);

	return Irp->IoStatus.Status;
}


NTSTATUS DeviceControl(PDEVICE_OBJECT DeviceObject, PIRP Irp) {
	UNREFERENCED_PARAMETER(DeviceObject);

	DebugPrint("[+] Device control called.\n");

	NTSTATUS Status = STATUS_UNSUCCESSFUL;

	// Stack location of I/O Request Packet (IRP) - Determine which code was passed
	PIO_STACK_LOCATION StackIrp = IoGetCurrentIrpStackLocation(Irp);

	auto Request = reinterpret_cast<Driver::Request*>(Irp->AssociatedIrp.SystemBuffer);

	if (StackIrp == nullptr || Request == nullptr) {
		IoCompleteRequest(Irp, IO_NO_INCREMENT);
		return Status;
	}

	static PEPROCESS TargetProcess = nullptr;

	const ULONG control_code = StackIrp->Parameters.DeviceIoControl.IoControlCode;

	switch (control_code) {

	case Driver::Codes::Attach:
		Status = PsLookupProcessByProcessId(Request->ProcessID, &TargetProcess);
		break;

	case Driver::Codes::Read:
		if (TargetProcess != nullptr)
			Status = MmCopyVirtualMemory(TargetProcess, Request->Target,
				PsGetCurrentProcess(), Request->Buffer,
				Request->Size, KernelMode, &Request->ReturnSize);
		break;

	case Driver::Codes::Write:
		if (TargetProcess != nullptr)
			Status = MmCopyVirtualMemory(PsGetCurrentProcess(), Request->Buffer,
				TargetProcess, Request->Target,
				Request->Size, KernelMode, &Request->ReturnSize);
		break;

	default:
		break;
	}

	Irp->IoStatus.Status = Status;
	Irp->IoStatus.Information = sizeof(Driver::Request);

	IoCompleteRequest(Irp, IO_NO_INCREMENT);

	return Status;
}


UNICODE_STRING DeviceName = {};
UNICODE_STRING SymbolicLink = {};


void DriverUnload(IN PDRIVER_OBJECT DeviceObject) {

	IoDeleteSymbolicLink(&SymbolicLink);
	IoDeleteDevice(DeviceObject->DeviceObject);

	DebugPrint("[+] Driver unloaded successfully.\n");

}


NTSTATUS DriverMain(IN PDRIVER_OBJECT DriverObject, IN PUNICODE_STRING RegistryPath) {
	UNREFERENCED_PARAMETER(RegistryPath);

	RtlInitUnicodeString(&DeviceName, L"\\Device\\Driver");

	PDEVICE_OBJECT DeviceObject = nullptr;
	NTSTATUS Status = IoCreateDevice(DriverObject, 0, &DeviceName, FILE_DEVICE_UNKNOWN,
		FILE_DEVICE_SECURE_OPEN, FALSE, &DeviceObject);

	if (Status != STATUS_SUCCESS) {
		DebugPrint("[-] Failed to create driver device.\n");

		return Status;
	}

	DebugPrint("[+] Driver device successfully created.\n");

	RtlInitUnicodeString(&SymbolicLink, L"\\DosDevices\\Driver");

	Status = IoCreateSymbolicLink(&SymbolicLink, &DeviceName);
	if (Status != STATUS_SUCCESS) {
		DebugPrint("[-] Failed to establish symbolic link.\n");
		return Status;
	}

	DebugPrint("[+] Driver symbolic link successfully established.\n");

	SetFlag(DeviceObject->Flags, DO_BUFFERED_IO);

	// Set-up the driver handles
	DriverObject->MajorFunction[IRP_MJ_CREATE] = Create;
	DriverObject->MajorFunction[IRP_MJ_CLOSE] = Close;
	DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = DeviceControl;
	DriverObject->DriverUnload = DriverUnload;

	ClearFlag(DeviceObject->Flags, DO_DEVICE_INITIALIZING);
	DebugPrint("[+] Driver initialized successfully.\n");

	return Status;
}


NTSTATUS DriverEntry() {
	DebugPrint("[+] Debuging ... \n");

	UNICODE_STRING DriverName = {};
	RtlInitUnicodeString(&DriverName, L"\\Driver\\Driver");

	return IoCreateDriver(&DriverName, &DriverMain);
}
