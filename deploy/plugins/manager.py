import importlib
import inspect
import pathlib
import sys

from . import interface

class PluginManager:
    def _get_module_name(self, file_path: pathlib.Path, plugin_root: pathlib.Path) -> str:
        relative_path = file_path.relative_to(plugin_root.parent)
        module_parts = list(relative_path.parts)
        module_parts[-1] = file_path.stem # remove *.py
        return ".".join(module_parts)

    def _attempt_store(self, file_path: pathlib.Path, plugin_root: pathlib.Path):
        try:
            # Create a full dotted module name
            module_name = self._get_module_name(file_path, plugin_root)
            # print(f"\nPluginManager->_attempt_store->module_name: {module_name}")

            # Import the module
            # Using import_module ensures __package__ and parent hierarchy
            # are handled correctly by Python's internal machinery.
            module = importlib.import_module(module_name)

            # inspect the module for classes
            for _, obj in inspect.getmembers(module, inspect.isclass):
                if (
                    issubclass(obj, interface.IDeployable)
                    and obj is not interface.IDeployable
                    and not inspect.isabstract(obj)
                    and obj.__module__ == module_name
                ):
                    # Instantiate and store
                    instance = obj()
                    self._plugins[instance.get_name()] = instance

        except Exception as e:
            print(f"Error loading plugin from {file_path}: {e}")

    def __init__(self, plugin_directory: pathlib.Path):
        self._plugins = dict[str, interface.IDeployable]()

        plugin_root = plugin_directory.resolve()
        # print(f"\nPluginManager->__init__->plugin_root: {plugin_root}")
        if not plugin_root.exists():
            raise ValueError(f"Plugin Directory does not exist: {plugin_root}")
        elif not plugin_root.is_dir():
            raise ValueError(f"Plugin Directory is not a directory: {plugin_root}")

        # The parent directory must be in sys.path for dotted imports to work
        parent_dir = str(plugin_root.parent)
        # print(f"\nPluginManager->__init__->parent_dir: {parent_dir}")
        if parent_dir not in sys.path:
            sys.path.insert(0, parent_dir)
        # print(f"\nPluginManager->__init__->sys.path: {sys.path}")

        try:
            # rglob("*.py") handles the recursion automatically
            for file_path in plugin_root.rglob("*.py"):
                # Skip __init__.py files and hidden files
                if file_path.name == "__init__.py" or file_path.name.startswith("."):
                    continue
                else:
                    self._attempt_store(file_path, plugin_root)
        finally:
            # Clean up sys.path
            if parent_dir in sys.path:
                sys.path.remove(parent_dir)
            # print(f"\nPluginManager->__init__->sys.path: {sys.path}")

    def get(self, name: str) -> interface.IDeployable | None:
        # print(f"\nplugin_manager->get({name})")
        keys_string = ', '.join(str(key) for key in self._plugins)
        # print(f"\nplugin_manager->_plugin->get_keys(): '{keys_string}'")
        if name in self._plugins:
            # print(f"\nplugin_manager->get({name}): Key Found!")
            return self._plugins[name]
        else:
            # print(f"\nplugin_manager->get({name}): Key Not Found!")
            return None

    def get_plugin_names(self) -> list[str]:
        keys_list = list(self._plugins.keys())
        # print(f"\nplugin_manager->get_plugin_names() keys: '{keys_list}'")
        return keys_list