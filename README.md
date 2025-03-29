# Last - Kernel Driver

## Overview
A kernel-mode driver designed for low-level interaction with a game, providing essential functionalities such as attaching to a target process, reading virtual memory, and writing virtual memory.

## Download and Setup
Install Visual Studio 2022: [Download Link](https://visualstudio.microsoft.com/downloads/)
  -  Install Desktop Development with C++
  -  Install the Individual components called `MSVC v143 - VS 2022 C++ x64/x86 Spectre-mitigated libs (Latest)`

Install Windows SDK: [Download Link](https://developer.microsoft.com/en-us/windows/downloads/windows-sdk/) 

Install Kdmapper [GitHub Repository](https://github.com/TheCruZ/kdmapper)

Install Windows Driver Kit (WDK): [Download Link](https://learn.microsoft.com/en-us/windows-hardware/drivers/download-the-wdk)

**Useful Tools**: WinGdb and DriverView

To download the source code: 
1. Open PowerShell and navigate to the directory where you want to save the source code.
2. Clone the repository by running: `git clone https://github.com/Joelfm4/Kernel-Driver.git`
3. Open Visual Studio via PowerShell: `Start-Process "devenv.exe" -ArgumentList Application.sln` Alternatively, open Visual Studio, choose Open a local folder, and select the folder where you downloaded the project source.

> [!CAUTION]
> It is highly recommended to test in a virtual machine and use WinDbg to handle debug messages, ensuring safety and preventing any potential harm to your primary system.

### Execution Order
1. Drag and drop the `Driver.sys` into kdmapper.exe
2. Start the **target application**.
3. Start the Application.exe

## How Everything Works (Overview)

### Kernel Driver

#### Driver Functions
- `DebugPrint()`: Prints diagnostic messages for debugging purposes.
- `DriverEntry()`: Initializes the kernel driver when loaded.
- `DriverMain()`: The "real" Entry point for the driver (read note).
- `Unload Driver()`: Handles cleanup when the driver is unloaded.
- `Create()`: Handles opening a connection to the driver. 
- `Close()`: Handles closing a connection to the driver.
- `DeviceControl()`: : Processes user-mode requests for memory operations.

#### IRP Major Functions
- Create
  - Defines an IRP (I/O Request Packet) handler for IRP_MJ_CREATE.
- Close
  - Defines an IRP (I/O Request Packet) handler for IRP_MJ_CLOSE.
- DeviceControl
  - Defines an IRP (I/O Request Packet) handler for IRP_MJ_DEVICE_CONTROL

#### Process flow
- Driver Entry
  - The first driver-supplied routine that is called after a driver is loaded.
  - Responsible for initializing the driver.
  - Called by KDMapper, which then returns `IoCreateDriver()`, creating a driver with DriverMain as the entry point.
- DriverMain
  - The `registry_path` is unreferenced because it is not used, and in driver development, warnings are treated as errors.
  - Creates the `device_object`, which represents a logical device for which the driver handles I/O requests. [Documentation](https://learn.microsoft.com/en-us/windows-hardware/drivers/ddi/wdm/ns-wdm-_device_object)
  - `IoCreateSymbolicLink` creates a symbolic link between the device_object **name** and the a user-visible **name** for the device. [Documentation](https://learn.microsoft.com/en-us/windows-hardware/drivers/ddi/wdm/nf-wdm-iocreatesymboliclink)
  - `SetFlag(device_object->Flags, DO_BUFFERED_IO)` enables buffered I/O for efficient small data transfers between user mode (application) and kernel mode (driver). [Documentation](https://learn.microsoft.com/en-us/windows-hardware/drivers/kernel/using-buffered-i-o)
  - Set-up the driver handlers for `driver_object`
    - Major Function Code [Documentation](https://learn.microsoft.com/en-us/windows-hardware/drivers/kernel/irp-major-function-codes)
	  - Unload Routine `driver_object->DriverUnload = DriverUnload;` [Documentation](https://learn.microsoft.com/en-us/windows-hardware/drivers/ddi/wdm/nc-wdm-driver_unload)
- `ClearFlag(device_object->Flags, DO_DEVICE_INITIALIZING);` – Clears the DO_DEVICE_INITIALIZING flag, indicating that the device object has finished initialization and is ready for I/O operations.


### Application 

#### Process flow to read/write memory from a process:
1. Attach the driver.
2. Instantiate `MemoryManager` class
3. Get the Process ID (PID) of the target process.
4. Attach process to the Driver.
5. Obtain the base address of the target module (e.g `client.dll`).
6. Use the `ReadMemory` and `WriteMemory` methods from `MemoryManager` class to read and write memory.

## Memory Management Class (`memory.hpp`)
- `ReadMemory` Method to read memory from a process.
- `WriteMemory` Method to write memory of process.
- `AttachToProcess` Method responsible to attach the driver to a process.
- `GetProcessID` Method responsible to get the process ID (PID) of a given process.
- `GetModuleBase` Method responsible to get the base module of a given process.

## IOCTL Codes (Driver.hpp)
The driver responds to the following control codes:
- `attach`: Attaches to a target process.
- `read`: Reads memory from the target process.
- `write`: Writes memory to the target process.
  
---

**Note**: DriverEntry returns to DriverMain because KDMapper manually maps the driver into memory. Since the operating system is unaware of the driver being loaded (as it was injected manually by KDMapper), it does not pass the DriverObject and RegistryPath parameters to the driver, which would normally be provided during a typical driver load process.

