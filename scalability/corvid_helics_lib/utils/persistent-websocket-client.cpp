#include "persistent-websocket-client.hpp"
#include <boost/asio/connect.hpp>
#include <boost/beast/core/buffers_to_string.hpp>
#include <boost/system/error_code.hpp>
#include <iostream>
#include <chrono>
#include <thread>

void connections::PersistentWebSocketClient::Run()
{
    Connect();
    m_io_context.run();
}

void connections::PersistentWebSocketClient::Connect()
{
    std::cout << "Resolving host: " << m_host << ":" << m_port << std::endl;

    m_resolver.async_resolve(m_host, m_port,
                             std::bind(&connections::PersistentWebSocketClient::OnResolve, this, std::placeholders::_1,
                                       std::placeholders::_2));
}

void connections::PersistentWebSocketClient::PerformHandshake()
{
    std::cout << "Performing WebSocket handshake..." << std::endl;

    m_ws.async_handshake(m_host, m_path,
                         [this](boost::system::error_code ec)
                         {
                             if (ec)
                             {
                                 std::cerr << "Handshake failed: " << ec.message() << std::endl;
                                 ScheduleReconnect();
                                 return;
                             }

                             std::cout << "Connected successfully to " << m_host << std::endl;

                             ReadMessage();
                         });
}

void connections::PersistentWebSocketClient::SendMessage(const std::string &message)
{
    // Post into the io_context to ensure thread safety
    boost::asio::post(m_io_context,
                      [this, message]()
                      {
                          m_send_queue.push_back(message);

                          if (!m_write_in_progress)
                          {
                              m_write_in_progress = true;
                              DoWrite();
                          }
                      });
}

void connections::PersistentWebSocketClient::DoWrite()
{
    if (m_send_queue.empty())
    {
        m_write_in_progress = false;
        return;
    }

    const std::string &msg = m_send_queue.front();

    m_ws.async_write(boost::asio::buffer(msg),
                     [this](boost::system::error_code ec, std::size_t bytes_transferred)
                     {
                         if (ec)
                         {
                             std::cerr << "Send failed: " << ec.message() << std::endl;
                             ScheduleReconnect();
                             return;
                         }

                         std::cout << "Sent " << bytes_transferred << " bytes" << std::endl;

                         m_send_queue.pop_front();
                         DoWrite(); // Continue sending queued messages
                     });
}

void connections::PersistentWebSocketClient::ReadMessage()
{
    m_ws.async_read(m_buffer,
                    [this](boost::system::error_code ec, std::size_t bytes_transferred)
                    {
                        if (ec)
                        {
                            std::cerr << "Read failed: " << ec.message() << std::endl;
                            ScheduleReconnect();
                            return;
                        }

                        std::string message(boost::beast::buffers_to_string(m_buffer.data()));
                        m_buffer.consume(m_buffer.size());

                        std::cout << "Received: " << message << std::endl;

                        // Continue reading messages
                        ReadMessage();
                    });
}

void connections::PersistentWebSocketClient::ScheduleReconnect()
{
    std::cout << "Reconnecting in 5 seconds..." << std::endl;

    boost::system::error_code ignored_ec;
    if (m_ws.next_layer().is_open()) m_ws.next_layer().close(ignored_ec);

    m_reconnect_timer.expires_after(std::chrono::seconds(5));
    m_reconnect_timer.async_wait(
        [this](const boost::system::error_code &)
        {
            std::cout << "Attempting reconnect..." << std::endl;

            // Recreate the websocket stream
            m_ws = boost::beast::websocket::stream<boost::asio::ip::tcp::socket>(m_io_context);

            // Reset write state
            m_write_in_progress = false;
            m_send_queue.clear();

            Connect();
        });
}

void connections::PersistentWebSocketClient::OnResolve(const boost::system::error_code &ec,
                                                       boost::asio::ip::tcp::resolver::results_type results)
{
    if (ec)
    {
        std::cerr << "Resolve failed: " << ec.message() << std::endl;
        ScheduleReconnect();
        return;
    }

    std::cout << "Connecting..." << std::endl;
    boost::asio::async_connect(m_ws.next_layer(), results,
                               std::bind(&connections::PersistentWebSocketClient::OnConnect, this,
                                         std::placeholders::_1, std::placeholders::_2));
}
void connections::PersistentWebSocketClient::OnConnect(const boost::system::error_code &ec,
                                                       const boost::asio::ip::tcp::endpoint &)
{
    if (ec)
    {
        std::cerr << "Connect failed: " << ec.message() << std::endl;
        ScheduleReconnect();
        return;
    }
    PerformHandshake();
}