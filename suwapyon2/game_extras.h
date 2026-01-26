#pragma once
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <list>
#include <tlhelp32.h>
#include "detours.h"

#define DllExport __declspec(dllexport)
#define DllImport __declspec(dllimport)

void InitDetours(HANDLE hModule);
void ExitDetours();
int RunControllerGame();