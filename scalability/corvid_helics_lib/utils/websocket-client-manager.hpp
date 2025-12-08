#pragma once

#include "persistent-websocket-client.hpp"
#include <boost/asio.hpp>
#include <boost/asio/io_context.hpp>
#include <thread>
#include <atomic>
#include <functional>
#include <string>
#include <iostream>

namespace connections
{

struct ClientDetails
{
    std::string host{};
    std::string port{};
    std::string path{ "/" };
    uint timeout_seconds{ 15 };
};

class WebSocketClientManager
{
  private:
    boost::asio::io_context &m_ioc;
    std::shared_ptr<PersistentWebSocketClient> m_client;
    std::thread m_io_thread;
    std::atomic<bool> m_stopping;
    boost::asio::signal_set m_signals;

    // Optional user callbacks
    std::function<void()> m_on_connect;
    std::function<void(const std::string &)> m_on_message;
    std::function<void(const boost::system::error_code &)> m_on_error;

    void InitCallbacks()
    {
        // Handle Ctrl+C / termination
        m_signals.async_wait(
            [this](const boost::system::error_code &, int)
            {
                std::cout << "\n[Signal] Exiting...\n";
                Stop();
            });

        // Forward client callbacks to wrapper callbacks if provided
        m_client->SetOnConnect(
            [this]()
            {
                if (m_on_connect) m_on_connect();
            });

        m_client->SetOnMessage(
            [this](const std::string &msg)
            {
                if (m_on_message) m_on_message(msg);
            });

        m_client->SetOnError(
            [this](const boost::system::error_code &ec)
            {
                if (m_on_error) m_on_error(ec);
            });
    }

  public:
    WebSocketClientManager(boost::asio::io_context &ioc, const ClientDetails &details)
        : m_ioc(ioc), m_stopping(false), m_client(std::make_shared<PersistentWebSocketClient>(
                                             ioc, details.host, details.port, details.path, details.timeout_seconds)),
          m_signals(ioc, SIGINT, SIGTERM)
    {
        InitCallbacks();
    }

    ~WebSocketClientManager()
    {
        Stop(); // Safe cleanup in destructor
    }

    // Start the client and io_context thread
    void Start()
    {
        m_stopping = false;
        if (m_client) m_client->Start();

        m_io_thread = std::thread([this]() { m_ioc.run(); });
    }

    // Stop the client and io_context, safely join thread
    void Stop()
    {
        bool expected = false;
        if (!m_stopping.compare_exchange_strong(expected, true)) return; // Already stopping

        if (m_client) m_client->Stop();

        m_ioc.stop();

        if (m_io_thread.joinable() && std::this_thread::get_id() != m_io_thread.get_id()) m_io_thread.join();
    }

    // Send a message through the internal client
    void Send(const std::string &msg)
    {
        if (m_client) m_client->Send(msg);
    }

    // Set optional callbacks
    void SetOnConnect(std::function<void()> on_connect) { m_on_connect = std::move(on_connect); }
    void SetOnMessage(std::function<void(const std::string &)> on_message) { m_on_message = std::move(on_message); }
    void SetOnError(std::function<void(const boost::system::error_code &)> on_error)
    {
        m_on_error = std::move(on_error);
    }

    // Access the internal client if needed
    std::shared_ptr<PersistentWebSocketClient> GetClient() const { return m_client; }
};

} // namespace connections
