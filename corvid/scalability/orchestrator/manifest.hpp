#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <optional>

namespace orchestrator
{

struct Model
{
    std::string model_path;
    std::string executable_name;
    std::vector<std::string> resource_paths;
};

class Manifest
{
  private:
    std::string m_root;
    std::unordered_map<std::string, orchestrator::Model> m_models;

  public:
    Manifest() : m_root(""), m_models() {}
    Manifest(std::string root) : m_root(root), m_models() {}

    bool HasModelName(const std::string &model_name) const { return m_models.count(model_name) == 1; }
    bool AddModel(const std::string &model_name, const orchestrator::Model &model)
    {
        return m_models.insert({ model_name, model }).second;
    }
    bool RemoveModel(const std::string &model_name) { return m_models.erase(model_name) == 1; }

    void SetRootPath(const std::string root_path) { m_root = root_path; }
    std::string GetRootPath() const { return m_root; }

    std::vector<std::string> GetModelNames() const
    {
        std::vector<std::string> keys;
        for (const auto &key_value : m_models)
        {
            keys.push_back(key_value.first);
        }
        return keys;
    }
    std::optional<orchestrator::Model> GetModel(const std::string &model_name) const
    {
        std::optional<orchestrator::Model> model;
        if (HasModelName(model_name))
        {
            model = m_models.at(model_name);
        }
        return model;
    }
};

} // namespace orchestrator