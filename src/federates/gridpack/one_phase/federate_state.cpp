#include "federate_state.hpp"

#include "common/utils/json_templates.hpp"

#include <memory>
#include <stdexcept>
#include <complex>

namespace
{

std::unordered_map<std::string, ::helics::Input> GetSubscriptions(const one_phase::PhaseInput &input,
                                                                  const std::shared_ptr<helics::ValueFederate> &fed)
{
    std::unordered_map<std::string, ::helics::Input> subs;
    const std::string field = "/" + input.subscription_field;
    for (const std::string &gridlabd_name : input.GetGridlabDNames())
    {
        subs[gridlabd_name] = fed->registerSubscription(gridlabd_name + field, "VA");
    }

    return subs;
}

std::vector<int> GetBusIds(const one_phase::PhaseInput &input)
{
    std::vector<int> bus_ids;
    for (const one_phase::GridlabDInputs &gridlabd_info : input.gridlabd_infos)
    {
        bus_ids.push_back(gridlabd_info.bus_id);
    }
    return bus_ids;
}

std::string FederateToString(const std::shared_ptr<helics::ValueFederate> &fed)
{
    std::string json_result = fed->query(fed->getName(), "federate");
    return common::utils::ToJsonString(json_result, true);
}

} // namespace

std::unique_ptr<one_phase::FederateState>
one_phase::FederateState::Create(const one_phase::PhaseInput &input, const std::shared_ptr<helics::ValueFederate> &fed,
                                 common::utils::LocalLogHelper &log)
{
    auto ptr = std::unique_ptr<one_phase::FederateState>(new one_phase::FederateState());
    ptr->Initialize(input, fed, log);
    return ptr;
}

void one_phase::FederateState::Initialize(const one_phase::PhaseInput &input,
                                          const std::shared_ptr<helics::ValueFederate> &fed,
                                          common::utils::LocalLogHelper &log)
{
    log << "Creating Federate..." << std::endl;
    m_fed = fed;

    log << "Registering published connections..." << std::endl;
    m_pub = m_fed->registerPublication(input.publication_field, "complex", "V");

    log << "Registering subscribed connections..." << std::endl;
    m_subs = GetSubscriptions(input, m_fed);

    log << "Getting Bus Ids..." << std::endl;
    m_bus_ids = GetBusIds(input);

    log << "Getting the period..." << std::endl;
    m_period = m_fed->getTimeProperty(HELICS_PROPERTY_TIME_PERIOD);

    log << "Getting Magnitude..." << std::endl;
    m_ln_magnitude = input.ln_magnitude;

    log << "Initializing the app..." << std::endl;
    m_app.Initialize(input.xml_file, m_bus_ids, input.phase_name, input.rotation_degrees);
}

one_phase::FederateState::FederateState() {}

double one_phase::FederateState::RunSimulation(const one_phase::PhaseInput &input, common::utils::LocalLogHelper &log)
{
    // Enter the federate into executing mode
    m_fed->enterExecutingMode();
    log << "GridPACK Federate has entered execution mode." << std::endl;

    // Initial voltage publish
    log << "Publish initial voltage." << std::endl;
    m_pub.publish(m_app.GetInitialPhasedVoltages() * m_ln_magnitude);
    log << "Published." << std::endl;

    // Perform Simulation
    /*
     * What performing the simulation looks like:
     * 1. Get the granted time
     * 2. Separate the distribution systems based on bus_id
     * 3. For each bus_id, aggregate the total power from the distribution systems. Meaning, limt the power for
     * each phase and keep a total of all limited power for each phase per bus_id.
     * 4. Run the powerflow application per bus id (maybe this means one application, or it means an application
     * per bus_id).
     * 5. Publish individual calculated V for each ID at the same granted time.
     */
    const double total_interval = input.total_time;
    double granted_time = 0.0;

    while (granted_time + m_period <= total_interval)
    {
        log << "\n##########################################\n"
            << "New Loop Iteration Information:\n\tGranted Time + Period: " << granted_time + m_period
            << "\n\tTotal Interval: " << total_interval << "\nRequesting New Granted Time: " << granted_time + m_period
            << "\n";

        granted_time = SimulateStep(input.gridlabd_infos, granted_time, log);

        log << "##########################################\n";
    }

    return granted_time;
}

double one_phase::FederateState::SimulateStep(const std::vector<one_phase::GridlabDInputs> &gridlabd_infos,
                                              const double current_time, common::utils::LocalLogHelper &log)
{
    double granted_time = m_fed->requestTime(current_time + m_period);
    log << "\n[Time " << granted_time << "]\n";

    for (const one_phase::GridlabDInputs &gridlabd_info : gridlabd_infos)
    {
        log << "\nBus Id: " << gridlabd_info.bus_id << "\nGridlabd Names:\n";

        std::complex<double> s_total;
        for (const std::string &gridlabd_name : gridlabd_info.names)
        {
            log << "\t\"" << gridlabd_name << "\"\n";

            const std::complex<double> sub_value = m_subs.at(gridlabd_name).getValue<std::complex<double>>();
            log << "\tsub value: " << sub_value << "\n";

            const std::complex<double> limited_power = common::helics::LimitPower(sub_value / 1e6, 100.0);
            log << "\tlimited value: " << limited_power << "\n";

            s_total += limited_power;
        }

        log << "\nTotal S received from Gridlab-D: " << s_total << "\n";

        const std::complex<double> v = m_app.ComputeVoltage(gridlabd_info.bus_id, s_total, log);
        const std::complex<double> scaled = v * m_ln_magnitude;

        log << "Updated V by GridPACK: " << scaled << "\n";

        m_pub.publish(scaled);
    }

    return granted_time;
}