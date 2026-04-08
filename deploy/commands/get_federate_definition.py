import json
import pathlib

import plugins.manager

def get_federate_definition(manager: plugins.manager.PluginManager, model_name: str, deploy_root: pathlib.Path, plugin_name: str = "") -> str:
    """:raises: ValueError if the Deploy Root does not exist.
                KeyError if a plugin name is given and the plugin does not exist"""
    if not deploy_root.exists():
        raise ValueError(f"Given Deploy Root '{str(deploy_root)}' does not exist!")

    json_dict = _get_json_dict(manager, model_name, deploy_root, plugin_name)

    if json_dict:
        return json.dumps(json_dict, indent=4)
    else:
        output = ""
        if plugin_name:
            output = f"Could not find '{model_name}' with given plugin '{plugin_name}' with deploy root '{str(deploy_root)}'!"
            output = f"{output}\nThe model name '{model_name}' may still exist within this deploy root with a different plugin."
        else:
            output = f"Could not find '{model_name}' within deploy root '{str(deploy_root)}'!"
        return output

def _get_json_dict(manager: plugins.manager.PluginManager, model_name: str, deploy_root: pathlib.Path, plugin_name: str = "") -> dict[str, str]:
    """:raises: KeyError if a plugin name is provided and no plugin with that name exists."""
    json_dict = dict[str, str]()

    if not plugin_name:
        for name in manager.get_plugin_names():
            plugin = manager.get(name)
            if plugin is None: continue # Simply to suppress warnings

            json_dict = plugin.get_exec_json(model_name, str(deploy_root))
            if not json_dict:
                # Did not find model in this plugin, skip
                continue
            else:
                # Found it, break the loop
                break
    else:
        plugin = manager.get(plugin_name)
        if plugin is not None:
            json_dict = plugin.get_exec_json(model_name, str(deploy_root))
        else:
            raise KeyError(f"Given Plugin Name '{plugin_name}' does not exist!")

    return json_dict

