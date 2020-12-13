#pragma once
#include<ntddk.h>
#include "VMBusIntercept.h"
#include "SharedStructures.h"

class VMBusFuzzer : public VMBusIntercept
{
protected:
	static UINT32 minChanges, maxChanges;
	static bool main, mdl;
	static UINT32 iterationNr;

	static void hookHandlerFuzzer(void* channel, void* packet, void* buffer, UINT32 bufferLen, UINT32 flags);
	virtual UINT64 getHookHandler();

public:
	bool fuzz(VMBusFuzzConf*);
};