#include "GpkRunner.hpp"

#include <complex>
#include <fstream>
#include <string>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <cmath>

namespace
{
std::string ComplexToString(const std::complex<double> &complex, uint precision)
{
    std::stringstream ss;
    ss << std::fixed << std::setprecision(precision) << "(" << complex.real() << ", " << complex.imag() << ")";
    return ss.str();
}

std::complex<double> GetAverage(const std::vector<std::complex<double>> &values)
{
    std::complex<double> total_value(0.0, 0.0);
    int nums = values.size();

    for (const std::complex<double> &value : values)
    {
        total_value += value;
    }

    return total_value / static_cast<double>(nums);
    ;
}
} // namespace

void corvid::GpkRunner::InitializeRunner(const std::string &left_fed_json_path, const std::string &out_file_path)
{
    // init federate
    m_gpk_left_fed = helics::ValueFederate(left_fed_json_path);
    std::cout << "HELICS GridPACK Federate create successfully." << std::endl;

    // init output file
    m_out_file = std::ofstream(out_file_path);

    // init pubs/subs
    m_vc_id = m_gpk_left_fed.getPublication("gpk-left-fed/Vc");
    for (int i = 0; i < m_gpk_left_fed.getInputCount(); i++)
    {
        m_sa_ids.push_back(m_gpk_left_fed.getInput(i));
    }
}

void corvid::GpkRunner::PrepareForSimulation(const std::complex<double> &initial_center_voltage)
{
    m_gpk_left_fed.enterExecutingMode();
    std::cout << "GridPACK Federeate has entered execution mode." << std::endl;

    m_vc_id.publish(initial_center_voltage);
}

double corvid::GpkRunner::AdvanceSimulation(double last_granted_time, const gridpack::powerflow::PFApp &app)
{
    // Request time
    double granted_time = m_gpk_left_fed.requestTime(last_granted_time + m_period);

    // Get Sa's from gridlab-d
    std::vector<std::complex<double>> sa_values;
    for (helics::Input &input : m_sa_ids)
    {
        std::complex<double> sa = input.getValue<std::complex<double>>() / 1000000.0;
        sa_values.push_back(sa);

        m_out_file << "Sub Name: " << input.getName() << "\n";
        m_out_file << "Sub value: " << ComplexToString(sa, 2) << " " << input.getUnits() << "\n";
    }

    std::complex<double> Sa = GetAverage(sa_values);

    // pass Sa to GridPACK and get back Vc
    std::complex<double> voltage = app.ComputeVc(Sa);

    // Log boundary signals
    m_out_file << "Time (s): " << granted_time << "\n";
    m_out_file << "Avg Sa received from Gridlab-D: " << Sa << "\n";
    m_out_file << "Update Vc by GridPACK:      " << voltage << "\n\n";

    // publish new center bus voltage
    if (!std::isnan(voltage.real()))
    {
        m_vc_id.publish(voltage * 69000.0);
    }

    return granted_time;
}

void corvid::GpkRunner::FinalizeSimulation()
{
    m_out_file << "End of Cosimulation.";
    m_out_file.close();

    m_gpk_left_fed.finalize();
    std::cout << "Federate finalized." << std::endl;
}