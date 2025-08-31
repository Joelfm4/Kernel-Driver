#pragma once

#include <TlHelp32.h>

#include "Driver.h"
#include "../Logger/Logger.h"



class MemoryManager {

public:

	// ----------------------------------------------------------------- //
	//                     Constructor & Destructor                      //
	// ----------------------------------------------------------------- //

	MemoryManager(const wchar_t* ProcessName) {
		m_KernelDriver = CreateFileW(L"\\\\.\\Driver", GENERIC_READ | GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);

		if (m_KernelDriver == INVALID_HANDLE_VALUE) Log::Error("Failed to create drive handle");

		m_ProcessID = GetProcessID(ProcessName);

		if (m_ProcessID == 0) {
			CloseHandle(m_KernelDriver);
			Log::Error("Failed to find process");
		}
		Log::Fine("Process found");

		if (!AttachToProcess(m_ProcessID)) {
			CloseHandle(m_KernelDriver);
			Log::Error("Failed to attach driver to process");
		}
		Log::Fine("Driver successfully attached to process");

		m_IsAttached = true;
	};


	~MemoryManager() {
		if (m_KernelDriver != INVALID_HANDLE_VALUE) {
			CloseHandle(m_KernelDriver);
		}
	};


	MemoryManager(const MemoryManager&) = delete;
	MemoryManager& operator=(const MemoryManager&) = delete;


	// ----------------------------------------------------------------- //
	//                        Read & Write Methods                       //
	// ----------------------------------------------------------------- //

	template <class T>
	T ReadMemory(const std::uintptr_t ADDR) {
		T temp = {};

		Driver::Request r;
		r.Target = reinterpret_cast<PVOID>(ADDR);
		r.Buffer = &temp;
		r.Size = sizeof(T);

		DeviceIoControl(m_KernelDriver, Driver::Codes::Read, &r, sizeof(r), &r, sizeof(r), nullptr, nullptr);

		return temp;
	}


	template <class T>
	void WriteMemory(const std::uintptr_t ADDR, const T& Value) {
		Driver::Request r;
		r.Target = reinterpret_cast<PVOID>(ADDR);
		r.Buffer = (PVOID)&Value;
		r.Size = sizeof(T);

		DeviceIoControl(m_KernelDriver, Driver::Codes::Write, &r, sizeof(r), &r, sizeof(r), nullptr, nullptr);
	}


	// ----------------------------------------------------------------- //
	//                         Module Utilities                          //
	// ----------------------------------------------------------------- //

	std::uintptr_t GetModuleBase(const wchar_t* ModuleName) const {
		std::uintptr_t ModuleBase = 0;

		HANDLE Snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, m_ProcessID);
		if (Snapshot == INVALID_HANDLE_VALUE) {
			return ModuleBase;
		}

		MODULEENTRY32W Entry = {};
		Entry.dwSize = sizeof(decltype(Entry));

		if (Module32FirstW(Snapshot, &Entry) == TRUE) {
			if (wcsstr(ModuleName, Entry.szModule) != nullptr) ModuleBase = reinterpret_cast<std::uintptr_t>(Entry.modBaseAddr);

			else {
				while (Module32NextW(Snapshot, &Entry) == TRUE) {
					if (wcsstr(ModuleName, Entry.szModule) != nullptr) {
						ModuleBase = reinterpret_cast<std::uintptr_t>(Entry.modBaseAddr);
						break;
					}
				}
			}
		}

		CloseHandle(Snapshot);

		return ModuleBase;
	}


	bool InForeground(const std::string& WindowName) const {
		HWND Current = GetForegroundWindow();

		if (!Current) return false;

		char title[256];
		GetWindowTextA(Current, title, sizeof(title));

		if (strstr(title, WindowName.c_str()) != nullptr) return true;

		return false;
	}


	bool IsAttached() const {
		return m_IsAttached;
	}



private:

	DWORD GetProcessID(const wchar_t* ProcessName) {

		HANDLE Snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
		if (Snapshot == INVALID_HANDLE_VALUE) {
			return 0;
		}

		PROCESSENTRY32W Entry = {};
		Entry.dwSize = sizeof(decltype(Entry));
		DWORD PID = 0;

		if (!Process32First(Snapshot, &Entry)) {
			CloseHandle(Snapshot);
			return 0;
		}

		do {
			if (_wcsicmp(Entry.szExeFile, ProcessName) == 0) {
				PID = Entry.th32ProcessID;
				break;
			}
		} while (Process32Next(Snapshot, &Entry));

		CloseHandle(Snapshot);
		return PID;
	}


	bool AttachToProcess(const DWORD PID) {
		Driver::Request r;
		r.ProcessID = ULongToHandle(PID);

		return DeviceIoControl(m_KernelDriver, Driver::Codes::Attach, &r, sizeof(r), &r, sizeof(r), nullptr, nullptr);
	}


	bool IsDriverLoaded() const {
		return m_KernelDriver != INVALID_HANDLE_VALUE;
	}



private:
	HANDLE m_KernelDriver { INVALID_HANDLE_VALUE };
	DWORD m_ProcessID { 0 };
	bool m_IsAttached { false };

};

