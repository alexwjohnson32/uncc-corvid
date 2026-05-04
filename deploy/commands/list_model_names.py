import pathlib
import plugins.manager
import plugins.interface

def get_model_names(manager: plugins.manager.PluginManager, deploy_dir: pathlib.Path, plugin_name: str = ""):
    if not deploy_dir.exists():
        raise ValueError(f"Given deploy directory does not exist: '{str(deploy_dir)}'")

    plugin_names = manager.get_plugin_names()
    # if plugin_name is not empty and does not exist in the list, raise exception
    if plugin_name:
        if plugin_name not in plugin_names:
            raise KeyError(f"Could not find plugin with name '{plugin_name}'")
        else:
            plugin_names = [plugin_name]

    model_names = list[str]()
    for plugin in [manager.get(name) for name in plugin_names]:
        # this should not be None, but this is to silence warnings
        if not plugin:
            continue
        model_names.append(_stringify_model_names(plugin, deploy_dir))

    return "\n".join(model_names)

def _stringify_model_names(plugin: plugins.interface.IDeployable, deploy_dir: pathlib.Path) -> str:
    return f"{plugin.get_name()}:\n\t" + "\n\t".join(plugin.list_model_names(str(deploy_dir)))