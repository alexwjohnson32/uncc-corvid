#include "ieee_118_federate.hpp"
#include "tools.hpp"
#include "json_templates.hpp"

#include <memory>
#include <string>
#include <stdexcept>
#include <sstream>
#include <iostream>
#include <unordered_map>
#include <complex>

#include <helics/application_api/ValueFederate.hpp>
#include <helics/application_api/Publications.hpp>
#include <helics/application_api/Inputs.hpp>

// GridPACK includes
#include "mpi.h"
#include <ga.h>
#include <macdecls.h>
#include "gridpack/include/gridpack.hpp"

namespace
{
std::string FederateToString(helics::ValueFederate &fed)
{
    std::string json_result = fed.query(fed.getName(), "federate");
    return common::utils::ToJsonString(json_result, true);
}

helics::ValueFederate GetGridpackFederate(const ieee_118::IEEE118Input &pf_input, common::utils::LocalLogHelper &log)
{
    // Create a FederateInfo object
    helics::FederateInfo fi;
    fi.loadInfoFromJson(pf_input.fed_info_json);

    helics::ValueFederate gpk_118(pf_input.federate_name, fi);
    log << "HELICS GridPACK Federate created successfully." << std::endl;

    return gpk_118;
}

std::unordered_map<std::string, common::helics::ThreePhaseSubscriptions>
GetSubscriptions(const ieee_118::IEEE118Input &input, helics::ValueFederate &fed)
{
    std::unordered_map<std::string, common::helics::ThreePhaseSubscriptions> subs;

    for (const std::string &gridlabd_name : input.GetGridlabDNames())
    {
        subs[gridlabd_name] = { fed.registerSubscription(gridlabd_name + "/Sa", "VA"),
                                fed.registerSubscription(gridlabd_name + "/Sb", "VA"),
                                fed.registerSubscription(gridlabd_name + "/Sc", "VA") };
    }

    return subs;
}

std::unordered_map<std::string, common::helics::ThreePhaseValues>
GetInitialPhaseValues(const ieee_118::IEEE118Input &input)
{
    std::unordered_map<std::string, common::helics::ThreePhaseValues> initial_values;
    for (const std::string &gridlabd_name : input.GetGridlabDNames())
    {
        initial_values[gridlabd_name] = common::helics::ThreePhaseValues();
    }
    return initial_values;
}

std::vector<int> GetBusIds(const ieee_118::IEEE118Input &input)
{
    std::vector<int> bus_ids;
    for (const ieee_118::GridlabDInputs &gridlabd_info : input.gridlabd_infos)
    {
        bus_ids.push_back(gridlabd_info.bus_id);
    }
    return bus_ids;
}

} // namespace

// ieee_118::IEEE118Federate::FederateState Implementation

class ieee_118::IEEE118Federate::FederateState
{
  public:
    helics::ValueFederate m_fed;
    common::helics::VoltagePublisher m_pub;
    std::unordered_map<std::string, common::helics::ThreePhaseSubscriptions> m_subs;
    std::unordered_map<std::string, common::helics::ThreePhaseValues> m_last_known_values;
    ieee_118::IEEE118App m_executor;
    const std::string m_xml_file;
    const std::complex<double> m_r120;
    const std::vector<int> m_bus_ids;
    const common::helics::ThreePhaseValues m_initial_phased_voltage;
    const double m_period;

    FederateState(const ieee_118::IEEE118Input &input, common::utils::LocalLogHelper &log)
        : m_fed{ GetGridpackFederate(input, log) }, m_pub{ m_fed, input.ln_magnitude },
          m_subs{ GetSubscriptions(input, m_fed) }, m_last_known_values{ GetInitialPhaseValues(input) }, m_executor(),
          m_xml_file{ "118.xml" }, m_r120{ -0.5, -0.866025 }, m_bus_ids{ GetBusIds(input) },
          m_initial_phased_voltage{ { 1.0, 0.0 }, { -0.5, -0.866025 }, { -0.5, 0.866025 } },
          m_period{ m_fed.getTimeProperty(HELICS_PROPERTY_TIME_PERIOD) }
    {
        if (!m_executor.Initialize(m_xml_file, m_bus_ids, m_r120))
        {
            log << "Failed to initialize the executor.\n" << "xml_file: " << m_xml_file << "\n";
            log << "bus_ids: ";
            for (int bus_id : m_bus_ids)
            {
                log << bus_id << " ";
            }
            log << "\nr120: " << m_r120 << "\n";
            throw std::runtime_error("Could Not Initialize Executor, Exiting.");
        }
        else
        {
            log << "Initialized State, Printing Federate:\n" << FederateToString(m_fed) << std::endl;
        }
    }
    ~FederateState() {}

    double SimulateStep(const std::vector<ieee_118::GridlabDInputs> &gridlabd_infos, const double current_time,
                        const double period, common::utils::LocalLogHelper &log)
    {
        double granted_time = m_fed.requestTime(current_time + period);
        log << "\n[Time " << granted_time << "]\n";

        for (const ieee_118::GridlabDInputs &gridlabd_info : gridlabd_infos)
        {
            log << "\nBus Id: " << gridlabd_info.bus_id << "\nGridlabd Names:\n\t";

            common::helics::ThreePhaseValues s_total;
            for (const std::string &gridlabd_name : gridlabd_info.names)
            {
                log << "\"" << gridlabd_name << "\" ";
                common::helics::ThreePhaseSubscriptions &current_subs = m_subs.at(gridlabd_name);
                if (current_subs.a.isUpdated() || current_subs.a.isValid())
                {
                    m_last_known_values.at(gridlabd_name).a = current_subs.a.getValue<std::complex<double>>();
                }
                if (current_subs.b.isUpdated() || current_subs.b.isValid())
                {
                    m_last_known_values.at(gridlabd_name).b = current_subs.b.getValue<std::complex<double>>();
                }
                if (current_subs.c.isUpdated() || current_subs.c.isValid())
                {
                    m_last_known_values.at(gridlabd_name).c = current_subs.c.getValue<std::complex<double>>();
                }

                common::helics::ThreePhaseValues limited_power =
                    common::helics::LimitPower(m_subs.at(gridlabd_name), 1.0);
                s_total.a += limited_power.a;
                s_total.b += limited_power.b;
                s_total.c += limited_power.c;
            }

            log << "\nTotal S received from Gridlab-D: [" << s_total.a << ", " << s_total.b << ", " << s_total.c
                << "]\n";

            common::helics::ThreePhaseValues v = m_executor.ComputeVoltage(s_total, gridlabd_info.bus_id);

            log << "Updated V by GridPACK: [" << v.a << ", " << v.b << ", " << v.c << "]\n";

            m_pub.Publish(v);
        }

        return granted_time;
    }
};

// ieee_118::IEEE118Federate Implementation

ieee_118::IEEE118Federate::IEEE118Federate() {}
ieee_118::IEEE118Federate::~IEEE118Federate() { Close(); }

void ieee_118::IEEE118Federate::Initialize(const ieee_118::IEEE118Input &input, ieee_118::GridpackEnvConfig config)
{
    m_fed_input = input;
    gridpack::Environment env(config.argc, config.argv);

    // Setup Logging
    m_log.SetOutputFile(m_fed_input.local_log_file);

    if (!m_log.IsOpen())
    {
        std::stringstream err_str;
        err_str << "Could not open file '" << m_fed_input.local_log_file << "'!" << std::endl;
        throw std::runtime_error(err_str.str());
    }

    m_log.SetOnWriteCallback([](const std::string &msg) { std::cout << msg; });

    // Print Federate Json String
    m_log << "Json Input:\n" << common::utils::ToJsonString(input, true) << std::endl;

    // Setup Federate State
    m_state = std::make_unique<ieee_118::IEEE118Federate::FederateState>(m_fed_input, m_log);
}

void ieee_118::IEEE118Federate::Run()
{
    double granted_time = -1;

    try
    {
        m_state->m_fed.enterExecutingMode();
        m_log << "GridPACK Federate has entered execution mode." << std::endl;

        // Initial voltage publish
        m_log << "Publish initial voltage." << std::endl;
        m_state->m_pub.Publish(m_state->m_initial_phased_voltage);
        m_log << "Published." << std::endl;

        // Perform Simulation
        /*
         * What performing the simulation looks like:
         * 1. Get the granted time
         * 2. Separate the distribution systems based on bus_id
         * 3. For each bus_id, aggregate the total power from the distribution systems. Meaning, limt the power for each
         * phase and keep a total of all limited power for each phase per bus_id.
         * 4. Run the powerflow application per bus id (maybe this means one application, or it means an application per
         * bus_id).
         * 5. Publish individual calculated V for each ID at the same granted time.
         */
        const double total_interval = m_fed_input.total_time;
        granted_time = 0.0;

        while (granted_time + m_state->m_period <= total_interval)
        {
            m_log << "\n##########################################\n"
                  << "New Loop Iteration Information:\n\tGranted Time + Period: " << granted_time + m_state->m_period
                  << "\n\tTotal Interval: " << total_interval
                  << "\nRequesting New Granted Time: " << granted_time + m_state->m_period << "\n";

            granted_time = m_state->SimulateStep(m_fed_input.gridlabd_infos, granted_time, m_state->m_period, m_log);

            m_log << "##########################################\n";
        }
    }
    catch (const std::exception &e)
    {
        m_log << "Error: " << e.what() << std::endl;
    }

    Close();

    if (granted_time < 0.0)
    {
        m_log << "Could not perform simulation! Federate finalized.\nGranted time: " << granted_time << std::endl;
    }
    else
    {
        m_log << "Federate finalized.\nGranted time: " << granted_time << std::endl;
    }
}

void ieee_118::IEEE118Federate::Close()
{
    gridpack::math::Finalize();
    if (m_state) m_state->m_fed.finalize();
}
