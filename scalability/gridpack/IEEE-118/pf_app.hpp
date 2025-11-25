/*
 *     Copyright (c) 2013 Battelle Memorial Institute
 *     Licensed under modified BSD License. A copy of this license can be found
 *     in the LICENSE file in the top level directory of this distribution.
 */
// -------------------------------------------------------------
/**
 * @file   pf_app.hpp
 * @author Bruce Palmer
 * @date   2014-01-28 11:30:49 d3g096
 *
 * @brief
 *
 *
 */
// -------------------------------------------------------------
#pragma once

#include "pf_state.hpp"

#include <complex>
#include <string>

namespace gridpack
{
namespace powerflow
{

// Calling program for powerflow application. This file has class definition
// and methods.

class PFApp
{
  public:
    /**
     * Basic constructor
     */
    PFApp(void);

    /**
     * Basic destructor
     */
    ~PFApp(void);

    std::complex<double> ComputeVoltageCurrent(const std::string &config_file, int target_bus_id,
                                               const std::string &phase_name, const std::complex<double> &Sa,
                                               gridpack::powerflow::PFState &state) const;
};

} // namespace powerflow
} // namespace gridpack