/*
 *     Copyright (c) 2013 Battelle Memorial Institute
 *     Licensed under modified BSD License. A copy of this license can be found
 *     in the LICENSE file in the top level directory of this distribution.
 */
// -------------------------------------------------------------
/**
 * @file   pf_app.hpp
 * @author Bruce Palmer
 * @date   2014-01-28 11:33:42 d3g096
 *
 * @brief
 *
 * I don't know if this is necessary, I have mostly copied a copy so here we are.
 */
// -------------------------------------------------------------

#ifndef PF_APP_H
#define PF_APP_H

#include "boost/smart_ptr/shared_ptr.hpp"
#include "pf_factory.hpp"

namespace gridpack
{
namespace powerflow
{
class PFApp
{
  private:
    std::string m_config_path;

    static inline std::string CompiledConfigPath()
    {
        return "/beegfs/users/lwilliamson/repos/uncc_root/uncc-corvid/corvid/custom-example/gridpack/build/input.xml";
    }

  public:
    PFApp(void) : m_config_path(CompiledConfigPath()) {};
    PFApp(const std::string &alt_config_path);
    ~PFApp(void) {}

    bool CanReadConfigFile() const;

    std::complex<double> ComputeVc(const std::complex<double> &Sa) const;
};
} // namespace powerflow
} // namespace gridpack

#endif