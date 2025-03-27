#include <iostream>
#include <Windows.h>
#include <TlHelp32.h>

#include "../util/Memory.hpp"

int main() {
	// Attach Driver
	const HANDLE driver_handle = CreateFile(L"\\\\.\\Driver", GENERIC_READ, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);

	if (driver_handle == INVALID_HANDLE_VALUE) {
		std::cout << "[-] Failed to create driver handle." << '\n';
		std::cin.get();
		return 1;
	}

	MemoryManager MemoryClass(driver_handle);

	// CODE GOES HERE

	return 0;
}