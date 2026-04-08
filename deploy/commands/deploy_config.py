import json
import pathlib
import typing

import plugins.manager

def deploy(manager: plugins.manager.PluginManager, install_root: pathlib.Path, deploy_root: pathlib.Path, json_file: pathlib.Path) -> str:
    """:raises: ValueError if any paths do not exist or issues arise related to paths."""

    _validate_input_paths(install_root, deploy_root, json_file)

    json_data = _get_json_data(json_file)

    _iterate_components(json_data, manager, install_root, deploy_root)
    helics_json_file = _write_helics_json(manager, deploy_root)

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
        raise ValueError(f"Error when preparing Deploy Root '{str(deploy_root)}'! Exception: {e.strerror}")
    if deploy_root.is_file():
        raise ValueError(f"Given Deploy Root '{str(deploy_root)}' is not a directory!")

    if not json_file.exists():
        raise ValueError(f"Given Json File '{str(json_file)}' does not exist!")

def _get_json_data(json_file: pathlib.Path) -> dict[str, typing.Any]:
    with open(json_file, "r") as file:
        return json.load(file)

def _iterate_components(json_data: dict[str, typing.Any], manager: plugins.manager.PluginManager, install_root: pathlib.Path, deploy_root: pathlib.Path) -> None:
    pass

def _write_helics_json(manager: plugins.manager.PluginManager, deploy_root: pathlib.Path) -> pathlib.Path:
    return pathlib.Path("")