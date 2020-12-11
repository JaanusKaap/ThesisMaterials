#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "DriverIO.h"

#ifdef VMBUSCHANNELSDLL_EXPORTS
#define VMBUSCHANNELSDLL_API extern "C" __declspec(dllexport)
#else
#define VMBUSCHANNELSDLL_API extern "C" __declspec(dllimport)
#endif


VMBUSCHANNELSDLL_API bool getVMBusChannelsCount(UINT32* count)
{
    DriverIoVMBusChannels io;
    if (!io.init())
        return false;
	return io.getChannelsCount(count);
}

VMBUSCHANNELSDLL_API bool getVMBusChannelsData(VMBusChannel** data, UINT32* count)
{
    DriverIoVMBusChannels io;
    if (!io.init())
        return false;
	return io.getChannelsData(data, count);
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
