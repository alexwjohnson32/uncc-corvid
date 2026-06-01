#include "federate.hpp"
#include "common/utils/json_templates.hpp"

#include <stdexcept>
#include <sstream>

// GridPACK includes
#include "mpi.h"
#include <ga.h>
#include <macdecls.h>
#include "gridpack/include/gridpack.hpp"

three_phase::ThreePhaseFederate::ThreePhaseFederate() {}
three_phase::ThreePhaseFederate::~ThreePhaseFederate() { Close(); }

void three_phase::ThreePhaseFederate::Initialize(const three_phase::ThreePhaseInput &input,
                                                 three_phase::GridpackEnvConfig config)
{
    m_fed_input = input;
    gridpack::Environment env(config.argc, config.argv);

    // Setup Logging
    m_log.SetOutputFile(m_fed_input.local_log_file);

    if (!m_log.IsOpen())
    {
        std::stringstream err_str;
        err_str << "Could not open file '" << m_fed_input.local_log_file << "'!" << std::endl;
        throw std::runtime_error(err_str.str());
    }

    m_log.SetOnWriteCallback([](const std::string &msg) { std::cout << msg; });

    // Print Federate Json String
    m_log << "Json Input:\n" << common::utils::ToJsonString(input, true) << std::endl;

    // Setup Federate State
    m_state = three_phase::FederateState::Create(m_fed_input, m_log);
}

void three_phase::ThreePhaseFederate::Run()
{
    double granted_time = -1;

    try
    {
        granted_time = m_state->RunSimulation(m_fed_input, m_log);
    }
    catch (const std::exception &e)
    {
        m_log << "Error: " << e.what() << std::endl;
    }

    Close();

    if (granted_time < 0.0)
    {
        m_log << "Could not perform simulation! Federate finalized.\nGranted time: " << granted_time << std::endl;
    }
    else
    {
        m_log << "Federate finalized.\nGranted time: " << granted_time << std::endl;
    }
}

void three_phase::ThreePhaseFederate::Close()
{
    gridpack::math::Finalize();
    if (m_state) m_state->m_fed.finalize();
}
