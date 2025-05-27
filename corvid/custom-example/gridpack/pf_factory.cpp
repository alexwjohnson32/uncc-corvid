/*
 *     Copyright (c) 2013 Battelle Memorial Institute
 *     Licensed under modified BSD License. A copy of this license can be found
 *     in the LICENSE file in the top level directory of this distribution.
 */
// -------------------------------------------------------------
/**
 * @file   pf_factory.cpp
 * @author Bruce Palmer
 * @date   2014-01-28 11:33:42 d3g096
 *
 * @brief
 *
 * I don't know if this is necessary, I have mostly copied a copy so here we are.
 */
// -------------------------------------------------------------

#include <vector>
#include "boost/smart_ptr/shared_ptr.hpp"
#include "gridpack/include/gridpack.hpp"
#include "pf_factory.hpp"

namespace gridpack
{
namespace powerflow
{
// Powerflow factory class implementation

PFFactory::PFFactory(PFFactory::NetworkPtr network) : gridpack::factory::BaseFactory<PFNetwork>(network)
{
    p_network = network;
}

gridpack::powerflow::PFFactory::~PFFactory() {}

// <latex> The $\overline{\overline{Y}}$-matrix relates the vector
// of currents $\overline{I}$ to the voltages $\overline{V}$ via
// $\overline{I} = \overline{\overline{Y}}\cdot\overline{V}$ </latex>.

/**
 * Create the admittance (Y-Bus) matrix.
 */
void gridpack::powerflow::PFFactory::setYBus(void)
{
    int num_bus = p_network->numBuses();
    int num_branch = p_network->numBranches();

    // Invoke setYBus method on all branch objects
    for (int i = 0; i < num_branch; i++)
    {
        p_network->getBranch(i).get()->setYBus();
    }

    // Invoke setYBus method on all bus objects
    for (int i = 0; i < num_bus; i++)
    {
        p_network->getBus(i).get()->setYBus();
    }
}

/**
 * Make SBus vector
 */
void gridpack::powerflow::PFFactory::setSBus(void)
{
    int num_bus = p_network->numBuses();

    // Invoke setSBus method on all bus objects
    for (int i = 0; i < num_bus; i++)
    {
        dynamic_cast<PFBus*>(p_network->getBus(i).get())->setSBus();
    }
}
}
} // namespace gridpack