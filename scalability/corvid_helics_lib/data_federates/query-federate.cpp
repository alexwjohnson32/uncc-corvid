#include <boost/system/error_code.hpp>
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
#include "websocket_client.hpp"
#include "json_templates.hpp"
#include "local_log_helper.hpp"

namespace
{

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

    query_input = utils::FromJsonFile<data::QueryFederateInput>(json_file);
    return query_input;
}

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

double ExecuteFederate(const data::QueryFederateInput &config, utils::LocalLogHelper &log)
{
    helics::MessageFederate msg_fed = GetFederate(config, log);
    const double period = msg_fed.getTimeProperty(HELICS_PROPERTY_TIME_PERIOD);

    double granted_time = -1.0;

    try
    {
        msg_fed.enterExecutingMode();

        // Sleep for a few seconds to enure the cosim is fully setup (this is the recommended approach....booo)
        std::this_thread::sleep_for(std::chrono::seconds(5));

        granted_time = PerformLoop(msg_fed, config.total_time, period, log);
    }
    catch (const std::exception &e)
    {
        log << "Error: " << e.what() << std::endl;
    }

    // Remember to finalize the fed
    msg_fed.finalize();

    return granted_time;
}
} // namespace

int main(int argc, char **argv)
{
    int ret_val = EXIT_FAILURE;
    auto client = std::make_shared<utils::WebSocketClient>();

    try
    {
        const std::optional<data::QueryFederateInput> query_input = GetQueryFederateInput(argc, argv);
        if (!query_input)
        {
            return EXIT_FAILURE;
        }

        utils::LocalLogHelper log(query_input.value().local_log_file);
        if (!log.IsOpen())
        {
            std::cerr << "Could not open file '" << query_input.value().local_log_file << "'!" << std::endl;
            return EXIT_FAILURE;
        }

        // Configure the client
        client->SetOnMessage([&log](const std::string &msg) { log << "Received: " << msg << std::endl; });
        client->SetOnError([&log](const boost::system::error_code &ec, const std::string &what)
                           { log << what << ": " << ec.message() << std::endl; });

        client->AsyncRun();
        client->Connect(query_input.value().client_details.host, query_input.value().client_details.port,
                        query_input.value().client_details.target,
                        [&log](const boost::system::error_code &ec)
                        {
                            if (ec)
                            {
                                log << "Error Connecting: " << ec.message() << "\n";
                            }
                            else
                            {
                                log << "Connected!\n";
                            }
                        });

        log.SetOnWriteCallback([&client](const std::string &msg) { client->Send(msg); });

        // Configure and launch the federate
        const double granted_time = ExecuteFederate(query_input.value(), log);
        if (granted_time < 0.0)
        {
            log << "Could not perform simulation! Federate finalized.\nGranted time: " << granted_time;
        }

        ret_val = EXIT_SUCCESS;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Fatal error: " << e.what() << std::endl;
    }
    catch (...)
    {
        std::cerr << "Unknown Exception: An object that does not inherit std::exception was thrown!" << std::endl;
    }

    client->CloseConnection();
    client->StopRun();

    return ret_val;
}