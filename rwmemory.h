#pragma once
#include <Windows.h>
#include <TlHelp32.h>
#include <string_view>

class rwMemory {
	
private:
	DWORD processId;
	HANDLE hProcess;

public:
	// Constructor finds process id and opens handle
	rwMemory(const std::string_view processName) {
		// Creating a snapshot of processes
		const HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

		PROCESSENTRY32 processEntry;
		processEntry.dwSize = sizeof(PROCESSENTRY32);

		// Find process in snapshot and open handle
		if (Process32First(hSnapshot, &processEntry)) {
			do {
				if (std::string_view(processEntry.szExeFile) == processName) {
					processId = processEntry.th32ProcessID;
					hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processId);
					break;
				}
			} while (Process32Next(hSnapshot, &processEntry));
		}

		// Free handle
		if (hSnapshot) {
			CloseHandle(hSnapshot);
		}
	}

	// Desctructor frees opened process handle
	~rwMemory() {
		if (hProcess) {
			CloseHandle(hProcess);
		}
	}

	// Returns the base address of the module by name
	std::uintptr_t GetBaseModuleAddress(const std::string_view moduleName) {
		const HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, 0);

		std::uintptr_t moduleAddress = 0;

		MODULEENTRY32 processEntry;
		processEntry.dwSize = sizeof(MODULEENTRY32);

		while (Module32Next(hSnapshot, &processEntry))
		{
			if (moduleName.compare(processEntry.szModule))
			{
				moduleAddress = reinterpret_cast<std::uintptr_t>(processEntry.modBaseAddr);
				break;
			}
		}

		if (hSnapshot) {
			CloseHandle(hSnapshot);
		}

		return moduleAddress;
	}

	// Read process memory
	template<typename T>
	const T Read(const std::uintptr_t address) {
		T value = {};

		ReadProcessMemory(hProcess, reinterpret_cast<LPCVOID>(address), &value, sizeof(T), NULL);
		return value;
	}

	// Write process memory
	template <typename T>
	void Write(const std::uintptr_t address, const T& value) {
		WriteProcessMemory(hProcess, reinterpret_cast<LPCVOID>(address), &value, sizeof(T), NULL);
	}
};
