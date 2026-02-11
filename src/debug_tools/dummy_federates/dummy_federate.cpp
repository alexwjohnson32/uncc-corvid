#include "dummy_federate.hpp"

#include "stopwatch.hpp"

#include <iostream>
#include <string>
#include <stdexcept>
#include <sstream>
#include <thread>

#include <boost/system/error_code.hpp>

#include <helics/application_api/MessageFederate.hpp>

namespace
{
helics::MessageFederate GetFederate(const std::string &name, const std::string &fed_json_str,
                                    common::utils::LocalLogHelper &log)
{
    helics::FederateInfo fi;
    fi.loadInfoFromJson(fed_json_str);

    helics::MessageFederate msg_fed(name, fi);
    log << "Message Federate created successfully.\n";

    return msg_fed;
}

std::string DiscreteQueriesLoop(double granted_time, helics::MessageFederate &msg_fed)
{
    common::utils::Stopwatch loop_watch;
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
                   common::utils::LocalLogHelper &log)
{
    common::utils::Stopwatch main_watch;

    double granted_time = 0.0;

    main_watch.Start();
    while (granted_time + period <= total_time)
    {
        granted_time = msg_fed.requestTime(granted_time + period);

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

dummy::DummyFederate::DummyFederate() {}
dummy::DummyFederate::~DummyFederate() { Close(); }

void dummy::DummyFederate::Initialize(const common::helics::HelicsInput &input)
{
    m_fed_input = input;
    m_log.SetOutputFile(m_fed_input.local_log_file);

    if (!m_log.IsOpen())
    {
        std::stringstream err_str;
        err_str << "Could not open file '" << m_fed_input.local_log_file << "'!" << std::endl;
        throw std::runtime_error(err_str.str());
    }

    m_log.SetOnWriteCallback([](const std::string &msg) { std::cout << msg << std::endl; });
}

void dummy::DummyFederate::Run()
{
    helics::MessageFederate msg_fed = GetFederate(m_fed_input.federate_name, m_fed_input.fed_info_json, m_log);
    const double period = msg_fed.getTimeProperty(HELICS_PROPERTY_TIME_PERIOD);
    double granted_time = -1.0;

    try
    {
        msg_fed.enterExecutingMode();

        // Sleep for a few seconds to enure the cosim is fully setup (this is the recommended approach....booo)
        std::this_thread::sleep_for(std::chrono::seconds(5));

        granted_time = PerformLoop(msg_fed, m_fed_input.total_time, period, m_log);
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

void dummy::DummyFederate::Close() {}