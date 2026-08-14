import plugins.interface as interface
import typing
import json
import pathlib
import shutil
import dataclasses
import copy
import datetime

T = typing.TypeVar("T")

@dataclasses.dataclass
class GridlabdInfo:
    bus_id: int = 0
    names: list[str] = dataclasses.field(default_factory=lambda: [])

class InputData:
    def _validate_config(self, json_dict: dict, key: str, expected_type: typing.Type[T], errors: list[str]) -> T:
        value = expected_type()

        if not key in json_dict:
            errors.append(f"Key {key} not found!")
        elif not isinstance(json_dict[key], expected_type):
            errors.append(f"Key {key} found, but the type is incorrect. Expected '{expected_type.__name__}', "
                          + f"Actual: '{type(json_dict[key]).__name__}'")
        else:
            value = json_dict[key]

        return value

    def _validate_gridlabd_infos(self, json_dict: dict, errors: list[str]) -> list[GridlabdInfo]:
        gridlabd_infos = list[GridlabdInfo]()
        json_infos = self._validate_config(json_dict, "gridlabd_infos", list, errors)
        for info in json_infos:
            if not isinstance(info, dict):
                errors.append("Key gridlabd_infos can only contain dictionaries of key 'bus_id: int' "
                              + "and values 'name: list[str] of gridlabd names'.")
                break

            if "bus_id" not in info or not isinstance(info["bus_id"], int):
                errors.append("gridlabd_infos objects must contain a bus_id: int key")
                break
            elif "names" not in info or not isinstance(info["names"], list):
                errors.append("gridlabd_infos object must contain a names: list[str] key")
                break
            else:
                gridlabd_info = GridlabdInfo(info["bus_id"])
                for name in info["names"]:
                    if not isinstance(name, str):
                        errors.append(f"gridlabd_info with bus_id '{gridlabd_info.bus_id}' "
                                      + f"names key must contain only strings. Bad Value: '{str(name)}'")
                    else:
                        gridlabd_info.names.append(name)
                gridlabd_infos.append(gridlabd_info)

        return gridlabd_infos

    def __init__(self, json_dict: dict, total_time: float) -> None:
        self.name: str = ""
        self.local_log_file: str = ""
        self.core_type: str = ""
        self.core_init: str = ""
        self.log_level: str = ""
        self.ln_magnitude: float = 0.0
        self.gridlabd_infos: list = []
        self.total_time = total_time

        errs = list[str]()

        self.name = self._validate_config(json_dict, "name", str, errs)
        self.local_log_file = self._validate_config(json_dict, "local_log_file", str, errs)
        self.core_type = self._validate_config(json_dict, "core_type", str, errs)
        self.core_init = self._validate_config(json_dict, "core_init", str, errs)
        self.log_level = self._validate_config(json_dict, "log_level", str, errs)
        self.ln_magnitude = self._validate_config(json_dict, "ln_magnitude", float, errs)
        self.gridlabd_infos = self._validate_gridlabd_infos(json_dict, errs)

        if errs:
            err_msgs = str.join("\n", errs)
            raise ValueError(f"Errors parsing configuration:\n{err_msgs}")

@dataclasses.dataclass
class FederateFiles:
    raw_file: pathlib.Path
    xml_file: pathlib.Path
    exec_file: pathlib.Path
    json_file: pathlib.Path
    readme_file: pathlib.Path

class IEEE118FederatePlugin(interface.IDeployable):
    @classmethod
    def _get_specific_path(cls) -> pathlib.Path:
        return pathlib.Path("gridpack", "IEEE-118")

    def _get_install_path(self, install_root: str) -> pathlib.Path:
        return pathlib.Path(install_root) / "federate" / self._get_specific_path()

    def _get_model_path(self, deploy_root: str, model_name: str) -> pathlib.Path:
        return pathlib.Path(deploy_root) / self._get_specific_path() / model_name

    def _get_federate_files(self, root: pathlib.Path) -> FederateFiles:
        path = pathlib.Path(root)
        return FederateFiles(
            path / "118.raw",
            path / "118.xml",
            path / "ieee-118-gridpack-federate",
            path / "helics_setup.json",
            path / "README.md"
        )

    def _get_federate_files_list(self, root: pathlib.Path) -> list[str]:
        model_files = self._get_federate_files(root)
        return [
            str(model_files.raw_file),
            str(model_files.xml_file),
            str(model_files.exec_file),
            str(model_files.json_file),
            str(model_files.readme_file)
        ]

    def _update_json_config(self, json_data: dict, input_data: InputData) -> dict:
        config = json_data

        config["fed_info_json"]["coreInit"] = input_data.core_init
        config["fed_info_json"]["coreType"] = input_data.core_type
        config["fed_info_json"]["log_level"] = input_data.log_level

        config["gridpack_name"] = input_data.name
        config["local_log_file"] = input_data.local_log_file
        config["ln_magnitude"] = input_data.ln_magnitude
        config["total_time"] = input_data.total_time

        gridlabd_infos = list()
        for gridlabd_info in input_data.gridlabd_infos:
            gridlabd_infos.append({
                "bus_id": gridlabd_info.bus_id,
                "names": gridlabd_info.names
            })
        config["gridlabd_infos"] = gridlabd_infos

        return config

    def deploy(self, json_config: dict, total_time_seconds: float, deploy_root: str, install_root: str) -> None:
        input_data = InputData(json_config, total_time_seconds)

        # Raise a ValueError if the path does not exist.
        install_path = self._get_install_path(install_root)
        if (not install_path.exists()):
            raise ValueError(f"Install path does not exist: '{install_path}'")
        baseline_files = self._get_federate_files(install_path)

        # If this deploy path does not exist
        model_path = self._get_model_path(deploy_root, input_data.name)
        model_path.mkdir(parents=True, exist_ok=True)
        model_files = self._get_federate_files(model_path)

        # Copy baseline files to models
        shutil.copy2(baseline_files.raw_file, model_files.raw_file)
        shutil.copy2(baseline_files.xml_file, model_files.xml_file)
        shutil.copy2(baseline_files.exec_file, model_files.exec_file)
        shutil.copy2(baseline_files.readme_file, model_files.readme_file)

        # Update the JSON file
        with open(baseline_files.json_file, "r") as baseline_json_file:
            baseline_json = json.load(baseline_json_file)
        updated_json = self._update_json_config(baseline_json, input_data)
        with open(model_files.json_file, "w") as model_json_file:
            json.dump(updated_json, model_json_file, indent=4)

    def get_baseline_files(self, install_root: str) -> list[str]:
        install_path = self._get_install_path(install_root)
        if (not install_path.exists()):
            raise ValueError(f"Install path does not exist: '{install_path}'")

        return self._get_federate_files_list(install_path)

    def get_model_files(self, model_name: str, deploy_root: str) -> list[str]:
        model_path = self._get_model_path(deploy_root, model_name)
        if model_path.exists():
            return self._get_federate_files_list(model_path)
        else:
            return list[str]()

    def get_exec_json(self, model_name: str, deploy_root: str) -> typing.Dict[str, str]:
        relative_working_directory = self._get_specific_path() / model_name
        absolute_working_directory = pathlib.Path(deploy_root) / relative_working_directory

        json_definition = dict()
        if absolute_working_directory.exists():
            json_definition["directory"] = str(relative_working_directory)
            json_definition["exec"] = "/bin/sh -c './ieee-118-gridpack-federate helics_setup.json'"
            json_definition["host"] = "localhost"
            json_definition["name"] = model_name

        return json_definition

    def get_name(self) -> str:
        return "gridpack/IEEE-118"

    def list_model_names(self, deploy_root: str) -> list[str]:
        model_names = list[str]()

        models_root_dir = pathlib.Path(deploy_root) / self._get_specific_path()
        if (models_root_dir.exists()):
            for model_dir in models_root_dir.iterdir():
                if (model_dir.is_dir()):
                    model_names.append(model_dir.name)

        return model_names