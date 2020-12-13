#include <iostream>
#include "DriverIO.h"

#define errorAndReturn(msg) {std::cout << msg << std::endl; return 0;}

int main()
{
    DriverIoVMBusFuzzer driverIo;
    VMBusFuzzConf conf;
    memset(&conf, 0, sizeof(conf));

    if (!driverIo.init())
        errorAndReturn("Could not open handler to driver");
    driverIo.stopFuzz();

    std::cout << "Address of the channel (in hex without 0x): ";
    std::cin >> std::hex >> conf.channel;
    if (!conf.channel)
        return 1;
    std::cout << "How many requests to fuzz: ";
    std::cin >> conf.count;
    std::cout << "Minimum mutations per request: ";
    std::cin >> conf.minChanges;
    std::cout << "Maximum mutations per request: ";
    std::cin >> conf.maxChanges;
    std::cout << "Fuzz main buffer(0/1): ";
    std::cin >> conf.main;
    std::cout << "Fuzz external buffer(0/1): ";
    std::cin >> conf.mdl;
    
    if (!driverIo.fuzz(&conf))
        errorAndReturn("Could not hook and fuzz the channel");
    std::cout << "All done" << std::endl;
}
