import argparse

import commands.list_names
import commands.get_federate_definition
import commands.list_source_files
import commands.list_model_files
import pathlib
import plugins.manager as manager

def main():
    top_parser = argparse.ArgumentParser("UNCC Deploy Tool for initializing runnable cosimulation models.")
    subparsers = top_parser.add_subparsers(dest="command", help="The Deploy Tool Commands")

    _setup_list_names_parser(subparsers)
    _setup_get_exec_parser(subparsers)
    _setup_list_source_files_parser(subparsers)
    _setup_list_model_files_parser(subparsers)

    args = top_parser.parse_args()

    try:
        # Initialize Plugins
        plugins_dir = pathlib.Path(__file__).resolve().parent / "plugins" # Safe way to get to plugins directory
        plugin_manager = manager.PluginManager(plugins_dir)

        if args.command == "list-names":
            _print_plugin_names(plugin_manager)
        elif args.command == "get-exec":
            _print_federate_definition(plugin_manager, args.plugin_name, args.model_name, pathlib.Path(args.deploy_root))
        elif args.command == "list-source-files":
            _print_list_source_files(plugin_manager, args.plugin_name, pathlib.Path(args.install_dir))
        elif args.command == "list-model-files":
            _print_list_model_files(plugin_manager, args.model_name, pathlib.Path(args.deploy_dir), args.plugin_name)
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

def _setup_list_source_files_parser(subparser: argparse._SubParsersAction) -> None:
    list_source_files_parser = subparser.add_parser("list-source-files", help="Attempts to list the given source files for a plugin "
                                                    + "located within the install directory")

    list_source_files_parser.add_argument("--plugin-name", required=True, help="The name of the plugin to get input files for", type=str)
    list_source_files_parser.add_argument("--install-dir", required=True, help="The path to the install directory to read from", type=str)

def _print_list_source_files(plugin_manager: manager.PluginManager, plugin_name: str, install_dir: pathlib.Path) -> None:
    print(f"Source Files for {plugin_name}")
    print(commands.list_source_files.get_source_files(plugin_manager, plugin_name, install_dir))

def _setup_list_model_files_parser(subparser: argparse._SubParsersAction) -> None:
    list_model_files_parser = subparser.add_parser("list-model-files", help="Attempts to list the given model files for a plugin and model name "
                                                   + "located within the deploy directory")

    list_model_files_parser.add_argument("--model-name", required=True, help="The name of the model to get input files for", type=str)
    list_model_files_parser.add_argument("--deploy-dir", required=True, help="The path to the deploy directory to read from", type=str)
    list_model_files_parser.add_argument("--plugin-name", required=False, help="The name of the plugin to search from to find the model's input files", type=str)

def _print_list_model_files(plugin_manager: manager.PluginManager, model_name: str, deploy_dir: pathlib.Path, plugin_name: str) -> None:
    print(f"Model Files for {model_name}")
    print(commands.list_model_files.get_model_files(plugin_manager, model_name, deploy_dir, plugin_name))

if __name__ == '__main__':
    main()