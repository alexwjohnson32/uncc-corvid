#pragma once

#include <boost/asio.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>

#include <deque>
#include <memory>
#include <string>
#include <functional>

namespace connections
{

class PersistentWebSocketClient : public std::enable_shared_from_this<PersistentWebSocketClient>
{
  public:
    using OnMessageCallback = std::function<void(const std::string &)>;
    using OnConnectCallback = std::function<void()>;
    using OnErrorCallback = std::function<void(const boost::system::error_code &)>;

  public:
    PersistentWebSocketClient(boost::asio::io_context &ioc, const std::string &host, const std::string &port,
                              const std::string &target, unsigned int timeout_seconds);

    void Start();
    void Stop();
    void Send(const std::string &msg);

    void SetOnMessage(OnMessageCallback cb) { m_on_message = std::move(cb); }
    void SetOnConnect(OnConnectCallback cb) { m_on_connect = std::move(cb); }
    void SetOnError(OnErrorCallback cb) { m_on_error = std::move(cb); }

  private:
    void Resolve();
    void Connect(boost::asio::ip::tcp::resolver::results_type results);
    void Handshake();
    void Read();
    void Reconnect();
    void Fail(const boost::system::error_code &ec);

    void StartWrite();
    void FinishWrite(const boost::system::error_code &ec, std::size_t bytes);

  private:
    boost::asio::io_context &m_io_context;

    std::string m_host;
    std::string m_port;
    std::string m_target;
    unsigned int m_timeout_seconds;

    boost::asio::ip::tcp::resolver m_resolver;
    boost::beast::websocket::stream<boost::asio::ip::tcp::socket> m_ws;

    boost::beast::flat_buffer m_buffer;
    boost::asio::steady_timer m_reconnect_timer;

    bool m_stopping = false;
    bool m_write_in_progress = false;

    std::deque<std::string> m_write_queue;

    OnMessageCallback m_on_message;
    OnConnectCallback m_on_connect;
    OnErrorCallback m_on_error;
};

} // namespace connections
