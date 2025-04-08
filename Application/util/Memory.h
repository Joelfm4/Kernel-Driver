#pragma once

#include <iostream>
#include <Windows.h>
#include <TlHelp32.h>
#include "../../Driver/src/Driver.h"

class MemoryManager {
private:
	HANDLE m_driver_handle;

public:
	explicit MemoryManager(HANDLE DriverHandle) : m_driver_handle(DriverHandle) {}

	struct ModuleData {
		HMODULE module;
		DWORD_PTR base;
		uintptr_t size;
	};

	~MemoryManager() {
		if (m_driver_handle) {
			CloseHandle(m_driver_handle);
		}
	};

	bool AttachToProcess(const DWORD pid) {
		Driver::Request r;
		r.process_id = reinterpret_cast<HANDLE>(pid);

		return DeviceIoControl(m_driver_handle, Driver::Codes::attach, &r, sizeof(r), &r, sizeof(r), nullptr, nullptr);
	}

	static DWORD GetProcessID(const wchar_t* process_name) {
		DWORD process_id = 0;

		HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
		if (snapshot == INVALID_HANDLE_VALUE) {
			return process_id;
		}

		// Define a structure to hold process entry information
		PROCESSENTRY32W entry = {};
		entry.dwSize = sizeof(decltype(entry));

		if (Process32First(snapshot, &entry) == TRUE) {
			while (Process32Next(snapshot, &entry) == TRUE) {
				if (_wcsicmp(entry.szExeFile, process_name) == 0) {
					process_id = entry.th32ProcessID;
					break;
				}
			}
		}

		CloseHandle(snapshot);

		return process_id;
	}

	static std::uintptr_t GetModuleBase(const DWORD pid, const wchar_t* module_name) {
		std::uintptr_t module_base = 0;

		HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, NULL);
		if (snapshot == INVALID_HANDLE_VALUE) {
			return module_base;
		}

		MODULEENTRY32W entry = {};
		entry.dwSize = sizeof(decltype(entry));

		if (Module32FirstW(snapshot, &entry) == TRUE) {
			if (wcsstr(module_name, entry.szModule) != nullptr) {
				module_base = reinterpret_cast<std::uintptr_t>(entry.modBaseAddr);
			}
			else {
				while (Module32NextW(snapshot, &entry) == TRUE) {
					if (wcsstr(module_name, entry.szModule) != nullptr) {
						module_base = reinterpret_cast<std::uintptr_t>(entry.modBaseAddr);
						break;
					}
				}
			}
		}
		CloseHandle(snapshot);

		return module_base;
	}

	template <class T>
	T ReadMemory(const std::uintptr_t addr) {
		T temp = {};

		Driver::Request r;
		r.target = reinterpret_cast<PVOID>(addr);
		r.buffer = &temp;
		r.size = sizeof(T);

		DeviceIoControl(m_driver_handle, Driver::Codes::read, &r, sizeof(r), &r, sizeof(r), nullptr, nullptr);

		return temp;
	}

	template <class T>
	void WriteMemory(const std::uintptr_t addr, const T& value) {
		Driver::Request r;
		r.target = reinterpret_cast<PVOID>(addr);
		r.buffer = (PVOID)&value;
		r.size = sizeof(T);

		DeviceIoControl(m_driver_handle, Driver::Codes::write, &r, sizeof(r), &r, sizeof(r), nullptr, nullptr);
	}

};

