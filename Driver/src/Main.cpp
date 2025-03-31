#include <ntifs.h>
#include "Driver.h"


extern "C" {
	NTKERNELAPI NTSTATUS IoCreateDriver(
		PUNICODE_STRING driver_name,
		PDRIVER_INITIALIZE initialization_function
	);

	// Copies virtual memory between processes.
	NTKERNELAPI NTSTATUS MmCopyVirtualMemory(
		PEPROCESS source_process,
		PVOID source_address,
		PEPROCESS target_process,
		PVOID target_address,
		SIZE_T buffer_size,
		KPROCESSOR_MODE previous_mode,
		PSIZE_T return_size
	);
}


void DebugPrint(PCSTR text) {
#ifndef DEBUG
	UNREFERENCED_PARAMETER(text);
#endif // DEBUG

	KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, text));
}

// Forward declaration for suppressing code analysis warnings.
DRIVER_INITIALIZE DriverEntry;


// IRP Major Functions
NTSTATUS Create(PDEVICE_OBJECT device_object, PIRP irp) {
	UNREFERENCED_PARAMETER(device_object);

	irp->IoStatus.Status = STATUS_SUCCESS;
	irp->IoStatus.Information = 0;

	IoCompleteRequest(irp, IO_NO_INCREMENT);

	return irp->IoStatus.Status;
}

NTSTATUS Close(PDEVICE_OBJECT device_object, PIRP irp) {
	UNREFERENCED_PARAMETER(device_object);

	irp->IoStatus.Status = STATUS_SUCCESS;
	irp->IoStatus.Information = 0;

	IoCompleteRequest(irp, IO_NO_INCREMENT);

	return irp->IoStatus.Status;
}

NTSTATUS DeviceControl(PDEVICE_OBJECT device_object, PIRP irp) {
	UNREFERENCED_PARAMETER(device_object);

	DebugPrint("[+] Device control called.\n");

	NTSTATUS status = STATUS_UNSUCCESSFUL;

	// Stack location of I/O Request Packet (IRP) - Determine which code was passed
	PIO_STACK_LOCATION stack_irp = IoGetCurrentIrpStackLocation(irp);

	// Request object sent from the user
	auto request = reinterpret_cast<Driver::Request*>(irp->AssociatedIrp.SystemBuffer);

	if (stack_irp == nullptr || request == nullptr) {
		IoCompleteRequest(irp, IO_NO_INCREMENT);
		return status;
	}

	// Target Process
	static PEPROCESS target_process = nullptr;

	const ULONG control_code = stack_irp->Parameters.DeviceIoControl.IoControlCode;

	switch (control_code) {

	case Driver::Codes::attach:
		status = PsLookupProcessByProcessId(request->process_id, &target_process);
		break;

	case Driver::Codes::read:
		if (target_process != nullptr)
			status = MmCopyVirtualMemory(target_process, request->target,
				PsGetCurrentProcess(), request->buffer,
				request->size, KernelMode, &request->return_size);
		break;

	case Driver::Codes::write:
		if (target_process != nullptr)
			status = MmCopyVirtualMemory(PsGetCurrentProcess(), request->buffer,
				target_process, request->target,
				request->size, KernelMode, &request->return_size);
		break;

	default:
		break;
	}

	irp->IoStatus.Status = status;
	irp->IoStatus.Information = sizeof(Driver::Request);

	IoCompleteRequest(irp, IO_NO_INCREMENT);

	return status;
}


UNICODE_STRING device_name = {};
UNICODE_STRING symbolic_link = {};


void DriverUnload(IN PDRIVER_OBJECT device_object) {

	IoDeleteSymbolicLink(&symbolic_link);
	IoDeleteDevice(device_object->DeviceObject);

	DebugPrint("[+] Driver unloaded successfully.\n");

}

NTSTATUS DriverMain(IN PDRIVER_OBJECT driver_object, IN PUNICODE_STRING registry_path) {
	UNREFERENCED_PARAMETER(registry_path);

	RtlInitUnicodeString(&device_name, L"\\Device\\Driver"); // Device

	// Create driver device object
	PDEVICE_OBJECT device_object = nullptr;
	NTSTATUS status = IoCreateDevice(driver_object, 0, &device_name, FILE_DEVICE_UNKNOWN,
		FILE_DEVICE_SECURE_OPEN, FALSE, &device_object);

	if (status != STATUS_SUCCESS) {
		DebugPrint("[-] Failed to create driver device.\n");

		return status;
	}

	DebugPrint("[+] Driver device successfully created.\n");

	// Establish Symbolic link
	RtlInitUnicodeString(&symbolic_link, L"\\DosDevices\\Driver"); // DosDevices

	status = IoCreateSymbolicLink(&symbolic_link, &device_name);
	if (status != STATUS_SUCCESS) {
		DebugPrint("[-] Failed to establish symbolic link.\n");
		return status;
	}

	DebugPrint("[+] Driver symbolic link successfully established.\n");

	// Enable buffered I/O for efficient small data transfers between UM and KM.
	SetFlag(device_object->Flags, DO_BUFFERED_IO);

	// Set-up the driver handles
	driver_object->MajorFunction[IRP_MJ_CREATE] = Create;
	driver_object->MajorFunction[IRP_MJ_CLOSE] = Close;
	driver_object->MajorFunction[IRP_MJ_DEVICE_CONTROL] = DeviceControl;
	driver_object->DriverUnload = DriverUnload;

	ClearFlag(device_object->Flags, DO_DEVICE_INITIALIZING);
	DebugPrint("[+] Driver initialized successfully.\n");

	return status;
}

NTSTATUS DriverEntry() {
	DebugPrint("[+] Debuging ... \n");

	UNICODE_STRING DriverName = {};
	RtlInitUnicodeString(&DriverName, L"\\Driver\\Driver"); // Driver

	return IoCreateDriver(&DriverName, &DriverMain);
}
