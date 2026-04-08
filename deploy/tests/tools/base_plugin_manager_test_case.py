import unittest
import unittest.mock as mock
import pathlib
import typing

import plugins.manager
import plugins.interface

class MockPluginA(plugins.interface.IDeployable):
    def deploy(self, json_config: dict, deploy_root: str, install_root: str):
        pass

    def get_baseline_files(self, install_root: str) -> list[str]:
        return [str(pathlib.Path(install_root, "mock_plugin_a"))]

    def get_model_files(self, model_name: str, deploy_root: str) -> list[str]:
        return [str(pathlib.Path(deploy_root, model_name))]

    def get_exec_json(self, model_name: str, deploy_root: str) -> typing.Dict[str, typing.Any]:
        return {
            "directory": ".",
            "exec": "./mock_plugin_a",
            "host": "localhost",
            "name": "mock_plugin_a"
        }

    def get_name(self) -> str:
        return "mock_plugin_a/interface"

class MockPluginB(plugins.interface.IDeployable):
    def deploy(self, json_config: dict, deploy_root: str, install_root: str):
        pass

    def get_baseline_files(self, install_root: str) -> list[str]:
        return [str(pathlib.Path(install_root, "mock_plugin_b"))]

    def get_model_files(self, model_name: str, deploy_root: str) -> list[str]:
        return [str(pathlib.Path(deploy_root, model_name))]

    def get_exec_json(self, model_name: str, deploy_root: str) -> typing.Dict[str, typing.Any]:
        return {
            "directory": ".",
            "exec": "./mock_plugin_b",
            "host": "localhost",
            "name": "mock_plugin_b"
        }

    def get_name(self) -> str:
        return "mock_plugin_b/interface"

class BaseTestPluginManager(unittest.TestCase):
    manager: plugins.manager.PluginManager

    @classmethod
    def setUpClass(cls) -> None:
        # Mock the path object for rglob
        cls.plugin_root = pathlib.Path("/fake/plugins")

        # Create mock file objects that rglob will "find"
        mock_file_a = mock.MagicMock(spec=pathlib.Path)
        mock_file_a.name = "module_a.py"
        mock_file_a.stem = "module_a"
        mock_file_a.suffix = ".py"
        # This needs to match the logic in _get_module_name
        # relative_to(plugin_root.parent) -> plugins/module_a
        mock_file_a.relative_to.return_value = pathlib.Path("plugins/module_a.py")
        mock_file_a.parts = ("plugins", "module_a.py")

        mock_file_b = mock.MagicMock(spec=pathlib.Path)
        mock_file_b.name = "module_b.py"
        mock_file_b.stem = "module_b"
        mock_file_b.suffix = ".py"
        mock_file_b.relative_to.return_value = pathlib.Path("plugins/module_b.py")
        mock_file_b.parts = ("plugins", "module_b.py")

        # Mock the directory itself
        cls.mock_path = mock.MagicMock(spec=pathlib.Path)
        cls.mock_path.resolve.return_value = cls.mock_path
        cls.mock_path.exists.return_value = True
        cls.mock_path.is_dir.return_value = True
        cls.mock_path.parent = pathlib.Path("/fake")
        cls.mock_path.rglob.return_value = [mock_file_a, mock_file_b]

        # Update __module__ so the PluginManager doesn't filter them out
        MockPluginA.__module__ = "plugins.module_a"
        MockPluginB.__module__ = "plugins.module_b"

        # Setup Mock Modules
        cls.mock_module_a = mock.MagicMock()
        cls.mock_module_a.PluginA = MockPluginA

        cls.mock_module_b = mock.MagicMock()
        cls.mock_module_b.PluginB = MockPluginB

        # inspect.getmembers uses the __dict__ or dir()
        cls.mock_module_a_members = [("PluginA", MockPluginA)]
        cls.mock_module_b_members = [("PluginB", MockPluginB)]

        # Patching
        cls.import_patcher = mock.patch("importlib.import_module")
        cls.inspect_patcher = mock.patch("inspect.getmembers")

        cls.mock_import = cls.import_patcher.start()
        cls.mock_inspect = cls.inspect_patcher.start()

        def import_side_effect(name):
            if name == 'plugins.module_a':
                return cls.mock_module_a
            elif name == 'plugins.module_b':
                return cls.mock_module_b
            else:
                raise ImportError(f"Module {name} not found")

        def inspect_side_effect(module, predicate=None):
            if module == cls.mock_module_a:
                return cls.mock_module_a_members
            elif module == cls.mock_module_b:
                return cls.mock_module_b_members
            else:
                return []

        cls.mock_import.side_effect = import_side_effect
        cls.mock_inspect.side_effect = inspect_side_effect

        # Initialize the manager with our mocked path
        cls.manager = plugins.manager.PluginManager(cls.mock_path)

    @classmethod
    def tearDownClass(cls) -> None:
        cls.import_patcher.stop()
        cls.inspect_patcher.stop()