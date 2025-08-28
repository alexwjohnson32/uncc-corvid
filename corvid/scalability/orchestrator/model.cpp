#include "model.hpp"

#include <iostream>
#include <sstream>
#include <fstream>

namespace
{

void PrintError(const std::filesystem::filesystem_error &err, const std::filesystem::path &source_path,
                const std::filesystem::path &destination_path)
{
    std::cout << "Error copying files: '" << err.what() << "'" << std::endl;
    std::cout << "CWD: '" << std::filesystem::current_path() << "'" << std::endl;
    std::cout << "Source: '" << source_path << "', exists: " << std::filesystem::exists(source_path) << std::endl;
    std::cout << "Destination: '" << destination_path << "', exists: " << std::filesystem::exists(destination_path)
              << std::endl;
}

void PrintSuccess(const std::filesystem::path &source_path, const std::filesystem::path &destination_path)
{
    std::cout << "Copied '" << source_path << "' to '" << destination_path << "'!" << std::endl;
}

} // namespace

// -------------------- GridPACK (IEEE-118) --------------------
std::string orchestrator::GridPack118BusModel::GetExecString() const
{
    return std::string("/bin/sh -c './powerflow_ex.x helics_setup.json'");
}

std::filesystem::path orchestrator::GridPack118BusModel::GetExecutableDirectory() const
{
    // Working dir is the instance folder (xml/raw/json live here too)
    return m_deploy_directory;
}

bool orchestrator::GridPack118BusModel::DeployExecutables() const
{
    if (!m_allow_create_dirs)
    {
        std::cout << "[skip] " << m_instance_name << ": deploy executables disabled by flag.\n";
        return true;
    }

    std::filesystem::path source_exec("bin");
    source_exec /= "gridpack";
    source_exec /= "IEEE-118";
    source_exec /= "powerflow_ex.x";

    std::filesystem::path destination_exec(m_deploy_directory);
    destination_exec /= "powerflow_ex.x";

    try
    {
        if (m_allow_create_dirs)
        {
            std::filesystem::create_directories(destination_exec.parent_path());
        }
        else if (!std::filesystem::exists(destination_exec.parent_path()))
        {
            throw std::filesystem::filesystem_error("Destination directory missing and creation disabled",
                                                    destination_exec.parent_path(),
                                                    std::make_error_code(std::errc::no_such_file_or_directory));
        }
        std::filesystem::copy_file(source_exec, destination_exec, std::filesystem::copy_options::overwrite_existing);
        PrintSuccess(source_exec, destination_exec);
    }
    catch (const std::filesystem::filesystem_error &err)
    {
        PrintError(err, source_exec, destination_exec);
        return false;
    }

    return true;
}

bool orchestrator::GridPack118BusModel::DeployResources() const
{
    if (!m_allow_create_dirs)
    {
        std::cout << "[skip] " << m_instance_name << ": deploy resources disabled by flag.\n";
        return true;
    }

    std::filesystem::path source_dir("bin");
    source_dir /= "gridpack";
    source_dir /= "IEEE-118";

    std::filesystem::path source_xml_path(source_dir);
    source_xml_path /= "118.xml";

    std::filesystem::path source_raw_path(source_dir);
    source_raw_path /= "118.raw";

    std::filesystem::path destination_xml_path(m_deploy_directory);
    destination_xml_path /= "118.xml";

    std::filesystem::path destination_raw_path(m_deploy_directory);
    destination_raw_path /= "118.raw";

    std::filesystem::path destination_json_path(m_deploy_directory);
    destination_json_path /= "helics_setup.json";

    try
    {
        if (m_allow_create_dirs)
        {
            std::filesystem::create_directories(destination_xml_path.parent_path());
        }
        else if (!std::filesystem::exists(destination_xml_path.parent_path()))
        {
            throw std::filesystem::filesystem_error("Destination directory missing and creation disabled",
                                                    destination_xml_path.parent_path(),
                                                    std::make_error_code(std::errc::no_such_file_or_directory));
        }
        std::filesystem::copy_file(source_xml_path, destination_xml_path,
                                   std::filesystem::copy_options::overwrite_existing);
        PrintSuccess(source_xml_path, destination_xml_path);
    }
    catch (const std::filesystem::filesystem_error &err)
    {
        PrintError(err, source_xml_path, destination_xml_path);
        return false;
    }

    try
    {
        if (m_allow_create_dirs)
        {
            std::filesystem::create_directories(destination_raw_path.parent_path());
        }
        else if (!std::filesystem::exists(destination_raw_path.parent_path()))
        {
            throw std::filesystem::filesystem_error("Destination directory missing and creation disabled",
                                                    destination_raw_path.parent_path(),
                                                    std::make_error_code(std::errc::no_such_file_or_directory));
        }
        std::filesystem::copy_file(source_raw_path, destination_raw_path,
                                   std::filesystem::copy_options::overwrite_existing);
        PrintSuccess(source_raw_path, destination_raw_path);
    }
    catch (const std::filesystem::filesystem_error &err)
    {
        PrintError(err, source_raw_path, destination_raw_path);
        return false;
    }

    // generate helics_setup.json here (instead of copying)
    try
    {
        if (m_allow_create_dirs)
        {
            std::filesystem::create_directories(destination_json_path.parent_path());
        }
        else if (!std::filesystem::exists(destination_json_path.parent_path()))
        {
            throw std::filesystem::filesystem_error("Destination directory missing and creation disabled",
                                                    destination_json_path.parent_path(),
                                                    std::make_error_code(std::errc::no_such_file_or_directory));
        }

        const std::string gld_conn_name = m_instance_name + "-D1_conn";

        std::ostringstream json;
        json << "{\n"
             << "  \"gridpack_name\": \"gridpack\",\n"
             << "  \"gridlabd_infos\": [\n"
             << "    { \"name\": \"" << gld_conn_name << "\", \"bus_id\": 2 }\n"
             << "  ],\n"
             << "  \"total_time\": 60.0,\n"
             << "  \"ln_magnitude\": 79600.0\n"
             << "}\n";

        {
            std::ofstream ofs(destination_json_path, std::ios::out | std::ios::trunc);
            if (!ofs)
                throw std::filesystem::filesystem_error("Failed to open helics_setup.json for write",
                                                        destination_json_path,
                                                        std::make_error_code(std::errc::io_error));
            ofs << json.str();
            ofs.flush();
            if (!ofs)
                throw std::filesystem::filesystem_error("Failed to write helics_setup.json", destination_json_path,
                                                        std::make_error_code(std::errc::io_error));
        }
        std::cout << "Generated '" << destination_json_path << "'!\n";
    }
    catch (const std::filesystem::filesystem_error &err)
    {
        // We pass a synthetic "<generated>" as the "source" for uniform logging
        PrintError(err, std::filesystem::path("<generated>"), destination_json_path);
        return false;
    }

    return true;
}

// -------------------- GridLAB-D (IEEE 8500) --------------------
std::string orchestrator::GridLabD8500NodeModel::GetExecString() const
{
    return "gridlabd.sh " + m_instance_name + ".glm";
}

std::filesystem::path orchestrator::GridLabD8500NodeModel::GetExecutableDirectory() const { return m_deploy_directory; }

bool orchestrator::GridLabD8500NodeModel::DeployExecutables() const
{
    if (!m_allow_create_dirs)
    {
        std::cout << "[skip] " << m_instance_name << ": deploy executables disabled by flag.\n";
        return true;
    }

    // Sources
    const std::filesystem::path source_baseline = std::filesystem::path("bin") / "gridlabd" / "baseline_IEEE_8500.glm";

    // Destinations
    const std::filesystem::path model_level_dir = m_deploy_directory.parent_path(); // .../distribution/IEEE_8500
    const std::filesystem::path dest_baseline = model_level_dir / "baseline_IEEE_8500.glm";
    const std::filesystem::path dest_instance =
        m_deploy_directory / (m_instance_name + ".glm"); // .../<instance>/<m_instance_name>.glm

    try
    {
        // Ensure directories exist (or fail if creation is disabled)
        if (m_allow_create_dirs)
        {
            std::filesystem::create_directories(model_level_dir);
            std::filesystem::create_directories(m_deploy_directory);
        }
        else
        {
            if (!std::filesystem::exists(model_level_dir))
                throw std::filesystem::filesystem_error("Model-level directory missing and creation disabled",
                                                        model_level_dir,
                                                        std::make_error_code(std::errc::no_such_file_or_directory));
            if (!std::filesystem::exists(m_deploy_directory))
                throw std::filesystem::filesystem_error("Instance directory missing and creation disabled",
                                                        m_deploy_directory,
                                                        std::make_error_code(std::errc::no_such_file_or_directory));
        }

        // Copy baseline to model level
        std::filesystem::copy_file(source_baseline, dest_baseline, std::filesystem::copy_options::overwrite_existing);
        PrintSuccess(source_baseline, dest_baseline);

        // Generate the per-federate .glm in the federate folder
        const std::string instance_glm_contents = "#include \"../baseline_IEEE_8500.glm\"\n"
                                                  "\n"
                                                  "object helics_msg {\n"
                                                  "    name " +
                                                  m_instance_name +
                                                  "_conn;\n"
                                                  "    configure " +
                                                  m_instance_name +
                                                  ".json;\n"
                                                  "}\n";

        {
            std::ofstream ofs(dest_instance, std::ios::out | std::ios::trunc);
            if (!ofs)
                throw std::filesystem::filesystem_error("Failed to open instance glm for write", dest_instance,
                                                        std::make_error_code(std::errc::io_error));
            ofs << instance_glm_contents;
            ofs.flush();
            if (!ofs)
                throw std::filesystem::filesystem_error("Failed to write instance glm", dest_instance,
                                                        std::make_error_code(std::errc::io_error));
        }
        PrintSuccess(std::filesystem::path("<generated>"), dest_instance);
    }
    catch (const std::filesystem::filesystem_error &err)
    {
        // Report whichever copy/create failed
        PrintError(err, source_baseline, dest_baseline);
        return false;
    }

    return true;
}

bool orchestrator::GridLabD8500NodeModel::DeployResources() const
{
    if (!m_allow_create_dirs)
    {
        std::cout << "[skip] " << m_instance_name << ": deploy resources disabled by flag.\n";
        return true;
    }

    std::filesystem::path source_json("bin");
    source_json /= "gridlabd";
    source_json /= "json";
    source_json /= "IEEE_8500node.json";

    std::filesystem::path destination_json(m_deploy_directory);
    destination_json /= (m_instance_name + ".json");

    try
    {
        if (m_allow_create_dirs)
        {
            std::filesystem::create_directories(destination_json.parent_path());
        }
        else if (!std::filesystem::exists(destination_json.parent_path()))
        {
            throw std::filesystem::filesystem_error("Destination directory missing and creation disabled",
                                                    destination_json.parent_path(),
                                                    std::make_error_code(std::errc::no_such_file_or_directory));
        }
        std::filesystem::copy_file(source_json, destination_json, std::filesystem::copy_options::overwrite_existing);
        PrintSuccess(source_json, destination_json);
    }
    catch (const std::filesystem::filesystem_error &err)
    {
        PrintError(err, source_json, destination_json);
        return false;
    }

    return true;
}