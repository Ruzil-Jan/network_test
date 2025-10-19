#include "../needed_files/Utils.h"
#include "NetworkTest.h"

#include <iostream>
#include <memory>
#include <thread>
#include <chrono>


int main()
{
    Utils::log("TCP Socket Test Application");
    Utils::log("============================");

    TCPTest::testWithoutServer();
    TCPTest::testWithServer();
   
    Utils::log("\n=== Test For TCP Socket Complete ===");

    UDPTest::testWithoutServer();
    UDPTest::testWithServer();

    Utils::log("UDP Socket Test Application");
    Utils::log("============================");

    Utils::log("\n=== Test For UDP Socket Complete ===");
    return 0;
}
