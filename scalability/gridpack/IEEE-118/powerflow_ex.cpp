#include <iostream>
#include <optional>
#include <sstream>
#include <string>
#include <vector>
#include <cmath>
#include <fstream>
#include <complex>
#include <unordered_map>

#include <helics/application_api/ValueFederate.hpp>
#include <helics/application_api/Publications.hpp>
#include <helics/application_api/Inputs.hpp>

// GridPACK includes
#include "mpi.h"
#include <ga.h>
#include <macdecls.h>
#include "gridpack/include/gridpack.hpp"
#include "three_phase_pf_app.hpp"
#include "pf_input.hpp"
#include "JsonTemplates.hpp"

#include <boost/json.hpp>

namespace
{

struct ThreePhaseSubscriptions
{
    helics::Input a{};
    helics::Input b{};
    helics::Input c{};
};

class VoltagePublisher
{
  private:
    helics::Publication m_a;
    helics::Publication m_b;
    helics::Publication m_c;
    const double m_ln_magnitude{};

  public:
    VoltagePublisher(helics::ValueFederate &fed, double ln_magnitude) : m_ln_magnitude(ln_magnitude)
    {
        m_a = fed.registerPublication("Va", "complex", "V");
        m_b = fed.registerPublication("Vb", "complex", "V");
        m_c = fed.registerPublication("Vc", "complex", "V");
    }

    void Publish(const gridpack::powerflow::ThreePhaseValues &v)
    {
        m_a.publish(v.a * m_ln_magnitude);
        m_b.publish(v.b * m_ln_magnitude);
        m_c.publish(v.c * m_ln_magnitude);
    }
};

std::complex<double> LimitPower(const std::complex<double> &s, double max_v)
{
    const double abs_s = std::abs(s);
    if (abs_s > max_v)
    {
        return s * (max_v / abs_s);
    }
    else
    {
        return s;
    }
}

gridpack::powerflow::ThreePhaseValues LimitPower(ThreePhaseSubscriptions &sub, double max_v)
{
    gridpack::powerflow::ThreePhaseValues limited_power;

    limited_power.a = LimitPower(sub.a.getValue<std::complex<double>>() / 1e8, max_v);
    limited_power.b = LimitPower(sub.b.getValue<std::complex<double>>() / 1e8, max_v);
    limited_power.c = LimitPower(sub.c.getValue<std::complex<double>>() / 1e8, max_v);

    return limited_power;
}

std::string FederateToString(helics::ValueFederate &fed)
{
    std::stringstream fed_string;

    fed_string << "Federate Name: " << fed.getName() << "\n";

    fed_string << "Publications (" << fed.getPublicationCount() << "):\n";
    for (int i = 0; i < fed.getPublicationCount(); i++)
    {
        helics::Publication pub = fed.getPublication(i);
        fed_string << "  - " << pub.getDisplayName() << " (" << pub.getType() << ")\n";
    }

    fed_string << "Subscriptions (" << fed.getInputCount() << "):\n";
    for (int i = 0; i < fed.getInputCount(); i++)
    {
        helics::Input input = fed.getInput(i);
        fed_string << "  - " << input.getDisplayName() << " (" << input.getType() << ")\n";
    }

    return fed_string.str();
}

std::vector<int> GetBusIds(const powerflow::PowerflowInput &input)
{
    std::vector<int> bus_ids;
    for (const powerflow::GridlabDInputs &gridlabd_info : input.gridlabd_infos)
    {
        bus_ids.push_back(gridlabd_info.bus_id);
    }
    return bus_ids;
}

std::optional<powerflow::PowerflowInput> GetPowerflowInput(int argc, char **argv, std::ofstream &output_console)
{
    std::optional<powerflow::PowerflowInput> pf_input{};

    if (argc < 2)
    {
        output_console << "[error] Missing JSON: No json file provided in execution call.\n";
        std::cerr << "[error] Missing JSON: No json file provided in execution call.\n";
        return pf_input;
    }

    const std::string json_file = std::string(argv[1]);
    const std::filesystem::path cwd = std::filesystem::current_path();
    output_console << "[preflight] CWD: " << cwd << "\n";

    const std::filesystem::path json_path = json_file;
    const std::filesystem::path xml_path = "118.xml";

    if (!std::filesystem::exists(json_path))
    {
        output_console << "[error] Missing JSON: " << json_path << " (expected in " << cwd << ")\n";
        std::cerr << "[error] Missing JSON: " << json_path << " (expected in " << cwd << ")\n";
        return pf_input;
    }

    if (!std::filesystem::exists(xml_path))
    {
        output_console << "[error] Missing XML: " << xml_path << " (expected in " << cwd << ")\n";
        std::cerr << "[error] Missing XML: " << xml_path << " (expected in " << cwd << ")\n";
        return pf_input;
    }

    pf_input = json_templates::FromJsonFile<powerflow::PowerflowInput>(json_file);

    return pf_input;
}

helics::ValueFederate GetGridpackFederate(const powerflow::PowerflowInput &pf_input, std::ofstream &output_console)
{
    // Create a FederateInfo object
    helics::FederateInfo fi;
    fi.loadInfoFromJson(pf_input.fed_info_json_file);

    helics::ValueFederate gpk_118(pf_input.gridpack_name, fi);
    output_console << "HELICS GridPACK Federate created successfully." << std::endl;

    return gpk_118;
}

double PerformLoop(helics::ValueFederate &gpk_118, const powerflow::PowerflowInput &pf_input,
                   std::ofstream &output_console)
{
    // Create output file
    std::ofstream output_file(pf_input.gridpack_name + "_gpk_118.csv");

    // Publications
    VoltagePublisher pub(gpk_118, pf_input.ln_magnitude);

    // Subscriptions
    std::unordered_map<std::string, ThreePhaseSubscriptions> subs;
    for (const std::string &gridlabd_name : pf_input.GetGridalabDNames())
    {
        subs[gridlabd_name] = { gpk_118.registerSubscription(gridlabd_name + "/Sa", "VA"),
                                gpk_118.registerSubscription(gridlabd_name + "/Sb", "VA"),
                                gpk_118.registerSubscription(gridlabd_name + "/Sc", "VA") };
    }

    output_console << "Registered Pubs/Subs" << std::endl;
    output_console << "\n" << FederateToString(gpk_118) << std::endl;

    // initialize constant values
    const std::string xml_file = "118.xml";
    const std::complex<double> r120({ -0.5, -0.866025 });
    const std::vector<int> bus_ids = GetBusIds(pf_input);
    const gridpack::powerflow::ThreePhaseValues initial_phased_voltage = { { 1.0, 0.0 },
                                                                           { -0.5, -0.866025 },
                                                                           { -0.5, 0.866025 } };
    const double period = gpk_118.getTimeProperty(HELICS_PROPERTY_TIME_PERIOD);

    // Initialize variables
    gridpack::powerflow::ThreePhasePFApp executor;
    if (!executor.Initialize(xml_file, bus_ids, r120))
    {
        output_console << "Failed to initialize the executor.\n" << "xml_file: " << xml_file << "\n";
        output_console << "bus_ids: ";
        for (int bus_id : bus_ids)
        {
            output_console << bus_id << " ";
        }
        output_console << "\nr120: " << r120 << "\n";
        return -1.0;
    }

    std::unordered_map<std::string, gridpack::powerflow::ThreePhaseValues> last_known_values;
    for (const std::string &gridlabd_name : pf_input.GetGridalabDNames())
    {
        last_known_values[gridlabd_name] = gridpack::powerflow::ThreePhaseValues();
    }

    // Enter execution mode
    gpk_118.enterExecutingMode();
    output_console << "GridPACK Federate has entered execution mode." << std::endl;

    // Initial voltage publish
    output_console << "Publish initial voltage." << std::endl;
    pub.Publish(initial_phased_voltage);
    output_console << "Published." << std::endl;

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
        std::stringstream ss;

        ss << "\n##########################################\n";
        ss << "New Loop Iteration Information:\n\tGranted Time + Period: " << granted_time + period
           << "\n\tTotal Interval: " << total_interval << std::endl;
        ss << "Requesting New Granted Time: " << granted_time + period << std::endl;

        granted_time = gpk_118.requestTime(granted_time + period);
        ss << " Received Granted Time: " << granted_time << std::endl;

        ss << "\n[Time " << granted_time << "]\n";
        output_console << ss.str();
        output_file << ss.str();
        ss.str(""); // reset

        for (const powerflow::GridlabDInputs &gridlabd_info : pf_input.gridlabd_infos)
        {
            ss << "\nBus Id: " << gridlabd_info.bus_id << "\n";
            ss << "Gridlabd Names:\n\t";

            gridpack::powerflow::ThreePhaseValues s_total;
            for (const std::string &gridlabd_name : gridlabd_info.names)
            {
                ss << "\"" << gridlabd_name << "\" ";
                ThreePhaseSubscriptions &current_subs = subs.at(gridlabd_name);
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

                gridpack::powerflow::ThreePhaseValues limited_power = LimitPower(subs.at(gridlabd_name), 1.0);
                s_total.a += limited_power.a;
                s_total.b += limited_power.b;
                s_total.c += limited_power.c;
            }

            ss << "\n";
            ss << "Total S received from Gridlab-D: [" << s_total.a << ", " << s_total.b << ", " << s_total.c << "]\n";

            gridpack::powerflow::ThreePhaseValues v = executor.ComputeVoltage(s_total, gridlabd_info.bus_id);

            ss << "Updated V by GridPACK: [" << v.a << ", " << v.b << ", " << v.c << "]\n";

            output_console << ss.str();
            output_file << ss.str();
            ss.str(""); // reset

            pub.Publish(v);
        }

        ss << "##########################################\n";
        output_console << ss.str();
        output_file << ss.str();
    }

    output_file << "End of Cosimulation.";
    output_file.close();

    return granted_time;
}
} // namespace

int main(int argc, char **argv)
{
    // Prepare GridPACK Environment
    gridpack::Environment env(argc, argv);

    // Use this instead of std::cout.
    std::ofstream output_console("gpk_118_console.txt");

    const std::optional<powerflow::PowerflowInput> pf_input = GetPowerflowInput(argc, argv, output_console);
    if (!pf_input)
    {
        return 1;
    }

    output_console << json_templates::ToJsonString(pf_input.value()) << std::endl;

    // Create a FederateInfo object
    helics::ValueFederate gpk_118 = GetGridpackFederate(pf_input.value(), output_console);

    // Perform Simulation
    const double granted_time = PerformLoop(gpk_118, pf_input.value(), output_console);

    gridpack::math::Finalize();
    gpk_118.finalize();

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