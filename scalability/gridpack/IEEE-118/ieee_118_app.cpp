#include "ieee_118_app.hpp"

#include <iostream>
#include <fstream>
#include <sstream>

#include "stopwatch.hpp"

#include "gridpack/include/gridpack.hpp"
#include "/usr/local/GridPACK/include/gridpack/applications/modules/powerflow/pf_factory_module.hpp"

namespace
{

enum class Parser
{
    UNSET,
    PTI23,
    PTI33
};

constexpr double PI = 3.14159265358979323846;

} // namespace

// ###################################
// IEEE118App::State Implementation
// ###################################

class ieee_118::IEEE118App::State
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
        else
        {
            opened = config->open("118.xml", m_world);
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

    std::complex<double> ComputeVoltageCurrent(const std::string &config_file, int target_bus_id,
                                               const std::string &phase_name, const std::complex<double> &Sa)
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
            const std::string filename_out = "bus_voltages_phase" + phase_name + ".csv";
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

        return std::polar(v_mag, v_ang_deg * PI / 180.0);
    }
};

// ###################################
// IEEE118App Implementation
// ###################################

ieee_118::IEEE118App::IEEE118App()
    : m_state(std::make_unique<ieee_118::IEEE118App::State>()), m_config_file(""), m_bus_ids(), m_r(0.0, 0.0),
      m_log("three_phase_timimng.log")
{
}

// At this point the inner State class has been defined, so we can default delete.
ieee_118::IEEE118App::~IEEE118App() = default;

bool ieee_118::IEEE118App::Initialize(const std::string &config_file, const std::vector<int> &bus_ids,
                                      const std::complex<double> &r)
{
    m_config_file = config_file;
    m_bus_ids = bus_ids;
    m_r = r;

    bool success = m_state->InitializeConfig(m_config_file);
    if (!success)
    {
        m_log << "Could not initialize pf state with config file: " << m_config_file << std::endl;
        return success;
    }

    success = m_state->InitializeBusIndeces(m_bus_ids);
    if (!success)
    {
        std::stringstream out;
        out << "Could not initialize bus indeces with the following ids:\n";
        for (int bus_ids : m_bus_ids)
        {
            out << bus_ids << " ";
        }
        out << "/n";
        m_log << out.str();
        return success;
    }

    m_state->InitializeFactoryAndFields();

    return success;
}

powerflow::tools::ThreePhaseValues
ieee_118::IEEE118App::ComputeVoltage(const powerflow::tools::ThreePhaseValues &power_s, int bus_id)
{
    powerflow::tools::ThreePhaseValues phased_voltage;

    std::stringstream out;
    out << "####################################\n";
    out << "Bus Id: " << bus_id << "\n";
    out << "Power A: " << power_s.a << "\n";
    out << "Power B: " << power_s.b << "\n";
    out << "Power C: " << power_s.c << "\n";

    utils::Stopwatch watch;
    watch.Start();
    phased_voltage.a = m_state->ComputeVoltageCurrent(m_config_file, bus_id, "A", power_s.a);
    long long time_a = watch.ElapsedMilliseconds();
    out << "Time A: " << time_a << " ms\n";

    watch.Start();
    phased_voltage.b = m_state->ComputeVoltageCurrent(m_config_file, bus_id, "B", power_s.b) * m_r;
    long long time_b = watch.ElapsedMilliseconds();
    out << "Time B: " << time_b << " ms\n";

    watch.Start();
    phased_voltage.c = m_state->ComputeVoltageCurrent(m_config_file, bus_id, "C", power_s.c) * m_r * m_r;
    long long time_c = watch.ElapsedMilliseconds();
    out << "Time C: " << time_c << " ms\n";

    out << "####################################\n\n";

    m_log << out.str();

    return phased_voltage;
}