#pragma once

#include <vector>
#include <unordered_map>
#include <memory>
#include <string>
#include <complex>

#include "boost/smart_ptr/shared_ptr.hpp"

#include "gridpack/include/gridpack.hpp"

#include "/usr/local/GridPACK/include/gridpack/applications/modules/powerflow/pf_factory_module.hpp"

namespace powerflow
{

class State
{
  private:
    std::unordered_map<int, int> m_bus_indeces;
    gridpack::parallel::Communicator m_world;

  public:
    boost::shared_ptr<gridpack::powerflow::PFNetwork> network;
    gridpack::utility::Configuration::CursorPtr cursor;

    double base_MVA = 100.0;

    std::unique_ptr<gridpack::powerflow::PFFactoryModule> pf_factory;
    std::unique_ptr<gridpack::mapper::BusVectorMap<gridpack::powerflow::PFNetwork>> v_map;
    std::unique_ptr<gridpack::mapper::FullMatrixMap<gridpack::powerflow::PFNetwork>> j_map;
    boost::shared_ptr<gridpack::math::Vector> PQ;
    boost::shared_ptr<gridpack::math::Vector> X;
    boost::shared_ptr<gridpack::math::Matrix> J;
    std::unique_ptr<gridpack::math::LinearSolver> solver;

    State() {}

    bool InitializeConfig(const std::string &config_file);
    bool InitializeBusIndeces(const std::vector<int> &bus_ids);
    void InitializeFactoryAndFields();
    int GetBusIndex(int bus_id) const;
    int GetWorldRank() const { return m_world.rank(); }

    std::complex<double> ComputeVoltageCurrent(const std::string &config_file, int target_bus_id,
                                               const std::string &phase_name, const std::complex<double> &Sa);
};

} // namespace powerflow