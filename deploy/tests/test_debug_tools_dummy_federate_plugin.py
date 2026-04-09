import pathlib
import json
import plugins.debug_tools.dummy_federate.dummy_federate_plugin as plugin
import tests.tools.base_plugin_test_case as base_test

def get_default_values() -> dict:
    values = dict()

    values["name"] = "test_name"
    values["local_log_file"] = "local_file.txt"
    values["total_time"] = 60.0
    values["core_type"] = "ipc"
    values["core_init"] = "--brokername=mainbroker"

    return values

class TestDummyFederatePlugin(base_test.BasePluginTestCase):
    @classmethod
    def _get_install_path(cls) -> pathlib.Path:
        return cls.install_dir / "debug_tools" / "dummy_federate"

    @classmethod
    def _get_safe_deployed_path(cls) -> pathlib.Path:
        return cls.deploy_dir / "debug_tools" / "dummy_federate" / cls.get_safe_model_name()

    @classmethod
    def _init_deployed_files(cls, deployed_path: pathlib.Path) -> None:
        with open(deployed_path / "dummy_federate", "w") as exec_file:
            exec_file.write("exec file")

        with open(deployed_path / "README.md", "w") as readme_file:
            readme_file.write("dummy")

        json_data = dict()
        default_values = get_default_values()
        json_data["federate_name"] = default_values["name"]
        json_data["local_log_file"] = default_values["local_log_file"]
        json_data["total_time"] = default_values["total_time"]
        json_data["fed_info_json"] = dict()
        json_data["fed_info_json"]["coreInit"] = default_values["core_init"]
        json_data["fed_info_json"]["coreType"] = default_values["core_type"]

        with open(deployed_path / "helics.json", "w") as helics_file:
            json.dump(json_data, helics_file, indent=4)

    def test_InputData_full_parse(self):
        # There should be no change in the backing values,so ensure that
        expected_values = get_default_values()

        backing_values = get_default_values()
        data = plugin.InputData(backing_values)
        total_time_seconds = backing_values["total_time"]

        # Check that backing values is unchanged
        self.assertDictEqual(expected_values, backing_values, "Expected first, then Actual.")

        # Check that the object was initialized
        self.assertEqual(expected_values["name"], data.name)
        self.assertEqual(expected_values["local_log_file"], data.local_log_file)
        self.assertEqual(expected_values["total_time"], total_time_seconds)
        self.assertEqual(expected_values["core_type"], data.core_type)
        self.assertEqual(expected_values["core_init"], data.core_init)

    def test_InputData_failed_key(self):
        # Get backing values and then manipulate the dict to mess it up
        backing_values = get_default_values()
        backing_values.pop("core_type")
        backing_values["local_log_file"] = 1234

        # Assert exception is raised, and then check the message
        with self.assertRaises(ValueError) as e:
            plugin.InputData(backing_values)
        self.assertIn("Key core_type not found", str(e.exception))
        self.assertIn("Key local_log_file found, but the type is incorrect", str(e.exception))

    def test_DummyFederatePlugin_deploy(self):
        default_data = get_default_values()
        total_time_seconds = default_data["total_time"]
        dummy_plugin = plugin.DummyFederatePlugin()
        dummy_plugin.deploy(default_data, total_time_seconds, str(self.deploy_dir), str(self.install_dir))

        expected_data = get_default_values()
        expected_total_time_seconds = expected_data["total_time"]
        expected_values = plugin.InputData(expected_data)
        expected_deployed_path = pathlib.Path(self.deploy_dir) / "debug_tools" / "dummy_federate" / self.get_safe_model_name()
        expected_paths = plugin.InputPaths(
            expected_deployed_path / "dummy_federate",
            expected_deployed_path / "helics.json",
            expected_deployed_path / "README.md"
        )

        self.assertTrue(expected_paths.executable.exists(), f"Deploy Exec Path does not exist: {expected_paths.executable}")
        self.assertTrue(expected_paths.configuration.exists(), f"Deploy Config Path does not exist: {expected_paths.configuration}")
        self.assertTrue(expected_paths.readme.exists(), f"Deploy README Path does not exist: {expected_paths.readme}")

        with open(expected_paths.configuration, "r") as json_file:
            actual_data = json.load(json_file)

        self.assertEqual(expected_values.name, actual_data["federate_name"])
        self.assertEqual(expected_values.local_log_file, actual_data["local_log_file"])
        self.assertEqual(expected_total_time_seconds, actual_data["total_time"])
        self.assertEqual(expected_values.core_init, actual_data["fed_info_json"]["coreInit"])
        self.assertEqual(expected_values.core_type, actual_data["fed_info_json"]["coreType"])

    def test_DummyFederatePlugin_get_baseline_files_real_path(self):
        dummy_plugin = plugin.DummyFederatePlugin()
        actual = dummy_plugin.get_baseline_files(str(self.install_dir))

        install_path = pathlib.Path(self.install_dir) / "debug_tools" / "dummy_federate"
        install_paths = plugin.InputPaths(
            install_path / "dummy_federate",
            install_path / "helics.json",
            install_path / "README.md"
        )
        expected = [str(install_paths.executable), str(install_paths.configuration), str(install_paths.readme)]

        self.assertCountEqual(expected, actual)

    def test_DummyFederatePlugin_get_baseline_files_fake_path(self):
        dummy_plugin = plugin.DummyFederatePlugin()
        self.assertRaises(ValueError, dummy_plugin.get_baseline_files, pathlib.Path("fake", "path"))

    def test_DummyFederatePlugin_get_model_files_real_path(self):
        dummy_plugin = plugin.DummyFederatePlugin()
        actual = dummy_plugin.get_model_files(self.get_safe_model_name(), str(self.deploy_dir))

        deployed_path = pathlib.Path(self.deploy_dir) / "debug_tools" / "dummy_federate" / self.get_safe_model_name()
        deployed_paths = plugin.InputPaths(
            deployed_path / "dummy_federate",
            deployed_path / "helics.json",
            deployed_path / "README.md"
        )
        expected = [str(deployed_paths.executable), str(deployed_paths.configuration), str(deployed_paths.readme)]

        self.assertCountEqual(expected, actual)

    def test_DummyFederatePlugin_get_model_files_fake_path(self):
        dummy_plugin = plugin.DummyFederatePlugin()
        actual = dummy_plugin.get_model_files(self.get_safe_model_name(), str(pathlib.Path("fake", "path")))
        self.assertFalse(actual, "The list should be empty but it is not.")

    def test_DummyFederatePlugin_get_exec_json(self):
        dummy_plugin = plugin.DummyFederatePlugin()
        actual = dummy_plugin.get_exec_json(self.get_safe_model_name(), str(self.deploy_dir))

        expected = {
            "directory": str(pathlib.Path("debug_tools", "dummy_federate", self.get_safe_model_name())),
            "exec": "/bin/sh -c './dummy_federate helics.json'",
            "host": "localhost",
            "name": self.get_safe_model_name()
        }

        self.assertDictEqual(expected, actual)

    def test_DummyFederatePlugin_get_name(self):
        dummy_plugin = plugin.DummyFederatePlugin()
        self.assertEqual("debug_tools/dummy_federate", dummy_plugin.get_name())