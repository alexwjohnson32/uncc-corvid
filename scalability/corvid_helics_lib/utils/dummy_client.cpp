#include "websocket-client-manager.hpp"

#include <boost/asio/io_context.hpp>

#include <cstdlib>
#include <exception>
#include <iostream>
#include <string>

int main(int argc, char **argv)
{
    if (argc != 4)
    {
        std::cerr << "Usage: dummy_client <address> <port> <path>\n"
                  << "Example:\n"
                  << "    dummy_client 0.0.0.0 8080 /\n";
        return EXIT_FAILURE;
    }

    const std::string address(argv[1]);
    const std::string port(argv[2]);
    const std::string path(argv[3]);

    try
    {
        std::cout << "Attempting to run client at '" << address << ":" << port << path << "'" << std::endl;

        connections::WebSocketClientManager client_mgr({ address, port, path });
        client_mgr.SetMessageCallback([](const std::string &msg)
                                      { std::cout << "[dummy client] Received: '" << msg << "'" << std::endl; });

        while (true)
        {
            std::cout << "Type message to send:" << std::endl;

            std::string msg;
            std::cin >> msg;

            if (msg == "end")
            {
                break;
            }

            client_mgr.SendMessage(msg);
        }

        std::cout << "\n\nClosing...";
        client_mgr.Close();
        std::cout << "complete.\n";
    }
    catch (const std::exception &e)
    {
        std::cerr << "Exception: " << e.what() << std::endl;
    }

    return EXIT_SUCCESS;
}