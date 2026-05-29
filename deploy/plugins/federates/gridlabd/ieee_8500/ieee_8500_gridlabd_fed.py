import plugins.interface as interface
import typing
import json
import pathlib
import shutil
import dataclasses
import copy
import datetime

T = typing.TypeVar("T")

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

    def __init__(self, json_dict: dict) -> None:
        self.name: str = ""
        self.local_log_file: str = ""
        self.core_type: str = ""
        self.core_init: str = ""
        self.three_part_subscription_name: str = ""

        errs = list[str]()

        self.name = self._validate_config(json_dict, "name", str, errs)
        self.local_log_file = self._validate_config(json_dict, "local_log_file", str, errs)
        self.core_type = self._validate_config(json_dict, "core_type", str, errs)
        self.core_init = self._validate_config(json_dict, "core_init", str, errs)
        self.three_part_subscription_name = self._validate_config(
            json_dict, "three_part_subscription_name", str, errs
        )

        if errs:
            err_msgs = str.join("\n", errs)
            raise ValueError(f"Errors parsing configuration:\n{err_msgs}")

@dataclasses.dataclass
class BaselineFiles:
    glm_file: pathlib.Path
    json_config: pathlib.Path
    readme: pathlib.Path

@dataclasses.dataclass
class ModelFiles:
    baseline_glm_file: pathlib.Path
    model_glm_file: pathlib.Path
    json_config: pathlib.Path
    readme: pathlib.Path

class IEEE8500FederatePlugin(interface.IDeployable):
    @classmethod
    def _get_specific_path(cls) -> pathlib.Path:
        return pathlib.Path("gridlabd", "IEEE-8500")

    def _get_install_path(self, install_root: str) -> pathlib.Path:
        return pathlib.Path(install_root) / "federate" / self._get_specific_path()

    def _get_model_path(self, deploy_root: str, model_name: str) -> pathlib.Path:
        return pathlib.Path(deploy_root) / self._get_specific_path() / model_name

    def _get_baseline_files(self, install_root: pathlib.Path) -> BaselineFiles:
        return BaselineFiles(
            install_root / "baseline_IEEE_8500.glm",
            install_root / "IEEE_8500node.json",
            install_root / "README.md"
        )

    def _get_model_files(self, model_path: pathlib.Path) -> ModelFiles:
        return ModelFiles(
            model_path.parent / "baseline_IEEE_8500.glm",
            model_path / "IEEE_8500node.glm",
            model_path / "IEEE_8500node.json",
            model_path.parent / "README.md"
        )

    def _update_json_config(self, json_data: dict, input_data: InputData) -> dict:
        # This copies the value, creating a local version of the variable
        json_data = copy.deepcopy(json_data)

        # update basic data
        json_data["coreInit"] = input_data.core_init
        json_data["coreType"] = input_data.core_type
        json_data["name"] = input_data.name
        json_data["logfile"] = input_data.local_log_file

        # update three-part publication
        publications = json_data["publications"]
        publications[0]["key"] = f"{input_data.name}/Sa"
        publications[1]["key"] = f"{input_data.name}/Sb"
        publications[2]["key"] = f"{input_data.name}/Sc"

        # update three-part subscription
        subscriptions = json_data["subscriptions"]
        subscriptions[0]["key"] = f"{input_data.three_part_subscription_name}/Va"
        subscriptions[1]["key"] = f"{input_data.three_part_subscription_name}/Vb"
        subscriptions[2]["key"] = f"{input_data.three_part_subscription_name}/Vc"

        return json_data

    def _get_model_glm_string(self, baseline_file_path: pathlib.Path, name: str, total_time_seconds: float) -> str:
        # get datetime setup
        now_obj = datetime.datetime.now()
        start_time_obj = now_obj.replace(year=(now_obj.year - 1), minute=0, second=0)
        start_time_str = start_time_obj.strftime("%Y-%m-%d %H:%M:%S")
        stop_time_obj = start_time_obj + datetime.timedelta(seconds=total_time_seconds)
        stop_time_str = stop_time_obj.strftime("%Y-%m-%d %H:%M:%S")

        return f"""#include "{baseline_file_path}"

object helics_msg {{
    name {name};
    configure {name}.json;
}}

clock {{
    timezone CST+6CDT;
    starttime '{start_time_str}';
    stoptime '{stop_time_str}';
}}"""

    def deploy(self, json_config: dict, total_time_seconds: float, deploy_root: str, install_root: str) -> None:
        input_data = InputData(json_config)

        # Raise a ValueError if the path does not exist.
        install_path = self._get_install_path(install_root)
        if (not install_path.exists()):
            raise ValueError(f"Install path does not exist: '{install_path}'")
        baseline_files = self._get_baseline_files(install_path)

        # If this deploy path does not exist
        model_path = self._get_model_path(deploy_root, input_data.name)
        model_path.mkdir(parents=True, exist_ok=True)
        model_files = self._get_model_files(model_path)

        # Copy baseline glm and readme
        shutil.copy2(baseline_files.glm_file, model_files.baseline_glm_file)
        shutil.copy2(baseline_files.readme, model_files.readme)

        # Update baseline json
        with open(baseline_files.json_config, "r") as baseline_json_file:
            baseline_json = json.load(baseline_json_file)
        updated_json = self._update_json_config(baseline_json, input_data)
        with open(model_files.json_config, "w") as model_json_file:
            json.dump(updated_json, model_json_file, indent=4)

        # Update the model glm
        model_glm_string = self._get_model_glm_string(baseline_files.glm_file, input_data.name, total_time_seconds)
        with open(model_files.model_glm_file, "w") as model_glm_file:
            model_glm_file.write(model_glm_string)

    def get_baseline_files(self, install_root: str) -> list[str]:
        install_path = self._get_install_path(install_root)
        if (not install_path.exists()):
            raise ValueError(f"Install path does not exist: '{install_path}'")

        baseline_files = self._get_baseline_files(install_path)
        return [
            str(baseline_files.glm_file),
            str(baseline_files.json_config),
            str(baseline_files.readme)
        ]

    def get_model_files(self, model_name: str, deploy_root: str) -> list[str]:
        model_path = self._get_model_path(deploy_root, model_name)
        if model_path.exists():
            model_files = self._get_model_files(model_path)
            return [
                str(model_files.baseline_glm_file),
                str(model_files.model_glm_file),
                str(model_files.json_config),
                str(model_files.readme)
            ]
        else:
            return list[str]()

    def get_exec_json(self, model_name: str, deploy_root: str) -> typing.Dict[str, str]:
        relative_working_directory = self._get_specific_path() / model_name
        absolute_working_directory = pathlib.Path(deploy_root) / relative_working_directory

        json_definition = dict()
        if absolute_working_directory.exists():
            json_definition["directory"] = str(relative_working_directory)
            json_definition["exec"] = f"gridlabd.sh {model_name}.glm"
            json_definition["host"] = "localhost"
            json_definition["name"] = model_name

        return json_definition

    def get_name(self) -> str:
        return "gridlabd/IEEE-8500"

    def list_model_names(self, deploy_root: str) -> list[str]:
        model_names = list[str]()

        models_root_dir = pathlib.Path(deploy_root) / self._get_specific_path()
        if (models_root_dir.exists()):
            for model_dir in models_root_dir.iterdir():
                if (model_dir.is_dir()):
                    model_names.append(model_dir.name)

        return model_names