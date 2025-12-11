#include <helics/application_api/MessageFederate.hpp>
#include <helics/application_api/Publications.hpp>
#include <helics/application_api/Inputs.hpp>
#include <helics/core/CoreTypes.hpp>

#include <iostream>
#include <sstream>
#include <string>
#include <fstream>
#include <chrono>
#include <thread>
#include <exception>
#include <cstdlib>
#include <filesystem>

#include <boost/optional.hpp>
#include <boost/json.hpp>

#include "stopwatch.hpp"

#include "query_federate_input.hpp"
#include "synchronous_websocket_client.hpp"
#include "JsonTemplates.hpp"

namespace
{

class WriteHelper
{
  private:
    connections::SynchronousWebSocketClient m_client;
    std::ofstream &m_output_console;
    const data::ClientDetails &m_details;

  public:
    WriteHelper(const data::ClientDetails &details, std::ofstream &&output_console)
        : m_client(), m_output_console(output_console), m_details(details)
    {
        m_output_console << "Attempting to connect!\n\tHost: " << Host() << "\n\tPort: " << Port()
                         << "\n\tTarget: " << Target() << std::endl;
        m_client.Connect(Host(), Port(), Target());
        m_output_console << "Connection Successful!" << std::endl;
    }
    ~WriteHelper()
    {
        m_client.Close();
        m_output_console.close();
    }

    std::string Host() const { return m_details.host; }
    std::string Port() const { return m_details.port; }
    std::string Target() const { return m_details.target; }

    void Write(const std::string &message)
    {
        m_output_console << message;
        m_client.Send(message);
    }
};

std::optional<data::QueryFederateInput> GetQueryFederateInput(int argc, char **argv)
{
    std::optional<data::QueryFederateInput> query_input{};

    if (argc < 2)
    {
        std::cerr << "Missing JSON: No json file provided in execution call.\n";
        return query_input;
    }

    const std::filesystem::path cwd = std::filesystem::current_path();
    const std::string json_file = std::string(argv[1]);
    const std::filesystem::path json_path = json_file;
    if (!std::filesystem::exists(json_path))
    {
        std::cerr << "Missing JSON: " << json_path << " (expected in " << cwd << ")\n";
        return query_input;
    }

    query_input = json_templates::FromJsonFile<data::QueryFederateInput>(json_file);
    return query_input;
}

helics::MessageFederate GetFederate(const data::QueryFederateInput &config, WriteHelper &helper)
{
    helics::FederateInfo fi;
    fi.loadInfoFromJson(config.fed_info_json);

    helics::MessageFederate msg_fed(config.federate_name, fi);
    helper.Write("Message Federate created successfully.\n");

    return msg_fed;
}

std::string DebugTimeQueryLoop(double granted_time, helics::MessageFederate &msg_fed)
{
    utils::Stopwatch loop_watch;
    std::stringstream ss;

    ss << "\n##########################################\n";
    ss << "Granted Time: " << granted_time << "\n";

    loop_watch.Start();

    ss << msg_fed.query("root", "global_time_debugging");

    double loop_execution_ms = loop_watch.ElapsedMilliseconds();

    ss << "\nQuery Execution Time: " << loop_execution_ms << " ms\n";
    ss << "##########################################\n";

    return ss.str();
}

std::string DiscreteQueriesLoop(double granted_time, helics::MessageFederate &msg_fed)
{
    utils::Stopwatch loop_watch;
    std::stringstream ss;

    ss << "\n##########################################\n";
    ss << "Granted Time: " << granted_time << "\n";

    loop_watch.Start();

    ss << "Name: " << msg_fed.query("root", "name") << "\n";
    ss << "Address: " << msg_fed.query("root", "address") << "\n";
    ss << "IsInit: " << msg_fed.query("root", "isinit") << "\n";
    ss << "IsConnected: " << msg_fed.query("root", "isconnected");

    double loop_execution_ms = loop_watch.ElapsedMilliseconds();

    ss << "\nQuery Execution Time: " << loop_execution_ms << " ms\n";
    ss << "##########################################\n";

    return ss.str();
}

double PerformLoop(helics::MessageFederate &msg_fed, const double total_time, const double period, WriteHelper &helper)
{
    utils::Stopwatch main_watch;

    double granted_time = 0.0;

    main_watch.Start();
    while (granted_time + period <= total_time)
    {
        granted_time = msg_fed.requestTime(granted_time + period);

        // std::string output_string = DebugTimeQueryLoop(granted_time, msg_fed);
        std::string output_string = DiscreteQueriesLoop(granted_time, msg_fed);

        helper.Write(output_string);
    }
    double main_loop_ms = main_watch.ElapsedMilliseconds();

    std::stringstream end;
    end << "\n##########################################\n"
        << "Total Loop Time: " << main_loop_ms << " ms"
        << "\n##########################################\n"
        << "\nFederate finalized.\nGranted time: " << granted_time << "\n";
    helper.Write(end.str());

    return granted_time;
}

double ExecuteFederate(const data::QueryFederateInput &config, WriteHelper &helper)
{
    helics::MessageFederate msg_fed = GetFederate(config, helper);
    const double period = msg_fed.getTimeProperty(HELICS_PROPERTY_TIME_PERIOD);

    double granted_time = -1.0;

    try
    {
        msg_fed.enterExecutingMode();

        // Sleep for a few seconds to enure the cosim is fully setup (this is the recommended approach....booo)
        std::this_thread::sleep_for(std::chrono::seconds(5));

        granted_time = PerformLoop(msg_fed, config.total_time, period, helper);
    }
    catch (const std::exception &e)
    {
        std::stringstream err;
        err << "Error: " << e.what() << std::endl;
        helper.Write(err.str());
    }

    // Remember to finalize the fed
    msg_fed.finalize();

    return granted_time;
}
} // namespace

int main(int argc, char **argv)
{
    try
    {
        const std::optional<data::QueryFederateInput> query_input = GetQueryFederateInput(argc, argv);
        if (!query_input)
        {
            return EXIT_FAILURE;
        }

        std::ofstream output_console(query_input.value().local_log_file);
        if (!output_console.is_open())
        {
            std::cerr << "Could not open file '" << query_input.value().local_log_file << "'!" << std::endl;
            return EXIT_FAILURE;
        }

        // Connect to a helper class
        WriteHelper helper(query_input.value().client_details, std::move(output_console));

        // Configure and launch the federate
        const double granted_time = ExecuteFederate(query_input.value(), helper);
        if (granted_time < 0.0)
        {
            helper.Write("Could not perform simulation! Federate finalized.\nGranted time: " +
                         std::to_string(granted_time));
        }

        return EXIT_SUCCESS;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    catch (...)
    {
        std::cerr << "Unknown Exception: An object that does not inherit std::exception was thrown!" << std::endl;
        return EXIT_FAILURE;
    }

    return 0;
}