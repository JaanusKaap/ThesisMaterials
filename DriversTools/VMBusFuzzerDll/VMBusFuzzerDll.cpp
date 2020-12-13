#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "DriverIO.h"

#ifdef VMBUSFUZZERDLL_EXPORTS
#define VMBUSFUZZERDLL_API extern "C" __declspec(dllexport)
#else
#define VMBUSFUZZERDLL_API extern "C" __declspec(dllimport)
#endif


VMBUSFUZZERDLL_API bool fuzz(VMBusFuzzConf* conf)
{
    DriverIoVMBusFuzzer io;
    if (!io.init())
        return false;
    return io.fuzz(conf);
}

VMBUSFUZZERDLL_API bool stopFuzz()
{
    DriverIoVMBusFuzzer io;
    if (!io.init())
        return false;
    return io.stopFuzz();
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
