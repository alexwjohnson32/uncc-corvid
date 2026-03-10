import importlib
import pkgutil
import inspect
from . import interface
import pathlib

class PluginManager:
    def __init__(self, plugin_directory: pathlib.Path):
        self._plugins = dict[str, interface.IDeployable]()

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
                    self._plugins[obj().get_name()] = obj()

        if not self._plugins:
            raise ValueError(f"Plugin directory contains no plugins: {plugin_directory}")


    def get(self, name: str) -> interface.IDeployable | None:
        if name in self._plugins:
            return self._plugins[name]
        else:
            return None

    def get_plugin_names(self) -> list[str]:
        return list(self._plugins.keys())