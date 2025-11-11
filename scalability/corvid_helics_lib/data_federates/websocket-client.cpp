#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <cstdlib>
#include <iostream>
#include <string>

int main(int argc, char **argv)
{
    // Check command line arguments
    if (argc != 4)
    {
        std::cerr << "Usage: websocket_client <host> <port> <text>\n"
                  << "Example:\n"
                  << "    websocket_client echo.websocket.events 23333 \"Hello world!\"\n";
        return EXIT_FAILURE;
    }

    const std::string host = argv[1]; // default: "echo.websocket.events"
    const std::string port = argv[2]; // default: "23333"
    const std::string text = argv[3];

    try
    {
        // All I/O requires an io_context
        boost::asio::io_context io_context;

        // These objects perform our I/O
        boost::asio::ip::tcp::resolver resolver(io_context);
        boost::beast::websocket::stream<boost::beast::tcp_stream> websocket(io_context);

        // Look up the domain name
        const auto results = resolver.resolve(host, port);

        // Make the connection on the IP address we get from a lookup
        boost::asio::connect(websocket.next_layer().socket(), results.begin(), results.end());

        // Perform the WebSocket handshake and send message
        websocket.handshake(host, "/");
        websocket.write(boost::asio::buffer(text));

        // Read incoming message into this buffer
        boost::beast::flat_buffer buffer;
        websocket.read(buffer);

        // Process the received message
        std::cout << boost::beast::make_printable(buffer.data()) << std::endl;

        // Close the WebSocket connection
        websocket.close(boost::beast::websocket::close_code::normal);
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}