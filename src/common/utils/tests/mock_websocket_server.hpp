#pragma once

#include <boost/asio/io_context.hpp>
#include <chrono>
#include <atomic>

#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <thread>

namespace utils
{
namespace tests
{
/**
 * @brief Minimal WebSocket Echo Server for testing.
 */
class MockWebSocketServer
{
  public:
    MockWebSocketServer() : m_acceptor(m_ioc) { Initialize(22000, 22100); }
    MockWebSocketServer(uint16_t start, uint16_t end) : m_acceptor(m_ioc) { Initialize(start, end); }
    ~MockWebSocketServer() { Stop(); }

    void RunOnce()
    {
        m_server_thread = std::thread(
            [this]()
            {
                try
                {
                    boost::asio::ip::tcp::socket socket(m_ioc);
                    m_acceptor.accept(socket);

                    boost::beast::websocket::stream<boost::asio::ip::tcp::socket> ws(std::move(socket));
                    ws.accept();

                    while (m_running)
                    {
                        boost::beast::flat_buffer buffer;
                        ws.read(buffer);
                        ws.text(ws.got_text());
                        ws.write(buffer.data());
                    }
                }
                catch (...)
                {
                    // Shutdown Expected?
                }
            });
    }

    void Stop()
    {
        m_running = false;

        // Force the acceptor to stop waiting
        if (m_acceptor.is_open())
        {
            boost::system::error_code ec;
            m_acceptor.close(ec);
        }

        // Force the active socket to close, breaking the blocking ws.read()
        if (m_socket && m_socket->is_open())
        {
            boost::system::error_code ec;
            m_socket->close(ec);
        }

        m_ioc.stop();
        if (m_server_thread.joinable())
        {
            m_server_thread.join();
        }
    }

    std::string GetPort() const { return std::to_string(m_port_num); }

  private:
    boost::asio::io_context m_ioc;
    boost::asio::ip::tcp::acceptor m_acceptor;
    std::unique_ptr<boost::asio::ip::tcp::socket> m_socket;
    uint16_t m_port_num;
    std::thread m_server_thread;
    std::atomic<bool> m_running{ true };

    void Initialize(uint16_t start, uint16_t end)
    {
        m_port_num = FindAvailablePort(start, end);

        m_acceptor.open(boost::asio::ip::tcp::v4());
        m_acceptor.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
        m_acceptor.bind({ boost::asio::ip::make_address("127.0.0.1"), m_port_num });
        m_acceptor.listen();
    }

    uint16_t FindAvailablePort(uint16_t start, uint16_t end)
    {
        std::stringstream error_string;

        if (end < start)
        {
            error_string << "You must place the lower port value as start. start=" << start << ", end=" << end << "\n";
            throw std::runtime_error(error_string.str());
        }
        boost::asio::io_context ioc;
        for (uint16_t port = start; port <= end; ++port)
        {
            try
            {
                boost::asio::ip::tcp::acceptor acceptor(ioc);
                acceptor.open(boost::asio::ip::tcp::v4());
                acceptor.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
                acceptor.bind({ boost::asio::ip::make_address("127.0.0.1"), port });
                return port;
            }
            catch (...)
            {
                continue; // Port in use, try next
            }
        }
        error_string << "No available ports in range " << start << "-" << end;
        throw std::runtime_error(error_string.str());
    }
};
} // namespace tests
} // namespace utils