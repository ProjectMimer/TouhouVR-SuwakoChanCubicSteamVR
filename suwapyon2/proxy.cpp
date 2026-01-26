#include "proxy.h"
#include "game_extras.h"

//----
// Logging
//----

std::ofstream ofOut;
std::stringstream logError;
bool doLog = false;

bool HasErrors()
{
    return ((logError.rdbuf()->in_avail() == 0) ? false : true);
}

std::string GetErrors()
{
    std::string curLog = logError.str();
    logError.str("");
    return curLog;
}

void PrintErrors()
{
    if (doLog && HasErrors())
        ofOut << GetErrors();
}

//----



//----

HINSTANCE gOriginalDll;
HINSTANCE gThisInstance;

void LoadOriginalDll()
{
    if (doLog) logError << "-- LoadOriginalDll Start" << std::endl;
    if (gOriginalDll) return;

    char buffer[MAX_PATH];
    GetSystemDirectoryA(buffer, MAX_PATH);
    strcat_s(buffer, MAX_PATH, "\\dinput8.dll");
    if (doLog) logError << "-- -- " << buffer << std::endl;

    TCHAR moduleName[1024];
    GetModuleFileName(gThisInstance, moduleName, sizeof(moduleName) / sizeof(TCHAR));

    if (!gOriginalDll) {
        gOriginalDll = LoadLibraryA(buffer);
    }
    if (!gOriginalDll) {
        ExitProcess(0);
    }

    if (doLog) logError << "-- LoadOriginalDll End" << std::endl;
    return;
}

void InitInstance(HANDLE hModule)
{
    if (doLog)
    {
        ofOut.open("./output.txt", std::ios::out);
        ofOut.precision(5);
    }

    gThisInstance = (HINSTANCE)hModule;
    if (!gOriginalDll) LoadOriginalDll();

    PrintErrors();
    InitDetours(hModule);
    PrintErrors();
}

void ExitInstance()
{
    ExitDetours();
    if (doLog) logError << "ExitInstance Start" << std::endl;
    if (gOriginalDll)
    {
        FreeLibrary(gOriginalDll);
        gOriginalDll = NULL;
    }
    if (doLog) logError << "ExitInstance Stop" << std::endl;
    PrintErrors();
    if(doLog)
        ofOut.close();
    return;
}

BOOL APIENTRY DllMain( HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH: InitInstance(hModule); break;
    case DLL_PROCESS_DETACH: ExitInstance(); break;
    case DLL_THREAD_ATTACH: break;
    case DLL_THREAD_DETACH: break;
    }
    return TRUE;
}

//----
//----


typedef
HRESULT(WINAPI* DirectInput8Create_Type)(HINSTANCE hinst, DWORD dwVersion, REFIID riidltf, LPVOID* ppvOut, LPUNKNOWN punkOuter);
DirectInput8Create_Type DirectInput8Create_fn = NULL;
HRESULT WINAPI DirectInput8Create(HINSTANCE hinst, DWORD dwVersion, REFIID riidltf, LPVOID* ppvOut, LPUNKNOWN punkOuter)
{
    if (!DirectInput8Create_fn)
        DirectInput8Create_fn = (DirectInput8Create_Type)GetProcAddress(gOriginalDll, "DirectInput8Create");
    if (DirectInput8Create_fn)
        return DirectInput8Create_fn(hinst, dwVersion, riidltf, ppvOut, punkOuter);
    return 0;
}
