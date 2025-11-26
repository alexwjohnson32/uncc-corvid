#pragma once

#include <boost/asio.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/beast.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/system/error_code.hpp>
#include <string>
#include <queue>
#include <functional>
#include <memory>
#include <chrono>

namespace connections
{
class PersistentWebSocketClient : public std::enable_shared_from_this<PersistentWebSocketClient>
{
  public:
    using MessageCallback = std::function<void(const std::string &)>;

    PersistentWebSocketClient(boost::asio::io_context &io_context, const std::string &host, const std::string &port,
                              const std::string &target);

    void Start();
    void SendMessage(const std::string &msg);
    void SetMessageCallback(MessageCallback callback);
    void Close();

  private:
    boost::asio::io_context &m_io_context;
    boost::asio::ip::tcp::resolver m_resolver;
    boost::beast::websocket::stream<boost::asio::ip::tcp::socket> m_websocket;
    boost::asio::strand<boost::asio::io_context::executor_type> m_strand;

    std::string m_host;
    std::string m_port;
    std::string m_target;

    boost::beast::flat_buffer m_buffer;

    std::queue<std::string> m_write_queue;
    bool m_write_in_progress;
    bool m_manual_close;

    MessageCallback m_message_callback;

    void StartResolve();
    void OnResolve(const boost::system::error_code &ec, boost::asio::ip::tcp::resolver::results_type results);

    void StartConnect(boost::asio::ip::tcp::resolver::results_type results);
    void OnConnect(const boost::system::error_code &ec);

    void StartHandshake();
    void OnHandshake(const boost::system::error_code &ec);

    void StartRead();
    void OnRead(const boost::system::error_code &ec, std::size_t bytes);

    void StartWrite();
    void OnWrite(const boost::system::error_code &ec, std::size_t bytes);

    void ScheduleReconnect();
};
} // namespace connections