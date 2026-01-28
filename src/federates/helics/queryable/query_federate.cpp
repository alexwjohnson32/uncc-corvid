#include "query_federate.hpp"
#include "stopwatch.hpp"

#include <string>
#include <stdexcept>
#include <sstream>
#include <thread>

#include <boost/system/error_code.hpp>

namespace
{

helics::MessageFederate GetFederate(const data::QueryFederateInput &config, utils::LocalLogHelper &log)
{
    helics::FederateInfo fi;
    fi.loadInfoFromJson(config.fed_info_json);

    helics::MessageFederate msg_fed(config.federate_name, fi);
    log << "Message Federate created successfully.\n";

    return msg_fed;
}

std::string DebugTimeQueryLoop(double granted_time, helics::MessageFederate &msg_fed)
{
    utils::Stopwatch loop_watch;
    loop_watch.Start();

    std::stringstream ss;
    ss << "\n##########################################\n";
    ss << "Granted Time: " << granted_time << "\n";
    ss << msg_fed.query("root", "global_time_debugging") << "\n";
    ss << "Query Execution Time: " << loop_watch.ElapsedMilliseconds() << " ms\n";
    ss << "##########################################\n";

    return ss.str();
}

std::string DiscreteQueriesLoop(double granted_time, helics::MessageFederate &msg_fed)
{
    utils::Stopwatch loop_watch;
    loop_watch.Start();

    std::stringstream ss;
    ss << "\n##########################################\n";
    ss << "Granted Time: " << granted_time << "\n";
    ss << "Name: " << msg_fed.query("root", "name") << "\n";
    ss << "Address: " << msg_fed.query("root", "address") << "\n";
    ss << "IsInit: " << msg_fed.query("root", "isinit") << "\n";
    ss << "IsConnected: " << msg_fed.query("root", "isconnected") << "\n";
    ss << "Query Execution Time: " << loop_watch.ElapsedMilliseconds() << " ms\n";
    ss << "##########################################\n";

    return ss.str();
}

double PerformLoop(helics::MessageFederate &msg_fed, const double total_time, const double period,
                   utils::LocalLogHelper &log)
{
    utils::Stopwatch main_watch;

    double granted_time = 0.0;

    main_watch.Start();
    while (granted_time + period <= total_time)
    {
        granted_time = msg_fed.requestTime(granted_time + period);

        // std::string output_string = DebugTimeQueryLoop(granted_time, msg_fed);
        std::string output_string = DiscreteQueriesLoop(granted_time, msg_fed);

        log << output_string;
    }
    double main_loop_ms = main_watch.ElapsedMilliseconds();

    std::stringstream end;
    end << "\n##########################################\n"
        << "Total Loop Time: " << main_loop_ms << " ms"
        << "\n##########################################\n"
        << "\nFederate finalized.\nGranted time: " << granted_time << "\n";
    log << end.str();

    return granted_time;
}

} // namespace

data::QueryFederate::QueryFederate() {}
data::QueryFederate::~QueryFederate() { Close(); }

void data::QueryFederate::Initialize(const data::QueryFederateInput &input)
{
    m_query_fed_input = input;
    m_log.SetOutputFile(m_query_fed_input.local_log_file);

    if (!m_log.IsOpen())
    {
        std::stringstream err_str;
        err_str << "Could not open file '" << m_query_fed_input.local_log_file << "'!" << std::endl;
        throw std::runtime_error(err_str.str());
    }

    m_client = std::make_shared<utils::WebSocketClient>();

    // Configure the client
    m_client->SetOnMessage([&log = this->m_log](const std::string &msg) { log << "Received: " << msg << std::endl; });
    m_client->SetOnError([&log = this->m_log](const boost::system::error_code &ec, const std::string &what)
                         { log << what << ": " << ec.message() << std::endl; });

    m_client->AsyncRun();
    m_client->Connect(m_query_fed_input.client_details.host, m_query_fed_input.client_details.port,
                      m_query_fed_input.client_details.target,
                      [&log = this->m_log](const boost::system::error_code &ec)
                      {
                          if (ec)
                          {
                              std::stringstream err_str;
                              err_str << "Error Connecting: " << ec.message() << "\n";
                              log << err_str.str();
                              throw std::runtime_error(err_str.str());
                          }
                          else
                          {
                              log << "Connected!\n";
                          }
                      });

    m_log.SetOnWriteCallback([&client = this->m_client](const std::string &msg) { client->Send(msg); });
}

void data::QueryFederate::Run()
{
    helics::MessageFederate msg_fed = GetFederate(m_query_fed_input, m_log);
    const double period = msg_fed.getTimeProperty(HELICS_PROPERTY_TIME_PERIOD);
    double granted_time = -1.0;

    try
    {
        msg_fed.enterExecutingMode();

        // Sleep for a few seconds to enure the cosim is fully setup (this is the recommended approach....booo)
        std::this_thread::sleep_for(std::chrono::seconds(5));

        granted_time = PerformLoop(msg_fed, m_query_fed_input.total_time, period, m_log);
    }
    catch (const std::exception &e)
    {
        m_log << "Error: " << e.what() << std::endl;
    }

    msg_fed.finalize();

    if (granted_time < 0.0)
    {
        m_log << "Could not perform simulation! --- Granted time: " << granted_time << "\n";
    }
}

void data::QueryFederate::Close()
{
    if (m_client)
    {
        m_client->CloseConnection();
        m_client->StopRun();
    }
}