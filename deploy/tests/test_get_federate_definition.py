import pathlib
import json
import unittest.mock as mock

import commands.get_federate_definition as get_fed_def
import tests.tools.base_plugin_manager_test_case as base_test

class TestGetFederateDefinition(base_test.BaseTestPluginManager):
    def test_bad_deploy_root(self):
        with self.assertRaises(ValueError) as e:
            get_fed_def.get_federate_definition(self.manager, "model_plugin_a", pathlib.Path("/no_dir"))
        self.assertIn("Given Deploy Root '/no_dir' does not exist!", str(e.exception))

    def test_bad_plugin_name(self):
        mock_path = mock.MagicMock(spec=pathlib.Path)
        mock_path.exists.return_value = True
        with self.assertRaises(KeyError) as e:
            get_fed_def.get_federate_definition(self.manager, "model_plugin_a", mock_path, plugin_name="does_not_exist")
        self.assertIn("Given Plugin Name 'does_not_exist' does not exist!", str(e.exception))

    def test_search_for_name_success(self):
        mock_path = mock.MagicMock(spec=pathlib.Path)
        mock_path.exists.return_value = True

        actual_result = ""
        try:
            actual_result = get_fed_def.get_federate_definition(self.manager, "model_plugin_a", mock_path)
        except Exception as e:
            self.fail(f"Exception occured: {e}")

        try:
            # We cannot enforce what gets returned within the plugin. The plugins are responsible
            # for ensuring the validity of the returned value (as well as integration tests).
            # However, we can check that the returned value is a valid json string, since our
            # function in a success case calls json.dumps
            json.loads(actual_result)
        except (ValueError, json.JSONDecodeError) as e:
            self.fail(f"Could not parse result as json:actual_result: '{actual_result}'Exception: '{e}'")

    def test_search_for_name_fail(self):
        mock_path = mock.MagicMock(spec=pathlib.Path)
        mock_path.exists.return_value = True
        mock_path.__str__.return_value = "fake_path"

        actual_result = ""
        try:
            actual_result = get_fed_def.get_federate_definition(self.manager, "bad_name", mock_path)
        except Exception as e:
            self.fail(f"Exception occured: {e}")

        self.assertIn("Could not find 'bad_name' within deploy root 'fake_path'", actual_result)

        # Just to be sure, assert that we can't parse this and we did not get lucky with the output string.
        try:
            json.loads(actual_result)
            # This should nor be reached in a good case
            self.fail(f"Did not expect to have a valid json string!\nActual string:\n{actual_result}")
        except (ValueError, json.JSONDecodeError) as e:
            pass # do nothing

    def test_specific_plugin_success(self):
        mock_path = mock.MagicMock(spec=pathlib.Path)
        mock_path.exists.return_value = True

        actual_result = ""
        try:
            actual_result = get_fed_def.get_federate_definition(self.manager, "model_plugin_a", mock_path, "mock_plugin_a/interface")
        except Exception as e:
            self.fail(f"Exception occured: {e}")

        try:
            # We cannot enforce what gets returned within the plugin. The plugins are responsible
            # for ensuring the validity of the returned value (as well as integration tests).
            # However, we can check that the returned value is a valid json string, since our
            # function in a success case calls json.dumps
            json.loads(actual_result)
        except (ValueError, json.JSONDecodeError) as e:
            self.fail(f"Could not parse result as json:actual_result: '{actual_result}'Exception: '{e}'")

    def test_specific_plugin_fail(self):
        mock_path = mock.MagicMock(spec=pathlib.Path)
        mock_path.exists.return_value = True
        mock_path.__str__.return_value = "fake_path"

        actual_result = ""
        try:
            actual_result = get_fed_def.get_federate_definition(self.manager, "bad_name", mock_path, "mock_plugin_a/interface")
        except Exception as e:
            self.fail(f"Exception occured: {e}")

        self.assertIn("Could not find 'bad_name' with given plugin 'mock_plugin_a/interface' with deploy root 'fake_path'", actual_result)

        # Just to be sure, assert that we can't parse this and we did not get lucky with the output string.
        try:
            json.loads(actual_result)
            # This should nor be reached in a good case
            self.fail(f"Did not expect to have a valid json string!\nActual string:\n{actual_result}")
        except (ValueError, json.JSONDecodeError) as e:
            pass # do nothing
