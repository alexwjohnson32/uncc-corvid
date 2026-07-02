import plugins.interface as interface
import typing
import json
import pathlib
import shutil
import dataclasses
import copy

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
        self.broker: str = ""
        self.broker_port: int = 0
        self.period: float = 1.0
        self.log_level: str = ""
        self.ln_magnitude: float = 0.0
        self.gridlabd_infos: list = []
        self.total_time: float = total_time

        errs = list[str]()

        self.name = self._validate_config(json_dict, "name", str, errs)
        self.local_log_file = self._validate_config(json_dict, "local_log_file", str, errs)
        self.core_type = self._validate_config(json_dict, "core_type", str, errs)
        self.core_init = self._validate_config(json_dict, "core_init", str, errs)
        self.broker = self._validate_config(json_dict, "broker", str, errs)
        self.broker_port = self._validate_config(json_dict, "broker_port", int, errs)
        self.log_level = self._validate_config(json_dict, "log_level", str, errs)
        self.ln_magnitude = self._validate_config(json_dict, "ln_magnitude", float, errs)
        self.gridlabd_infos = self._validate_gridlabd_infos(json_dict, errs)

        if "period" in json_dict and isinstance(json_dict["period"], float):
            self.period = json_dict["period"]

        if errs:
            err_msgs = str.join("\n", errs)
            raise ValueError(f"Errors parsing configuration:\n{err_msgs}")

@dataclasses.dataclass
class PhaseFiles:
    exec_file: pathlib.Path
    readme_file: pathlib.Path
    raw_file: pathlib.Path
    xml_file: pathlib.Path
    json_file: pathlib.Path

@dataclasses.dataclass
class FederateFiles:
    phase_a: PhaseFiles
    phase_b: PhaseFiles
    phase_c: PhaseFiles

class IEEE3FederatePlugin(interface.IDeployable):
    @classmethod
    def _get_federate_deploy_path(cls) -> pathlib.Path:
        return pathlib.Path("gridpack", "IEEE-3")

    def _get_install_path(self, install_root: str) -> pathlib.Path:
        return pathlib.Path(install_root) / "federate" / "gridpack" / "one_phase"

    def _get_model_root(self, deploy_root: str) -> pathlib.Path:
        return pathlib.Path(deploy_root) / self._get_federate_deploy_path()

    def _get_model_path(self, model_root: pathlib.Path, model_name: str, phase_name: str) -> pathlib.Path:
        return model_root / f"{model_name}_{phase_name}"

    def _get_baseline_files(self, root: pathlib.Path) -> FederateFiles:
        return FederateFiles(
            PhaseFiles(
                root / "one-phase-gridpack-federate",
                root / "README.md",
                root / "IEEE3_phase_A.raw",
                root / "input_3_bus_phase_A.xml",
                root / "helics_setup_phase_a.json"
            ),
            PhaseFiles(
                root / "one-phase-gridpack-federate",
                root / "README.md",
                root / "IEEE3_phase_B.raw",
                root / "input_3_bus_phase_B.xml",
                root / "helics_setup_phase_b.json"
            ),
            PhaseFiles(
                root / "one-phase-gridpack-federate",
                root / "README.md",
                root / "IEEE3_phase_C.raw",
                root / "input_3_bus_phase_C.xml",
                root / "helics_setup_phase_c.json"
            )
        )

    def _get_model_files(self, deploy_root: str, model_name: str) -> FederateFiles:
        model_root = self._get_model_root(deploy_root)

        phase_a_path = self._get_model_path(model_root, model_name, "a")
        phase_b_path = self._get_model_path(model_root, model_name, "b")
        phase_c_path = self._get_model_path(model_root, model_name, "c")

        phase_a_path.mkdir(parents=True, exist_ok=True)
        phase_b_path.mkdir(parents=True, exist_ok=True)
        phase_c_path.mkdir(parents=True, exist_ok=True)

        return FederateFiles(
            PhaseFiles(
                phase_a_path / "one-phase-gridpack-federate",
                phase_a_path / "README.md",
                phase_a_path / "IEEE3_phase_A.raw",
                phase_a_path / "input_3_bus_phase_A.xml",
                phase_a_path / "helics_setup_phase_a.json"
            ),
            PhaseFiles(
                phase_b_path / "one-phase-gridpack-federate",
                phase_b_path / "README.md",
                phase_b_path / "IEEE3_phase_B.raw",
                phase_b_path / "input_3_bus_phase_B.xml",
                phase_b_path / "helics_setup_phase_b.json"
            ),
            PhaseFiles(
                phase_c_path / "one-phase-gridpack-federate",
                phase_c_path / "README.md",
                phase_c_path / "IEEE3_phase_C.raw",
                phase_c_path / "input_3_bus_phase_C.xml",
                phase_c_path / "helics_setup_phase_c.json"
            )
        )

    def _to_files_list(self, model_files: FederateFiles | PhaseFiles) -> list[str]:
        if (isinstance(model_files, FederateFiles)):
            return [
                str(model_files.phase_a.exec_file),
                str(model_files.phase_a.readme_file),
                str(model_files.phase_a.raw_file),
                str(model_files.phase_a.xml_file),
                str(model_files.phase_a.json_file),
                str(model_files.phase_b.exec_file),
                str(model_files.phase_b.readme_file),
                str(model_files.phase_b.raw_file),
                str(model_files.phase_b.xml_file),
                str(model_files.phase_b.json_file),
                str(model_files.phase_c.exec_file),
                str(model_files.phase_c.readme_file),
                str(model_files.phase_c.raw_file),
                str(model_files.phase_c.xml_file),
                str(model_files.phase_c.json_file)
            ]
        elif (isinstance(model_files, PhaseFiles)):
            return [
                str(model_files.exec_file),
                str(model_files.readme_file),
                str(model_files.raw_file),
                str(model_files.xml_file),
                str(model_files.json_file)
            ]
        else:
            return []

    def _update_json_config(self, json_data: dict, input_data: InputData) -> dict:
        config = json_data

        config["fed_info_json"]["coreInit"] = input_data.core_init
        config["fed_info_json"]["coreType"] = input_data.core_type
        config["fed_info_json"]["log_level"] = input_data.log_level
        config["fed_info_json"]["broker"] = input_data.broker
        config["fed_info_json"]["broker_port"] = input_data.broker_port
        config["fed_info_json"]["period"] = input_data.period

        config["federate_name"] = input_data.name
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

    def _copy_files(self, baseline_files: PhaseFiles, installation_files: PhaseFiles, input_data: InputData) -> None:
        shutil.copy2(baseline_files.raw_file, installation_files.raw_file)
        shutil.copy2(baseline_files.xml_file, installation_files.xml_file)
        shutil.copy2(baseline_files.exec_file, installation_files.exec_file)
        shutil.copy2(baseline_files.readme_file, installation_files.readme_file)

        # Update the JSON file
        with open(baseline_files.json_file, "r") as baseline_json_file:
            baseline_json = json.load(baseline_json_file)

        updated_json = self._update_json_config(baseline_json, input_data)
        with open(installation_files.json_file, "w") as model_json_file:
            json.dump(updated_json, model_json_file, indent=4)

    def deploy(self, json_config: dict, total_time_seconds: float, deploy_root: str, install_root: str) -> None:
        input_data = InputData(json_config, total_time_seconds)
        input_data_a = copy.deepcopy(input_data)
        input_data_a.name = f"{input_data_a.name}_a"
        input_data_b = copy.deepcopy(input_data)
        input_data_b.name = f"{input_data_b.name}_b"
        input_data_c = copy.deepcopy(input_data)
        input_data_c.name = f"{input_data_c.name}_c"

        # Raise a ValueError if the path does not exist.
        install_path = self._get_install_path(install_root)
        if (not install_path.exists()):
            raise ValueError(f"Install path does not exist: '{install_path}'")

        baseline_files = self._get_baseline_files(install_path)
        model_files = self._get_model_files(deploy_root, input_data.name) # This needs to be the original model names

        self._copy_files(baseline_files.phase_a, model_files.phase_a, input_data_a)
        self._copy_files(baseline_files.phase_b, model_files.phase_b, input_data_b)
        self._copy_files(baseline_files.phase_c, model_files.phase_c, input_data_c)

    def get_baseline_files(self, install_root: str) -> list[str]:
        install_path = self._get_install_path(install_root)
        if (not install_path.exists()):
            raise ValueError(f"Install path does not exist: '{install_path}'")

        return self._to_files_list(self._get_baseline_files(install_path))

    def get_model_files(self, model_name: str, deploy_root: str) -> list[str]:
        model_path = self._get_model_files(deploy_root, model_name)
        model_paths = list[str]()

        if model_path.phase_a.exec_file.parent.exists():
            model_paths.extend(self._to_files_list(model_path.phase_a))

        if model_path.phase_b.exec_file.parent.exists():
            model_paths.extend(self._to_files_list(model_path.phase_b))

        if model_path.phase_c.exec_file.parent.exists():
            model_paths.extend(self._to_files_list(model_path.phase_c))

        return model_paths

    def get_exec_json(self, model_name: str, deploy_root: str) -> typing.Dict[str, str]:
        relative_working_directory = self._get_federate_deploy_path() / model_name
        absolute_working_directory = pathlib.Path(deploy_root) / relative_working_directory

        json_definition = dict[str, str]()
        if absolute_working_directory.exists():
            helics_config_files = [str(file) for file in absolute_working_directory.glob("helics_setup_*.json") if file.is_file()]

            json_definition["directory"] = str(relative_working_directory)
            json_definition["exec"] = f"/bin/sh -c './one-phase-gridpack-federate {helics_config_files[0] if helics_config_files else str()}'"
            json_definition["host"] = "localhost"
            json_definition["name"] = model_name

        return json_definition

    def get_name(self) -> str:
        return "gridpack/IEEE-3"

    def list_model_names(self, deploy_root: str) -> list[str]:
        model_names = list[str]()

        models_root_dir = self._get_model_root(deploy_root)
        if (models_root_dir.exists()):
            for model_dir in models_root_dir.iterdir():
                if (model_dir.is_dir()):
                    model_names.append(model_dir.name)

        return model_names


