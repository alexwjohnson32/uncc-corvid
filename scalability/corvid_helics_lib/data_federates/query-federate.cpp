#include <boost/property_tree/json_parser/error.hpp>
#include <boost/property_tree/ptree_fwd.hpp>
#include <exception>
#include <helics/application_api/MessageFederate.hpp>
#include <helics/application_api/Publications.hpp>
#include <helics/application_api/Inputs.hpp>
#include <helics/core/CoreTypes.hpp>

#include <boost/json.hpp>

#include <iostream>
#include <sstream>
#include <string>
#include <fstream>
#include <vector>
#include <filesystem>
#include <chrono>
#include <thread>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

namespace pt = boost::property_tree;

namespace
{

class Stopwatch
{
  private:
    std::chrono::high_resolution_clock::time_point m_start_time;

  public:
    void start() { m_start_time = std::chrono::high_resolution_clock::now(); }

    double elapsedMilliseconds() const
    {
        auto now = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> elapsed = now - m_start_time;
        return elapsed.count();
    }
};

helics::CoreType GetCoreType(const std::string &core_type)
{
    if (core_type == "mpi")
    {
        return helics::CoreType::MPI;
    }
    else
    {
        return helics::CoreType::ZMQ;
    }
}

helics::MessageFederate GetFederate(const pt::ptree &config)
{
    std::string name = config.get<std::string>("name");
    std::string core_type = config.get_optional<std::string>("core_type").value_or("zmq");
    std::string core_init_str = config.get_optional<std::string>("core_init_string").value_or("--federates=1");
    double time_property = config.get_optional<double>("time_property").value_or(1.0);

    helics::FederateInfo fi;
    fi.coreType = GetCoreType(core_type);
    fi.coreInitString = core_init_str;
    fi.setProperty(HELICS_PROPERTY_INT_LOG_LEVEL, HELICS_LOG_LEVEL_DEBUG);
    fi.setProperty(HELICS_PROPERTY_TIME_PERIOD, time_property);
    fi.setFlagOption(HELICS_FLAG_UNINTERRUPTIBLE, false);
    fi.setFlagOption(HELICS_FLAG_TERMINATE_ON_ERROR, true);
    fi.setFlagOption(HELICS_FLAG_WAIT_FOR_CURRENT_TIME_UPDATE, true);

    return helics::MessageFederate(name, fi);
}

double PerformLoop(helics::MessageFederate &msg_fed, const double total_time, const double period,
                   std::ofstream &output_console)
{
    Stopwatch main_watch;
    Stopwatch loop_watch;
    double granted_time = 0.0;

    main_watch.start();
    while (granted_time + period <= total_time)
    {
        granted_time = msg_fed.requestTime(granted_time + period);

        std::stringstream ss;
        ss << "\n##########################################\n";
        ss << "Granted Time: " << granted_time << "\n";
        loop_watch.start();
        ss << msg_fed.query("root", "global_time_debugging");
        double loop_execution_ms = loop_watch.elapsedMilliseconds();
        ss << "\nQuery Execution Time: " << loop_execution_ms << " ms\n";
        ss << "\n##########################################\n";

        output_console << ss.str();
    }
    double main_loop_ms = main_watch.elapsedMilliseconds();

    output_console << "\n##########################################\n";
    output_console << "Total Loop Time: " << main_loop_ms << " ms";
    output_console << "\n##########################################\n";

    return granted_time;
}

double ExecuteFederate(const pt::ptree &config, std::ofstream &output_console)
{
    const double total_time = config.get<double>("total_time");
    const double period = config.get_optional<double>("period").value_or(1.0);

    helics::MessageFederate msg_fed = GetFederate(config);
    double granted_time = -1.0;

    try
    {
        msg_fed.enterExecutingMode();

        // Sleep for a few seconds to enure the cosim is fully setup (this is the recommended approach....booo)
        std::this_thread::sleep_for(std::chrono::seconds(5));

        granted_time = PerformLoop(msg_fed, total_time, period, output_console);
    }
    catch (const std::exception &e)
    {
        output_console << "Error: " << e.what() << std::endl;
    }

    // Remember to finalize the fed
    msg_fed.finalize();

    return granted_time;
}
} // namespace

int main(int argc, char **argv)
{
    std::ofstream output_console("query-federate-cpp.log");

    if (argc != 2)
    {
        output_console << "Cannot launch: This federate only accepts one argument, a json input file." << std::endl;
        return 1;
    }

    std::string json_input_file(argv[1]);
    pt::ptree config;

    try
    {
        pt::read_json(json_input_file, config);
    }
    catch (const pt::json_parser::json_parser_error &e)
    {
        output_console << "Error parsing json file '" << json_input_file << "'\n" << e.what() << std::endl;
        return 1;
    }
    catch (const std::exception &e)
    {
        output_console << "Error: " << e.what() << std::endl;
        return 1;
    }

    const double granted_time = ExecuteFederate(config, output_console);

    if (granted_time < 0.0)
    {
        output_console << "Could not perform simulation! Federate finalized.\nGranted time: " << granted_time
                       << std::endl;
    }
    else
    {
        output_console << "Federate finalized.\nGranted time: " << granted_time << std::endl;
    }

    return 0;
}