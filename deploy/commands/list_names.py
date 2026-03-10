import plugins.manager

def get_plugin_names(manager: plugins.manager.PluginManager) -> list[str]:
    return manager.get_plugin_names()