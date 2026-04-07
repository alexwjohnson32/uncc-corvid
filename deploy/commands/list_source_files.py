import pathlib

import plugins.manager

def get_source_files(manager: plugins.manager.PluginManager, plugin_name: str, install_dir: pathlib.Path) -> str:
    if not install_dir.exists():
        raise ValueError(f"Given install directory does not exist: '{str(install_dir)}'")

    plugin = manager.get(plugin_name)
    if not plugin:
        raise KeyError(f"Could not find plugin with name '{plugin_name}'")

    source_files = plugin.get_baseline_files(str(install_dir))
    if not source_files:
        return ""
    else:
        # We need to tab the first object as well
        return "\t" + "\n\t".join(source_files)