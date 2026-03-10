import pathlib
import plugins.manager as manager

def main():
    plugins_dir = pathlib.Path(__file__).resolve().parent / "plugins" # Safe way to get to plugins directory
    plugin_manager = manager.PluginManager(plugins_dir)
    for name in plugin_manager.get_plugin_names():
        print(name)

if __name__ == '__main__':
    main()