#include <boost/asio/io_context.hpp>
#include <boost/asio/socket_base.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/core/error.hpp>
#include <boost/beast/core/flat_buffer.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/strand.hpp>
#include <boost/core/ignore_unused.hpp>
#include <cstddef>
#include <cstdlib>
#include <ios>
#include <iostream>
#include <memory>
#include <thread>

#include <string>
#include <fstream>

// Session class handles a single WebSocket connection
class WebSocketSession : public std::enable_shared_from_this<WebSocketSession>
{
  private:
    int m_id;
    boost::beast::websocket::stream<boost::beast::tcp_stream> m_websocket;
    boost::beast::flat_buffer m_buffer;
    std::ofstream &m_output_file_stream;

    void OnAccept(boost::beast::error_code error)
    {
        m_output_file_stream << "WebSocketSession: " << m_id << ", OnAccept" << std::endl;
        if (error)
        {
            std::cerr << "Accept error: " << error.message() << std::endl;
            return;
        }

        DoRead();
    }

    void DoRead()
    {
        m_output_file_stream << "WebSocketSession: " << m_id << ", DoRead" << std::endl;
        m_websocket.async_read(m_buffer,
                               boost::beast::bind_front_handler(&WebSocketSession::OnRead, shared_from_this()));
    }

    void OnRead(boost::beast::error_code error, std::size_t bytes_transferred)
    {
        m_output_file_stream << "WebSocketSession: " << m_id << ", OnRead" << std::endl;
        boost::ignore_unused(bytes_transferred);

        if (error == boost::beast::websocket::error::closed) return;

        if (error)
        {
            std::cerr << "Read error: " << error.message() << std::endl;
            return;
        }

        // Echo the received message back
        m_output_file_stream << "WebSocketSession: " << m_id << ", Received message: '"
                             << boost::beast::buffers_to_string(m_buffer.data()) << "'" << std::endl;

        m_websocket.text(m_websocket.got_text());
        m_websocket.async_write(m_buffer.data(),
                                boost::beast::bind_front_handler(&WebSocketSession::OnWrite, shared_from_this()));
    }

    void OnWrite(boost::beast::error_code error, std::size_t bytes_transferred)
    {
        m_output_file_stream << "WebSocketSession: " << m_id << ", OnRead" << std::endl;
        boost::ignore_unused(bytes_transferred);

        if (error)
        {
            std::cerr << "Write error: " << error.message() << std::endl;
            return;
        }

        // Clear the buffer and wait for the next message
        m_buffer.consume(m_buffer.size());
        DoRead();
    }

  public:
    explicit WebSocketSession(boost::asio::ip::tcp::socket socket, std::ofstream &output_file_stream)
        : m_websocket(std::move(socket)), m_output_file_stream(output_file_stream)
    {
        static int id = 0;
        m_id = ++id;
    }
    void Run()
    {
        m_output_file_stream << "WebSocketSession: " << m_id << ", Running..." << std::endl;
        // Run the WebSocket handshake asynchronously
        m_websocket.async_accept(boost::beast::bind_front_handler(&WebSocketSession::OnAccept, shared_from_this()));
    }
};

// Listener class accepts incoming TCP connections
class WebSocketListener : public std::enable_shared_from_this<WebSocketListener>
{
  private:
    boost::asio::ip::tcp::acceptor m_acceptor;
    boost::asio::ip::tcp::socket m_socket;
    std::ofstream &m_output_file_stream;

    void DoAccept()
    {
        m_acceptor.async_accept(m_socket,
                                boost::beast::bind_front_handler(&WebSocketListener::OnAccept, shared_from_this()));
    }

    void OnAccept(boost::beast::error_code error)
    {
        if (error)
        {
            std::cerr << "Accept error: " << error.message() << std::endl;
        }
        else
        {
            std::make_shared<WebSocketSession>(std::move(m_socket), m_output_file_stream)->Run();
        }

        // Accept another connection
        DoAccept();
    }

  public:
    WebSocketListener(boost::asio::io_context &io_context, boost::asio::ip::tcp::endpoint endpoint,
                      std::ofstream &output_file_stream)
        : m_acceptor(io_context), m_socket(io_context), m_output_file_stream(output_file_stream)
    {
        boost::beast::error_code error;

        // Open the acceptor
        m_acceptor.open(endpoint.protocol(), error);
        if (error)
        {
            std::cerr << "Open error: " << error.message() << std::endl;
            return;
        }

        // Allow address reuse
        m_acceptor.set_option(boost::asio::socket_base::reuse_address(true), error);
        if (error)
        {
            std::cerr << "set_option error: " << error.message() << std::endl;
            return;
        }

        // Bind to the server address
        m_acceptor.bind(endpoint, error);
        if (error)
        {
            std::cerr << "Bind error: " << error.message() << std::endl;
            return;
        }

        // Start listening for connections
        m_acceptor.listen(boost::asio::socket_base::max_listen_connections, error);
        if (error)
        {
            std::cerr << "Listen error: " << error.message() << std::endl;
            return;
        }
    }

    void Run() { DoAccept(); }
};

int main(int argc, char **argv)
{
    if (argc != 4)
    {
        std::cerr << "Usage: websocket_server <address> <port> <output_file_path>\n"
                  << "Example:\n"
                  << "    websocket_server 0.0.0.0 8080 output.txt\n";
        return EXIT_FAILURE;
    }

    const auto address = boost::asio::ip::make_address(argv[1]);
    const auto port = static_cast<unsigned short>(std::atoi(argv[2]));
    const auto output_file_path = std::string(argv[3]);

    // Open the filestream
    std::ofstream output_file_stream(output_file_path, std::ios_base::app);
    if (!output_file_stream.is_open())
    {
        std::cerr << "Could not open output file at path '" << output_file_path << "'" << std::endl;
        return EXIT_FAILURE;
    }

    // Launch the server
    boost::asio::io_context io_context;

    std::make_shared<WebSocketListener>(io_context, boost::asio::ip::tcp::endpoint(address, port), output_file_stream)
        ->Run();

    // Run the I/O service on multiple threads to keep server alive
    std::vector<std::thread> threads;
    const int thread_count = std::max<int>(1, 4);
    threads.reserve(thread_count - 1);
    for (int i = 0; i < thread_count - 1; i++)
    {
        threads.emplace_back([&io_context]() { io_context.run(); });
    }

    io_context.run();

    return EXIT_SUCCESS;
}