import pathlib
import json
import typing
import unittest.mock as mock

import commands.deploy_config as deploy
import tests.tools.base_plugin_manager_test_case as base_test

def _expected_models() -> dict[str, list[str]]:
    return {
        "mock_plugin_a/interface": ["model_plugin_a", "model_plugin_a_sub1"],
        "mock_plugin_b/interface": ["model_plugin_b"]
    }

def _expected_federates() -> list[dict[str, str]]:
    return [
        {
            "directory": ".",
            "exec": "helics-broker --federates=3 --port 23500",
            "host": "localhost",
            "name": "main_broker"
        },
        {
            "directory": ".",
            "exec": "./mock_plugin_a",
            "host": "localhost",
            "name": "mock_plugin_a"
        },
        {
            "directory": ".",
            "exec": "./model_plugin_a_sub1",
            "host": "localhost",
            "name": "model_plugin_a_sub1"
        },
        {
            "directory": ".",
            "exec": "./mock_plugin_b",
            "host": "localhost",
            "name": "mock_plugin_b"
        }
    ]

def _expected_helics_json() -> dict[str, typing.Any]:
    return {
        "name": "cosim_test",
        "federates": _expected_federates()
    }

def _baseline_components() -> dict[str, typing.Any]:
    return {
        "type": "component",
        "components": [
            {
                "type": "mock_plugin_a/interface",
                "options": { }
            },
            {
                "type": "component",
                "components": [
                    {
                        "type": "mock_plugin_b/interface",
                        "options": { }
                    }
                ]
            }
        ]
    }

def _expected_components() -> dict[str, typing.Any]:
    components = _baseline_components()
    mock_a_opts = components["components"][0]["options"]
    mock_a_opts["deploy"] = True

    mock_b_opts = components["components"][1]["components"][0]["options"]
    mock_b_opts["deploy"] = True

    return components

def _baseline_deploy_config() -> dict[str, typing.Any]:
    return {
        "cosim_name": "cosim_test",
        "total_time_seconds": 100.0,
        "components": [ _baseline_components() ]
    }

def _expected_deploy_config() -> dict[str, typing.Any]:
    deploy_config = _baseline_deploy_config()
    deploy_config["components"][0] = _expected_components()

    return deploy_config

def _get_basic_json() -> dict[str, str]:
    return { "key": "value" }

def _get_mock_path() -> mock.MagicMock:
    mock_path = mock.MagicMock(spec=pathlib.Path)
    mock_path.__str__.return_value = "fake_path"
    mock_path.exists.return_value = True
    return mock_path

def _append_mock_path(path_str: str) -> mock.MagicMock:
    """Creates a mock path that supports infinite chaining and .parent."""
    mock_path = mock.MagicMock(spec=pathlib.Path)
    mock_path.__str__.return_value = path_str

    def join_paths(other):
        # Create the child mock
        child_path = f"{path_str}/{other}"
        child_mock = _append_mock_path(child_path)
        # Link the child's .parent back to THIS mock instance
        child_mock.parent = mock_path
        return child_mock

    # Handle / operator
    mock_path.__truediv__.side_effect = join_paths
    mock_path.exists.return_value = True
    mock_path.mkdir.return_value = None
    return mock_path

def _get_mock_deploy_path() -> mock.MagicMock:
    mock_deploy = _append_mock_path("fake_deploy_path")
    mock_deploy.is_file.return_value = False
    return mock_deploy

def _get_mock_install_path() -> mock.MagicMock:
    mock_install = mock.MagicMock(spec=pathlib.Path)
    mock_install.__str__.return_value = "fake_install_path"
    mock_install.exists.return_value = True
    mock_install.is_file.return_value = False
    return mock_install

def _get_mock_json_path() -> mock.MagicMock:
    mock_json = mock.MagicMock(spec=pathlib.Path)
    mock_json.__str__.return_value = "fake_json_file"
    mock_json.exists.return_value = True
    return mock_json

class TestDeployConfig(base_test.BaseTestPluginManager):
    def test_get_models(self):
        actual = deploy._get_models(self.manager, _get_mock_path())
        self.assertDictEqual(_expected_models(), actual)

    def test_get_helics_json_data(self):
        actual = deploy._get_helics_json_data("cosim_test", self.manager, _get_mock_path())
        self.maxDiff = None # Use this because errors need it to display the diff
        self.assertDictEqual(_expected_helics_json(), actual)

    def test_get_model_execs(self):
        actual = deploy._get_model_execs(self.manager, _get_mock_path())
        self.assertCountEqual(_expected_federates(), actual)

    def test_get_cosim_name(self):
        with mock.patch.object(deploy, "_get_json_data") as mock_func:
            mock_func.return_value = { "cosim_name": "cosim_test" }
            actual = deploy._get_cosim_name(_get_mock_path())
        self.assertEqual("cosim_test", actual)

    def test_get_value_or_default_success(self):
        actual = deploy._get_value_or_default(_get_basic_json(), "key", str)
        self.assertEqual("value", actual)

    def test_get_value_or_default_bad_key(self):
        actual = deploy._get_value_or_default(_get_basic_json(), "key1", str)
        self.assertEqual("", actual)

    def test_get_value_or_default_bad_type(self):
        actual = deploy._get_value_or_default(_get_basic_json(), "key", float)
        self.assertEqual(0.0, actual)

    def test_validate_key_value_success(self):
        actual = deploy._validate_key_value(_get_basic_json(), "key", str)
        self.assertEqual("value", actual)

    def test_validate_key_value_bad_key(self):
        with self.assertRaises(ValueError) as err:
            deploy._validate_key_value(_get_basic_json(), "key1", str)

        self.assertEqual("Key 'key1' not found!", str(err.exception))

    def test_validate_key_value_bad_type(self):
        with self.assertRaises(ValueError) as err:
            deploy._validate_key_value(_get_basic_json(), "key", float)

        expecected = "Key 'key' found, but the type is incorrect. Expected 'float', Actual: 'str'"
        self.assertEqual(expecected, str(err.exception))

    def test_deploy_components(self):
        actual = _baseline_components()
        deploy._deploy_components(actual, 0.0, self.manager, _get_mock_install_path(), _get_mock_deploy_path())

        expected = _expected_components()

        self.assertDictEqual(expected, actual)

    def test_deploy_config_success(self):
        actual = _baseline_deploy_config()
        with mock.patch.object(deploy, "_write_json") as mock_func:
            mock_func.return_value = None
            cosim_defintion_path = deploy._deploy_config(actual, self.manager, _get_mock_install_path(), _get_mock_deploy_path())

        self.assertDictEqual(_expected_deploy_config(), actual)
        expected_path = pathlib.Path("fake_deploy_path", "cosim_test", "cosim_def.json")
        self.assertEqual(str(expected_path), str(cosim_defintion_path))

    def test_deploy_config_missing_fields(self):
        mock_install = _get_mock_install_path()
        mock_deploy = _get_mock_deploy_path()

        errors = list[str]()
        missing_time_data = {"cosim_name": "test", "components": [{}]}
        missing_name_data = {"components": [{}], "total_time_seconds": 0.0}
        missing_component_data = {"cosim_name": "test", "total_time_seconds": 0.0}

        with mock.patch.object(deploy, "_write_json") as mock_func:
            mock_func.return_value = None
            try:
                deploy._deploy_config(missing_time_data, self.manager, mock_install, mock_deploy)
                errors.append("Should have thrown exception with missing key 'total_time_seconds'")
            except ValueError:
                pass

            try:
                deploy._deploy_config(missing_name_data, self.manager, mock_install, mock_deploy)
                errors.append("Should have thrown exception with missing key 'cosim_name'")
            except ValueError:
                pass

            try:
                deploy._deploy_config(missing_component_data, self.manager, mock_install, mock_deploy)
                errors.append("Should have thrown exception with missing key 'components'")
            except ValueError:
                pass

        self.assertFalse(errors, "\n".join(errors))

    def test_validate_input_paths_success(self):
        mock_install = _get_mock_install_path()
        mock_deploy = _get_mock_deploy_path()
        mock_json = _get_mock_json_path()

        try:
            deploy._validate_input_paths(mock_install, mock_deploy, mock_json)
        except ValueError as e:
            self.fail(f"my_function() raised error unexpectedly!\nError:{str(e)}")

    def test_validate_input_paths_install_does_not_exist(self):
        mock_install = _get_mock_install_path()
        mock_install.exists.return_value = False
        mock_deploy = _get_mock_deploy_path()
        mock_json = _get_mock_json_path()

        with self.assertRaises(ValueError) as err:
            deploy._validate_input_paths(mock_install, mock_deploy, mock_json)

        expected = "Given Install Root 'fake_install_path' does not exist!"
        self.assertEqual(expected, str(err.exception))

    def test_validate_input_paths_install_is_file(self):
        mock_install = _get_mock_install_path()
        mock_install.is_file.return_value = True
        mock_deploy = _get_mock_deploy_path()
        mock_json = _get_mock_json_path()

        with self.assertRaises(ValueError) as err:
            deploy._validate_input_paths(mock_install, mock_deploy, mock_json)

        expected = "Given Install Root 'fake_install_path' is not a directory!"
        self.assertEqual(expected, str(err.exception))

    def test_validate_input_paths_deploy_mkdir_raises_error(self):
        mock_install = _get_mock_install_path()
        mock_deploy = _get_mock_deploy_path()
        mock_deploy.mkdir.side_effect = OSError("mocked error")
        mock_json = _get_mock_json_path()

        with self.assertRaises(ValueError) as err:
            deploy._validate_input_paths(mock_install, mock_deploy, mock_json)

        expected = "Error when preparing Deploy Root 'fake_deploy_path'! Exception: mocked error"
        self.assertEqual(expected, str(err.exception))

    def test_validate_input_paths_deploy_is_file(self):
        mock_install = _get_mock_install_path()
        mock_deploy = _get_mock_deploy_path()
        mock_deploy.is_file.return_value = True
        mock_json = _get_mock_json_path()

        with self.assertRaises(ValueError) as err:
            deploy._validate_input_paths(mock_install, mock_deploy, mock_json)

        expected = "Given Deploy Root 'fake_deploy_path' is not a directory!"
        self.assertEqual(expected, str(err.exception))

    def test_validate_input_paths_json_does_not_exist(self):
        mock_install = _get_mock_install_path()
        mock_deploy = _get_mock_deploy_path()
        mock_json = _get_mock_json_path()
        mock_json.exists.return_value = False

        with self.assertRaises(ValueError) as err:
            deploy._validate_input_paths(mock_install, mock_deploy, mock_json)

        expected = "Given Json File 'fake_json_file' does not exist!"
        self.assertEqual(expected, str(err.exception))

    def test_deploy(self):
        mock_install = _get_mock_install_path()
        mock_deploy = _get_mock_deploy_path()
        mock_json = _get_mock_json_path()

        with (
            mock.patch.object(deploy, "_get_json_data") as mock_get_json,
            mock.patch.object(deploy, "_deploy_config", wraps=deploy._deploy_config) as wrapped_deploy_config,
            mock.patch.object(deploy, "_write_json") as mock_write_json
        ):
            mock_get_json.return_value = _baseline_deploy_config()
            mock_write_json.return_value = None

            actual_message = deploy.deploy(self.manager, mock_install, mock_deploy, mock_json)
            deploy_args, _ = wrapped_deploy_config.call_args
            actual_deploy_data, _, _, _ = deploy_args
            write_args, _ = mock_write_json.call_args
            actual_helics_data, _ = write_args

        self.assertDictEqual(_expected_deploy_config(), actual_deploy_data)
        self.assertDictEqual(_expected_helics_json(), actual_helics_data)
        expected_message = "Successfully deployed and wrote json configuration to 'fake_deploy_path/cosim_test/helics_runner.json'!"
        self.assertEqual(expected_message, actual_message)