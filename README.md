## Overview
A kernel-mode driver that enables low-level interaction with applications.

## Download & Setup
0. Disable secure-boot and go to `Windows Security-> Device Security-> Core Isotation` and disable all options

1. Install Visual Studio 2022: [Download Link](https://visualstudio.microsoft.com/downloads/)
    -  Install Desktop Development with C++
     -  Install the Individual components called `MSVC v143 - VS 2022 C++ x64/x86 Spectre-mitigated libs (Latest)`

2. Install Windows SDK: [Download Link](https://developer.microsoft.com/en-us/windows/downloads/windows-sdk/) 

3. Install Kdmapper [GitHub Repository](https://github.com/TheCruZ/kdmapper)

4. Install Windows Driver Kit (WDK): [Download Link](https://learn.microsoft.com/en-us/windows-hardware/drivers/download-the-wdk)

5. Open PowerShell and navigate to the directory where you want to save the source code.

6. Clone the repository and go inside the directory by running: `git clone https://github.com/Joelfm4/Kernel-Driver.git; Set-Location Kernel-Driver`

7. Open Visual Studio via PowerShell: `Start-Process "devenv.exe" -ArgumentList Application.sln` Alternatively, open Visual Studio, choose Open a local folder, and select the folder where you downloaded the project source.

### Usage
1. Drag and drop the `Driver.sys` into `kdmapper.exe`.
2. Start the target application (e.g. `Notepad.exe`).
3. Start the `Application.exe`.

## Technical Details

### Application
The user-mode application uses the `MemoryManager` class to connect to the kernel driver and perform memory operations on a target process.
#### Process Flow
1. Instantiate the MemoryManager class with the target process name: `MemoryManager Memory(L"TargetProcess.exe");`

   The MemoryManager class constructor performs the following steps:
    - Calls `CreateFileW` to open a handle to the kernel driver.
	- Calls `GetProcessID()` to query the system for the process ID matching the executable name.
	- Calls `AttachToProcess()` to attach the driver to the target process, enabling memory operations.
	- Sets `m_IsAttached = true` on successful attachment.

2. Attach Check & Base Address
	- Checks if attachment succeeded using `IsAttached()`.
	- Retrieves the base address of a module (e.g. `client.dll`) using `GetModuleBase`, which queries the driver for the module’s memory address.

4. Loop Start


### Kernel Driver
The kernel driver processes I/O requests from the user-mode application.
#### Process Flow
1.  Driver Entry
  - Called by KDMapper, which invokes `IoCreateDriver()` to create a driver with `DriverMain` as the entry point.

2.  DriverMain
  - Creates a `DeviceObject`to handle I/O requests. [Documentation](https://learn.microsoft.com/en-us/windows-hardware/drivers/ddi/wdm/ns-wdm-_device_object)

  - Calls `IoCreateSymbolicLink` to create a user-mode-accessible device name. [Documentation](https://learn.microsoft.com/en-us/windows-hardware/drivers/ddi/wdm/nf-wdm-iocreatesymboliclink)

  - Sets `DO_BUFFERED_IO` flag for efficient data transfers. [Documentation](https://learn.microsoft.com/en-us/windows-hardware/drivers/kernel/using-buffered-i-o)

  - Assigns I/O handlers:
    - Major Function Code [Documentation](https://learn.microsoft.com/en-us/windows-hardware/drivers/kernel/irp-major-function-codes)
        ```
        DriverObject->MajorFunction[IRP_MJ_CREATE] = Create;
        DriverObject->MajorFunction[IRP_MJ_CLOSE] = Close;
        DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = DeviceControl;
        ```
	- Sets the unload routine to clean up resources (e.g. delete symbolic link). [Documentation](https://learn.microsoft.com/en-us/windows-hardware/drivers/ddi/wdm/nc-wdm-driver_unload)
        ```
        DriverObject->DriverUnload = DriverUnload;
        ```
- Clears `DO_DEVICE_INITIALIZING` flag to mark the device as ready
  	```
  		ClearFlag(DeviceObject->Flags, DO_DEVICE_INITIALIZING);
   	```


> [!CAUTION]
> This project is currently under development and is still missing some features.
