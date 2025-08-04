#pragma once

#include <string>
#include <filesystem>

namespace orchestrator
{

class IModel
{
  protected:
    std::string m_instance_name{};
    std::filesystem::path m_deploy_directory{};

  public:
    IModel(const std::string &instance_name, const std::string &deploy_directory)
        : m_instance_name(instance_name), m_deploy_directory(std::filesystem::canonical(deploy_directory))
    {
        m_deploy_directory /= m_instance_name;
    }

    std::string GetName() const { return m_instance_name; }
    std::string GetHost() const { return "localhost"; }
    std::filesystem::path GetDeployDir() const { return m_deploy_directory; }

    virtual std::string GetExecString() const = 0;
    virtual std::filesystem::path GetExecutableDirectory() const = 0;

    virtual bool DeployExecutables() const = 0;
    virtual bool DeployResources() const = 0;
};

class GridPack118BusModel : public IModel
{
  public:
    GridPack118BusModel() : IModel("gpk_fed", "") {}
    GridPack118BusModel(const std::string &deploy_directory) : IModel("gpk_fed", deploy_directory)
    {
        m_deploy_directory /= "IEEE-118";
    }

    virtual std::string GetExecString() const override;
    virtual std::filesystem::path GetExecutableDirectory() const override;

    virtual bool DeployExecutables() const override;
    virtual bool DeployResources() const override;
};

class GridLabD8500NodeModel : public IModel
{
  public:
    GridLabD8500NodeModel() : IModel("gld_fed", "") {}
    GridLabD8500NodeModel(const std::string &deploy_directory) : IModel("gld_fed", deploy_directory)
    {
        m_deploy_directory /= "IEEE_8500";
    }

    virtual std::string GetExecString() const override;
    virtual std::filesystem::path GetExecutableDirectory() const override;

    virtual bool DeployExecutables() const override;
    virtual bool DeployResources() const override;
};

} // namespace orchestrator