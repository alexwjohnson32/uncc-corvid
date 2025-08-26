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
    IModel(const std::string &instance_name, const std::string &deploy_directory) : m_instance_name(instance_name)
    {
        std::filesystem::path base =
            deploy_directory.empty() ? std::filesystem::current_path() : std::filesystem::path(deploy_directory);

        // Don’t throw if parts don’t exist yet
        std::error_code ec;
        std::filesystem::path normalized = std::filesystem::weakly_canonical(base, ec);
        if (ec) normalized = std::filesystem::absolute(base);

        m_deploy_directory = normalized;
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
    GridPack118BusModel() : IModel("T1", "") {}
    GridPack118BusModel(const std::string &deploy_directory)
        : IModel("T1", (std::filesystem::path(deploy_directory) / "transmission" / "IEEE-118").string())
    {
    }

    virtual std::string GetExecString() const override;
    virtual std::filesystem::path GetExecutableDirectory() const override;

    virtual bool DeployExecutables() const override;
    virtual bool DeployResources() const override;
};

class GridLabD8500NodeModel : public IModel
{
  public:
    GridLabD8500NodeModel() : IModel("T1-D1", "") {}
    GridLabD8500NodeModel(const std::string &deploy_directory)
        : IModel("T1-D1", (std::filesystem::path(deploy_directory) / "distribution" / "IEEE_8500").string())
    {
    }

    virtual std::string GetExecString() const override;
    virtual std::filesystem::path GetExecutableDirectory() const override;

    virtual bool DeployExecutables() const override;
    virtual bool DeployResources() const override;
};

} // namespace orchestrator