#include "clsMemManager.h"

clsMemManager::clsMemManager() : pid(0), hWnd(0), hProcess(0), attached(0)
{
	return;
}

bool clsMemManager::attachWindow(const wchar_t* windowName) {
	if (pid == 0) {
		hWnd = FindWindow(NULL, windowName);
		return attachWindow(hWnd);
	}
	return false;
}

bool clsMemManager::attachWindow(HWND thWnd) {
	if (pid == 0) {
		hWnd = thWnd;
		if (hWnd) {
			GetWindowThreadProcessId(hWnd, &pid);
			return attachWindow(pid);
		}
		return false;
	}
	return false;
}

bool clsMemManager::attachWindow(DWORD pID) {
	if (pid == 0) {
		pid = pID;
		hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
		listModules();
		return true;
	}
	return false;
}

void clsMemManager::closeProcess() {
	if (hProcess != 0) {
		CloseHandle(hProcess);
	}
	moduleList.clear();
	pid = 0;
	hWnd = 0;
	hProcess = 0;
	attached = 0;
	return;
}

bool clsMemManager::isConnected() {
	return attached;
}

DWORD clsMemManager::getPID() {
	return pid;
}

HWND clsMemManager::getHWND() {
	return hWnd;
}

HANDLE clsMemManager::getProcessHandle()
{
	return hProcess;
}

DWORD clsMemManager::getDLLAddress(const wchar_t* moduleName) {
	std::wstring strKey(moduleName);
	std::map<std::wstring, DWORD>::iterator it = moduleList.begin();
	for (it = moduleList.begin(); it != moduleList.end(); ++it) {
		//std::string output(it->first);
		//MessageBox(NULL, (LPCSTR)output.c_str(), "DLL List", 0);
		if (it->first == strKey) {
			return it->second;
		}
	}
	return 0;
}

void clsMemManager::listModules() {
	HMODULE hMods[1024];
	DWORD cbNeeded;

	moduleList.clear();
	if (EnumProcessModulesEx(hProcess, hMods, sizeof(hMods), &cbNeeded, LIST_MODULES_ALL)) {
		for (int i = 0; i < (int)(cbNeeded / sizeof(HMODULE)); i++) {
			wchar_t szModName[MAX_PATH];
			if (GetModuleBaseName(hProcess, hMods[i], szModName, MAX_PATH)) {
				moduleList.insert({ szModName, (DWORD)hMods[i] });
			}
		}
	}
	return;
}

int clsMemManager::readMemoryL(DWORD address, int readSize) {
	int retVal = 0;
	if(hProcess)
		ReadProcessMemory(hProcess, (PVOID)address, &retVal, readSize, 0);
	return retVal;
}

float clsMemManager::readMemoryF(DWORD address) {
	float retVal = 0.0;
	if (hProcess)
		ReadProcessMemory(hProcess, (PVOID)address, &retVal, 4, 0);
	return retVal;
}

double clsMemManager::readMemoryD(DWORD address) {
	double retVal = 0.0;
	if (hProcess)
		ReadProcessMemory(hProcess, (PVOID)address, &retVal, 8, 0);
	return retVal;
}

void clsMemManager::writeMemoryL(DWORD address, int writeSize, int writeVal) {
	if (hProcess)
		WriteProcessMemory(hProcess, (PVOID)address, &writeVal, writeSize, 0);
	return;
}

void clsMemManager::writeMemoryF(DWORD address, float writeVal) {
	if (hProcess)
		WriteProcessMemory(hProcess, (PVOID)address, &writeVal, 4, 0);
	return;
}

void clsMemManager::writeMemoryD(DWORD address, double writeVal) {
	if (hProcess)
		WriteProcessMemory(hProcess, (PVOID)address, &writeVal, 8, 0);
	return;
}

void clsMemManager::writeMemoryS(DWORD address, int writeSize, BYTE writeVal[]) {
	if (hProcess)
		WriteProcessMemory(hProcess, (PVOID)address, writeVal, writeSize, 0);
	return;
}
