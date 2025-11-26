#include "persistent-websocket-client.hpp"
#include <boost/asio/bind_executor.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/dispatch.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/asio/strand.hpp>
#include <boost/beast/websocket/rfc6455.hpp>
#include <boost/system/error_code.hpp>
#include <chrono>
#include <cstddef>
#include <iostream>
#include <queue>

connections::PersistentWebSocketClient::PersistentWebSocketClient(boost::asio::io_context &io_context,
                                                                  const std::string &host, const std::string &port,
                                                                  const std::string &target)
    : m_io_context(io_context), m_resolver(io_context), m_websocket(io_context), m_strand(io_context.get_executor()),
      m_host(host), m_port(port), m_target(target), m_write_in_progress(false), m_manual_close(false)
{
}

void connections::PersistentWebSocketClient::Start()
{
    boost::asio::dispatch(m_strand, std::bind(&PersistentWebSocketClient::StartResolve, shared_from_this()));
}

void connections::PersistentWebSocketClient::SetMessageCallback(
    connections::PersistentWebSocketClient::MessageCallback callback)
{
    m_message_callback = callback;
}

void connections::PersistentWebSocketClient::SendMessage(const std::string &msg)
{
    auto msg_lambda = [self = shared_from_this(), msg]()
    {
        if (self->m_manual_close) return;

        self->m_write_queue.push(msg);

        if (!self->m_write_in_progress)
        {
            self->m_write_in_progress = true;
            self->StartWrite();
        }
    };

    boost::asio::post(m_strand, msg_lambda);
}

void connections::PersistentWebSocketClient::Close()
{
    auto close_lambda = [self = shared_from_this()]()
    {
        if (self->m_manual_close) return;

        self->m_manual_close = true;

        // Clear the queue
        self->m_write_queue = std::queue<std::string>();

        if (self->m_websocket.next_layer().is_open())
        {
            // If handshake not yet done, next_layer() close is safer
            if (!self->m_websocket.is_open())
            {
                boost::system::error_code ignored;
                self->m_websocket.next_layer().close(ignored);
            }
            else
            {
                self->m_websocket.async_close(
                    boost::beast::websocket::close_code::normal,
                    boost::asio::bind_executor(self->m_strand,
                                               [self](const boost::system::error_code &)
                                               {
                                                   boost::system::error_code ignored;
                                                   if (self->m_websocket.next_layer().is_open())
                                                       self->m_websocket.next_layer().close(ignored);
                                               }));
            }
        }
    };

    boost::asio::dispatch(m_strand, close_lambda);
}

void connections::PersistentWebSocketClient::StartResolve()
{
    m_resolver.async_resolve(
        m_host, m_port,
        boost::asio::bind_executor(m_strand, std::bind(&PersistentWebSocketClient::OnResolve, shared_from_this(),
                                                       std::placeholders::_1, std::placeholders::_2)));
}

void connections::PersistentWebSocketClient::OnResolve(const boost::system::error_code &ec,
                                                       boost::asio::ip::tcp::resolver::results_type results)
{
    if (ec)
    {
        ScheduleReconnect();
        return;
    }

    StartConnect(results);
}

void connections::PersistentWebSocketClient::StartConnect(boost::asio::ip::tcp::resolver::results_type results)
{
    boost::asio::async_connect(
        m_websocket.next_layer(), results,
        boost::asio::bind_executor(
            m_strand, std::bind(&PersistentWebSocketClient::OnConnect, shared_from_this(), std::placeholders::_1)));
}

void connections::PersistentWebSocketClient::OnConnect(const boost::system::error_code &ec)
{
    if (ec)
    {
        ScheduleReconnect();
        return;
    }

    StartHandshake();
}

void connections::PersistentWebSocketClient::StartHandshake()
{
    m_websocket.async_handshake(
        m_host, m_target,
        boost::asio::bind_executor(
            m_strand, std::bind(&PersistentWebSocketClient::OnHandshake, shared_from_this(), std::placeholders::_1)));
}

void connections::PersistentWebSocketClient::OnHandshake(const boost::system::error_code &ec)
{
    if (ec)
    {
        ScheduleReconnect();
        return;
    }

    // Ensure buffer is empty before starting reads for safety
    m_buffer.consume(m_buffer.size());

    StartRead();
}

void connections::PersistentWebSocketClient::StartRead()
{
    m_websocket.async_read(
        m_buffer, boost::asio::bind_executor(m_strand, std::bind(&PersistentWebSocketClient::OnRead, shared_from_this(),
                                                                 std::placeholders::_1, std::placeholders::_2)));
}

void connections::PersistentWebSocketClient::OnRead(const boost::system::error_code &ec, std::size_t)
{
    if (ec)
    {
        ScheduleReconnect();
        return;
    }

    std::string msg(static_cast<const char *>(m_buffer.data().data()), m_buffer.data().size());
    m_buffer.consume(m_buffer.size());

    if (m_message_callback) m_message_callback(msg);

    StartRead();
}

void connections::PersistentWebSocketClient::StartWrite()
{
    if (m_write_queue.empty())
    {
        m_write_in_progress = false;
        return;
    }

    const std::string &msg = m_write_queue.front();

    m_websocket.async_write(
        boost::asio::buffer(msg),
        boost::asio::bind_executor(m_strand, std::bind(&PersistentWebSocketClient::OnWrite, shared_from_this(),
                                                       std::placeholders::_1, std::placeholders::_2)));
}

void connections::PersistentWebSocketClient::OnWrite(const boost::system::error_code &ec, std::size_t)
{
    if (ec)
    {
        ScheduleReconnect();
        return;
    }

    m_write_queue.pop();
    StartWrite();
}

void connections::PersistentWebSocketClient::ScheduleReconnect()
{
    if (m_manual_close) return;

    // Close only if open
    if (m_websocket.next_layer().is_open())
    {
        boost::system::error_code ignored;
        m_websocket.next_layer().close(ignored);
    }

    auto timer = std::make_shared<boost::asio::steady_timer>(m_io_context);
    timer->expires_after(std::chrono::seconds(3));

    timer->async_wait(boost::asio::bind_executor(m_strand,
                                                 [self = shared_from_this(), timer](const boost::system::error_code &)
                                                 {
                                                     if (!self->m_manual_close) self->StartResolve();
                                                 }));
}