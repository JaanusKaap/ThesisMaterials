#include <iostream>
#include "DriverIO.h"

#define errorAndReturn(msg) {std::cout << msg << std::endl; return 0;}

int main()
{
    DriverIoVMBusIntercept driverIo;
    VMBusInteceptConf conf;
    std::string filename;
    memset(&conf, 0, sizeof(conf));
    if (!driverIo.init())
        errorAndReturn("Could not open handler to driver");
    driverIo.unhookChannel();

    std::cout << "Address of the channel (in hex without 0x): ";
    std::cin >> std::hex >> conf.channel;
    if (!conf.channel)
        return 1;
    std::cout << "How many requests to log: ";
    std::cin >> conf.count;
    std::cout << "Log to debug messages(0/1): ";
    std::cin >> conf.logToDebug;
    if (conf.logToDebug)
    {
        std::cout << "After logging to debug, also break(0/1): ";
        std::cin >> conf.logToDebugAndBreak;
    }
    std::cout << "Log to file(0/1): ";
    std::cin >> conf.logToFile;
    if (conf.logToFile)
    {
        std::cout << "Filename (full path): ";
        std::cin >> filename;
        if (!driverIo.setFilename(filename))
            errorAndReturn("Could not set log file");
    }
    if (!driverIo.hookChannel(&conf))
        errorAndReturn("Could not hook the channel");
    std::cout << "All done" << std::endl;
}
