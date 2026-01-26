#pragma once
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <sstream>
#include <fstream>
//#include <d3d9.h>
//#include <version>
#include <dinput.h>


#define DllExport __declspec(dllexport)
#define DllImport __declspec(dllimport)

bool HasErrors();
std::string GetErrors();
void PrintErrors();
