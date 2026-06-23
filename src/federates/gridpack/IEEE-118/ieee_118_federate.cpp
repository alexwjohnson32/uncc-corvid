#include "ieee_118_federate.hpp"
#include "common/utils/json_templates.hpp"

#include <string>
#include <stdexcept>
#include <sstream>
#include <iostream>

#include <helics/application_api/ValueFederate.hpp>
#include <helics/application_api/Publications.hpp>
#include <helics/application_api/Inputs.hpp>

ieee_118::IEEE118Federate::IEEE118Federate() {}
ieee_118::IEEE118Federate::~IEEE118Federate() {}

void ieee_118::IEEE118Federate::Initialize(const ieee_118::IEEE118Input &input,
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
    m_state.Initialize(m_fed_input, fed, m_log);
}

void ieee_118::IEEE118Federate::Run()
{
    double granted_time = -1;

    try
    {
        granted_time = m_state.RunSimulation(m_fed_input, m_log);
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