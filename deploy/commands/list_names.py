import plugins.manager

def get_plugin_names(manager: plugins.manager.PluginManager) -> str:
    names = manager.get_plugin_names()
    if not names:
        return ""
    else:
        # We need to tab the first object as well
        return "\t" + "\n\t".join(names)