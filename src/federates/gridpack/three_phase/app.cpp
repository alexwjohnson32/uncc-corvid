#include "app.hpp"

#include <complex>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cmath>
#include <stdexcept>

#include "common/utils/stopwatch.hpp"

#include "gridpack/include/gridpack.hpp"
#include "/usr/local/GridPACK/include/gridpack/applications/modules/powerflow/pf_factory_module.hpp"

#include "inputs.hpp"

namespace
{

enum class Parser
{
    UNSET,
    PTI23,
    PTI33
};

double ToRadian(double degrees) { return degrees * (M_PI / 180.0); }

std::complex<double> ToComplexRadian(double degrees) { return std::polar(1.0, ToRadian(degrees)); }

// ###################################
// PhaseAppState Implementation
// ###################################

class PhaseAppState
{
  private:
    std::unordered_map<int, int> m_bus_indeces;
    gridpack::parallel::Communicator m_world;

  public:
    boost::shared_ptr<gridpack::powerflow::PFNetwork> network;
    gridpack::utility::Configuration::CursorPtr cursor;

    std::unique_ptr<gridpack::powerflow::PFFactoryModule> pf_factory;
    std::unique_ptr<gridpack::mapper::BusVectorMap<gridpack::powerflow::PFNetwork>> v_map;
    std::unique_ptr<gridpack::mapper::FullMatrixMap<gridpack::powerflow::PFNetwork>> j_map;
    boost::shared_ptr<gridpack::math::Vector> PQ;
    boost::shared_ptr<gridpack::math::Vector> X;
    boost::shared_ptr<gridpack::math::Matrix> J;
    std::unique_ptr<gridpack::math::LinearSolver> solver;

    PhaseAppState() {}

    bool InitializeConfig(const std::string &config_file)
    {
        bool is_initialized = false;
        network.reset(new gridpack::powerflow::PFNetwork(m_world));

        gridpack::utility::Configuration *config = gridpack::utility::Configuration::configuration();
        config->enableLogging(&std::cout);

        bool opened;
        if (!config_file.empty())
        {
            opened = config->open(config_file, m_world);
        }
        else
        {
            opened = config->open("118.xml", m_world);
        }

        if (!opened)
        {
            return is_initialized;
        }

        cursor = config->getCursor("Configuration.Powerflow");

        std::string filename = "";
        Parser file_type = Parser::PTI23;
        if (!cursor->get("networkConfiguration", &filename))
        {
            if (cursor->get("networkConfiguration_v33", &filename))
            {
                file_type = Parser::PTI33;
            }
            else
            {
                std::cerr << "No network configuration file specified\n";
                return is_initialized;
            }
        }

        const double phase_shift_sign = cursor->get("phaseShiftSign", 1.0);
        if (m_world.rank() == 0)
        {
            std::cout << "Network filename: (" << filename << ")\n";
        }

        if (file_type == Parser::PTI23)
        {
            if (m_world.rank() == 0)
            {
                std::cout << "Using V23 parser\n";
            }
            gridpack::parser::PTI23_parser<gridpack::powerflow::PFNetwork> parser(network);
            parser.parse(filename.c_str());
            if (phase_shift_sign == -1.0)
            {
                parser.changePhaseShiftSign();
            }
        }
        else if (file_type == Parser::PTI33)
        {
            if (m_world.rank() == 0)
            {
                std::cout << "Using V33 parser\n";
            }
            gridpack::parser::PTI33_parser<gridpack::powerflow::PFNetwork> parser(network);
            parser.parse(filename.c_str());
            if (phase_shift_sign == -1.0)
            {
                parser.changePhaseShiftSign();
            }
        }

        is_initialized = true;
        return is_initialized;
    }

    bool InitializeBusIndeces(const std::vector<int> &bus_ids)
    {
        m_bus_indeces.clear();

        for (int bus_id : bus_ids)
        {
            int bus_index = -1;
            for (int i = 0; i < network->numBuses(); i++)
            {
                if (network->getOriginalBusIndex(i) == bus_id)
                {
                    bus_index = i;
                    break;
                }
            }

            if (bus_index == -1)
            {
                if (m_world.rank() == 0)
                {
                    std::cerr << "Bus " << bus_id << " not found\n";
                }
                m_bus_indeces.clear();
                return false;
            }
            else
            {
                m_bus_indeces[bus_id] = bus_index;
            }
        }

        return true;
    }

    void InitializeFactoryAndFields()
    {
        // One time build
        network->partition();

        pf_factory = std::make_unique<gridpack::powerflow::PFFactoryModule>(network);
        pf_factory->load();
        pf_factory->setComponents();
        pf_factory->setExchange();

        network->initBusUpdate();

        pf_factory->setYBus();
        pf_factory->setSBus();

        // Solver / maps
        pf_factory->setMode(gridpack::powerflow::RHS);
        v_map = std::make_unique<gridpack::mapper::BusVectorMap<gridpack::powerflow::PFNetwork>>(network);
        PQ = v_map->mapToVector();

        pf_factory->setMode(gridpack::powerflow::Jacobian);
        j_map = std::make_unique<gridpack::mapper::FullMatrixMap<gridpack::powerflow::PFNetwork>>(network);
        J = j_map->mapToMatrix();

        X.reset(PQ->clone());
        solver = std::make_unique<gridpack::math::LinearSolver>(*J);
        solver->configure(cursor);
    }

    int GetBusIndex(int bus_id) const
    {
        int bus_index = -1;

        if (m_bus_indeces.count(bus_id))
        {
            bus_index = m_bus_indeces.at(bus_id);
        }

        return bus_index;
    }

    int GetWorldRank() const { return m_world.rank(); }

    std::complex<double> ComputeVoltageCurrent(double time_step, int target_bus_id, const std::complex<double> &Sa,
                                               const std::string &phase_name, double rotation_degrees,
                                               common::utils::LocalLogHelper &log)
    {
        // Apply S (pu in MW/Mvar) and solve
        const double tolerance = this->cursor->get("tolerance", 1.0e-6);
        const int max_iteration = this->cursor->get("maxIteration", 50);

        const int bus_index = this->GetBusIndex(target_bus_id);

        log << "Target Bus: " << target_bus_id << "\n";
        log << "bus_index: " << bus_index << "\n";
        log << "Tolerance: " << tolerance << "\n";
        log << "Max Iterations: " << max_iteration << "\n";
        log << "S: " << Sa << "\n";

        this->network->getBusData(bus_index)->setValue(LOAD_PL, Sa.real(), 0);
        this->network->getBusData(bus_index)->setValue(LOAD_QL, Sa.imag(), 0);

        this->pf_factory->setMode(gridpack::powerflow::RHS);
        this->v_map->mapToVector(*this->PQ);

        this->pf_factory->setMode(gridpack::powerflow::Jacobian);
        this->j_map->mapToMatrix(*this->J);

        this->X->zero();
        this->solver->solve(*this->PQ, *this->X);
        auto tol = this->PQ->normInfinity();

        int iterator = 0;
        while (std::real(tol) > tolerance && iterator < max_iteration)
        {
            this->pf_factory->setMode(gridpack::powerflow::RHS);
            this->v_map->mapToBus(*this->X);
            this->network->updateBuses();
            this->v_map->mapToVector(*this->PQ);

            this->pf_factory->setMode(gridpack::powerflow::Jacobian);
            this->j_map->mapToMatrix(*this->J);

            this->X->zero();
            this->solver->solve(*this->PQ, *this->X);
            tol = this->PQ->normInfinity();
            iterator++;
        }

        // Push solution and return bus id voltage
        this->pf_factory->setMode(gridpack::powerflow::RHS);
        this->v_map->mapToBus(this->X);
        this->network->updateBuses();

        if (this->GetWorldRank() == 0)
        {
            const std::string filename_out = "bus_voltages_phase_" + phase_name + ".csv";
            std::ofstream out_file(filename_out);

            out_file << "Original Bus Number,Voltage Magnitude (pu),Voltage Angle (deg)\n";
            for (int i = 0; i < this->network->numBuses(); i++)
            {
                out_file << this->network->getOriginalBusIndex(i) << "," << this->network->getBus(i)->getVoltage()
                         << "," << this->network->getBus(i)->getPhase() << "\n";
            }
            out_file.close();

            std::cout << "Bus voltages written to " << filename_out << "\n";
        }

        const double v_mag = this->network->getBus(bus_index)->getVoltage();
        const double v_ang = this->network->getBus(bus_index)->getPhase();
        log << "v_mag: " << v_mag << "\n";
        log << "v_ang: " << v_ang << "\n";
        const std::complex<double> solver = std::polar(v_mag, v_ang);
        log << "v_solver: " << solver << "\n";
        const double rotation_radians = ToRadian(rotation_degrees);
        log << "rotation_radians: " << rotation_radians << "\n";
        const std::complex<double> result = solver * std::polar(1.0, rotation_radians);
        log << "boundary result: " << result << "\n";

        return result;
    }
};

} // namespace

// ###################################
// PhaseApp Implementation
// ###################################

three_phase::PhaseApp::PhaseApp()
    : m_bus_ids(), m_rotation_degrees(0.0), m_phase_name(""), m_ln_magnitude(0.0), m_config_file("")
{
}

three_phase::PhaseApp::~PhaseApp() = default;

bool three_phase::PhaseApp::Initialize(const std::string &config_file, const std::vector<int> &bus_ids,
                                       const std::string &phase_name, double rotation_degrees,
                                       common::utils::LocalLogHelper &log, double ln_magnitude)
{
    m_bus_ids = bus_ids;
    m_rotation_degrees = rotation_degrees;
    m_phase_name = phase_name;
    m_ln_magnitude = ln_magnitude;
    m_config_file = config_file;

    return true;
}

std::complex<double> three_phase::PhaseApp::ComputeVoltageCurrent(double time_step, int target_bus_id,
                                                                  const std::complex<double> &Sa,
                                                                  common::utils::LocalLogHelper &log)

{
    PhaseAppState state;
    if (!state.InitializeConfig(m_config_file))
    {
        log << "Phase " << m_phase_name << ": Could not initialize pf state with config file: " << m_config_file
            << std::endl;
        throw std::runtime_error("Could not initialize gridpack state!");
    }

    if (!state.InitializeBusIndeces(m_bus_ids))
    {
        std::stringstream out;
        log << "Phase " << m_phase_name << ": Could not initialize bus indeces with the following ids:\n";
        for (int bus_ids : m_bus_ids)
        {
            log << bus_ids << " ";
        }
        log << "/n";
        throw std::runtime_error("Could not initialize gridpack bus indeces!");
    }

    state.InitializeFactoryAndFields();

    const std::complex<double> result =
        state.ComputeVoltageCurrent(time_step, target_bus_id, Sa, m_phase_name, m_rotation_degrees, log);
    const std::complex<double> rotated = result * ToComplexRadian(m_rotation_degrees);

    log << "rotated: " << rotated << "\n";

    return rotated;
}

double three_phase::PhaseApp::GetRotationAngle() const { return m_rotation_degrees; }

// ###################################
// ThreePhaseApp Implementation
// ###################################

three_phase::ThreePhaseApp::ThreePhaseApp() {}

bool three_phase::ThreePhaseApp::Initialize(const three_phase::PhaseInput &phase_a,
                                            const three_phase::PhaseInput &phase_b,
                                            const three_phase::PhaseInput &phase_c, const std::vector<int> &bus_ids,
                                            common::utils::LocalLogHelper &log, double ln_magnitude)
{
    auto logger_lambda =
        [](const three_phase::PhaseInput &phase, const std::vector<int> &bus_ids, common::utils::LocalLogHelper &log)
    {
        log << "Failed to initialize the executor.\nxml_file: " << phase.xml_file << "\n";
        log << "bus_ids: ";
        for (int bus_id : bus_ids)
        {
            log << bus_id << " ";
        }
        log << "\rotation (degrees): " << phase.rotation_degrees << "\n";
    };

    bool success_a, success_b, success_c = false;

    success_a = m_phase_a.Initialize(phase_a.xml_file, bus_ids, "A", phase_a.rotation_degrees, log, ln_magnitude);
    if (!success_a)
    {
        logger_lambda(phase_a, bus_ids, log);
    }

    success_b = m_phase_b.Initialize(phase_b.xml_file, bus_ids, "B", phase_b.rotation_degrees, log, ln_magnitude);
    if (!success_b)
    {
        logger_lambda(phase_b, bus_ids, log);
    }

    success_c = m_phase_c.Initialize(phase_c.xml_file, bus_ids, "C", phase_c.rotation_degrees, log, ln_magnitude);
    if (!success_c)
    {
        logger_lambda(phase_c, bus_ids, log);
    }

    return success_a && success_b && success_c;
}

common::helics::ThreePhaseValues
three_phase::ThreePhaseApp::ComputeVoltage(double time_step, const common::helics::ThreePhaseValues &power_s,
                                           int bus_id, common::utils::LocalLogHelper &log)
{
    common::helics::ThreePhaseValues phased_voltage;

    log << "####################################\n";
    log << "Bus Id: " << bus_id << "\n";

    log << "\nPhase A\n";
    phased_voltage.a = m_phase_a.ComputeVoltageCurrent(time_step, bus_id, power_s.a, log);

    log << "\nPhase B\n";
    phased_voltage.b = m_phase_b.ComputeVoltageCurrent(time_step, bus_id, power_s.b, log);

    log << "\nPhase C\n";
    phased_voltage.c = m_phase_c.ComputeVoltageCurrent(time_step, bus_id, power_s.c, log);

    log << "####################################\n\n";

    return phased_voltage;
}

common::helics::ThreePhaseValues three_phase::ThreePhaseApp::GetInitialPhasedVoltages() const
{
    const common::helics::ThreePhaseValues initial_phased = { ToComplexRadian(m_phase_a.GetRotationAngle()),
                                                              ToComplexRadian(m_phase_b.GetRotationAngle()),
                                                              ToComplexRadian(m_phase_c.GetRotationAngle()) };
    return initial_phased;
}