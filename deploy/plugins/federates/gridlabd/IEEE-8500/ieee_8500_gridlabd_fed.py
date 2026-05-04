import plugins.interface as interface
import typing
import json
import pathlib
import shutil
import dataclasses

class IEEE8500FederatePlugin(interface.IDeployable):
    @classmethod
    def _get_specific_path(cls) -> pathlib.Path:
        return pathlib.Path("gridlabd", "IEEE-8500")

    def _get_install_path(self, install_root: str) -> pathlib.Path:
        return pathlib.Path(install_root) / self._get_specific_path()

    def _get_deploy_path(self, deploy_root: str, model_name: str) -> pathlib.Path:
        return pathlib.Path(deploy_root) / self._get_specific_path() / model_name

    def deploy(self, json_config: dict, total_time_seconds: float, deploy_root: str, install_root: str) -> None:
        return super().deploy(json_config, total_time_seconds, deploy_root, install_root)

    def get_baseline_files(self, install_root: str) -> list[str]:
        return super().get_baseline_files(install_root)

    def get_model_files(self, model_name: str, deploy_root: str) -> list[str]:
        deploy_path = self._get

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