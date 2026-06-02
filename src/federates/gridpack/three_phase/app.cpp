#include "app.hpp"

#include <iostream>
#include <fstream>
#include <sstream>
#include <cmath>

#include "common/utils/stopwatch.hpp"

#include "gridpack/include/gridpack.hpp"
#include "/usr/local/GridPACK/include/gridpack/applications/modules/powerflow/pf_factory_module.hpp"

#include "common_math.hpp"
#include "inputs.hpp"

namespace
{

enum class Parser
{
    UNSET,
    PTI23,
    PTI33
};

} // namespace

// ###################################
// ThreePhaseApp::State Implementation
// ###################################

class three_phase::PhaseApp::State
{
  private:
    std::unordered_map<int, int> m_bus_indeces;
    gridpack::parallel::Communicator m_world;

  public:
    boost::shared_ptr<gridpack::powerflow::PFNetwork> network;
    gridpack::utility::Configuration::CursorPtr cursor;

    double base_MVA = 100.0;

    std::unique_ptr<gridpack::powerflow::PFFactoryModule> pf_factory;
    std::unique_ptr<gridpack::mapper::BusVectorMap<gridpack::powerflow::PFNetwork>> v_map;
    std::unique_ptr<gridpack::mapper::FullMatrixMap<gridpack::powerflow::PFNetwork>> j_map;
    boost::shared_ptr<gridpack::math::Vector> PQ;
    boost::shared_ptr<gridpack::math::Vector> X;
    boost::shared_ptr<gridpack::math::Matrix> J;
    std::unique_ptr<gridpack::math::LinearSolver> solver;

    State() {}

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

        if (!opened)
        {
            return is_initialized;
        }

        cursor = config->getCursor("Configuration.Powerflow");
        base_MVA = cursor->get("baseMVA", 100.0);

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

    std::complex<double> ComputeVoltageCurrent(int target_bus_id, const std::complex<double> &Sa,
                                               const std::string &phase_name)
    {
        // Apply S (pu in MW/Mvar) and solve
        const double P_MW = Sa.real() * this->base_MVA;
        const double Q_Mvar = Sa.imag() * this->base_MVA;

        const int bus_index = this->GetBusIndex(target_bus_id);

        this->network->getBusData(bus_index)->setValue(LOAD_PL, P_MW, 0);
        this->network->getBusData(bus_index)->setValue(LOAD_QL, Q_Mvar, 0);

        const double tolerance = this->cursor->get("tolerance", 1.0e-6);
        const int max_iteration = this->cursor->get("maxIteration", 50);

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
        this->v_map->mapToBus(*this->X);
        this->network->updateBuses();

        const double v_mag = this->network->getBus(bus_index)->getVoltage();
        const double v_ang_deg = this->network->getBus(bus_index)->getPhase(); // deg

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

        return three_phase::RotationToRadians(v_mag, v_ang_deg);
    }
};

// ###################################
// PhaseApp Implementation
// ###################################

three_phase::PhaseApp::PhaseApp() : m_state(std::make_unique<three_phase::PhaseApp::State>()) {}

// At this point the inner State class has been defined, so we can default delete.
three_phase::PhaseApp::~PhaseApp() = default;

bool three_phase::PhaseApp::Initialize(const std::string &config_file, const std::vector<int> &bus_ids,
                                       common::utils::LocalLogHelper &log)
{
    bool success = m_state->InitializeConfig(config_file);
    if (!success)
    {
        log << "Could not initialize pf state with config file: " << config_file << std::endl;
        return success;
    }

    success = m_state->InitializeBusIndeces(bus_ids);
    if (!success)
    {
        std::stringstream out;
        out << "Could not initialize bus indeces with the following ids:\n";
        for (int bus_ids : bus_ids)
        {
            out << bus_ids << " ";
        }
        out << "/n";
        log << out.str();
        return success;
    }

    m_state->InitializeFactoryAndFields();

    return success;
}

std::complex<double> three_phase::PhaseApp::ComputeVoltageCurrent(int target_bus_id, const std::complex<double> &Sa,
                                                                  const std::string &phase_name,
                                                                  const std::complex<double> rotation_radians)
{
    return m_state->ComputeVoltageCurrent(target_bus_id, Sa, phase_name) * rotation_radians;
}

// ###################################
// Rotation Implementation
// ###################################

three_phase::Rotation::Rotation(double a, double b, double c) : m_a(a), m_b(b), m_c(c) {}

std::complex<double> three_phase::Rotation::GetRotation(double rot) const
{
    return three_phase::RotationToRadians(1.0, rot);
}

std::complex<double> three_phase::Rotation::GetARad() const { return GetRotation(m_a); }
std::complex<double> three_phase::Rotation::GetBRad() const { return GetRotation(m_b); }
std::complex<double> three_phase::Rotation::GetCRad() const { return GetRotation(m_c); }

// ###################################
// ThreePhaseApp Implementation
// ###################################

three_phase::ThreePhaseApp::ThreePhaseApp() {}

bool three_phase::ThreePhaseApp::Initialize(const std::string &xml_file, double a_rotation_degrees,
                                            double b_rotation_degrees, double c_rotation_degrees,
                                            const std::vector<int> &bus_ids, common::utils::LocalLogHelper &log)
{
    if (m_phase.Initialize(xml_file, bus_ids, log))
    {
        m_rotation = three_phase::Rotation(a_rotation_degrees, b_rotation_degrees, c_rotation_degrees);
        return true;
    }
    else
    {
        log << "Failed to initialize the executor.\nxml_file: " << xml_file << "\n";
        log << "bus_ids: ";
        for (int bus_id : bus_ids)
        {
            log << bus_id << " ";
        }
        log << "\n";
        return false;
    }
}

common::helics::ThreePhaseValues
three_phase::ThreePhaseApp::ComputeVoltage(const common::helics::ThreePhaseValues &power_s, int bus_id,
                                           common::utils::LocalLogHelper &log)
{
    common::helics::ThreePhaseValues phased_voltage;

    std::stringstream out;
    out << "####################################\n";
    out << "Bus Id: " << bus_id << "\n";
    out << "Power A: " << power_s.a << "\n";
    out << "Power B: " << power_s.b << "\n";
    out << "Power C: " << power_s.c << "\n";

    common::utils::Stopwatch watch;
    watch.Start();
    phased_voltage.a = m_phase.ComputeVoltageCurrent(bus_id, power_s.a, "A", m_rotation.GetARad());
    long long time_a = watch.ElapsedMilliseconds();
    out << "Time A: " << time_a << " ms\n";

    watch.Start();
    phased_voltage.b = m_phase.ComputeVoltageCurrent(bus_id, power_s.b, "B", m_rotation.GetBRad());
    long long time_b = watch.ElapsedMilliseconds();
    out << "Time B: " << time_b << " ms\n";

    watch.Start();
    phased_voltage.c = m_phase.ComputeVoltageCurrent(bus_id, power_s.c, "C", m_rotation.GetCRad());
    long long time_c = watch.ElapsedMilliseconds();
    out << "Time C: " << time_c << " ms\n";

    out << "Computed A: " << phased_voltage.a << "\n";
    out << "Computed B: " << phased_voltage.b << "\n";
    out << "Computed C: " << phased_voltage.c << "\n";

    out << "####################################\n\n";

    log << out.str();

    return phased_voltage;
}

common::helics::ThreePhaseValues three_phase::ThreePhaseApp::GetInitialPhasedVoltages() const
{
    return { m_rotation.GetARad(), m_rotation.GetBRad(), m_rotation.GetCRad() };
}