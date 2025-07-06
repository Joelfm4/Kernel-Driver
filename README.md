# Last - Kernel Driver

## Overview
A kernel-mode driver designed for low-level interaction with a game, providing essential functionalities such as attaching to a target process, reading virtual memory, and writing virtual memory.

## Download and Setup
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

### Execution Order
1. Drag and drop the `Driver.sys` into kdmapper.exe
2. Start the target application.
3. Start the Application.exe

## How Everything Works 
### Application
#### Process Flow (Overview)
1. Instantiate the MemoryManager class with the target process name: 
    ```
    MemoryManager Memory(L"TargetProcess.exe");
    ```

2. The constructor performs the following steps:
    - Opens a handle to the kernel-mode driver (CreateFileW)
    - Retrieves the target process ID via GetProcessID()
    - Attaches the driver to the target process using AttachToProcess()
    - Sets m_IsAttached = true on success


### Kernel Driver
#### Process flow (Overview)
1.  Driver Entry
  - Called by KDMapper, which then returns `IoCreateDriver()`, creating a driver with DriverMain as the entry point.

2.  DriverMain
  - Creates the `DeviceObject`, which represents a logical device for which the driver handles I/O requests. [Documentation](https://learn.microsoft.com/en-us/windows-hardware/drivers/ddi/wdm/ns-wdm-_device_object)

  - `IoCreateSymbolicLink` creates a symbolic link between the DeviceObject name and a user-visible name for the device. [Documentation](https://learn.microsoft.com/en-us/windows-hardware/drivers/ddi/wdm/nf-wdm-iocreatesymboliclink)

  - `SetFlag(DeviceObject->Flags, DO_BUFFERED_IO)` enables buffered I/O for efficient small data transfers between user mode (application) and kernel mode (driver). [Documentation](https://learn.microsoft.com/en-us/windows-hardware/drivers/kernel/using-buffered-i-o)

  - Set-up the driver handlers for `DriverObject`
    - Major Function Code [Documentation](https://learn.microsoft.com/en-us/windows-hardware/drivers/kernel/irp-major-function-codes)
        ```
        DriverObject->MajorFunction[IRP_MJ_CREATE] = Create;
        DriverObject->MajorFunction[IRP_MJ_CLOSE] = Close;
        DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = DeviceControl;
        ```
	- Unload Routine [Documentation](https://learn.microsoft.com/en-us/windows-hardware/drivers/ddi/wdm/nc-wdm-driver_unload)
        ```
        DriverObject->DriverUnload = DriverUnload;
        ```
- `ClearFlag(DeviceObject->Flags, DO_DEVICE_INITIALIZING);` – Clears the DO_DEVICE_INITIALIZING flag, indicating that the device object has finished initialization and is ready for I/O operations.

> [!CAUTION]
> This project is currently under active development and is still missing some features.
