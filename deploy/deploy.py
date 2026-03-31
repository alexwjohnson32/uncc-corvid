import argparse
import pathlib
import plugins.manager as manager

def main():
    top_parser = argparse.ArgumentParser("UNCC Deploy Tool for initializing runnable cosimulation models")
    subparsers = top_parser.add_subparsers(dest="command", help="The Deploy Tool Commands")

    args = top_parser.parse_args()

    try:
        # Initialize Plugins
        plugins_dir = pathlib.Path(__file__, "plugins") # Safe way to get to plugins directory
        plugin_manager = manager.PluginManager(plugins_dir)
    except ValueError as e:
        print(str(e))
if __name__ == '__main__':
    main()