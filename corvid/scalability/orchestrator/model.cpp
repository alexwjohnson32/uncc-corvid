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

std::string orchestrator::GridPack118BusModel::GetExecString() const
{
    // Binary sits directly in the instance folder
    std::filesystem::path exec_path(m_deploy_directory);
    exec_path /= "powerflow_ex.x";
    return exec_path.string();

    // std::filesystem::path helics_setup_path(m_deploy_directory);
    // helics_setup_path /= "resources";
    // helics_setup_path /= "helics_setup.json";

    // std::stringstream exec_string;
    // exec_string << exec_path << " " << helics_setup_path;

    // return exec_string.str();
}

std::filesystem::path orchestrator::GridPack118BusModel::GetExecutableDirectory() const
{
    // Working dir is the instance folder (xml/raw/json live here too)
    return m_deploy_directory;
}

bool orchestrator::GridPack118BusModel::DeployExecutables() const
{
    std::filesystem::path source_exec("bin");
    source_exec /= "gridpack";
    source_exec /= "IEEE-118";
    source_exec /= "powerflow_ex.x";

    std::filesystem::path destination_exec(m_deploy_directory);
    destination_exec /= "powerflow_ex.x";

    try
    {
        std::filesystem::create_directories(destination_exec.parent_path());
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
    std::filesystem::path source_dir("bin");
    source_dir /= "gridpack";
    source_dir /= "IEEE-118";

    std::filesystem::path source_xml_path(source_dir);
    source_xml_path /= "118.xml";

    std::filesystem::path source_raw_path(source_dir);
    source_raw_path /= "118.raw";

    std::filesystem::path source_json_path(source_dir);
    source_json_path /= "helics_setup.json";

    std::filesystem::path destination_xml_path(m_deploy_directory);
    destination_xml_path /= "118.xml";

    std::filesystem::path destination_raw_path(m_deploy_directory);
    destination_raw_path /= "118.raw";

    std::filesystem::path destination_json_path(m_deploy_directory);
    destination_json_path /= "helics_setup.json";

    try
    {
        std::filesystem::create_directories(destination_xml_path.parent_path());
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
        std::filesystem::create_directories(destination_raw_path.parent_path());
        std::filesystem::copy_file(source_raw_path, destination_raw_path,
                                   std::filesystem::copy_options::overwrite_existing);
        PrintSuccess(source_raw_path, destination_raw_path);
    }
    catch (const std::filesystem::filesystem_error &err)
    {
        PrintError(err, source_raw_path, destination_raw_path);
        return false;
    }

    try
    {
        std::filesystem::create_directories(destination_json_path.parent_path());
        std::filesystem::copy_file(source_json_path, destination_json_path,
                                   std::filesystem::copy_options::overwrite_existing);
        PrintSuccess(source_json_path, destination_json_path);
    }
    catch (const std::filesystem::filesystem_error &err)
    {
        PrintError(err, source_json_path, destination_json_path);
        return false;
    }

    return true;
}

std::string orchestrator::GridLabD8500NodeModel::GetExecString() const
{
    std::filesystem::path glm_path(m_deploy_directory);
    glm_path /= m_instance_name + ".glm";

    return "gridlabd.sh " + glm_path.string();
}

std::filesystem::path orchestrator::GridLabD8500NodeModel::GetExecutableDirectory() const
{
    std::filesystem::path exec_dir(m_deploy_directory);

    return exec_dir;
}

bool orchestrator::GridLabD8500NodeModel::DeployExecutables() const
{
    // Sources
    const std::filesystem::path source_baseline = std::filesystem::path("bin") / "gridlabd" / "baseline_IEEE_8500.glm";

    // Destinations
    const std::filesystem::path model_level_dir = m_deploy_directory.parent_path(); // .../distribution/IEEE_8500
    const std::filesystem::path dest_baseline = model_level_dir / "baseline_IEEE_8500.glm";
    const std::filesystem::path dest_instance =
        m_deploy_directory / (m_instance_name + ".glm"); // .../<instance>/<m_instance_name>.glm

    try
    {
        // Ensure directories exist
        std::filesystem::create_directories(model_level_dir);
        std::filesystem::create_directories(m_deploy_directory);

        // Copy baseline to model level
        std::filesystem::copy_file(source_baseline, dest_baseline, std::filesystem::copy_options::overwrite_existing);
        PrintSuccess(source_baseline, dest_baseline);

        // Generate the per-federate .glm in the federate folder
        // Contents reference baseline via a relative path, and use m_instance_name for both fields.
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
    std::filesystem::path source_json("bin");
    source_json /= "gridlabd";
    source_json /= "json";
    source_json /= "IEEE_8500node.json";

    std::filesystem::path destination_json(m_deploy_directory);
    destination_json /= (m_instance_name + ".json");

    try
    {
        std::filesystem::create_directories(destination_json.parent_path());
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