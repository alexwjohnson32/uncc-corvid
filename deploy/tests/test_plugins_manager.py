import plugins.interface as interface
import tests.tools.base_plugin_manager_test_case as base_test

class TestPluginManager(base_test.BaseTestPluginManager):
    def test_manager_exists(self):
        self.assertIsNotNone(self.manager)

    def test_get_plugin_names(self):
        actual_names = self.manager.get_plugin_names()
        expected_names = ["mock_plugin_a/interface", "mock_plugin_b/interface"]

        self.assertCountEqual(expected_names, actual_names)

    def test_get_name_exists(self):
        plugin = self.manager.get("mock_plugin_a/interface")

        self.assertIsNotNone(plugin)
        self.assertIsInstance(plugin, interface.IDeployable)
        self.assertIsInstance(plugin, base_test.MockPluginA)

    def test_get_name_does_not_exist(self):
        plugin = self.manager.get("non_name")

        self.assertIsNone(plugin)