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
    public:
        PFApp(void);
        ~PFApp(void);

        void execute(int argc, char** argv, std::complex<double>& Vc, std::complex<double>& Sa);
};
}
}

#endif