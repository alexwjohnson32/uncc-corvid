#pragma once

#include <string>

namespace orchestrator
{

class IModel
{
  protected:
    std::string m_instance_name{};

  public:
    IModel(const std::string &instance_name) : m_instance_name(instance_name) {}

    std::string GetName() const { return m_instance_name; }
    std::string GetHost() const { return "localhost"; }

    virtual std::string GetExecString() const = 0;
    virtual std::string GetExecutableDirectory() const = 0;

    virtual bool DeployExecutables(const std::string &deploy_root_dir) const = 0;
    virtual bool DeployResources(const std::string &deploy_root_dir) const = 0;
};

class GridPack118BusModel : public IModel
{
  public:
    GridPack118BusModel() : IModel("gpk_fed") {}

    virtual std::string GetExecString() const override;
    virtual std::string GetExecutableDirectory() const override;

    virtual bool DeployExecutables(const std::string &deploy_root_dir) const override;
    virtual bool DeployResources(const std::string &deploy_root_dir) const override;
};

class GridLabD8500NodeModel : public IModel
{
  public:
    GridLabD8500NodeModel() : IModel("gld_fed") {}

    virtual std::string GetExecString() const override;
    virtual std::string GetExecutableDirectory() const override;

    virtual bool DeployExecutables(const std::string &deploy_root_dir) const override;
    virtual bool DeployResources(const std::string &deploy_root_dir) const override;
};

} // namespace orchestrator