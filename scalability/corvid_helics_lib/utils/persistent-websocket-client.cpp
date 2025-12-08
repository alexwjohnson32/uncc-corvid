#include "persistent-websocket-client.hpp"

namespace connections
{

PersistentWebSocketClient::PersistentWebSocketClient(boost::asio::io_context &ioc, const std::string &host,
                                                     const std::string &port, const std::string &target,
                                                     unsigned int timeout_seconds)
    : m_io_context(ioc), m_host(host), m_port(port), m_target(target), m_timeout_seconds(timeout_seconds),
      m_resolver(ioc), m_ws(ioc), m_reconnect_timer(ioc)
{
}

void PersistentWebSocketClient::Start()
{
    m_stopping = false;
    Resolve();
}

void PersistentWebSocketClient::Stop()
{
    m_stopping = true;
    boost::system::error_code ec;

    m_ws.next_layer().shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
    m_ws.next_layer().close(ec);
}

void PersistentWebSocketClient::Send(const std::string &msg)
{
    m_write_queue.push_back(msg);
    if (!m_write_in_progress) StartWrite();
}

void PersistentWebSocketClient::Resolve()
{
    m_resolver.async_resolve(
        m_host, m_port,
        [self = shared_from_this()](boost::system::error_code ec, boost::asio::ip::tcp::resolver::results_type results)
        {
            if (ec) return self->Fail(ec);
            self->Connect(results);
        });
}

void PersistentWebSocketClient::Connect(boost::asio::ip::tcp::resolver::results_type results)
{
    boost::asio::async_connect(m_ws.next_layer(), results,
                               [self = shared_from_this()](boost::system::error_code ec, const auto &)
                               {
                                   if (ec) return self->Fail(ec);
                                   self->Handshake();
                               });
}

void PersistentWebSocketClient::Handshake()
{
    m_ws.async_handshake(m_host, m_target,
                         [self = shared_from_this()](boost::system::error_code ec)
                         {
                             if (ec) return self->Fail(ec);

                             if (self->m_on_connect) self->m_on_connect();

                             self->Read();
                         });
}

void PersistentWebSocketClient::Read()
{
    m_ws.async_read(m_buffer,
                    [self = shared_from_this()](boost::system::error_code ec, std::size_t bytes)
                    {
                        (void)bytes;

                        if (ec) return self->Fail(ec);

                        std::string msg(boost::asio::buffer_cast<const char *>(self->m_buffer.data()),
                                        self->m_buffer.size());

                        self->m_buffer.consume(self->m_buffer.size());

                        if (self->m_on_message) self->m_on_message(msg);

                        self->Read();
                    });
}

void PersistentWebSocketClient::StartWrite()
{
    if (m_write_queue.empty()) return;

    m_write_in_progress = true;

    const std::string &front = m_write_queue.front();

    m_ws.async_write(boost::asio::buffer(front),
                     [self = shared_from_this()](boost::system::error_code ec, std::size_t bytes)
                     { self->FinishWrite(ec, bytes); });
}

void PersistentWebSocketClient::FinishWrite(const boost::system::error_code &ec, std::size_t)
{
    if (ec) return Fail(ec);

    m_write_queue.pop_front();

    if (!m_write_queue.empty())
        StartWrite();
    else
        m_write_in_progress = false;
}

void PersistentWebSocketClient::Fail(const boost::system::error_code &ec)
{
    if (m_on_error) m_on_error(ec);

    if (!m_stopping) Reconnect();
}

void PersistentWebSocketClient::Reconnect()
{
    m_reconnect_timer.expires_after(std::chrono::seconds(3));
    m_reconnect_timer.async_wait(
        [self = shared_from_this()](boost::system::error_code ec)
        {
            if (!ec && !self->m_stopping) self->Resolve();
        });
}

} // namespace connections
