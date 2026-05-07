import pathlib
import json
import plugins.federates.gridlabd.ieee_8500.ieee_8500_gridlabd_fed as ieee_8500
import tests.tools.base_plugin_test_case as base_test
import tests.metadata.federates_gridlabd_ieee_8500_expected as ieee_8500_expected
import unittest.mock as mock

def _get_basic_json_input() -> dict:
    return {
        "name": "test_name",
        "local_log_file": "local.log",
        "core_type": "core1",
        "core_init": "init1",
        "three_part_subscription_name": "sub_name"
    }

class TestIEEE8500FederatePlugin(base_test.BasePluginTestCase):
    @classmethod
    def __get_specific_path(cls) -> pathlib.Path:
        return pathlib.Path("gridlabd", "IEEE-8500")

    @classmethod
    def _get_install_path(cls) -> pathlib.Path:
        return cls.install_dir / "federate" / cls.__get_specific_path()

    @classmethod
    def _get_safe_deployed_path(cls) -> pathlib.Path:
        return cls.deploy_dir / cls.__get_specific_path() / cls.get_safe_model_name()

    @classmethod
    def _init_deployed_files(cls, deployed_path: pathlib.Path) -> None:
        with open(deployed_path.parent / "README.md", "w") as readme:
            readme.write("readme")
        with open(deployed_path.parent / "baseline_IEEE_8500.glm", "w") as baseline:
            baseline.write("baseline")
        with open(deployed_path / "IEEE_8500node.glm", "w") as model_glm:
            model_glm.write("model_glm")
        with open(deployed_path / "IEEE_8500node.json", "w") as json_file:
            json.dump({ "key": "value" }, json_file, indent=4)

    def test_InputData_full_parse(self):
        input_dict = _get_basic_json_input()
        expected_data = input_dict.copy()
        actual_data = ieee_8500.InputData(input_dict)

        self.assertDictEqual(expected_data, input_dict) # Asserts that the data was not modified
        self.assertEqual(expected_data["name"], actual_data.name)
        self.assertEqual(expected_data["local_log_file"], actual_data.local_log_file)
        self.assertEqual(expected_data["core_type"], actual_data.core_type)
        self.assertEqual(expected_data["core_init"], actual_data.core_init)
        self.assertEqual(expected_data["three_part_subscription_name"], actual_data.three_part_subscription_name)

    def test_InputData_failed_key(self):
        input_dict = _get_basic_json_input()
        input_dict.pop("name")
        input_dict["core_type"] = 12.34

        # Assert exception is raised, and then check the message
        with self.assertRaises(ValueError) as e:
            ieee_8500.InputData(input_dict)
        self.assertIn("Key name not found", str(e.exception))
        self.assertIn("Key core_type found, but the type is incorrect", str(e.exception))

    def test_IEEE8500_deploy(self):
        input_data = _get_basic_json_input()
        expected_json_data = input_data.copy()
        total_time_seconds = 100.0
        install_path = pathlib.Path(self.install_dir) / "federate" / self.__get_specific_path()
        baseline_paths = ieee_8500.BaselineFiles(
            install_path / "baseline_IEEE_8500.glm",
            install_path / "IEEE_8500node.json",
            install_path / "README.md"
        )
        deploy_path = pathlib.Path(self.deploy_dir) / self.__get_specific_path() / input_data["name"]
        expected_model_paths = ieee_8500.ModelFiles(
            deploy_path.parent / "baseline_IEEE_8500.glm",
            deploy_path / "IEEE_8500node.glm",
            deploy_path / "IEEE_8500node.json",
            deploy_path.parent / "README.md"
        )

        # Run the deploy
        plugin = ieee_8500.IEEE8500FederatePlugin()
        plugin.deploy(input_data, total_time_seconds, str(self.deploy_dir), str(self.install_dir))

        # Assert that the files exists
        self.assertTrue(expected_model_paths.baseline_glm_file.exists())
        self.assertTrue(expected_model_paths.model_glm_file.exists())
        self.assertTrue(expected_model_paths.json_config.exists())
        self.assertTrue(expected_model_paths.readme.exists())

        # Assert inputs did not change
        self.assertDictEqual(expected_json_data, input_data, "The input data should not have been modified by the deploy command")

        # Check that the deployed json config is setup correctly
        expected_json_data = ieee_8500_expected.get_ieee_8500_model_config(expected_json_data)
        with open(expected_model_paths.json_config, "r") as file:
            actual_data = json.load(file)
        self.maxDiff = None
        self.assertDictEqual(expected_json_data, actual_data)

        # Check that the deployed model glm is setup correctly
        expected_glm_data = ieee_8500_expected.get_model_glm(baseline_paths.glm_file, actual_data["name"], total_time_seconds)
        with open(expected_model_paths.model_glm_file, "r") as model_glm_file:
            actual_glm_data = model_glm_file.read()
        self.assertEqual(expected_glm_data, actual_glm_data)

    def test_IEEE8500_get_baseline_files_real_path(self):
        plugin = ieee_8500.IEEE8500FederatePlugin()
        actual = plugin.get_baseline_files(str(self.install_dir))

        install_path = pathlib.Path(self.install_dir) / "federate" / "gridlabd" / "IEEE-8500"
        baseline_paths = ieee_8500.BaselineFiles(
            install_path / "baseline_IEEE_8500.glm",
            install_path / "IEEE_8500node.json",
            install_path / "README.md"
        )
        expected = [str(baseline_paths.glm_file), str(baseline_paths.json_config), str(baseline_paths.readme)]

        self.assertCountEqual(expected, actual)

    def test_IEEE8500_get_baseline_files_fake_path(self):
        plugin = ieee_8500.IEEE8500FederatePlugin()

        with self.assertRaises(ValueError) as e:
            plugin.get_baseline_files(str(pathlib.Path("fake", "path")))

        self.assertEqual("Install path does not exist: 'fake/path/federate/gridlabd/IEEE-8500'", str(e.exception))

    def test_IEEE8500_get_model_files_real_path(self):
        plugin = ieee_8500.IEEE8500FederatePlugin()
        actual = plugin.get_model_files(self.get_safe_model_name(), str(self.deploy_dir))

        deploy_path = pathlib.Path(self.deploy_dir) / self.__get_specific_path() / self.get_safe_model_name()
        model_paths = ieee_8500.ModelFiles(
            deploy_path.parent / "baseline_IEEE_8500.glm",
            deploy_path / "IEEE_8500node.glm",
            deploy_path / "IEEE_8500node.json",
            deploy_path.parent / "README.md"
        )
        expected = [
            str(model_paths.baseline_glm_file),
            str(model_paths.model_glm_file),
            str(model_paths.json_config),
            str(model_paths.readme)
        ]

        self.assertCountEqual(expected, actual)

    def test_IEEE8500_get_model_files_fake_path(self):
        plugin = ieee_8500.IEEE8500FederatePlugin()
        actual = plugin.get_model_files(self.get_safe_model_name(), str(pathlib.Path("fake", "path")))
        self.assertFalse(actual, "The list should be empty but it is not.")

    def test_IEEE8500_get_exec_json(self):
        plugin = ieee_8500.IEEE8500FederatePlugin()
        actual = plugin.get_exec_json(self.get_safe_model_name(), str(self.deploy_dir))

        expected = {
            "directory": str(self.__get_specific_path() / self.get_safe_model_name()),
            "exec": f"gridlabd.sh {self.get_safe_model_name()}.glm",
            "host": "localhost",
            "name": self.get_safe_model_name()
        }

        self.assertDictEqual(expected, actual)

    def test_IEEE8500_get_name(self):
        plugin = ieee_8500.IEEE8500FederatePlugin()
        self.assertEqual("gridlabd/IEEE-8500", plugin.get_name())

    def test_IEEE8500_list_model_names_real_path(self):
        plugin = ieee_8500.IEEE8500FederatePlugin()

        actual = plugin.list_model_names(str(self.deploy_dir))
        expected = [self.get_safe_model_name()]
        # To ensure this does not matter when its ran, make sure that we have a check for test_name
        # only if we have 2 items
        if len(actual) == 2:
            expected.append("test_name")

        self.assertCountEqual(expected, actual)

    def test_IEEE8500_list_model_names_fake_path(self):
        plugin = ieee_8500.IEEE8500FederatePlugin()
        actual = plugin.list_model_names(str(pathlib.Path("fake", "path")))
        self.assertFalse(actual, "The list should be empty but it is not.")