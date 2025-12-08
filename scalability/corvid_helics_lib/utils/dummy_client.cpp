#include "websocket-client-manager.hpp"

#include <boost/asio/io_context.hpp>

#include <cstdlib>
#include <exception>
#include <iostream>
#include <string>
#include <thread>
#include <memory>

#include "persistent-websocket-client.hpp"

namespace
{

template <typename T> void ClientExecution(T &client)
{
    std::cout << "Setup callback." << std::endl;

    client.SetMessageCallback([](const std::string &msg)
                              { std::cout << "[dummy client] Received: '" << msg << "'" << std::endl; });

    std::cout << "Client Execution Begins: " << std::endl;

    while (true)
    {
        std::cout << "Type message to send:" << std::endl;

        std::string msg;
        std::cin >> msg;

        if (msg == "end")
        {
            break;
        }

        client.SendMessage(msg);
    }

    std::cout << "\n\nClosing...";
    client.Close();
    std::cout << "Complete.\n";
}

void ExecuteWebSocketClientManager(const connections::ClientDetails &details)
{
    std::cout << "Creating WebSocketClientManager" << std::endl;
    connections::WebSocketClientManager client_mgr(details);

    std::cout << "Calling ClientExecution" << std::endl;
    ClientExecution(client_mgr);
}

// void ExecutePersistentWebSocketClient(const connections::ClientDetails &details)
// {
//     std::cout << "Creating IO Context" << std::endl;
//     boost::asio::io_context io_context;

//     std::cout << "Creating PersistentWebSocketClient" << std::endl;
//     std::shared_ptr<connections::PersistentWebSocketClient> client =
//         std::make_shared<connections::PersistentWebSocketClient>(io_context, details.address, details.port,
//                                                                  details.websocket_path, details.timeout_seconds);
//     std::cout << "Created PersistentWebSocketClient" << std::endl;

//     // This should be a deferred operation
//     std::cout << "Setting up Post" << std::endl;
//     io_context.post(
//         [client]()
//         {
//             std::cout << "Calling client->Start()" << std::endl;
//             client->Start();

//             // std::cout << "Calling ClientExecution" << std::endl;
//             // ClientExecution(*client);
//         });

//     std::cout << "Running threading and starting on post" << std::endl;
//     std::thread io_thread([&]() { io_context.run(); });
//     io_thread.join();
// }

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
        const uint timeout_seconds = 15;
        std::cout << "Attempting to run client at '" << address << ":" << port << path
                  << "', timeout: " << timeout_seconds << std::endl;

        connections::ClientDetails details = { address, port, path, timeout_seconds };
        ExecuteWebSocketClientManager(details);
        // ExecutePersistentWebSocketClient(details);
    }
    catch (const std::exception &e)
    {
        std::cerr << "Exception: " << e.what() << std::endl;
    }

    return EXIT_SUCCESS;
}