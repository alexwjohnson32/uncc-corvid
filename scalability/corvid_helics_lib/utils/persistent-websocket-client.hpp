#pragma once

#include <boost/asio/io_context.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/system/error_code.hpp>
#include <string>
#include <deque>

namespace connections
{

/**
 * The bones of this is generated from chatpgpt, so take everything under scrutiny
 */
class PersistentWebSocketClient
{
  private:
    const std::string m_host;
    const std::string m_port;
    const std::string m_path;

    boost::asio::io_context m_io_context;
    boost::asio::ip::tcp::resolver m_resolver;
    boost::beast::websocket::stream<boost::asio::ip::tcp::socket> m_ws;
    boost::asio::steady_timer m_reconnect_timer;
    boost::beast::flat_buffer m_buffer;

    std::deque<std::string> m_send_queue;
    bool m_write_in_progress;

    void Connect();
    void OnConnect(const boost::system::error_code &ec, const boost::asio::ip::tcp::endpoint &);
    void OnResolve(const boost::system::error_code &ec, boost::asio::ip::tcp::resolver::results_type results);
    void PerformHandshake();
    void ReadMessage();

    void DoWrite();

    void ScheduleReconnect();

  public:
    PersistentWebSocketClient(const std::string &host, const std::string &port, const std::string &path)
        : m_host(host), m_port(port), m_path(path), m_io_context(), m_resolver(m_io_context), m_ws(m_io_context),
          m_reconnect_timer(m_io_context), m_buffer(), m_send_queue(), m_write_in_progress(false)
    {
    }

    void Run();
    void SendMessage(const std::string &message); // Thread safe
};

} // namespace connections