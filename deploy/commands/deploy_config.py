import json
import pathlib
import typing

import plugins.manager

T = typing.TypeVar("T")

def deploy(manager: plugins.manager.PluginManager, install_root: pathlib.Path, deploy_root: pathlib.Path, json_file: pathlib.Path) -> str:
    """:raises: ValueError if any paths do not exist or issues arise related to paths."""

    _validate_input_paths(install_root, deploy_root, json_file)

    json_data = _get_json_data(json_file)

    cosim_def_file = _deploy_config(json_data, manager, install_root, deploy_root)
    deploy_path = cosim_def_file.parent
    helics_json_data = _get_helics_json_data(_get_cosim_name(cosim_def_file), manager, deploy_path)
    helics_json_file = deploy_path / "helics_runner.json"
    _write_json(helics_json_data, helics_json_file)

    return f"Successfully deployed and wrote json configuration to '{str(helics_json_file)}'!"

def _validate_input_paths(install_root: pathlib.Path, deploy_root: pathlib.Path, json_file: pathlib.Path) -> None:
    """:raises: ValueError if any paths do not exist, or if expected directories are actually files."""
    if not install_root.exists():
        raise ValueError(f"Given Install Root '{str(install_root)}' does not exist!")
    elif install_root.is_file():
        raise ValueError(f"Given Install Root '{str(install_root)}' is not a directory!")

    try:
        deploy_root.mkdir(parents=True, exist_ok=True)
    except OSError as e:
        raise ValueError(f"Error when preparing Deploy Root '{str(deploy_root)}'! Exception: {str(e)}")
    if deploy_root.is_file():
        raise ValueError(f"Given Deploy Root '{str(deploy_root)}' is not a directory!")

    if not json_file.exists():
        raise ValueError(f"Given Json File '{str(json_file)}' does not exist!")

def _get_json_data(json_file: pathlib.Path) -> dict[str, typing.Any]:
    with open(json_file, "r") as file:
        return json.load(file)

def _deploy_config(json_data: dict[str, typing.Any], manager: plugins.manager.PluginManager,
                   install_root: pathlib.Path, deploy_root: pathlib.Path) -> pathlib.Path:
    # Validate that a total time has been set
    total_time_seconds = _validate_key_value(json_data, "total_time_seconds", float)

    # Validate that a components key has been set
    components = _validate_key_value(json_data, "components", list)

    # Validate that a cosim name has been set
    cosim_name = _validate_key_value(json_data, "cosim_name", str)

    # Append cosim name to deploy root
    deploy_root = deploy_root / cosim_name
    deploy_root.mkdir(exist_ok=True)
    cosim_def_path = deploy_root / "cosim_def.json"

    try:
        # Iterate over the components
        for component in components:
            _deploy_components(component, total_time_seconds, manager, install_root, deploy_root)
    finally:
        # Do this in a finally to ensure we write out the info regardless of how we got here.
        # write cosim json file to deploy root
        _write_json(json_data, cosim_def_path)

    # Do not return within the finally clause, it may silence an exception
    return cosim_def_path

def _validate_key_value(json_dict: dict[str, typing.Any], key: str, expected_type: typing.Type[T]) -> T:
    value = expected_type()

    if not key in json_dict:
        raise ValueError(f"Key '{key}' not found!")
    elif not isinstance(json_dict[key], expected_type):
        raise ValueError(f"Key '{key}' found, but the type is incorrect. Expected '{expected_type.__name__}', Actual: '{type(json_dict[key]).__name__}'")
    else:
        value = json_dict[key]

    return value

def _deploy_components(json_data: dict[str, typing.Any], total_time_seconds: float, manager: plugins.manager.PluginManager,
                        install_root: pathlib.Path, deploy_root: pathlib.Path) -> None:
    # If no type key is found within the component, assume it is a sub-component
    # If the type is not recognized in the plugin names, assume it is a sub-component
    plugin_name = _get_value_or_default(json_data, "type", str)
    plugin = manager.get(plugin_name)

    if plugin:
        plugin_options = _get_value_or_default(json_data, "options", dict)
        plugin.deploy(plugin_options, total_time_seconds, str(deploy_root), str(install_root))

    components = _get_value_or_default(json_data, "components", list)
    for component in components:
        _deploy_components(component, total_time_seconds, manager, install_root, deploy_root)

def _get_value_or_default(json_dict: dict[str, typing.Any], key: str, expected_type: typing.Type[T]) -> T:
    value = expected_type()

    if key in json_dict and isinstance(json_dict[key], expected_type):
        value = json_dict[key]

    return value

def _get_cosim_name(cosim_def_file: pathlib.Path) -> str:
    # Get cosim name
    cosim_def_data = _get_json_data(cosim_def_file)
    return cosim_def_data["cosim_name"]

def _get_model_execs(manager: plugins.manager.PluginManager, deploy_root: pathlib.Path) -> list[dict[str, str]]:
    # Initialize the federates
    federates = list[dict[str, str]]()
    models = _get_models(manager, deploy_root)
    for plugin_name, model_names in models.items():
        plugin = manager.get(plugin_name)
        if not plugin:
            continue # this should never happen

        for model_name in model_names:
            federates.append(plugin.get_exec_json(model_name, str(deploy_root)))

    # Setup the broker
    federate_count = len(federates)
    broker_federate = {
        "directory": ".",
        "exec": f"helics_broker --federates={federate_count} --port 23500",
        "host": "localhost",
        "name": "main_broker"
    }
    federates.insert(0, broker_federate)

    return federates

def _get_helics_json_data(cosim_name: str, manager: plugins.manager.PluginManager, deploy_root: pathlib.Path) -> dict[str, typing.Any]:
    # Setup helics json
    helics_json_data = dict()
    helics_json_data["name"] = cosim_name
    helics_json_data["federates"] = _get_model_execs(manager, deploy_root)

    return helics_json_data

def _write_json(json_data: dict[str, typing.Any], json_path: pathlib.Path) -> None:
    with open(json_path, "w") as json_file:
        json.dump(json_data, json_file, indent=4)

def _get_models(manager: plugins.manager.PluginManager, deploy_root: pathlib.Path) -> dict[str, list[str]]:
    models = dict[str, list[str]]()

    for plugin_name in manager.get_plugin_names():
        plugin = manager.get(plugin_name)
        if not plugin:
            continue

        models[plugin_name] = plugin.list_model_names(str(deploy_root))

    return models