#include <Windows.h>
#include <TlHelp32.h>
#include <iostream>

#include "Memory/Memory.h"
#include "Logger/Logger.h"


int main() {
	MemoryManager Memory(L"ProcessName.exe");

	if (!Memory.IsAttached()) Log::Error("Failed to attach to process");

	const std::uintptr_t Client = Memory.GetModuleBase(L"ModuleName.dll");

	if (Client == 0) Log::Error("Client not found");
	Log::Fine("Client found");


	// #############################################################################################
	//                                         Main Loop
	// #############################################################################################
	



}