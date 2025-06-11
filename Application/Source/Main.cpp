#include <iostream>
#include <Windows.h>
#include <TlHelp32.h>

#include "Util/Memory.h"



int main() {

	MemoryManager Memory(L"cs2.exe");

	// TODO: Base Code
	if (Memory.IsAttached()) {

		const std::uintptr_t client = Memory.GetModuleBase(L"client.dll");

		if (client != 0) {
			// Client Found
		}
	}

	return 0;
}