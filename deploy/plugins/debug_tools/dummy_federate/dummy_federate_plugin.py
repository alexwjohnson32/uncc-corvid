import plugins.interface as interface
import typing
import json
import pathlib
import dataclasses
import typing
import shutil

T = typing.TypeVar("T")

@dataclasses.dataclass
class InputPaths:
    executable: pathlib.Path
    configuration: pathlib.Path
    readme: pathlib.Path

class InputData:
    def _validate_config(self, json_dict: dict, key: str, expected_type: typing.Type[T], errors: list[str]) -> T:
        value = expected_type()

        if not key in json_dict:
            errors.append(f"Key {key} not found!")
        elif not isinstance(json_dict[key], expected_type):
            errors.append(f"Key {key} found, but the type is incorrect. Expected '{expected_type.__name__}', Actual: '{type(json_dict[key]).__name__}'")
        else:
            value = json_dict[key]

        return value

    def __init__(self, json_dict: dict):
        self.name: str = ""
        self.local_log_file: str = ""
        self.core_type: str = ""
        self.core_init: str = ""

        errs = list[str]()

        self.name = self._validate_config(json_dict, "name", str, errs)
        self.local_log_file = self._validate_config(json_dict, "local_log_file", str, errs)
        self.core_type = self._validate_config(json_dict, "core_type", str, errs)
        self.core_init = self._validate_config(json_dict, "core_init", str, errs)

        if errs:
            err_msgs = str.join("\n", errs)
            raise ValueError(f"Errors parsing configuration:\n{err_msgs}")

class DummyFederatePlugin(interface.IDeployable):
    @classmethod
    def _get_specific_path(cls) -> pathlib.Path:
        return pathlib.Path("debug_tools", "dummy_federate")

    def _get_input_paths(self, root: pathlib.Path) -> InputPaths:
        return InputPaths(
            root / "dummy_federate",
            root / "helics.json",
            root / "README.md"
        )

    def _get_install_path(self, install_root: str) -> pathlib.Path:
        return pathlib.Path(install_root) / self._get_specific_path()

    def _get_deploy_path(self, deploy_root: str, model_name: str) -> pathlib.Path:
        return pathlib.Path(deploy_root) / self._get_specific_path() / model_name

    def deploy(self, json_config: dict, total_time_seconds: float, deploy_root: str, install_root: str) -> None:
        # If this cannot be parsed, it will raise a ValueError with a list of parse errors
        input_data = InputData(json_config)

        # Raise a ValueError if the path does not exist.
        install_path = self._get_install_path(install_root)
        if (not install_path.exists()):
            raise ValueError(f"Install path does not exist: '{install_path}'")
        source_files = self._get_input_paths(install_path)

        # If this deploy path does not exist, create it
        deploy_path = self._get_deploy_path(deploy_root, input_data.name)
        deploy_path.mkdir(parents=True, exist_ok=True)
        destination_files = self._get_input_paths(deploy_path)

        # Copy the source to the destination, modifying the config file data
        shutil.copy2(source_files.executable, destination_files.executable)
        shutil.copy2(source_files.readme, destination_files.readme)
        with open(source_files.configuration, "r") as json_file:
            json_data = json.load(json_file)
        json_data["federate_name"] = input_data.name
        json_data["local_log_file"] = input_data.local_log_file
        json_data["total_time"] = total_time_seconds
        json_data["fed_info_json"]["coreInit"] = input_data.core_init
        json_data["fed_info_json"]["coreType"] = input_data.core_type
        with open(destination_files.configuration, "w") as source_file:
            json.dump(json_data, source_file, indent=4)

    def get_baseline_files(self, install_root: str) -> list[str]:
        install_path = self._get_install_path(install_root)
        if (not install_path.exists()):
            raise ValueError(f"Install path does not exist: '{install_path}'")

        install_paths = self._get_input_paths(install_path)
        return [str(install_paths.executable), str(install_paths.configuration), str(install_paths.readme)]

    def get_model_files(self, model_name: str, deploy_root: str) -> list[str]:
        deploy_path = self._get_deploy_path(deploy_root, model_name)
        if deploy_path.exists():
            deploy_paths = self._get_input_paths(deploy_path)
            return [str(deploy_paths.executable), str(deploy_paths.configuration), str(deploy_paths.readme)]
        else:
            return list[str]()

    def get_exec_json(self, model_name: str, deploy_root: str) -> typing.Dict[str, str]:
        relative_working_directory = self._get_specific_path() / model_name
        absolute_working_directory = pathlib.Path(deploy_root) / relative_working_directory

        json_definition = dict()
        if absolute_working_directory.exists():
            json_definition["directory"] = str(relative_working_directory)
            json_definition["exec"] = "/bin/sh -c './dummy_federate helics.json'"
            json_definition["host"] = "localhost"
            json_definition["name"] = model_name

        return json_definition

    def get_name(self) -> str:
        # Don't return the path, because we want this name to be unchanging independent of filesystem
        return "debug_tools/dummy_federate"

    def list_model_names(self, deploy_root: str) -> list[str]:
        model_names = list[str]()

        models_root_dir = pathlib.Path(deploy_root) / self._get_specific_path()
        if models_root_dir.exists():
            for model_dir in models_root_dir.iterdir():
                if (model_dir.is_dir()):
                    model_names.append(model_dir.name)

        return model_names
