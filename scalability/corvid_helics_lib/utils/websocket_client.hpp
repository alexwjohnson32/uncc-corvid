#pragma once

#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/executor_work_guard.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/core/error.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/system/error_code.hpp>
#include <string>
#include <functional>
#include <queue>
#include <memory>
#include <thread>
#include <optional>

namespace utils
{
class WebSocketClient : public std::enable_shared_from_this<WebSocketClient>
{
  public:
    WebSocketClient();
    ~WebSocketClient();

    // Prevent copying, allow moving
    WebSocketClient(const WebSocketClient &) = delete;
    WebSocketClient &operator=(const WebSocketClient &) = delete;

    /**
     * @brief Connects to the WebSocket server asynchronously.
     * @param host The hostname (e.g., "localhost")
     * @param port The port (e.g., "8080")
     * @param target The path (e.g., "/api/ws")
     * @param on_connect Callback fired when connection succeeds or fails.
     */
    void Connect(const std::string &host, const std::string &port, const std::string &target,
                 std::function<void(const boost::system::error_code &)> on_connect);

    /**
     * @brief Queues a message to be sent asynchronously.
     * Thread-safe.
     */
    void Send(const std::string &message);

    /**
     * @brief Sets the callback to be fired when a message is received.
     */
    void SetOnMessage(std::function<void(const std::string &)> callback);

    /**
     * @brief Sets the callback to be fired when an error occurs during Read/Write.
     */
    void SetOnError(std::function<void(const boost::system::error_code &, const std::string &)> callback);

    /**
     * @brief Initiates the WebSocket close handshake.
     * Does NOT stop the IO thread.
     */
    void CloseConnection();

    /**
     * @brief Spawns a background thread to run the IO context.
     * Returns immediately. Callbacks will run on the background thread.
     */
    void AsyncRun();

    /**
     * @brief Runs the IO context on the CURRENT thread.
     * BLOCKS until StopContext() is called. Callbacks run on this thread.
     */
    void BlockingRun();

    /**
     * @brief Stops the IO context and joins the background thread (if AsyncRun was used).
     */
    void StopRun();

  private:
    // Data members
    boost::asio::io_context m_ioc;
    std::optional<boost::asio::executor_work_guard<boost::asio::io_context::executor_type>> m_work_guard;
    boost::asio::ip::tcp::resolver m_resolver;
    boost::beast::websocket::stream<boost::asio::ip::tcp::socket> m_ws;

    boost::beast::flat_buffer m_buffer;
    std::string m_host;
    std::string m_target; // saved for handshake

    // Write queue
    std::queue<std::string> m_write_queue;

    // User callbacks
    std::function<void(const boost::system::error_code &)> m_on_connect;
    std::function<void(const std::string &)> m_on_message;
    std::function<void(const boost::system::error_code &, const std::string &)> m_on_error;

    // Error suppression state
    boost::system::error_code m_last_error;

    // Thread management
    std::thread m_io_thread;

    // Internal async steps
    void OnResolve(boost::beast::error_code ec, boost::asio::ip::tcp::resolver::results_type results);
    void OnConnect(boost::beast::error_code ec, boost::asio::ip::tcp::resolver::results_type::endpoint_type ep);
    void OnHandshake(boost::beast::error_code ec);

    // ReadLoop
    void DoRead();
    void OnRead(boost::beast::error_code ec, std::size_t bytes_transferred);

    // Write Logic
    void DoWrite();
    void OnWrite(boost::beast::error_code ec, std::size_t bytes_transferred);

    // Helper functions to filter duplicates
    void ReportError(boost::beast::error_code ec, const std::string &context);
    void ClearErrorState();
};
} // namespace utils