#pragma once

#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <string>

namespace connections
{
class SynchronousWebSocketClient
{
  private:
    boost::asio::io_context m_ioc;
    boost::asio::ip::tcp::resolver m_resolver;
    boost::beast::websocket::stream<boost::asio::ip::tcp::socket> m_ws;

  public:
    SynchronousWebSocketClient();

    // Connect and perform WebSocket handshake
    void Connect(const std::string &host, const std::string &port, const std::string &target = "/");

    // Send text message (blocking)
    void Send(const std::string &message);

    // Receive one message (blocking)
    std::string Receive();

    // Close connection cleanly
    void Close();
};
} // namespace connections