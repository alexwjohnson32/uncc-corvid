#include "federate.hpp"
#include "common/utils/json_templates.hpp"

#include <stdexcept>
#include <sstream>
#include <iostream>

three_phase::ThreePhaseFederate::ThreePhaseFederate() {}
three_phase::ThreePhaseFederate::~ThreePhaseFederate() {}

void three_phase::ThreePhaseFederate::Initialize(const three_phase::ThreePhaseInput &input,
                                                 const std::shared_ptr<helics::ValueFederate> &fed)
{
    m_fed_input = input;

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
    m_state = three_phase::FederateState::Create(m_fed_input, fed, m_log);
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

    if (granted_time < 0.0)
    {
        m_log << "Could not perform simulation! Federate finalized.\nGranted time: " << granted_time << std::endl;
    }
    else
    {
        m_log << "Federate finalized.\nGranted time: " << granted_time << std::endl;
    }
}