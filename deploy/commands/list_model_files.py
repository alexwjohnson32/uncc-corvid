import pathlib

import plugins.manager

def get_model_files(manager: plugins.manager.PluginManager, model_name: str, deploy_dir: pathlib.Path, plugin_name: str = "") -> str:
    """:raises: ValueError if the Deploy Root does not exist.
                KeyError if a plugin name is given and the plugin does not exist"""
    if not deploy_dir.exists():
        raise ValueError(f"Given deploy directory does not exist: '{str(deploy_dir)}'")

    model_files = _get_model_files_list(manager, model_name, deploy_dir, plugin_name)

    if model_files:
        # We need to tab the first object as well
        return "\t" + "\n\t".join(model_files)
    else:
        output = ""
        if plugin_name:
            output = f"Could not find '{model_name}' with given plugin '{plugin_name}' with deploy root '{str(deploy_dir)}'!"
            output = f"{output}\nThe model name '{model_name}' may still exist within this deploy root with a different plugin."
        else:
            output = f"Could not find '{model_name}' within deploy root '{str(deploy_dir)}'!"
        return output

def _get_model_files_list(manager: plugins.manager.PluginManager, model_name: str, deploy_dir: pathlib.Path, plugin_name: str) -> list[str]:
    """:raises: KeyError if a plugin name is provided and no plugin with that name exists"""
    model_files = list[str]()

    if not plugin_name:
        for name in manager.get_plugin_names():
            plugin = manager.get(name)
            if plugin is None: continue # Simply to suppress warnings

            model_files = plugin.get_model_files(model_name, str(deploy_dir))
            if not model_files:
                continue # Did not find it here, keep looking
            else:
                break # Found it
    else:
        plugin = manager.get(plugin_name)
        if not plugin:
            raise KeyError(f"Could not find plugin with name '{plugin_name}'")

        model_files = plugin.get_model_files(model_name, str(deploy_dir))

    return model_files