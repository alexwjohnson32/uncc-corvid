import unittest.mock as mock
import pathlib

import commands.list_model_files as list_model
import tests.tools.base_plugin_manager_test_case as base_test

class TestListModelFiles(base_test.BaseTestPluginManager):
    def test_bad_deploy_root(self):
        with self.assertRaises(ValueError) as e:
            list_model.get_model_files(self.manager, "mock_plugin_a/interface", pathlib.Path("/no_dir"))
        self.assertIn("Given deploy directory does not exist: '/no_dir'", str(e.exception))

    def test_bad_plugin_name(self):
        mock_path = mock.MagicMock(spec=pathlib.Path)
        mock_path.exists.return_value = True
        with self.assertRaises(KeyError) as e:
            list_model.get_model_files(self.manager, "bad_model", mock_path, "does_not_exist")
        self.assertIn("Could not find plugin with name 'does_not_exist'", str(e.exception))

    def test_search_for_name_success_multiple(self):
        mock_path = mock.MagicMock(spec=pathlib.Path)
        mock_path.exists.return_value = True
        mock_path.__str__.return_value = "fake_path"

        actual_result = ""
        try:
            actual_result = list_model.get_model_files(self.manager, "model_plugin_a", mock_path)
        except Exception as e:
            self.fail(f"Exception occured: {e}")

        # Make sure the form is correct. Should be two lines, both beginning with tabs, separated by newlines
        # \tfoo\n\tbar
        # The specific paths returned do not matter to the test, only the form
        results = actual_result.split('\n')
        self.assertEqual(2, len(results), f"Actual Message: '{repr(actual_result)}'")
        self.assertTrue(results[0], "Expected to not be empty") # Assert not empty
        self.assertTrue(results[0].startswith("\t"), f"Should start with a tab: repr: '{repr(results[0])}'") # This should start with a tab
        self.assertTrue(results[1], "Expected to not be empty") # Assert not empty
        self.assertTrue(results[1].startswith("\t"), f"Should start with a tab: repr: '{repr(results[1])}'") # This should start with a tab
        self.assertFalse("\n" in repr(results[1]), f"Should not contain a newline char: repr: '{repr(results[1])}'") # This should not contain a newline char anywhere

    def test_search_for_name_fail(self):
        mock_path = mock.MagicMock(spec=pathlib.Path)
        mock_path.exists.return_value = True
        mock_path.__str__.return_value = "fake_path"

        actual_result = ""
        try:
            actual_result = list_model.get_model_files(self.manager, "bad_name", mock_path)
        except Exception as e:
            self.fail(f"Exception occured: {e}")

        self.assertIn("Could not find 'bad_name' within deploy root 'fake_path'", actual_result)

    def test_specific_name_success(self):
        mock_path = mock.MagicMock(spec=pathlib.Path)
        mock_path.exists.return_value = True

        actual_result = ""

        try:
            actual_result = list_model.get_model_files(self.manager, "model_plugin_b", mock_path, "mock_plugin_b/interface")
        except Exception as e:
            self.fail(f"Exception occured: {e}")

        # Make sure the form is correct. Should be one line beginning with a tab
        # \tfoo\n\tbar
        # The specific paths returned do not matter to the test, only the form
        self.assertTrue(actual_result, "Expected to not be empty") # Assert not empty
        self.assertTrue(actual_result.startswith("\t"), f"Should start with a tab: repr: '{repr(actual_result)}'") # This should start with a tab char
        self.assertFalse("\n" in repr(actual_result), f"Should not contain a newline char: repr: '{repr(actual_result)}'") # This should not contain a newline char anywhere


    def test_specific_name_fail(self):
        mock_path = mock.MagicMock(spec=pathlib.Path)
        mock_path.exists.return_value = True
        mock_path.__str__.return_value = "fake_path"

        actual_result = ""

        try:
            actual_result = list_model.get_model_files(self.manager, "bad_name", mock_path, "mock_plugin_b/interface")
        except Exception as e:
            self.fail(f"Exception occured: {e}")

        self.assertIn("Could not find 'bad_name' with given plugin 'mock_plugin_b/interface' with deploy root 'fake_path'", actual_result)