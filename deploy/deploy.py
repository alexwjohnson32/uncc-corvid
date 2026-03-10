<<<<<<< HEAD
import argparse
=======
>>>>>>> 17-create-a-plugin-manager
import pathlib
import plugins.manager as manager

def main():
<<<<<<< HEAD
    top_parser = argparse.ArgumentParser("UNCC Deploy Tool for initializing runnable cosimulation models")
    subparsers = top_parser.add_subparsers(dest="command", help="The Deploy Tool Commands")

    args = top_parser.parse_args()

    try:
        # Initialize Plugins
        plugins_dir = pathlib.Path(__file__, "plugins") # Safe way to get to plugins directory
        plugin_manager = manager.PluginManager(plugins_dir)
    except ValueError as e:
        print(str(e))
=======
    plugins_dir = pathlib.Path(__file__).resolve().parent / "plugins" # Safe way to get to plugins directory
    plugin_manager = manager.PluginManager(plugins_dir)
    for name in plugin_manager.get_plugin_names():
        print(name)
>>>>>>> 17-create-a-plugin-manager

if __name__ == '__main__':
    main()