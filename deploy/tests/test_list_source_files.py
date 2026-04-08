import unittest.mock as mock
import pathlib

import commands.list_source_files as list_source
import plugins.manager as manager
import tests.tools.base_plugin_manager_test_case as base_test
import plugins.interface as interface

class TestListSourceFiles(base_test.BaseTestPluginManager):
    def test_bad_deploy_root(self):
        with self.assertRaises(ValueError) as e:
            list_source.get_source_files(self.manager, "mock_plugin_a/interface", pathlib.Path("/no_dir"))
        self.assertIn("Given install directory does not exist: '/no_dir'", str(e.exception))

    def test_bad_plugin_name(self):
        mock_path = mock.MagicMock(spec=pathlib.Path)
        mock_path.exists.return_value = True
        with self.assertRaises(KeyError) as e:
            list_source.get_source_files(self.manager, "does_not_exist", mock_path)
        self.assertIn("Could not find plugin with name 'does_not_exist'", str(e.exception))

    def test_get_source_files_multiple_success(self):
        mock_path = mock.MagicMock(spec=pathlib.Path)
        mock_path.exists.return_value = True

        actual_result = ""
        try:
            actual_result = list_source.get_source_files(self.manager, "mock_plugin_a/interface", mock_path)
        except Exception as e:
            self.fail(f"Exception occured: {e}")

        self.assertTrue(actual_result) # make sure its not empty

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

    def test_get_source_files_single_success(self):
        mock_path = mock.MagicMock(spec=pathlib.Path)
        mock_path.exists.return_value = True

        actual_result = ""
        try:
            actual_result = list_source.get_source_files(self.manager, "mock_plugin_b/interface", mock_path)
        except Exception as e:
            self.fail(f"Exception occured: {e}")

        self.assertTrue(actual_result) # make sure its not empty

        # Make sure the form is correct. Should be one line beginning with a tab
        # \tfoo\n\tbar
        # The specific paths returned do not matter to the test, only the form
        self.assertTrue(actual_result, "Expected to not be empty") # Assert not empty
        self.assertTrue(actual_result.startswith("\t"), f"Should start with a tab: repr: '{repr(actual_result)}'") # This should start with a tab char
        self.assertFalse("\n" in repr(actual_result), f"Should not contain a newline char: repr: '{repr(actual_result)}'") # This should not contain a newline char anywhere

    def test_get_source_files_fail(self):
        mock_path = mock.MagicMock(spec=pathlib.Path)
        mock_path.exists.return_value = True
        mock_path.__str__.return_value = "fake_path"

        mock_interface = mock.MagicMock(spec=interface.IDeployable)
        mock_interface.get_baseline_files.return_value = list[str]()

        try:
            with mock.patch.object(manager.PluginManager, 'get') as mock_get:
                mock_get.return_value = mock_interface
                actual_result = list_source.get_source_files(self.manager, "mock_plugin_a/interface", mock_path)
                self.assertFalse(actual_result) # Assert that result is empty
        except Exception as e:
            self.fail(f"Exception occured: {e}")

