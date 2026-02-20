import interface
import typing
import json
import pathlib
import dataclasses

@dataclasses.dataclass
class DummyFederateInputData:
    name: str
    local_log_file: str
    total_time: float
    core_type: str
    core_init: str


def get_install_path(install_root: str) -> pathlib.Path:
    # Assume install_root is valid and exists, we need to check if we need to append "release" or "debug"
    # to the file. Then attempt to append the rest of the path.
    my_path = pathlib.Path(install_root)
    temp_path = my_path / "release"
    if not temp_path.exists():
        temp_path = my_path / "debug"
        if not temp_path.exists():
            temp_path = None

    if temp_path is not None:
        my_path = temp_path

    return my_path / "debug_tools" / "dummy_federates"

def get_model_files_root(model_name: str, deploy_root: str) -> pathlib.Path:
    # Assume that the deploy_root is valid and does exist. Append the rest of the
    # path with the given model_name
    my_path = pathlib.Path(deploy_root)
    return my_path / "debug_tools" / "dummy_federate" / model_name

class DummyFederatePlugin(interface.IDeployable):
    def deploy(self, json_config: dict, deploy_root: str, install_root: str) -> bool:
        # We can assume that deploy_root and install_root are valid directories and that they exist.
        install_path = get_install_path(install_root)
        deploy_path = get_model_files_root(json_config["name"], deploy_root)

        return True

    def get_baseline_files(self, install_root: str) -> list[str]:
        files = list()

        install_path = get_install_path(install_root)
        if (install_path.exists()):
            # Converts list of Paths to list of Strings
            files = [str(p) for p in list(install_path.iterdir())]

        return files

    def get_model_files(self, model_name: str, deploy_root: str) -> list[str]:
        files = list()

        deploy_path = get_model_files_root(model_name, deploy_root)
        if (deploy_path.exists()):
            files.append(str(deploy_path / "helics.json"))
            files.append(str(deploy_path / "dummy_federate"))
            files.append(str(deploy_path / "README.md"))

        return files

    def get_exec_json(self, model_name: str, deploy_root: str) -> typing.Dict[str, typing.Any]:
        relative_working_directory = pathlib.Path("debug_tools", "dummy_federate", model_name)
        absolute_working_directory = pathlib.Path(deploy_root) / relative_working_directory

        json_definition = dict()
        if absolute_working_directory.exists():
            json_definition["directory"] = str(relative_working_directory)
            json_definition["exec"] = "/bin/sh -c './dummy_federate helics.json'"
            json_definition["host"] = "localhost"
            json_definition["name"] = model_name

        return json_definition

    def get_name(self) -> str:
        return "debug_tools/dummy_federate"