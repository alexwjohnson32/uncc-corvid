#include "pf_state.hpp"

#include <iostream>

namespace
{
enum class Parser
{
    UNSET,
    PTI23,
    PTI33
};
}

bool gridpack::powerflow::PFState::InitializeConfig(const std::string &config_file)
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

bool gridpack::powerflow::PFState::InitializeBusIndeces(const std::vector<int> &bus_ids)
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

void gridpack::powerflow::PFState::InitializeFactoryAndFields()
{
    // One time build
    network->partition();

    pf_factory = std::make_unique<gridpack::powerflow::PFFactory>(network);
    pf_factory->load();
    pf_factory->setComponents();
    pf_factory->setExchange();

    network->initBusUpdate();

    pf_factory->setYBus();
    pf_factory->setSBus();

    // Solver / maps
    pf_factory->setMode(RHS);
    v_map = std::make_unique<gridpack::mapper::BusVectorMap<gridpack::powerflow::PFNetwork>>(network);
    PQ = v_map->mapToVector();

    pf_factory->setMode(Jacobian);
    j_map = std::make_unique<gridpack::mapper::FullMatrixMap<gridpack::powerflow::PFNetwork>>(network);
    J = j_map->mapToMatrix();

    X.reset(PQ->clone());
    solver = std::make_unique<gridpack::math::LinearSolver>(*J);
    solver->configure(cursor);
}

int gridpack::powerflow::PFState::GetBusIndex(int bus_id) const
{
    int bus_index = -1;

    if (m_bus_indeces.count(bus_id))
    {
        bus_index = m_bus_indeces.at(bus_id);
    }

    return bus_index;
}