import unittest.mock as mock

import commands.list_names as list_names
import plugins.manager as manager
import tests.tools.base_plugin_manager_test_case as base_test

class TestListNames(base_test.BaseTestPluginManager):
    def test_get_plugin_names(self):
        plugin_names = list_names.get_plugin_names(self.manager)
        expected_string = "\tmock_plugin_a/interface\n\tmock_plugin_b/interface"

        self.assertEqual(expected_string, plugin_names)

    def test_get_plugin_names_single_name(self):
        with mock.patch.object(manager.PluginManager, 'get_plugin_names') as mock_get_names:
            mock_get_names.return_value = ["one_name"]

            plugin_names = list_names.get_plugin_names(self.manager)
            expected_string = "\tone_name"

            self.assertEqual(expected_string, plugin_names)

    def test_get_plugin_names_no_names(self):
        with mock.patch.object(manager.PluginManager, 'get_plugin_names') as mock_get_names:
            mock_get_names.return_value = []

            plugin_names = list_names.get_plugin_names(self.manager)
            expected_string = ""

            self.assertEqual(expected_string, plugin_names)