/*
 *     Copyright (c) 2013 Battelle Memorial Institute
 *     Licensed under modified BSD License. A copy of this license can be found
 *     in the LICENSE file in the top level directory of this distribution.
 */
// -------------------------------------------------------------
/**
 * @file   factory.cpp
 * @author Bruce Palmer
 * @date   2014-01-28 11:31:23 d3g096
 *
 * @brief
 *
 * This has been modified by me in various ways, this header may not be necessary
 *
 */
// -------------------------------------------------------------

#include "gridpack/include/gridpack.hpp"

#include "factory.hpp"

powerflow::PowerflowFactory::PowerflowFactory(powerflow::PowerflowFactory::NetworkPtr network)
    : gridpack::factory::BaseFactory<powerflow::Network>(network)
{
    p_network = network;
}

powerflow::PowerflowFactory::~PowerflowFactory() {}

// <latex> The $\overline{\overline{Y}}$-matrix relates the vector
// of currents $\overline{I}$ to the voltages $\overline{V}$ via
// $\overline{I} = \overline{\overline{Y}}\cdot\overline{V}$ </latex>.

void powerflow::PowerflowFactory::setYBus(void)
{
    int numBus = p_network->numBuses();
    int numBranch = p_network->numBranches();
    int i;

    // Invoke setYBus method on all branch objects
    for (i = 0; i < numBranch; i++)
    {
        p_network->getBranch(i).get()->setYBus();
    }

    // Invoke setYBus method on all bus objects
    for (i = 0; i < numBus; i++)
    {
        p_network->getBus(i).get()->setYBus();
    }
}

void powerflow::PowerflowFactory::setSBus(void)
{
    int numBus = p_network->numBuses();
    int i;

    // Invoke setSBus method on all bus objects
    for (i = 0; i < numBus; i++)
    {
        dynamic_cast<PFBus *>(p_network->getBus(i).get())->setSBus();
    }
}
