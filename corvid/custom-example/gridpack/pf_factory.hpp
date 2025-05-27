/*
 *     Copyright (c) 2013 Battelle Memorial Institute
 *     Licensed under modified BSD License. A copy of this license can be found
 *     in the LICENSE file in the top level directory of this distribution.
 */
// -------------------------------------------------------------
/**
 * @file   pf_factory.hpp
 * @author Bruce Palmer
 * @date   2014-01-28 11:33:42 d3g096
 *
 * @brief
 *
 * I don't know if this is necessary, I have mostly copied a copy so here we are.
 */
// -------------------------------------------------------------

#ifndef PF_FACTORY_H
#define PF_FACTORY_H

#include "boost/smart_ptr/shared_ptr.hpp"
#include "gridpack/include/gridpack.hpp"
#include "gridpack/applications/components/pf_matrix/pf_components.hpp"

namespace gridpack
{
namespace powerflow
{
// Define the type of network used in the powerflow application
typedef network::BaseNetwork<PFBus, PFBranch> PFNetwork;

class PFFactory : public gridpack::factory::BaseFactory<PFNetwork>
{
  private:
    NetworkPtr p_network;

  public:
    PFFactory(NetworkPtr network);
    ~PFFactory();

    void setYBus(void);
    void setSBus(void);
};
} // namespace powerflow
} // namespace gridpack

#endif