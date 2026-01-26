#include "websocket_client.hpp"
#include "mock_websocket_server.hpp"

#include <gtest/gtest.h>

class WebSocketClientTest : public ::testing::Test
{
  protected:
    void SetUp() override { m_client = std::make_shared<utils::WebSocketClient>(); }

    void TearDown() override
    {
        m_client->CloseConnection();
        m_client->StopRun();
    }

    std::shared_ptr<utils::WebSocketClient> m_client;
};

TEST_F(WebSocketClientTest, RangeBasedConnectionTest)
{
    utils::tests::MockWebSocketServer server;
    server.RunOnce();

    std::atomic<bool> connected{ false };
    boost::system::error_code result_ec;

    m_client->AsyncRun();
    m_client->Connect("127.0.0.1", server.GetPort(), "/",
                      [&](const boost::system::error_code &ec)
                      {
                          result_ec = ec;
                          connected = true;
                      });

    auto start = std::chrono::steady_clock::now();
    while (!connected && (std::chrono::steady_clock::now() - start < std::chrono::seconds(2)))
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    EXPECT_TRUE(connected);
    EXPECT_FALSE(result_ec);

    // Sequence matters: close connection, then stop server
    m_client->CloseConnection();
    server.Stop();
}

TEST_F(WebSocketClientTest, RangeBasedEchoTest)
{
    utils::tests::MockWebSocketServer server;
    server.RunOnce();

    std::string received_msg;
    std::atomic<bool> msg_received{ false };

    m_client->AsyncRun();
    m_client->Connect("127.0.0.1", server.GetPort(), "/", [](const boost::system::error_code &) {});

    m_client->SetOnMessage(
        [&](const std::string &msg)
        {
            received_msg = msg;
            msg_received = true;
        });

    // Short wait for handshake to complete
    const std::string testing_message{ "Testing For Connection" };
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    m_client->Send(testing_message);

    auto start = std::chrono::steady_clock::now();
    while (!msg_received && (std::chrono::steady_clock::now() - start < std::chrono::seconds(2)))
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    EXPECT_EQ(received_msg, testing_message);

    // Sequence matters: close connection, then stop server
    m_client->CloseConnection();
    server.Stop();
}