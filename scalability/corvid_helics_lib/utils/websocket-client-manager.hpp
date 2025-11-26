#pragma once

#include "persistent-websocket-client.hpp"

#include <boost/asio/io_context.hpp>
#include <string>
#include <thread>
#include <functional>

namespace connections
{
struct ClientDetails
{
    std::string address{};
    std::string port{};
    std::string websocket_path{};
};

class WebSocketClientManager
{
  private:
    const ClientDetails m_details;
    boost::asio::io_context m_io_context;
    connections::PersistentWebSocketClient m_client;
    std::thread m_io_thread;

  public:
    WebSocketClientManager() = delete;
    WebSocketClientManager(const ClientDetails &details)
        : m_details(details), m_io_context(),
          m_client(m_io_context, details.address, details.port, details.websocket_path), m_io_thread()
    {
        // Start io_context first
        m_io_thread = std::thread([this]() { m_io_context.run(); });

        // Post Start() to io_context to ensure safety
        m_io_context.post([this]() { m_client.Start(); });
    }

    ~WebSocketClientManager() { Close(); }

    void SetMessageCallback(std::function<void(const std::string &)> callback)
    {
        m_client.SetMessageCallback(callback);
    }

    void SendMessage(const std::string &msg) { m_client.SendMessage(msg); }

    ClientDetails GetClientDetails() const { return m_details; }

    bool IsOpen() const { return m_io_thread.joinable(); }

    void Close()
    {
        if (m_io_thread.joinable())
        {
            m_client.Close();
            m_io_context.stop();
            m_io_thread.join();
        }
    }
};
} // namespace connections