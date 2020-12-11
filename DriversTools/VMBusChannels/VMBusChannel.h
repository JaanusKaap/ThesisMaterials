#pragma once
#include "SharedStructures.h"

void* getKmclChannelListLocation();
void* getFirstChannel();
void* getNextChannel(void*);

void* getChannelFromLinkListPtr(void*);

UINT32 getChannelCount();
bool getChannelData(void* channel, VMBusChannel* data);
UINT8 getInt8FromChannel(void* channel, UINT32 offset);
UINT32 getInt32FromChannel(void* channel, UINT32 offset);
UINT64 getInt64FromChannel(void* channel, UINT32 offset);
void getBufferFromChannel(void* channel, UINT32 offset, void* buffer, UINT32 size);



