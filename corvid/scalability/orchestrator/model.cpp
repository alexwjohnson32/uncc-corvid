#include "model.hpp"

#include <iostream>

#include <boost/filesystem.hpp>
#include <boost/filesystem/operations.hpp>

namespace fs = boost::filesystem;

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
    fs::path relative_dir("IEEE-118");
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
        fs::copy_file(source_exec, destination_exec, fs::copy_option::overwrite_if_exists);
    }
    catch (const fs::filesystem_error &err)
    {
        std::cout << err.what() << std::endl;
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
        fs::copy_file(source_xml_path, destination_xml_path, fs::copy_option::overwrite_if_exists);
        fs::copy_file(source_raw_path, destination_raw_path, fs::copy_option::overwrite_if_exists);
    }
    catch (const fs::filesystem_error &err)
    {
        std::cout << err.what() << std::endl;
        return false;
    }

    return true;
}

std::string orchestrator::GridLabD8500NodeModel::GetExecString() const { return "gridlabd.sh IEEE_8500node.glm"; }

std::string orchestrator::GridLabD8500NodeModel::GetExecutableDirectory() const
{
    fs::path relative_dir("IEEE_8500");
    relative_dir /= "resources";

    return relative_dir.generic_string();
}

bool orchestrator::GridLabD8500NodeModel::DeployExecutables(const std::string &deploy_root_dir) const
{
    fs::path source_exec("bin");
    source_exec /= "gridlab";
    source_exec /= "IEEE_8500node.glm";

    fs::path destination_exec(deploy_root_dir);
    destination_exec /= m_instance_name;
    destination_exec /= "IEEE_8500";
    destination_exec /= "resources";
    destination_exec /= "IEEE_8500node.glm";

    try
    {
        fs::copy_file(source_exec, destination_exec, fs::copy_option::overwrite_if_exists);
    }
    catch (const fs::filesystem_error &err)
    {
        std::cout << err.what() << std::endl;
        return false;
    }

    return true;
}

bool orchestrator::GridLabD8500NodeModel::DeployResources(const std::string &deploy_root_dir) const
{
    fs::path source_json("bin");
    source_json /= "gridlab";
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
        fs::copy_file(source_json, destination_json, fs::copy_option::overwrite_if_exists);
    }
    catch (const fs::filesystem_error &err)
    {
        std::cout << err.what() << std::endl;
        return false;
    }

    return true;
}