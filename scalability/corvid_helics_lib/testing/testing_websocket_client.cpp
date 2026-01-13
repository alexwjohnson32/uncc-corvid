#include <boost/system/error_code.hpp>
#include <cstdlib>
#include <iostream>
#include <string>
#include <exception>
#include <memory>

#include <boost/asio/io_context.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/core.hpp>

#include "websocket_client.hpp"

#include <atomic>

namespace
{

void DoAsyncClient(const std::string &address, const std::string &port, const std::string &path)
{
    auto client = std::make_shared<utils::WebSocketClient>();
    client->SetOnMessage([](const std::string &msg) { std::cout << "Received: " << msg << std::endl; });
    client->SetOnError([](const boost::system::error_code &ec, const std::string &what)
                       { std::cerr << what << ": " << ec.message() << std::endl; });

    client->AsyncRun();

    std::atomic<bool> is_connecting = true;
    client->Connect(address, port, path,
                    [client, &is_connecting](const boost::system::error_code &ec)
                    {
                        if (!ec)
                        {
                            std::cout << "Connected!" << std::endl;
                        }
                        else
                        {
                            std::cout << "Connect failed: " << ec.message() << std::endl;
                        }

                        is_connecting = false;
                    });

    while (is_connecting);

    std::cout << "Type message to send:" << std::endl;

    std::string msg;
    while (std::getline(std::cin, msg))
    {
        if (msg == "end" || msg == "quit")
        {
            break;
        }
        client->Send(msg);
        std::cout << "Type message to send:" << std::endl;
    }

    std::cout << "\n\nClosing...";
    client->CloseConnection();
    std::cout << "Shutting Down...";
    client->StopRun();
    std::cout << "Complete.\n";
}
} // namespace

int main(int argc, char **argv)
{
    if (argc != 4)
    {
        std::cerr << "Usage: testing_client <address> <port> <path>\n"
                  << "Example:\n"
                  << "    testing_client 0.0.0.0 8080 /\n";
        return EXIT_FAILURE;
    }

    const std::string address(argv[1]);
    const std::string port(argv[2]);
    const std::string path(argv[3]);

    try
    {
        std::cout << "Attempting to run client at '" << address << ":" << port << path << std::endl;
        DoAsyncClient(address, port, path);
    }
    catch (const std::exception &e)
    {
        std::cerr << "Exception: " << e.what() << std::endl;
    }

    return EXIT_SUCCESS;
}