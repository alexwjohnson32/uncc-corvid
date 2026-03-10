import plugins.manager

def print_plugin_names(manager: plugins.manager.PluginManager) -> None:
    print("Printing IDeployable Plugin Name:")
    for name in manager.get_plugin_names():
        print(f"\t{name}")