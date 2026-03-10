import unittest
import unittest.mock as mock
import pathlib
import typing

import plugins.manager as manager
import plugins.interface as interface

class TestInterfaceImplementation(interface.IDeployable):
    def deploy(self, json_config: dict, deploy_root: str, install_root: str):
        pass

    def get_baseline_files(self, install_root: str) -> list[str]:
        return [str(pathlib.Path(install_root, "test"))]

    def get_model_files(self, model_name: str, deploy_root: str) -> list[str]:
        return [str(pathlib.Path(deploy_root, model_name))]

    def get_exec_json(self, model_name: str, deploy_root: str) -> typing.Dict[str, typing.Any]:
        return {
            "directory": ".",
            "exec": "./test",
            "host": "localhost",
            "name": "test"
        }

    def get_name(self) -> str:
        return "test/interface"

class TestPluginManager(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        print("-" * 40)
        print(f"\nStarting {cls.__name__}")
        # Start the patches
        cls.iter_patcher = mock.patch("pkgutil.iter_modules")
        cls.import_patcher = mock.patch("importlib.import_module")
        cls.mock_iter = cls.iter_patcher.start()
        cls.mock_import = cls.import_patcher.start()

        # Launch the test plugin
        cls.TestPluginClass = TestInterfaceImplementation

        # Configure the mocks
        cls.mock_iter.return_value = [(None, "test_mod", False)]
        mock_module = mock.MagicMock()
        mock_module.TestPlugin = cls.TestPluginClass
        cls.mock_import.return_value = mock_module

        # Mock the path object
        mock_path = mock.MagicMock(spec=pathlib.Path)
        mock_path.exists.return_value = True
        mock_path.__str__.return_value = "plugins"

        # Instantiate the manager once
        cls.manager = manager.PluginManager(mock_path)

    @classmethod
    def tearDownClass(cls) -> None:
        cls.iter_patcher.stop()
        cls.import_patcher.stop()

    def test_get_plugin_names(self):
        actual_names = self.manager.get_plugin_names()
        expected_names = ["test/interface"]

        self.assertListEqual(expected_names, actual_names)

    def test_get_name_exists(self):
        plugin = self.manager.get("test/interface")

        self.assertIsNotNone(plugin)
        self.assertIsInstance(plugin, interface.IDeployable)

    def test_get_name_does_not_exist(self):
        plugin = self.manager.get("non_name")

        self.assertIsNone(plugin)