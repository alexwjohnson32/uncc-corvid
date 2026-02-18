#include <boost/asio.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <iostream>
#include <memory>
#include <string>

class WebSocketSession : public std::enable_shared_from_this<WebSocketSession>
{
  public:
    explicit WebSocketSession(boost::asio::ip::tcp::socket socket) : m_ws(std::move(socket)) {}

    void start()
    {
        m_ws.async_accept(std::bind(&WebSocketSession::onAccept, shared_from_this(), std::placeholders::_1));
    }

  private:
    boost::beast::websocket::stream<boost::asio::ip::tcp::socket> m_ws;
    boost::beast::flat_buffer m_buffer;

    void onAccept(const boost::system::error_code &ec)
    {
        if (ec)
        {
            std::cout << "WebSocket accept error: " << ec.message() << "\n";
            return;
        }
        doRead();
    }

    void doRead()
    {
        m_ws.async_read(m_buffer, std::bind(&WebSocketSession::onRead, shared_from_this(), std::placeholders::_1,
                                            std::placeholders::_2));
    }

    void onRead(const boost::system::error_code &ec, std::size_t)
    {
        if (ec)
        {
            if (ec != boost::beast::websocket::error::closed)
                std::cout << "WebSocket read error: " << ec.message() << "\n";
            return;
        }

        std::string msg = boost::beast::buffers_to_string(m_buffer.data());
        std::cout << "[Received] " << msg << "\n";

        m_buffer.consume(m_buffer.size());
        doRead();
    }
};

class WebSocketServer
{
  public:
    WebSocketServer(boost::asio::io_context &ioc, const boost::asio::ip::tcp::endpoint &endpoint)
        : m_ioc(ioc), m_acceptor(ioc), m_socket(ioc)
    {
        m_acceptor.open(endpoint.protocol());
        m_acceptor.set_option(boost::asio::socket_base::reuse_address(true));
        m_acceptor.bind(endpoint);
        m_acceptor.listen();
    }

    void startAccept()
    {
        m_acceptor.async_accept(m_socket, std::bind(&WebSocketServer::onAccept, this, std::placeholders::_1));
    }

  private:
    boost::asio::io_context &m_ioc;
    boost::asio::ip::tcp::acceptor m_acceptor;
    boost::asio::ip::tcp::socket m_socket;

    void onAccept(const boost::system::error_code &ec)
    {
        if (!ec)
        {
            std::cout << "Client connected\n";
            std::make_shared<WebSocketSession>(std::move(m_socket))->start();
        }
        else
        {
            std::cout << "Accept error: " << ec.message() << "\n";
        }

        startAccept();
    }
};

int main(int argc, char *argv[])
{
    try
    {
        if (argc != 4)
        {
            std::cout << "Usage: testing-server <host> <port> <path>\n";
            return 1;
        }

        std::string host(argv[1]);
        unsigned short port = static_cast<unsigned short>(std::stoi(argv[2]));
        std::string path(argv[3]); // Currently unused

        boost::asio::io_context ioc;

        WebSocketServer server(ioc, { boost::asio::ip::make_address(host), port });

        std::cout << "Starting WebSocket server on ws://" << host << ":" << port << std::endl;
        server.startAccept();
        std::cout << "Running io_context..." << std::endl;
        ioc.run();
        std::cout << "Waiting for clients..." << std::endl;
    }
    catch (const std::exception &ex)
    {
        std::cerr << "Fatal error: " << ex.what() << "\n";
    }
}