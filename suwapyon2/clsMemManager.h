#pragma once
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <iostream>
#include <map>
#include <string>
#include <tlhelp32.h>
#include <psapi.h>

class clsMemManager
{
private:
	DWORD pid;
	HWND hWnd;
	HANDLE hProcess;
	bool attached;
	std::map<std::wstring, DWORD> moduleList;

public:
	clsMemManager();
	bool attachWindow(const wchar_t*);
	bool attachWindow(HWND);
	bool attachWindow(DWORD);
	void closeProcess();
	bool isConnected();
	DWORD getPID();
	DWORD getDLLAddress(const wchar_t*);
	HWND getHWND();
	HANDLE getProcessHandle();
	void listModules();
	int readMemoryL(DWORD, int);
	float readMemoryF(DWORD);
	double readMemoryD(DWORD);
	void writeMemoryL(DWORD, int, int);
	void writeMemoryF(DWORD, float);
	void writeMemoryD(DWORD, double);
	void writeMemoryS(DWORD, int, BYTE[]);
};
