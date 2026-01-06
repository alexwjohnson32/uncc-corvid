#include <boost/property_tree/ptree_fwd.hpp>
#include <boost/json.hpp>

#include <iostream>
#include <optional>
#include <string>
#include <vector>
#include <cmath>
#include <complex>
#include <unordered_map>

#include <helics/application_api/ValueFederate.hpp>
#include <helics/application_api/Publications.hpp>
#include <helics/application_api/Inputs.hpp>

#include "ieee_118_app.hpp"
#include "json_templates.hpp"
#include "local_log_helper.hpp"

#include "powerflow/tools.hpp"
#include "powerflow/input.hpp"

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
    return utils::GetPrettyJsonString(json_result);
}

std::vector<int> GetBusIds(const powerflow::input::PowerflowInput &input)
{
    std::vector<int> bus_ids;
    for (const powerflow::input::GridlabDInputs &gridlabd_info : input.gridlabd_infos)
    {
        bus_ids.push_back(gridlabd_info.bus_id);
    }
    return bus_ids;
}

std::optional<powerflow::input::PowerflowInput> GetPowerflowInput(int argc, char **argv, utils::LocalLogHelper &log)
{
    std::optional<powerflow::input::PowerflowInput> pf_input{};

    if (argc < 2)
    {
        log << "[error] Missing JSON: No json file provided in execution call.\n";
        return pf_input;
    }

    const std::filesystem::path cwd = std::filesystem::current_path();
    log << "[preflight] CWD: " << cwd << "\n";

    const std::string json_file = std::string(argv[1]);
    const std::filesystem::path json_path = json_file;
    if (!std::filesystem::exists(json_path))
    {
        log << "[error] Missing JSON: " << json_path << " (expected in " << cwd << ")\n";
        return pf_input;
    }

    const std::filesystem::path xml_path = "118.xml";
    if (!std::filesystem::exists(xml_path))
    {
        log << "[error] Missing XML: " << xml_path << " (expected in " << cwd << ")\n";
        return pf_input;
    }

    pf_input = utils::FromJsonFile<powerflow::input::PowerflowInput>(json_file);

    return pf_input;
}

helics::ValueFederate GetGridpackFederate(const powerflow::input::PowerflowInput &pf_input, utils::LocalLogHelper &log)
{
    // Create a FederateInfo object
    helics::FederateInfo fi;
    fi.loadInfoFromJson(pf_input.fed_info_json);

    helics::ValueFederate gpk_118(pf_input.gridpack_name, fi);
    log << "HELICS GridPACK Federate created successfully." << std::endl;

    return gpk_118;
}

double PerformLoop(helics::ValueFederate &gpk_118, const powerflow::input::PowerflowInput &pf_input,
                   utils::LocalLogHelper &log)
{
    // Publications
    powerflow::tools::VoltagePublisher pub(gpk_118, pf_input.ln_magnitude);

    // Subscriptions
    std::unordered_map<std::string, powerflow::tools::ThreePhaseSubscriptions> subs;
    for (const std::string &gridlabd_name : pf_input.GetGridalabDNames())
    {
        subs[gridlabd_name] = { gpk_118.registerSubscription(gridlabd_name + "/Sa", "VA"),
                                gpk_118.registerSubscription(gridlabd_name + "/Sb", "VA"),
                                gpk_118.registerSubscription(gridlabd_name + "/Sc", "VA") };
    }

    log << "Registered Pubs/Subs" << std::endl;
    log << "\n" << FederateToString(gpk_118) << std::endl;

    // initialize constant values
    const std::string xml_file = "118.xml";
    const std::complex<double> r120({ -0.5, -0.866025 });
    const std::vector<int> bus_ids = GetBusIds(pf_input);
    const powerflow::tools::ThreePhaseValues initial_phased_voltage = { { 1.0, 0.0 },
                                                                        { -0.5, -0.866025 },
                                                                        { -0.5, 0.866025 } };
    const double period = gpk_118.getTimeProperty(HELICS_PROPERTY_TIME_PERIOD);

    // Initialize variables
    ieee_118::IEEE118App executor;
    if (!executor.Initialize(xml_file, bus_ids, r120))
    {
        log << "Failed to initialize the executor.\n" << "xml_file: " << xml_file << "\n";
        log << "bus_ids: ";
        for (int bus_id : bus_ids)
        {
            log << bus_id << " ";
        }
        log << "\nr120: " << r120 << "\n";
        return -1.0;
    }

    std::unordered_map<std::string, powerflow::tools::ThreePhaseValues> last_known_values;
    for (const std::string &gridlabd_name : pf_input.GetGridalabDNames())
    {
        last_known_values[gridlabd_name] = powerflow::tools::ThreePhaseValues();
    }

    // Enter execution mode
    gpk_118.enterExecutingMode();
    log << "GridPACK Federate has entered execution mode." << std::endl;

    // Initial voltage publish
    log << "Publish initial voltage." << std::endl;
    pub.Publish(initial_phased_voltage);
    log << "Published." << std::endl;

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
    const double total_interval = pf_input.total_time;
    double granted_time = 0.0;
    while (granted_time + period <= total_interval)
    {
        log << "\n##########################################\n"
            << "New Loop Iteration Information:\n\tGranted Time + Period: " << granted_time + period
            << "\n\tTotal Interval: " << total_interval << "\nRequesting New Granted Time: " << granted_time + period
            << "\n";

        granted_time = gpk_118.requestTime(granted_time + period);
        log << "\n[Time " << granted_time << "]\n";

        for (const powerflow::input::GridlabDInputs &gridlabd_info : pf_input.gridlabd_infos)
        {
            log << "\nBus Id: " << gridlabd_info.bus_id << "\nGridlabd Names:\n\t";

            powerflow::tools::ThreePhaseValues s_total;
            for (const std::string &gridlabd_name : gridlabd_info.names)
            {
                log << "\"" << gridlabd_name << "\" ";
                powerflow::tools::ThreePhaseSubscriptions &current_subs = subs.at(gridlabd_name);
                if (current_subs.a.isUpdated() || current_subs.a.isValid())
                {
                    last_known_values.at(gridlabd_name).a = current_subs.a.getValue<std::complex<double>>();
                }
                if (current_subs.b.isUpdated() || current_subs.b.isValid())
                {
                    last_known_values.at(gridlabd_name).b = current_subs.b.getValue<std::complex<double>>();
                }
                if (current_subs.c.isUpdated() || current_subs.c.isValid())
                {
                    last_known_values.at(gridlabd_name).c = current_subs.c.getValue<std::complex<double>>();
                }

                powerflow::tools::ThreePhaseValues limited_power =
                    powerflow::tools::LimitPower(subs.at(gridlabd_name), 1.0);
                s_total.a += limited_power.a;
                s_total.b += limited_power.b;
                s_total.c += limited_power.c;
            }

            log << "\nTotal S received from Gridlab-D: [" << s_total.a << ", " << s_total.b << ", " << s_total.c
                << "]\n";

            powerflow::tools::ThreePhaseValues v = executor.ComputeVoltage(s_total, gridlabd_info.bus_id);

            log << "Updated V by GridPACK: [" << v.a << ", " << v.b << ", " << v.c << "]\n";

            pub.Publish(v);
        }

        log << "##########################################\n";
    }

    return granted_time;
}

} // namespace

int main(int argc, char **argv)
{
    // Prepare GridPACK Environment
    gridpack::Environment env(argc, argv);

    // Use this instead of std::cout.
    utils::LocalLogHelper log("gpk_118_console.txt");
    log.SetOnWriteCallback([](const std::string &msg) { std::cout << msg; });

    // Read PowerFlowInput and print json string
    const std::optional<powerflow::input::PowerflowInput> pf_input = GetPowerflowInput(argc, argv, log);
    if (!pf_input)
    {
        return 1;
    }

    log << "pf_input.value():\n" << utils::GetPrettyJsonString(utils::ToJsonString(pf_input.value())) << std::endl;
    log << "pf_input.value().fed_info_json:\n"
        << utils::GetPrettyJsonString(pf_input.value().fed_info_json) << std::endl;

    // Create a FederateInfo object
    helics::ValueFederate gpk_118 = GetGridpackFederate(pf_input.value(), log);

    // Perform Simulation
    const double granted_time = PerformLoop(gpk_118, pf_input.value(), log);

    gridpack::math::Finalize();
    gpk_118.finalize();

    if (granted_time < 0.0)
    {
        log << "Could not perform simulation! Federate finalized.\nGranted time: " << granted_time << std::endl;
    }
    else
    {
        log << "Federate finalized.\nGranted time: " << granted_time << std::endl;
    }

    return 0;
}