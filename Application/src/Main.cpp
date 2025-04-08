#include <iostream>
#include <Windows.h>
#include <TlHelp32.h>

#include "../util/Memory.h"

int main() {
	// Attach Driver
	const HANDLE driver_handle = CreateFile(L"\\\\.\\Driver", GENERIC_READ, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);

	if (driver_handle == INVALID_HANDLE_VALUE) {
		std::cout << "[-] Failed to create driver handle." << '\n';
		std::cin.get();
		return 1;
	}

	MemoryManager MemoryClass(driver_handle);

	const DWORD pid = MemoryClass.GetProcessID(L"PROCESS.exe");

	if (pid == 0) {
		std::cout << "[-] Failed to find process." << '\n';
		std::cin.get();
		return 1;
	}

	if (MemoryClass.AttachToProcess(pid) == true) {
		std::cout << "Attachment successful." << '\n';

		const std::uintptr_t client = MemoryClass.GetModuleBase(pid, L"client.dll");

		if (client != 0) {
			std::cout << "client found." << '\n';

			while (true) {

				// CODE GOES HERE

			}

		}
	}

	return 0;
}