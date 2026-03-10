import argparse

import commands.list_names
import pathlib
import plugins.manager as manager

def main():
    top_parser = argparse.ArgumentParser("UNCC Deploy Tool for initializing runnable cosimulation models")
    subparsers = top_parser.add_subparsers(dest="command", help="The Deploy Tool Commands")

    # list-names command
    list_names_parser = subparsers.add_parser("list-names", help="List the names of the found IDeployable Plugins")

    args = top_parser.parse_args()

    try:
        # Initialize Plugins
        plugins_dir = pathlib.Path(__file__).resolve().parent / "plugins" # Safe way to get to plugins directory
        plugin_manager = manager.PluginManager(plugins_dir)

        if args.command == "list-names":
            commands.list_names.get_plugin_names(plugin_manager)
    except ValueError as e:
        print(str(e))
    except Exception as e:
        print(f"An unexpected error occurred: {e}")
        print(f"Exception type: {type(e).__name__}")

if __name__ == '__main__':
    main()