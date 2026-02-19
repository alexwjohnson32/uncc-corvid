import interface
import typing
import json
import pathlib

def get_install_plugin_root(install_root: str) -> str:
    # Assume install_root is valid and exists, we need to check if we need to append "release" or "debug"
    # to the file. Then attempt to append the rest of the path. If no valid path exists, then
    # simply return ""
    my_path = pathlib.Path(install_root)
    temp_path = my_path / "release"
    if not temp_path.exists():
        temp_path = my_path / "debug"
        if not temp_path.exists():
            temp_path = None

    if temp_path is not None:
        my_path = temp_path

    my_path = my_path / "debug_tools" / "dummy_federates"
    return str(my_path) if my_path.exists() else ""

def get_model_files(model_name: str, deploy_root: str) -> str:
    # Assume that the deploy_root is valid and does exist. Check that the rest of the path
    # exists with the given model name.
    my_path = pathlib.Path(deploy_root)
    my_path = my_path / "debug_tools" / "dummy_federate" / model_name

    return str(my_path) if my_path.exists() else ""


class DummyFederatePlugin(interface.IDeployable):
    def deploy(self, json_config: dict, deploy_root: str, install_root: str) -> bool:
        pass

    def get_baseline_files(self, install_root: str) -> list[str]:
        files = list()

        install_path = get_install_plugin_root(install_root)
        if (install_path):
            # Converts list of Paths to list of Strings
            files = [str(p) for p in list(pathlib.Path(install_path).iterdir())]

        return files

    def get_model_files(self, model_name: str, deploy_root: str) -> list[str]:
        files = list()

        deploy_path = get_model_files(model_name, deploy_root)
        if (deploy_path):
            # Converts list of Paths to list of Strings
            files = [str(p) for p in list(pathlib.Path(deploy_path).iterdir())]

        return files

    def get_exec_json(self, model_name: str, deploy_root: str) -> typing.Dict[str, typing.Any]:
        pass

    def get_name(self) -> str:
        return "debug_tools/dummy_federate"