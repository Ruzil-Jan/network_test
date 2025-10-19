#include "../headers/network/TCPSocket.h"
#include "../headers/network/UDPSocket.h"
#include "../server_for_test/SimpleServer.h"
#include "../headers/network/ISocket.h"
#include "../needed_files/Utils.h"
#include "../status_checker/socketStatusChecker.h"

#include <thread>
#include <memory>
#include <chrono>

namespace NetworkTest {
    const std::string loopback = "127.0.0.1";
    const int TIME = 500;
    int port = 1;

    void testSocket(ISocket *socket, const std::string &message)
    {
        if (socket->open())
        {
            Utils::log("Socket opened successfully!");

            if (socket->send(message))
            {
                Utils::log("Data sent successfully.");

                // Give server time to respond
                std::this_thread::sleep_for(std::chrono::milliseconds(100));

                std::string response = socket->receive();
                if (!response.empty())
                {
                    Utils::log("Received: " + response);
                }
                else
                {
                    Utils::log("No response received.");
                }
            }

            socket->close();
        }
        else
        {
            Utils::log("Failed to open socket.");
        }
    }
}

namespace TCPTest {
    using namespace NetworkTest;

    void testWithoutServer()
    {
        Utils::log("=== Testing TCP WITHOUT Server (Expected to fail) ===");
        TCPSocket tcpSocket(loopback, 9999); // Non-existent server

        if (tcpSocket.open())
        {
            Utils::log("Unexpected: Connected to non-existent server!");
            tcpSocket.close();
        }
        else
        {
            Utils::log("Expected: Failed to connect to non-existent server.");
        }
    }

    void testWithServer()
    {
        Utils::log("\n=== Testing TCP WITH Server ===");

        // Start a simple echo server
        if (Network::SocketStatusChecker::isPortBusy(port))
        {
            Utils::log("Port: " + std::to_string(port) + " is busy! Skip");
            port++;
        }
        SimpleServer server(port);
        if (!server.start())
        {
            Utils::log("Failed to start server!");
            return;
        }

        // Give server time to start
        std::this_thread::sleep_for(std::chrono::milliseconds(TIME));

        // Test direct usage
        Utils::log("\n--- Direct TCP Socket Usage ---");
        TCPSocket tcpSocket(loopback, port);

        if (tcpSocket.open())
        {
            Utils::log("TCP Connected successfully!");

            // Use TCP-specific features
            tcpSocket.setReceiveTimeout(5);
            tcpSocket.setSendTimeout(5);
            tcpSocket.setKeepAlive(true);

            if (tcpSocket.send("Hello, Server!"))
            {
                Utils::log("Data sent successfully.");

                std::string response = tcpSocket.receiveWithTimeout(3);
                if (!response.empty())
                {
                    Utils::log("Received: " + response);
                }
            }

            tcpSocket.close();
        }

        // Test polymorphic usage
        Utils::log("\n--- Polymorphic Usage ---");
        std::unique_ptr<ISocket> socket = std::make_unique<TCPSocket>(loopback, port);
        testSocket(socket.get(), "Hello from polymorphic interface!");

        // Test multiple connections
        Utils::log("\n--- Multiple Connections Test ---");
        for (int i = 1; i <= 3; ++i)
        {
            TCPSocket client(loopback, port);
            std::string message = "Message #" + std::to_string(i);
            Utils::log("Sending: " + message);
            testSocket(&client, message);
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
        }

        // Give time for last client to finish
        std::this_thread::sleep_for(std::chrono::milliseconds(TIME));

        Utils::log("\n--- Stopping Server ---");
        // Stop server
        server.stop();
        Utils::log("Server cleanup complete.");
    }
}

namespace UDPTest {
    using namespace NetworkTest;

    void testWithoutServer()
    {
        Utils::log("=== Testing UDP WITHOUT Server (Expected to succeed for UDP) ===");
        UDPSocket udpSocket(loopback, 9999); // Non-existent server

        if (udpSocket.open())
        {
            Utils::log("Expected: UDP socket opened successfully (no connection required).");
            udpSocket.close();
        }
        else
        {
            Utils::log("Unexpected: Failed to open UDP socket.");
        }
    }

    void testWithServer()
    {
        Utils::log("\n=== Testing UDP WITH Server ===");
        // Start a simple UDP echo server
        
        if (Network::SocketStatusChecker::isPortBusy(port))
        {
            Utils::log("Port: " + std::to_string(port) + " is busy! Skip");
            port++;
        }
        SimpleUDPServer server(port);
        if (!server.start())
        {
            Utils::log("Failed to start server!");
            return;
        }

        // Give server time to start
        std::this_thread::sleep_for(std::chrono::milliseconds(TIME));

        // Test direct usage
        Utils::log("\n--- Direct UDP Socket Usage ---");
        UDPSocket udpSocket(loopback, port);

        if (udpSocket.open())
        {
            Utils::log("UDP socket created successfully!");

            // Use UDP-specific features
            udpSocket.setReceiveTimeout(5);
            udpSocket.setSendTimeout(5);

            if (udpSocket.send("Hello, Server!"))
            {
                Utils::log("Data sent successfully.");

                std::string response = udpSocket.receiveWithTimeout(3);
                if (!response.empty())
                {
                    Utils::log("Received: " + response);
                }
            }

            udpSocket.close();
        }

        // Test polymorphic usage
        Utils::log("\n--- Polymorphic Usage ---");
        std::unique_ptr<ISocket> socket = std::make_unique<UDPSocket>(loopback, port);
        testSocket(socket.get(), "Hello from polymorphic interface!");

        // Test multiple messages
        Utils::log("\n--- Multiple Messages Test ---");
        for (int i = 1; i <= 3; ++i)
        {
            UDPSocket client(loopback, port);
            std::string message = "Message #" + std::to_string(i);
            Utils::log("Sending: " + message);
            testSocket(&client, message);
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
        }

        // Give time for last client to finish
        std::this_thread::sleep_for(std::chrono::milliseconds(TIME));

        Utils::log("\n--- Stopping Server ---");
        // Stop server
        server.stop();
        Utils::log("Server cleanup complete.");
    }
}