#include "websocket_client.hpp"

#include <boost/asio/connect.hpp>
#include <boost/asio/executor_work_guard.hpp>
#include <boost/asio/post.hpp>
#include <boost/beast/core/buffers_to_string.hpp>
#include <boost/beast/core/error.hpp>
#include <boost/beast/websocket/rfc6455.hpp>
#include <boost/beast/websocket/stream.hpp>
#include <cstddef>
#include <functional>
#include <iostream>

utils::WebSocketClient::WebSocketClient()
    : m_ioc(), m_work_guard(boost::asio::make_work_guard(m_ioc)), m_resolver(m_ioc), m_ws(m_ioc)
{
    // Set User-Agent
    m_ws.set_option(boost::beast::websocket::stream_base::decorator(
        [](boost::beast::websocket::request_type &req)
        {
            req.set(boost::beast::http::field::user_agent,
                    std::string(BOOST_BEAST_VERSION_STRING) + "websocket-client-async");
        }));
}

utils::WebSocketClient::~WebSocketClient() { StopRun(); }

void utils::WebSocketClient::AsyncRun()
{
    if (!m_io_thread.joinable())
    {
        m_io_thread = std::thread([this]() { m_ioc.run(); });
    }
}

void utils::WebSocketClient::StopRun()
{
    m_work_guard.reset(); // Allow run() to exit if out of work
    m_ioc.stop();         // Force stop

    if (m_io_thread.joinable())
    {
        m_io_thread.join();
    }

    m_ioc.restart();                                    // Reset for potential reuse
    m_work_guard.emplace(boost::asio::make_work_guard(m_ioc)); // Re-acquire work guard
}

void utils::WebSocketClient::BlockingRun() { m_ioc.run(); }

void utils::WebSocketClient::Connect(const std::string &host, const std::string &port, const std::string &target,
                                     std::function<void(const boost::system::error_code &)> on_connect)
{
    m_host = host;
    m_target = target;
    m_on_connect = on_connect;

    // Start resolution
    m_resolver.async_resolve(
        host, port,
        std::bind(&WebSocketClient::OnResolve, shared_from_this(), std::placeholders::_1, std::placeholders::_2));
}

void utils::WebSocketClient::OnResolve(boost::beast::error_code ec,
                                       boost::asio::ip::tcp::resolver::results_type results)
{
    if (ec)
    {
        if (m_on_connect) m_on_connect(ec);
        return;
    }

    // Connect via TCP
    boost::asio::async_connect(
        m_ws.next_layer(), results,
        std::bind(&WebSocketClient::OnConnect, shared_from_this(), std::placeholders::_1, std::placeholders::_2));
}

void utils::WebSocketClient::OnConnect(boost::beast::error_code ec,
                                       boost::asio::ip::tcp::resolver::results_type::endpoint_type)
{
    if (ec)
    {
        if (m_on_connect) m_on_connect(ec);
        return;
    }

    // Perform WebSocket Handshake
    m_ws.async_handshake(m_host, m_target,
                         std::bind(&WebSocketClient::OnHandshake, shared_from_this(), std::placeholders::_1));
}

void utils::WebSocketClient::OnHandshake(boost::beast::error_code ec)
{
    if (ec)
    {
        if (m_on_connect) m_on_connect(ec);
        return;
    }

    // Connection successful!
    if (m_on_connect) m_on_connect(ec); // ec is success (0) here

    // Start the Read Loop immediately
    DoRead();
}

void utils::WebSocketClient::SetOnMessage(std::function<void(const std::string &)> callback)
{
    m_on_message = callback;
}

void utils::WebSocketClient::SetOnError(
    std::function<void(const boost::system::error_code &, const std::string &)> callback)
{
    m_on_error = callback;
}

void utils::WebSocketClient::DoRead()
{
    m_ws.async_read(m_buffer, std::bind(&WebSocketClient::OnRead, shared_from_this(), std::placeholders::_1,
                                        std::placeholders::_2));
}

void utils::WebSocketClient::OnRead(boost::beast::error_code ec, std::size_t bytes_transferred)
{
    if (ec == boost::beast::websocket::error::closed)
    {
        // Normal closure, stop reading
        return;
    }

    if (ec)
    {
        if (m_on_error) m_on_error(ec, "Read Error");
        return;
    }

    // Process Message
    if (m_on_message)
    {
        m_on_message(boost::beast::buffers_to_string(m_buffer.data()));
    }

    // Clear buffer and queue next read
    m_buffer.consume(bytes_transferred);
    DoRead();
}

void utils::WebSocketClient::Send(const std::string &message)
{
    // Post to the IO context to ensure thread safety
    boost::asio::post(m_ioc,
                      [this, message, self = shared_from_this()]()
                      {
                          bool write_in_progress = !m_write_queue.empty();
                          m_write_queue.push(message);

                          // If not currently writing, kick off the write loop
                          if (!write_in_progress)
                          {
                              DoWrite();
                          }
                      });
}

void utils::WebSocketClient::DoWrite()
{
    // Note: We are already in the io_context strand here because DoWrite
    // is called from Send's post() or OnWrite

    if (m_write_queue.empty())
    {
        return;
    }

    m_ws.async_write(
        boost::asio::buffer(m_write_queue.front()),
        std::bind(&WebSocketClient::OnWrite, shared_from_this(), std::placeholders::_1, std::placeholders::_2));
}

void utils::WebSocketClient::OnWrite(boost::beast::error_code ec, std::size_t)
{
    if (ec)
    {
        if (m_on_error) m_on_error(ec, "Write Error");

        // Even on error, we usually pop the failed message and try the next,
        // or clear the queue depending on desired robustness.
        // Here we pop and try to continue.
        m_write_queue.pop();
        if (!m_write_queue.empty())
        {
            DoWrite();
        }

        return;
    }

    // Success, remove the sent message
    m_write_queue.pop();

    // If more messages, send the next one
    if (!m_write_queue.empty())
    {
        DoWrite();
    }
}

void utils::WebSocketClient::CloseConnection()
{
    boost::asio::post(m_ioc,
                      [this, self = shared_from_this()]()
                      {
                          if (m_ws.is_open())
                          {
                              m_ws.async_close(boost::beast::websocket::close_code::normal,
                                               [self](boost::beast::error_code ec)
                                               {
                                                   // Optional: Handle close completion
                                               });
                          }
                      });
}