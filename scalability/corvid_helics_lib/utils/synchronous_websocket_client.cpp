#include "synchronous_websocket_client.hpp"

#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <cstdlib>
#include <string>

connections::SynchronousWebSocketClient::SynchronousWebSocketClient() : m_resolver(m_ioc), m_ws(m_ioc)
{
    m_ws.set_option(boost::beast::websocket::stream_base::decorator(
        [](boost::beast::websocket::request_type &req)
        {
            req.set(boost::beast::http::field::user_agent,
                    std::string(BOOST_BEAST_VERSION_STRING) + " websocket-client-coro");
        }));
}

void connections::SynchronousWebSocketClient::Connect(const std::string &host, const std::string &port,
                                                      const std::string &target)
{
    // Resolve DNS
    auto results = m_resolver.resolve(host, port);

    // Connect via TCP
    boost::asio::connect(m_ws.next_layer(), results);

    // Perform WebSocket handshake
    m_ws.handshake(host, target);
}

void connections::SynchronousWebSocketClient::Send(const std::string &message)
{
    m_ws.write(boost::asio::buffer(message));
}

std::string connections::SynchronousWebSocketClient::Receive()
{
    boost::beast::flat_buffer buffer;
    m_ws.read(buffer);

    return boost::beast::buffers_to_string(buffer.data());
}

void connections::SynchronousWebSocketClient::Close()
{
    boost::beast::websocket::close_reason reason;
    reason.code = boost::beast::websocket::close_code::normal;
    reason.reason = "Client closed";

    m_ws.close(reason);
}
