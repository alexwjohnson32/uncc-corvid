#include "ieee_118_federate_state.hpp"
#include <fstream>

#include "common/utils/json_templates.hpp"

namespace
{

std::string FederateToString(const std::shared_ptr<helics::ValueFederate> fed)
{
    std::string json_result = fed->query(fed->getName(), "federate");
    return common::utils::ToJsonString(json_result, true);
}

std::unordered_map<std::string, common::helics::ThreePhaseSubscriptions>
GetSubscriptions(const ieee_118::IEEE118Input &input, const std::shared_ptr<helics::ValueFederate> fed)
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
GetInitialPhaseValues(const ieee_118::IEEE118Input &input)
{
    std::unordered_map<std::string, common::helics::ThreePhaseValues> initial_values;
    for (const std::string &gridlabd_name : input.GetGridlabDNames())
    {
        initial_values[gridlabd_name] = common::helics::ThreePhaseValues();
    }
    return initial_values;
}

std::vector<int> GetBusIds(const ieee_118::IEEE118Input &input)
{
    std::vector<int> bus_ids;
    for (const ieee_118::GridlabDInputs &gridlabd_info : input.gridlabd_infos)
    {
        bus_ids.push_back(gridlabd_info.bus_id);
    }
    return bus_ids;
}

} // namespace

ieee_118::FederateState::FederateState() {}

ieee_118::FederateState::~FederateState() {}

void ieee_118::FederateState::Initialize(const ieee_118::IEEE118Input &input,
                                         const std::shared_ptr<helics::ValueFederate> fed,
                                         common::utils::LocalLogHelper &log)
{
    log << "Creating Federate..." << std::endl;
    m_fed = fed;

    log << "Registering published connections..." << std::endl;
    m_pub = common::helics::VoltagePublisher(m_fed);

    log << "Registering subscribed connections..." << std::endl;
    m_subs = GetSubscriptions(input, m_fed);

    log << "Getting Bus Ids..." << std::endl;
    m_bus_ids = GetBusIds(input);

    log << "Getting the period..." << std::endl;
    m_period = m_fed->getTimeProperty(HELICS_PROPERTY_TIME_PERIOD);

    log << "Getting Magnitude..." << std::endl;
    m_ln_magnitude = input.ln_magnitude;

    log << "Initializing the executor..." << std::endl;
    if (!m_executor.Initialize("118.xml", m_bus_ids, std::complex<double>(-0.5, -0.866025)))
    {
        throw std::runtime_error("Could Not Initialize Executor, Exiting.");
    }
    else
    {
        log << "Initialized State, Printing Federate:\n" << FederateToString(m_fed) << std::endl;
    }
}

double ieee_118::FederateState::RunSimulation(const ieee_118::IEEE118Input &input, common::utils::LocalLogHelper &log)
{
    // Enter the federate into executing mode
    m_fed->enterExecutingMode();
    log << "GridPACK Federate has entered execution mode." << std::endl;

    // Initial voltage publish
    log << "Publish initial voltage." << std::endl;
    common::helics::ThreePhaseValues initial_values = { { 1.0, 0.0 }, { -0.5, -0.866025 }, { -0.5, 0.866025 } };
    initial_values = initial_values.Multiply(m_ln_magnitude);
    m_pub.Publish(initial_values);
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

double ieee_118::FederateState::SimulateStep(const std::vector<ieee_118::GridlabDInputs> &gridlabd_infos,
                                             const double current_time, common::utils::LocalLogHelper &log)
{
    double granted_time = m_fed->requestTime(current_time + m_period);
    log << "\n[Time " << granted_time << "]\n";

    for (const ieee_118::GridlabDInputs &gridlabd_info : gridlabd_infos)
    {
        log << "\nBus Id: " << gridlabd_info.bus_id << "\nGridlabd Names:\n\t";

        common::helics::ThreePhaseValues s_total;
        for (const std::string &gridlabd_name : gridlabd_info.names)
        {
            log << "\"" << gridlabd_name << "\" ";
            common::helics::ThreePhaseValues limited_power =
                common::helics::LimitPower(m_subs.at(gridlabd_name).GetValues(), 1.0);
            s_total.a += limited_power.a;
            s_total.b += limited_power.b;
            s_total.c += limited_power.c;
        }

        log << "\nTotal S received from Gridlab-D: [" << s_total.a << ", " << s_total.b << ", " << s_total.c << "]\n";

        common::helics::ThreePhaseValues v = m_executor.ComputeVoltage(s_total, gridlabd_info.bus_id);
        common::helics::ThreePhaseValues scaled = v.Multiply(m_ln_magnitude);

        log << "Updated V by GridPACK: [" << scaled.a << ", " << scaled.b << ", " << scaled.c << "]\n";

        m_pub.Publish(scaled);
    }

    return granted_time;
}