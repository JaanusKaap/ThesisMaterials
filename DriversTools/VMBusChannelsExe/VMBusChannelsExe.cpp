#include <iostream>
#include "DriverIO.h"

#define errorAndReturn(msg) {std::cout << msg << std::endl; return 0;}

std::ostream& operator<<(std::ostream& os, REFGUID guid) {

	os << std::uppercase;
	os.width(8);
	os.fill('0');
	os << std::hex << guid.Data1 << '-';
	os.width(4);
	os.fill('0');
	os << std::hex << guid.Data2 << '-';
	os.width(4);
	os.fill('0');
	os << std::hex << guid.Data3 << '-';
	os.width(2);
	os.fill('0');
	os << std::hex << static_cast<short>(guid.Data4[0]);
	os.width(2);
	os.fill('0');
	os << std::hex << static_cast<short>(guid.Data4[1]);
	os << std::hex << '-';
	os.width(2);
	os.fill('0');
	os << std::hex << static_cast<short>(guid.Data4[2]);
	os.width(2);
	os.fill('0');
	os << std::hex << static_cast<short>(guid.Data4[3]);
	os.width(2);
	os.fill('0');
	os << std::hex << static_cast<short>(guid.Data4[4]);
	os.width(2);
	os.fill('0');
	os << std::hex << static_cast<short>(guid.Data4[5]);
	os.width(2);
	os.fill('0');
	os << std::hex << static_cast<short>(guid.Data4[6]);
	os.width(2);
	os.fill('0');
	os << std::hex << static_cast<short>(guid.Data4[7]);
	os << std::nouppercase;
	return os;
}

int main()
{
	DriverIoVMBusChannels io;
	if (!io.init())
		errorAndReturn("Could not open handler to driver");
	UINT32 count;
	if (!io.getChannelsCount(&count))
		errorAndReturn("Could not get count of channels");
	std::cout << "There are " << count << " channels" << std::endl;
	
	VMBusChannelData* data;
	if (!io.getChannelsData(&data , &count))
		errorAndReturn("Could not get channels data");

	for (UINT32 x = 0; x < count; x++)
	{
		std::cout << "Channel at 0x" << std::hex << data[x].baseAddress << std::endl;

		std::cout << "  >baseAddress = 0x" << std::hex << data[x].baseAddress << std::endl;
		std::cout << "  >interfaceType = " << data[x].interfaceType << std::endl;
		std::cout << "  >interfaceInstance = " << data[x].interfaceInstance << std::endl;
		std::cout << "  >vmId = " << data[x].vmId << std::endl;
		std::cout << "  >maxPacketCount = 0x" << std::hex << data[x].maxPacketCount << std::endl;
		std::cout << "  >maxPacketSize = 0x" << std::hex << data[x].maxPacketSize << std::endl;
		std::cout << "  >clientContextSize = 0x" << std::hex << data[x].clientContextSize << std::endl;
		std::cout << "  >isPipe = " << (data[x].isPipe ? "yes" : "no") << std::endl;
		std::cout << "  >vtl = " << (UINT32)data[x].vtl << std::endl;
		std::cout << "  >processPacketCallback = 0x" << std::hex << data[x].processPacketCallback << std::endl;
		std::cout << "  >processingCompleteCallback = 0x" << std::hex << data[x].processingCompleteCallback << std::endl;
		std::cout << "  >callbackContext = 0x" << std::hex << data[x].callbackContext << std::endl;
		std::cout << "  >channelOpenedCallback = 0x" << std::hex << data[x].channelOpenedCallback << std::endl;
		std::cout << "  >channelClosedCallback = 0x" << std::hex << data[x].channelClosedCallback << std::endl;
		std::cout << "  >channelSuspendCallback = 0x" << std::hex << data[x].channelSuspendCallback << std::endl;
		std::cout << "  >channelStartedCallback = 0x" << std::hex << data[x].channelStartedCallback << std::endl;
		std::cout << "  >channelPostStartedCallback = 0x" << std::hex << data[x].channelPostStartedCallback << std::endl;
		std::cout << "  >pointer = 0x" << std::hex << data[x].pointer << std::endl;
		std::cout << std::endl << std::endl;
	}

}

