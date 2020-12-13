#include "VMBusFuzzer.h"
#include "CodeFromOthers.h"
#include "conf.h"
#include "Random.h"


UINT32 VMBusFuzzer::minChanges = 1, VMBusFuzzer::maxChanges = 2, VMBusFuzzer::iterationNr = 1;
bool VMBusFuzzer::main = true, VMBusFuzzer::mdl = false;


void VMBusFuzzer::hookHandlerFuzzer(void* channel, void* packet, void* buffer, UINT32 bufferLen, UINT32 flags)
{
	if (count)
	{
		bool mutationsDone = false;
		if (main && bufferLen)
		{
			DbgPrint("\n\n****** ITERATION 0x%X ******\n", iterationNr);
			mutationsDone = true;
			UINT32 mutationsCount = minChanges;
			if (maxChanges > minChanges)
				mutationsCount += Random::rand() % (maxChanges - minChanges + 1);
			DbgPrint("Doing 0x%X changes in main buffer @ 0x%llX with size 0x%X\n", mutationsCount, buffer, bufferLen);
			for (UINT32 x = 0; x < mutationsCount; x++)
			{
				UINT32 pos = Random::rand() % bufferLen;
				UINT8 valOriginal = ((PUINT8)buffer)[pos];
				UINT8 valNew = (UINT8)Random::rand() % 0x100;
				((PUINT8)buffer)[pos] = valNew;
				DbgPrint("  @ 0x%X: 0x%02X -> 0x%02X\n", pos, valOriginal, valNew);
			}
		}
		if (mdl && (flags & 0x1))
		{
			PMDL pmdl = NULL;
			if (VmbChannelPacketGetExternalData(packet, 0, &pmdl) == STATUS_SUCCESS)
			{
				if(!mutationsDone)
					DbgPrint("\n\n****** ITERATION 0x%X ******\n", iterationNr);
				mutationsDone = true;
				PUINT8 bufferMdl = (PUINT8)MmGetSystemAddressForMdlSafe(pmdl, NormalPagePriority);
				UINT32 bufferMdlLen = MmGetMdlByteCount(pmdl);
				UINT32 mutationsCount = minChanges;
				if (maxChanges > minChanges)
					mutationsCount += Random::rand() % (maxChanges - minChanges + 1);
				DbgPrint("Doing 0x%X changes in mdl buffer @ 0x%llX with size 0x%X\n", mutationsCount, bufferMdl, bufferMdlLen);
				for (UINT32 x = 0; x < mutationsCount; x++)
				{
					UINT32 pos = Random::rand() % bufferMdlLen;
					UINT8 valOriginal = ((PUINT8)bufferMdl)[pos];
					UINT8 valNew = (UINT8)Random::rand() % 0x100;
					((PUINT8)bufferMdl)[pos] = valNew;
					DbgPrint("  @ 0x%X: 0x%02X -> 0x%02X\n", pos, valOriginal, valNew);
				}
			}
		}

		if (mutationsDone)
		{
			count -= 1;
			iterationNr++;
		}
	}
	return originalHandler(channel, packet, buffer, bufferLen, flags);
}

UINT64 VMBusFuzzer::getHookHandler()
{
	return (UINT64)&hookHandlerFuzzer;
}

bool VMBusFuzzer::fuzz(VMBusFuzzConf* conf)
{
	VMBusInteceptConf confIntercept;
	confIntercept.channel = conf->channel;
	confIntercept.count = conf->count;
	confIntercept.logToDebug = 0;
	confIntercept.logToDebugAndBreak = 0;
	confIntercept.logToFile = 0;

	if (!hook(&confIntercept))
		return false;
	iterationNr = 1;
	minChanges = conf->minChanges;
	maxChanges = conf->maxChanges;
	main = (conf->main & 1) > 0;
	mdl = (conf->mdl & 1) > 0;
	return true;
}