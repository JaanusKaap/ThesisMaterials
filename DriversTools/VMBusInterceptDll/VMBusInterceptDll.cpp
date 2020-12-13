#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "DriverIO.h"

#ifdef VMBUSINTERCEPTDLL_EXPORTS
#define VMBUSINTERCEPTDLL_API extern "C" __declspec(dllexport)
#else
#define VMBUSINTERCEPTDLL_API extern "C" __declspec(dllimport)
#endif


VMBUSINTERCEPTDLL_API bool hookChannel(VMBusInteceptConf* conf)
{
    DriverIoVMBusIntercept io;
    if (!io.init())
        return false;
    return io.hookChannel(conf);
}

VMBUSINTERCEPTDLL_API bool unhookChannel()
{
    DriverIoVMBusIntercept io;
    if (!io.init())
        return false;
    return io.unhookChannel();
}

VMBUSINTERCEPTDLL_API bool setFilename(char* filenameRaw)
{
    std::string filename = filenameRaw;
    DriverIoVMBusIntercept io;
    if (!io.init())
        return false;
    return io.setFilename(filename);
}

BOOL APIENTRY DllMain(HMODULE hModule,
    DWORD  ul_reason_for_call,
    LPVOID lpReserved
)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}
