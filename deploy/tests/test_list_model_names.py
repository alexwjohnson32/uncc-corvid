import unittest.mock as mock
import pathlib

import commands.list_model_names as list_names
import tests.tools.base_plugin_manager_test_case as base_test

class TestListModelFiles(base_test.BaseTestPluginManager):
    def test_bad_deploy_root(self):
        with self.assertRaises(ValueError) as e:
            list_names.get_model_names(self.manager, pathlib.Path("/no_dir"))
        self.assertIn("Given deploy directory does not exist: '/no_dir'", str(e.exception))

    def test_bad_plugin_name(self):
        mock_path = mock.MagicMock(spec=pathlib.Path)
        mock_path.exists.return_value = True
        with self.assertRaises(KeyError) as e:
            list_names.get_model_names(self.manager, mock_path, "does_not_exist")
        self.assertIn("Could not find plugin with name 'does_not_exist'", str(e.exception))

    def test_list_all_plugin_names(self):
        mock_path = mock.MagicMock(spec=pathlib.Path)
        mock_path.exists.return_value = True

        # I don't want to care about model return order, so we check by parts
        # This should be ok since we are checking a full string specifically
        actual = list_names.get_model_names(self.manager, mock_path)
        self.assertIn("mock_plugin_a/interface:", actual)
        self.assertIn("\n\tmodel_plugin_a", actual)
        self.assertIn("\n\tmodel_plugin_a_sub1", actual)
        self.assertIn("mock_plugin_b/interface:", actual)
        self.assertIn("\n\tmodel_plugin_b", actual)

    def test_list_specific_plugin_names(self):
        mock_path = mock.MagicMock(spec=pathlib.Path)
        mock_path.exists.return_value = True

        actual = list_names.get_model_names(self.manager, mock_path, "mock_plugin_a/interface")
        expected = "mock_plugin_a/interface:\n\tmodel_plugin_a\n\tmodel_plugin_a_sub1"
        self.assertEqual(expected, actual)

