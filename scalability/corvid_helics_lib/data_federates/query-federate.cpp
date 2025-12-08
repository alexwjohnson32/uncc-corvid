// #include <boost/property_tree/json_parser/error.hpp>
// #include <boost/property_tree/ptree_fwd.hpp>
// #include <cstdlib>
// #include <helics/application_api/MessageFederate.hpp>
// #include <helics/application_api/Publications.hpp>
// #include <helics/application_api/Inputs.hpp>
// #include <helics/core/CoreTypes.hpp>

// #include <boost/json.hpp>

// #include <iostream>
// #include <sstream>
// #include <string>
// #include <fstream>
// #include <chrono>
// #include <thread>
// #include <exception>

// #include <boost/property_tree/ptree.hpp>
// #include <boost/property_tree/json_parser.hpp>
// #include <boost/optional.hpp>

// #include "stopwatch.hpp"

// #include "websocket-client-manager.hpp"

// namespace pt = boost::property_tree;

// namespace
// {

// class WriteHelper
// {
//   private:
//     connections::WebSocketClientManager m_client_mgr;
//     std::ofstream &m_output_console;

//   public:
//     WriteHelper(const connections::ClientDetails &details, std::ofstream &&output_console)
//         : m_client_mgr(details), m_output_console(output_console)
//     {
//     }
//     ~WriteHelper()
//     {
//         m_client_mgr.Close();
//         m_output_console.close();
//     }

//     void Write(const std::string &message)
//     {
//         m_output_console << message;
//         m_client_mgr.SendMessage(message);
//     }
// };

// connections::ClientDetails GetClientDetails(const pt::ptree &config)
// {
//     connections::ClientDetails details;

//     boost::optional<const pt::ptree &> details_tree = config.get_child_optional("client_details");
//     if (details_tree)
//     {
//         details.address = details_tree.value().get_optional<std::string>("address").value_or("127.0.0.1");
//         details.port = details_tree.value().get_optional<std::string>("port").value_or("23333");
//         details.websocket_path = details_tree.value().get_optional<std::string>("websocket_path").value_or("/");
//         details.timeout_seconds = details_tree.value().get_optional<uint>("timeout_seconds").value_or(30);
//     }

//     return details;
// }

// helics::CoreType GetCoreType(const std::string &core_type)
// {
//     if (core_type == "mpi")
//     {
//         return helics::CoreType::MPI;
//     }
//     else
//     {
//         return helics::CoreType::ZMQ;
//     }
// }

// helics::MessageFederate GetFederate(const pt::ptree &config)
// {
//     std::string name = config.get<std::string>("name");
//     std::string core_type = config.get_optional<std::string>("core_type").value_or("zmq");
//     std::string core_init_str = config.get_optional<std::string>("core_init_string").value_or("--federates=1");
//     double time_property = config.get_optional<double>("time_property").value_or(1.0);

//     helics::FederateInfo fi;
//     fi.coreType = GetCoreType(core_type);
//     fi.coreInitString = core_init_str;
//     fi.setProperty(HELICS_PROPERTY_INT_LOG_LEVEL, HELICS_LOG_LEVEL_DEBUG);
//     fi.setProperty(HELICS_PROPERTY_TIME_PERIOD, time_property);
//     fi.setFlagOption(HELICS_FLAG_UNINTERRUPTIBLE, false);
//     fi.setFlagOption(HELICS_FLAG_TERMINATE_ON_ERROR, true);
//     fi.setFlagOption(HELICS_FLAG_WAIT_FOR_CURRENT_TIME_UPDATE, true);

//     return helics::MessageFederate(name, fi);
// }

// std::string DebugTimeQueryLoop(double granted_time, helics::MessageFederate &msg_fed)
// {
//     utils::Stopwatch loop_watch;
//     std::stringstream ss;

//     ss << "\n##########################################\n";
//     ss << "Granted Time: " << granted_time << "\n";

//     loop_watch.Start();

//     ss << msg_fed.query("root", "global_time_debugging");

//     double loop_execution_ms = loop_watch.ElapsedMilliseconds();

//     ss << "\nQuery Execution Time: " << loop_execution_ms << " ms\n";
//     ss << "##########################################\n";

//     return ss.str();
// }

// std::string DiscreteQueriesLoop(double granted_time, helics::MessageFederate &msg_fed)
// {
//     utils::Stopwatch loop_watch;
//     std::stringstream ss;

//     ss << "\n##########################################\n";
//     ss << "Granted Time: " << granted_time << "\n";

//     loop_watch.Start();

//     ss << "Name: " << msg_fed.query("root", "name") << "\n";
//     ss << "Address: " << msg_fed.query("root", "address") << "\n";
//     ss << "IsInit: " << msg_fed.query("root", "isinit") << "\n";
//     ss << "IsConnected: " << msg_fed.query("root", "isconnected");

//     double loop_execution_ms = loop_watch.ElapsedMilliseconds();

//     ss << "\nQuery Execution Time: " << loop_execution_ms << " ms\n";
//     ss << "##########################################\n";

//     return ss.str();
// }

// double PerformLoop(helics::MessageFederate &msg_fed, const double total_time, const double period, WriteHelper
// &helper)
// {
//     utils::Stopwatch main_watch;

//     double granted_time = 0.0;

//     main_watch.Start();
//     while (granted_time + period <= total_time)
//     {
//         granted_time = msg_fed.requestTime(granted_time + period);

//         // std::string output_string = DebugTimeQueryLoop(granted_time, msg_fed);
//         std::string output_string = DiscreteQueriesLoop(granted_time, msg_fed);

//         helper.Write(output_string);
//     }
//     double main_loop_ms = main_watch.ElapsedMilliseconds();

//     std::stringstream end;
//     end << "\n##########################################\n"
//         << "Total Loop Time: " << main_loop_ms << " ms"
//         << "\n##########################################\n"
//         << "\nFederate finalized.\nGranted time: " << granted_time << "\n";
//     helper.Write(end.str());

//     return granted_time;
// }

// double ExecuteFederate(const pt::ptree &config, WriteHelper &helper)
// {
//     const double total_time = config.get<double>("total_time");
//     const double period = config.get_optional<double>("period").value_or(1.0);

//     helics::MessageFederate msg_fed = GetFederate(config);
//     double granted_time = -1.0;

//     try
//     {
//         msg_fed.enterExecutingMode();

//         // Sleep for a few seconds to enure the cosim is fully setup (this is the recommended approach....booo)
//         std::this_thread::sleep_for(std::chrono::seconds(5));

//         granted_time = PerformLoop(msg_fed, total_time, period, helper);
//     }
//     catch (const std::exception &e)
//     {
//         std::stringstream err;
//         err << "Error: " << e.what() << std::endl;
//         helper.Write(err.str());
//     }

//     // Remember to finalize the fed
//     msg_fed.finalize();

//     return granted_time;
// }
// } // namespace

int main(int argc, char **argv)
{
    // // Parse the command line
    // if (argc != 2)
    // {
    //     std::cerr << "Cannot launch: This federate only accepts one argument, a json input file." << std::endl;
    //     return EXIT_FAILURE;
    // }

    // std::string json_input_file(argv[1]);

    // try
    // {
    //     // Parse json into the ptree object
    //     pt::ptree config;
    //     pt::read_json(json_input_file, config);

    //     // Get local file to write to
    //     std::string local_log_file =
    //         config.get_optional<std::string>("local_log_file").value_or("query-federate-cpp.log");
    //     std::cout << "Attempting to write to file: '" << local_log_file << "'." << std::endl;
    //     std::ofstream output_console(local_log_file);
    //     if (!output_console.is_open())
    //     {
    //         std::cerr << "Could not open file '" << local_log_file << "'!" << std::endl;
    //         return EXIT_FAILURE;
    //     }

    //     // Connect to a helper class
    //     WriteHelper helper(GetClientDetails(config), std::move(output_console));

    //     // Configure and launch the federate
    //     const double granted_time = ExecuteFederate(config, helper);
    //     if (granted_time < 0.0)
    //     {
    //         std::cerr << "Could not perform simulation! Federate finalized.\nGranted time: " << granted_time
    //                   << std::endl;
    //     }

    //     return EXIT_SUCCESS;
    // }
    // catch (const pt::json_parser::json_parser_error &e)
    // {
    //     std::cerr << "Error parsing json file '" << json_input_file << "'\n" << e.what() << std::endl;
    //     return EXIT_FAILURE;
    // }
    // catch (const std::exception &e)
    // {
    //     std::cerr << "Fatal error: " << e.what() << std::endl;
    //     return EXIT_FAILURE;
    // }
    // catch (...)
    // {
    //     std::cerr << "Unknown Exception: An object that does not inherit std::exception was thrown!" << std::endl;
    //     return EXIT_FAILURE;
    // }

    return 0;
}