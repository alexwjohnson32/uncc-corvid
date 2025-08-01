#include "model.hpp"

#include <iostream>

#include <boost/filesystem.hpp>
#include <boost/filesystem/operations.hpp>

namespace fs = boost::filesystem;

namespace
{

void PrintError(const fs::filesystem_error &err, const fs::path &source_path, const fs::path &destination_path)
{
    std::cout << "Error copying files: '" << err.what() << "'" << std::endl;
    std::cout << "CWD: '" << fs::current_path() << "'" << std::endl;
    std::cout << "Source: '" << source_path.generic_string() << "', exists: " << fs::exists(source_path) << std::endl;
    std::cout << "Destination: '" << destination_path.generic_string() << "', exists: " << fs::exists(destination_path)
              << std::endl;
}

void PrintSuccess(const fs::path &source_path, const fs::path &destination_path)
{
    std::cout << "Copied '" << source_path.generic_path() << "' to '" << destination_path.generic_path() << "'!"
              << std::endl;
}

} // namespace

std::string orchestrator::GridPack118BusModel::GetExecString() const
{
    fs::path relative_dir(".");
    relative_dir /= "..";
    relative_dir /= "exec";
    relative_dir /= "powerflow_ex.x";

    return relative_dir.generic_string();
}

std::string orchestrator::GridPack118BusModel::GetExecutableDirectory() const
{
    fs::path relative_dir(m_instance_name);
    relative_dir /= "IEEE-118";
    relative_dir /= "resources";

    return relative_dir.generic_string();
}

bool orchestrator::GridPack118BusModel::DeployExecutables(const std::string &deploy_root_dir) const
{
    fs::path source_exec("bin");
    source_exec /= "gridpack";
    source_exec /= "IEEE-118";
    source_exec /= "powerflow_ex.x";

    fs::path destination_exec(deploy_root_dir);
    destination_exec /= m_instance_name;
    destination_exec /= "IEEE-118";
    destination_exec /= "exec";
    destination_exec /= "powerflow_ex.x";

    try
    {
        fs::create_directories(destination_exec.parent_path());
        fs::copy_file(source_exec, destination_exec, fs::copy_option::overwrite_if_exists);
        PrintSuccess(source_exec, destination_exec);
    }
    catch (const fs::filesystem_error &err)
    {
        PrintError(err, source_exec, destination_exec);
        return false;
    }

    return true;
}

bool orchestrator::GridPack118BusModel::DeployResources(const std::string &deploy_root_dir) const
{
    fs::path source_dir("bin");
    source_dir /= "gridpack";
    source_dir /= "IEEE-118";

    fs::path source_xml_path(source_dir);
    source_xml_path /= "118.xml";

    fs::path source_raw_path(source_dir);
    source_raw_path /= "118.raw";

    fs::path destination_dir(deploy_root_dir);
    destination_dir /= m_instance_name;
    destination_dir /= "IEEE-118";
    destination_dir /= "resources";

    fs::path destination_xml_path(destination_dir);
    destination_xml_path /= "118.xml";

    fs::path destination_raw_path(destination_dir);
    destination_raw_path /= "118.raw";

    try
    {
        fs::create_directories(destination_xml_path.parent_path());
        fs::copy_file(source_xml_path, destination_xml_path, fs::copy_option::overwrite_if_exists);
        PrintSuccess(source_xml_path, destination_xml_path);
    }
    catch (const fs::filesystem_error &err)
    {
        PrintError(err, source_xml_path, destination_xml_path);
        return false;
    }

    try
    {
        fs::create_directories(destination_raw_path.parent_path());
        fs::copy_file(source_raw_path, destination_raw_path, fs::copy_option::overwrite_if_exists);
        PrintSuccess(source_raw_path, destination_raw_path);
    }
    catch (const fs::filesystem_error &err)
    {
        PrintError(err, source_raw_path, destination_raw_path);
        return false;
    }

    return true;
}

std::string orchestrator::GridLabD8500NodeModel::GetExecString() const { return "gridlabd.sh IEEE_8500node.glm"; }

std::string orchestrator::GridLabD8500NodeModel::GetExecutableDirectory() const
{
    fs::path relative_dir(m_instance_name);
    relative_dir /= "IEEE_8500";
    relative_dir /= "resources";

    return relative_dir.generic_string();
}

bool orchestrator::GridLabD8500NodeModel::DeployExecutables(const std::string &deploy_root_dir) const
{
    fs::path source_exec("bin");
    source_exec /= "gridlabd";
    source_exec /= "IEEE_8500node.glm";

    fs::path destination_exec(deploy_root_dir);
    destination_exec /= m_instance_name;
    destination_exec /= "IEEE_8500";
    destination_exec /= "resources";
    destination_exec /= "IEEE_8500node.glm";

    try
    {
        fs::create_directories(destination_exec.parent_path());
        fs::copy_file(source_exec, destination_exec, fs::copy_option::overwrite_if_exists);
        PrintSuccess(source_exec, destination_exec);
    }
    catch (const fs::filesystem_error &err)
    {
        PrintError(err, source_exec, destination_exec);
        return false;
    }

    return true;
}

bool orchestrator::GridLabD8500NodeModel::DeployResources(const std::string &deploy_root_dir) const
{
    fs::path source_json("bin");
    source_json /= "gridlabd";
    source_json /= "json";
    source_json /= "IEEE_8500node.json";

    fs::path destination_json(deploy_root_dir);
    destination_json /= m_instance_name;
    destination_json /= "IEEE_8500";
    destination_json /= "resources";
    destination_json /= "json";
    destination_json /= "IEEE_8500node.json";

    try
    {
        fs::create_directories(destination_json.parent_path());
        fs::copy_file(source_json, destination_json, fs::copy_option::overwrite_if_exists);
        PrintSuccess(source_json, destination_json);
    }
    catch (const fs::filesystem_error &err)
    {
        PrintError(err, source_json, destination_json);
        return false;
    }

    return true;
}