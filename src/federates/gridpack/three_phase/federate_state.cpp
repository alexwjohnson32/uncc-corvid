#include "federate_state.hpp"

#include "common/utils/json_templates.hpp"

#include <memory>
#include <stdexcept>
#include <complex>

namespace
{

std::unordered_map<std::string, common::helics::ThreePhaseSubscriptions>
GetSubscriptions(const three_phase::ThreePhaseInput &input, const std::shared_ptr<helics::ValueFederate> &fed)
{
    std::unordered_map<std::string, common::helics::ThreePhaseSubscriptions> subs;

    for (const std::string &gridlabd_name : input.GetGridlabDNames())
    {
        subs[gridlabd_name] = { fed->registerSubscription(gridlabd_name + "/Sa", "VA"),
                                fed->registerSubscription(gridlabd_name + "/Sb", "VA"),
                                fed->registerSubscription(gridlabd_name + "/Sc", "VA") };
    }

    return subs;
}

std::unordered_map<std::string, common::helics::ThreePhaseValues>
GetInitialPhaseValues(const three_phase::ThreePhaseInput &input)
{
    std::unordered_map<std::string, common::helics::ThreePhaseValues> initial_values;
    for (const std::string &gridlabd_name : input.GetGridlabDNames())
    {
        initial_values[gridlabd_name] = common::helics::ThreePhaseValues();
    }
    return initial_values;
}

std::vector<int> GetBusIds(const three_phase::ThreePhaseInput &input)
{
    std::vector<int> bus_ids;
    for (const three_phase::GridlabDInputs &gridlabd_info : input.gridlabd_infos)
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

std::unique_ptr<three_phase::FederateState>
three_phase::FederateState::Create(const three_phase::ThreePhaseInput &input,
                                   const std::shared_ptr<helics::ValueFederate> &fed,
                                   common::utils::LocalLogHelper &log)
{
    auto ptr = std::unique_ptr<three_phase::FederateState>(new three_phase::FederateState());
    ptr->Initialize(input, fed, log);
    return ptr;
}

void three_phase::FederateState::Initialize(const three_phase::ThreePhaseInput &input,
                                            const std::shared_ptr<helics::ValueFederate> &fed,
                                            common::utils::LocalLogHelper &log)
{
    log << "Creating Federate..." << std::endl;
    m_fed = fed;

    log << "Registering published connections..." << std::endl;
    m_pub = common::helics::ThreePhaseVoltagePublisher(m_fed);

    log << "Registering subscribed connections..." << std::endl;
    m_subs = GetSubscriptions(input, m_fed);

    log << "Getting Bus Ids..." << std::endl;
    m_bus_ids = GetBusIds(input);

    log << "Getting the period..." << std::endl;
    m_period = m_fed->getTimeProperty(HELICS_PROPERTY_TIME_PERIOD);

    log << "Getting Magnitude..." << std::endl;
    m_ln_magnitude = input.ln_magnitude;

    log << "Initializing the executor..." << std::endl;
    if (!m_executor.Initialize(input.phase_a, input.phase_b, input.phase_c, m_bus_ids, log, input.ln_magnitude))
    {
        throw std::runtime_error("Could Not Initialize Executor, Exiting.");
    }
    else
    {
        log << "Initialized State, Printing Federate:\n" << FederateToString(m_fed) << std::endl;
    }
}

three_phase::FederateState::FederateState() {}

double three_phase::FederateState::RunSimulation(const three_phase::ThreePhaseInput &input,
                                                 common::utils::LocalLogHelper &log)
{
    // Enter the federate into executing mode
    m_fed->enterExecutingMode();
    log << "GridPACK Federate has entered execution mode." << std::endl;

    // Initial voltage publish
    log << "Publish initial voltage." << std::endl;
    m_pub.Publish(m_executor.GetInitialPhasedVoltages().Multiply(m_ln_magnitude));
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

double three_phase::FederateState::SimulateStep(const std::vector<three_phase::GridlabDInputs> &gridlabd_infos,
                                                const double current_time, common::utils::LocalLogHelper &log)
{
    double granted_time = m_fed->requestTime(current_time + m_period);
    log << "\n[Time " << granted_time << "]\n";

    for (const three_phase::GridlabDInputs &gridlabd_info : gridlabd_infos)
    {
        log << "\nBus Id: " << gridlabd_info.bus_id << "\nGridlabd Names:\n";

        common::helics::ThreePhaseValues s_total;
        for (const std::string &gridlabd_name : gridlabd_info.names)
        {
            log << "\t\"" << gridlabd_name << "\"\n";
            common::helics::ThreePhaseValues current_values = m_subs.at(gridlabd_name).GetValues();
            log << "\tsub values: [" << current_values.a << "," << current_values.b << "," << current_values.c << "\n";
            common::helics::ThreePhaseValues limited_power = common::helics::LimitPower(current_values, 100.0, 1e6);
            log << "\tlimited values: [" << limited_power.a << "," << limited_power.b << "," << limited_power.c << "\n";
            s_total.a += limited_power.a;
            s_total.b += limited_power.b;
            s_total.c += limited_power.c;
        }

        log << "\nTotal S received from Gridlab-D: [" << s_total.a << ", " << s_total.b << ", " << s_total.c << "]\n";

        common::helics::ThreePhaseValues v =
            m_executor.ComputeVoltage(granted_time, s_total, gridlabd_info.bus_id, log);
        common::helics::ThreePhaseValues scaled = v.Multiply(m_ln_magnitude);

        log << "Updated V by GridPACK: [" << scaled.a << ", " << scaled.b << ", " << scaled.c << "]\n";

        m_pub.Publish(scaled);
    }

    return granted_time;
}