#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/strand.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/beast/core.hpp>
#include <iostream>
#include <memory>
#include <string>

// =========================================================
// WebSocket Session
// =========================================================
class WebSocketSession : public std::enable_shared_from_this<WebSocketSession>
{
  private:
    boost::beast::websocket::stream<boost::asio::ip::tcp::socket> m_websocket;
    boost::beast::flat_buffer m_buffer;
    boost::asio::strand<boost::asio::io_context::executor_type> m_strand;

  public:
    WebSocketSession(boost::asio::ip::tcp::socket socket, boost::asio::io_context &io_context)
        : m_websocket(std::move(socket)), m_strand(io_context.get_executor())
    {
    }

    void start()
    {
        m_websocket.async_accept(boost::asio::bind_executor(
            m_strand, std::bind(&WebSocketSession::onAccept, shared_from_this(), std::placeholders::_1)));
    }

  private:
    void onAccept(const boost::system::error_code &ec)
    {
        if (ec)
        {
            std::cout << "WebSocket accept error: " << ec.message() << std::endl;
            return;
        }
        startRead();
    }

    void startRead()
    {
        m_websocket.async_read(
            m_buffer, boost::asio::bind_executor(m_strand, std::bind(&WebSocketSession::onRead, shared_from_this(),
                                                                     std::placeholders::_1, std::placeholders::_2)));
    }

    void onRead(const boost::system::error_code &ec, std::size_t bytes_transferred)
    {
        if (ec)
        {
            if (ec != boost::beast::websocket::error::closed)
            {
                std::cout << "WebSocket read error: " << ec.message() << std::endl;
            }
            return;
        }

        std::string message(boost::asio::buffer_cast<const char *>(m_buffer.data()), bytes_transferred);
        std::cout << "[Received] " << message << std::endl;

        m_websocket.async_write(
            boost::asio::buffer(message),
            boost::asio::bind_executor(m_strand, std::bind(&WebSocketSession::onWrite, shared_from_this(),
                                                           std::placeholders::_1, std::placeholders::_2)));

        m_buffer.consume(m_buffer.size());
    }

    void onWrite(const boost::system::error_code &ec, std::size_t)
    {
        if (ec)
        {
            std::cout << "WebSocket write error: " << ec.message() << std::endl;
            return;
        }
        startRead();
    }
};

// =========================================================
// WebSocket Server
// =========================================================
class WebSocketServer
{
  private:
    boost::asio::io_context &m_io_context;
    boost::asio::ip::tcp::acceptor m_acceptor;

  public:
    WebSocketServer(boost::asio::io_context &io_context, const std::string &host, unsigned short port)
        : m_io_context(io_context), m_acceptor(io_context)
    {
        boost::asio::ip::address address = boost::asio::ip::make_address(host);
        boost::asio::ip::tcp::endpoint endpoint(address, port);

        m_acceptor.open(endpoint.protocol());
        m_acceptor.set_option(boost::asio::socket_base::reuse_address(true));
        m_acceptor.bind(endpoint);
        m_acceptor.listen();
    }

    void startAccept()
    {
        m_acceptor.async_accept(
            std::bind(&WebSocketServer::onAccept, this, std::placeholders::_1, std::placeholders::_2));
    }

  private:
    void onAccept(const boost::system::error_code &ec, boost::asio::ip::tcp::socket socket)
    {
        if (!ec)
        {
            std::cout << "Client connected" << std::endl;
            std::make_shared<WebSocketSession>(std::move(socket), m_io_context)->start();
        }
        else
        {
            std::cout << "Accept error: " << ec.message() << std::endl;
        }

        // Accept next connection
        startAccept();
    }
};

// =========================================================
// MAIN
// =========================================================
int main(int argc, char *argv[])
{
    if (argc != 4)
    {
        std::cout << "Usage: testing-server <host> <port> <path>\n";
        return 1;
    }

    try
    {
        std::string host(argv[1]);
        unsigned short port = static_cast<unsigned short>(std::stoi(argv[2]));
        std::string path(argv[3]); // Currently unused

        std::cout << "Starting WebSocket server on ws://" << host << ":" << port << path << std::endl;

        boost::asio::io_context io_context;

        WebSocketServer server(io_context, host, port);
        server.startAccept();

        io_context.run();
    }
    catch (const std::exception &ex)
    {
        std::cout << "Fatal error: " << ex.what() << std::endl;
        return 1;
    }

    return 0;
}
