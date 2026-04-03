import argparse

import commands.list_names
import commands.get_federate_definition
import pathlib
import plugins.manager as manager

def main():
    top_parser = argparse.ArgumentParser("UNCC Deploy Tool for initializing runnable cosimulation models.")
    subparsers = top_parser.add_subparsers(dest="command", help="The Deploy Tool Commands")

    _setup_list_names_parser(subparsers)
    _setup_get_exec_parser(subparsers)

    args = top_parser.parse_args()

    try:
        # Initialize Plugins
        plugins_dir = pathlib.Path(__file__).resolve().parent / "plugins" # Safe way to get to plugins directory
        plugin_manager = manager.PluginManager(plugins_dir)

        if args.command == "list-names":
            _print_plugin_names(plugin_manager)
        elif args.command == "get-exec":
            _print_federate_definition(plugin_manager, args.plugin_name, args.model_name, pathlib.Path(args.deploy_root))
        else:
            print(f"Unrecognized Command '{args.command}', terminating.")

    except (ValueError, RuntimeError, KeyError) as e:
        print(str(e))
    except Exception as e:
        print(f"An unexpected error occurred: {e}")
        print(f"Exception type: {type(e).__name__}")

def _setup_list_names_parser(subparser: argparse._SubParsersAction) -> None:
    subparser.add_parser("list-names", help="List the names of the found IDeployable Plugins")

def _print_plugin_names(plugin_manager: manager.PluginManager) -> None:
    print("IDeployable Plugin Names:")
    print(commands.list_names.get_plugin_names(plugin_manager))

def _setup_get_exec_parser(subparser: argparse._SubParsersAction) -> None:
    get_exec_parser = subparser.add_parser("get-exec", help="Attempts to find a specific model-name within a given deployed directory")
    get_exec_parser.add_argument("--model-name", required=True, help="The name of the model to find", type=str)
    get_exec_parser.add_argument("--deploy-root", required=True, help="The deploy directory to search within", type=str)
    get_exec_parser.add_argument("--plugin-name", required=False, help="(Optional) The name of the plugin that is expected to contain the name.", type=str, default="")

def _print_federate_definition(plugin_manager: manager.PluginManager, plugin_name: str, model_name: str, deploy_root: pathlib.Path) -> None:
    print(f"Federate Executable String for {model_name}:")
    print(commands.get_federate_definition.get_federate_definition(plugin_manager, model_name, deploy_root, plugin_name))

if __name__ == '__main__':
    main()