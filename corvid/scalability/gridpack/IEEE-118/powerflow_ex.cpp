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
#include "pf_app.hpp"
#include "pf_input.hpp"
#include "JsonTemplates.hpp"

#include <boost/json.hpp>

struct ThreePhaseSubscriptions
{
    helics::Input a{};
    helics::Input b{};
    helics::Input c{};
};

struct PhasedPower
{
    std::complex<double> a;
    std::complex<double> b;
    std::complex<double> c;
};

struct PhasedVoltage
{
    std::complex<double> a;
    std::complex<double> b;
    std::complex<double> c;
};

class VoltagePublisher
{
  private:
    helics::Publication &m_a;
    helics::Publication &m_b;
    helics::Publication &m_c;
    const double m_ln_magnitude{};

  public:
    VoltagePublisher(helics::Publication &a, helics::Publication &b, helics::Publication &c, double ln_magnitude)
        : m_a{ a }, m_b{ b }, m_c{ c }, m_ln_magnitude{}
    {
    }

    void Publish(const PhasedVoltage &v)
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

PhasedPower LimitPower(ThreePhaseSubscriptions &sub, double max_v)
{
    PhasedPower limited_power;

    limited_power.a = LimitPower(sub.a.getValue<std::complex<double>>() / 1e8, max_v);
    limited_power.b = LimitPower(sub.b.getValue<std::complex<double>>() / 1e8, max_v);
    limited_power.c = LimitPower(sub.c.getValue<std::complex<double>>() / 1e8, max_v);

    return limited_power;
}

class VoltageComputer
{
  private:
    gridpack::powerflow::PFApp m_app_A{};
    gridpack::powerflow::PFApp m_app_B{};
    gridpack::powerflow::PFApp m_app_C{};

    const std::string m_xml_file{};
    const PhasedVoltage &m_default_voltage{};

  public:
    VoltageComputer(const std::string &xml_file, const PhasedVoltage &default_voltage)
        : m_xml_file{ xml_file }, m_default_voltage{ default_voltage }
    {
    }

    PhasedVoltage ComputeVoltage(const PhasedPower &s, int bus_id, const std::complex<double> &r)
    {
        PhasedVoltage v;
        v.a = m_app_A.ComputeVoltageCurrent(m_xml_file, bus_id, "A", s.a).value_or(m_default_voltage.a);
        v.b = m_app_B.ComputeVoltageCurrent(m_xml_file, bus_id, "B", s.b).value_or(m_default_voltage.b);
        v.c = m_app_C.ComputeVoltageCurrent(m_xml_file, bus_id, "C", s.c).value_or(m_default_voltage.c);

        v.b = v.b * r;
        v.c = v.c * r * r;

        return v;
    }
};

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

int main(int argc, char **argv)
{
    std::ofstream output_console("gpk_118_console.txt");

    // const static int expected_argc = 2;
    // if (argc != expected_argc)
    // {
    //     output_console << "Cannot launch, no json file given to read!" << std::endl;
    //     return 0;
    // }

    // const std::string json_file(argv[1]);
    const std::string json_file("helics_setup.json");

    const powerflow::PowerflowInput pf_input = json_templates::FromJsonFile<powerflow::PowerflowInput>(json_file);
    output_console << json_templates::ToJsonString(pf_input) << std::endl;

    // Create a FederateInfo object
    helics::FederateInfo fi;
    fi.coreType = helics::CoreType::ZMQ;
    fi.coreInitString = "--federates=1";
    fi.setProperty(HELICS_PROPERTY_INT_LOG_LEVEL, HELICS_LOG_LEVEL_DEBUG);
    const double period = 1.0;
    fi.setProperty(HELICS_PROPERTY_TIME_PERIOD, period);
    fi.setFlagOption(HELICS_FLAG_UNINTERRUPTIBLE, false);
    fi.setFlagOption(HELICS_FLAG_TERMINATE_ON_ERROR, true);
    fi.setFlagOption(HELICS_FLAG_WAIT_FOR_CURRENT_TIME_UPDATE, true);

    helics::ValueFederate gpk_118(pf_input.gridpack_name, fi);
    output_console << "HELICS GridPACK Federate created successfully." << std::endl;

    // Publications
    VoltagePublisher pub(gpk_118.registerPublication("Va", "complex", "V"),
                         gpk_118.registerPublication("Vb", "complex", "V"),
                         gpk_118.registerPublication("Vc", "complex", "V"), pf_input.ln_magnitude);

    // Subscriptions
    std::unordered_map<std::string, ThreePhaseSubscriptions> subs;
    for (const powerflow::GridlabDInput &gridlabd_info : pf_input.gridlabd_infos)
    {
        subs[gridlabd_info.name] = { gpk_118.registerSubscription(gridlabd_info.name + "/Sa", "VA"),
                                     gpk_118.registerSubscription(gridlabd_info.name + "/Sb", "VA"),
                                     gpk_118.registerSubscription(gridlabd_info.name + "/Sc", "VA") };
    }

    output_console << "Registered Pubs/Subs" << std::endl;
    output_console << "\n" << FederateToString(gpk_118) << std::endl;
    ;

    // output file
    std::ofstream outFile(pf_input.gridpack_name + "_gpk_118.csv");

    // initialize constants
    const std::string xml_file = "118.xml";
    const PhasedVoltage default_phased_voltage = { { 1.0, 0.0 }, { -0.5, -0.866025 }, { -0.5, 0.866025 } };
    const std::complex<double> r120({ -0.5, -0.866025 });

    // Prepare GridPACK Environment
    gridpack::Environment env(argc, argv);
    gridpack::math::Initialize(&argc, &argv);
    VoltageComputer executor(xml_file, default_phased_voltage);

    // Enter execution mode
    gpk_118.enterExecutingMode();
    output_console << "GridPACK Federate has entered execution mode." << std::endl;

    // Initial voltage publish
    output_console << "Publish initial voltage." << std::endl;
    pub.Publish(default_phased_voltage);
    output_console << "Published." << std::endl;

    const double total_interval = pf_input.total_time;
    double granted_time = 0.0;
    while (granted_time + period <= total_interval)
    {
        output_console << "Granted Time + Period: " << granted_time + period << ", Total Interval: " << total_interval
                       << std::endl;
        granted_time = gpk_118.requestTime(granted_time + period);
        output_console << "Granted Time: " << granted_time << ", Period: " << period
                       << ", Total Interval: " << total_interval << std::endl;

        for (const powerflow::GridlabDInput &gridlabd_input : pf_input.gridlabd_infos)
        {
            output_console << "GridlabD Name Computation: " << gridlabd_input.name << std::endl;
            PhasedPower s = LimitPower(subs.at(gridlabd_input.name), 1.0);
            PhasedVoltage v = executor.ComputeVoltage(s, gridlabd_input.bus_id, r120);

            std::stringstream ss;
            ss << "GridlabdD Name: " << gridlabd_input.name << "\n"
               << "[Time " << granted_time << "]\n"
               << "S received from Gridlab-D, Sa: " << s.a << ", Sb: " << s.b << ", Sc: " << s.c << "\n"
               << "Updated V by GridPACK, Va: " << v.a << ", Vb: " << v.b << ", Vc: " << v.c << "\n";

            output_console << ss.str();
            outFile << ss.str();

            pub.Publish(v);
        }

        output_console << "Completed Computation of current Granted Time." << std::endl;
    }

    outFile << "End of Cosimulation.";
    outFile.close();

    gridpack::math::Finalize();
    gpk_118.finalize();
    output_console << "Federate finalized.\nGranted time: " << granted_time << std::endl;
    return 0;
}
