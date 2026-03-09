import importlib
import pkgutil
import inspect
import interface
import pathlib
import typing

class PluginManager:
    def _check_plugin_name(self, plugin_name: str) -> None:
        if not plugin_name in self.plugins:
            raise ValueError(f"Plugin Name '{plugin_name}' not recognized!")

    def __init__(self, plugin_directory: pathlib.Path):
        self.plugins = dict[str, interface.IDeployable]()

        if not plugin_directory.exists():
            raise ValueError(f"Plugin Directory does not exist: {plugin_directory}")

        # Discover modules in the folder
        for _, name, _ in pkgutil.iter_modules([plugin_directory]):
            # Dynamically import the module
            module = importlib.import_module(f"{plugin_directory}.{name}")

            # Look for classes that inherit from IPlugin
            for _, obj in inspect.getmembers(module):
                if (
                    inspect.isclass(obj)
                    and issubclass(obj, interface.IDeployable)
                    and obj != interface.IDeployable
                ):
                    # Set the plugins dict to the name and instance
                    self.plugins[obj().get_name()] = obj()

    def get_plugin_names(self) -> list[str]:
        return list(self.plugins.keys())

    def get_exec_info(self, plugin_name: str, model_name: str, deploy_root: pathlib.Path) -> typing.Dict[str, typing.Any]:
        self._check_plugin_name(plugin_name)
        return self.plugins[plugin_name].get_exec_json(model_name, str(deploy_root))

    def get_model_files(self, plugin_name: str, model_name: str, deploy_root: pathlib.Path) -> list[str]:
        self._check_plugin_name(plugin_name)
        return self.plugins[plugin_name].get_model_files(model_name, str(deploy_root))

    def get_baseline_files(self, plugin_name: str, install_root: pathlib.Path) -> list[str]:
        self._check_plugin_name(plugin_name)
        return self.plugins[plugin_name].get_baseline_files(str(install_root))

    def deploy(self, plugin_name: str, json_config: dict, deploy_root: pathlib.Path, install_root: pathlib.Path) -> None:
        self._check_plugin_name(plugin_name)
        return self.plugins[plugin_name].deploy(json_config, str(deploy_root), str(install_root))