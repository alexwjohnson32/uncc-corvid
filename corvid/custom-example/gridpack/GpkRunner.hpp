#ifndef GPK_RUNNER_H
#define GPK_RUNNER_H

#include <vector>
#include <complex>
#include <fstream>

#include <helics/application_api/ValueFederate.hpp>
#include <helics/application_api/Publications.hpp>
#include <helics/application_api/Inputs.hpp>

#include "pf_app.hpp"

namespace corvid
{

class GpkRunner
{
  private:
    helics::ValueFederate m_gpk_left_fed;
    double m_period;
    std::ofstream m_out_file;
    helics::Publication m_vc_id;
    std::vector<helics::Input> m_sa_ids;

    void InitializeRunner(const std::string &left_fed_json_path, const std::string &out_file_path);

  public:
    GpkRunner(const std::string &left_fed_json_path, double period, const std::string &out_file_path)
        : m_gpk_left_fed(), m_period(period), m_out_file(), m_vc_id(), m_sa_ids()
    {
        InitializeRunner(left_fed_json_path, out_file_path);
    }
    ~GpkRunner()
    {
        if (m_out_file.is_open()) m_out_file.close();
    }

    void PrepareForSimulation(const std::complex<double> &initial_center_voltage);
    double AdvanceSimulation(double granted_time, const gridpack::powerflow::PFApp &app);
    void FinalizeSimulation();
};

} // namespace corvid

#endif