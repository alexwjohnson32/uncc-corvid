#pragma once

/*
 *     Copyright (c) 2013 Battelle Memorial Institute
 *     Licensed under modified BSD License. A copy of this license can be found
 *     in the LICENSE file in the top level directory of this distribution.
 */
// -------------------------------------------------------------
/**
 * @file   factory.hpp
 * @author Bruce Palmer
 * @date   2014-01-28 11:33:42 d3g096
 *
 * @brief
 *
 * This has been modified by me in various ways, this header may not be necessary
 *
 */
// -------------------------------------------------------------

#include "gridpack/include/gridpack.hpp"
#include "gridpack/applications/components/pf_matrix/pf_components.hpp"

namespace powerflow
{
typedef network::BaseNetwork<PFBus, PFBranch> Network;

class PowerflowFactory : gridpack::factory::BaseFactory<Network>
{
  private:
    NetworkPtr p_network;

  public:
    /**
     * Basic constructor
     * @param network: network associated with factory
     */
    PowerflowFactory(NetworkPtr network);

    /**
     * Basic destructor
     */
    ~PowerflowFactory();

    /**
     * Create the admittance (Y-Bus) matrix.
     */
    void setYBus();

    /**
     * Make SBus vector
     */
    void setSBus();
};
} // namespace powerflow