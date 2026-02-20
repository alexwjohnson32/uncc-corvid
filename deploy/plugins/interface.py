import abc
import typing

class IDeployable(abc.ABC):
    @abc.abstractmethod
    def deploy(self, json_config: dict, deploy_root: str, install_root: str) -> bool:
        """Writes the plugin info to the corresponding deploy directory."""
        pass

    @abc.abstractmethod
    def get_baseline_files(self, install_root: str) -> list[str]:
        """Gets a list of filepaths for the baseline files. These files should NOT be modified by the user."""
        pass

    @abc.abstractmethod
    def get_model_files(self, model_name: str, deploy_root: str) -> list[str]:
        """Gets a list of filepaths that have been modified for model within the given deploy directory.
        You may change any values in these files, but this does not guarantee that the cosim will still work."""
        pass

    @abc.abstractmethod
    def get_exec_json(self, model_name: str, deploy_root: str) -> typing.Dict[str, typing.Any]:
        """Returns a JSON object that represents a valid HELICS Runner file federate object.
        Should contain the fields directory, exec, host, name."""
        pass

    @abc.abstractmethod
    def get_name(self) -> str:
        """Returns the name of the Plugin (NOT the model). This is the unique identifier for the plugin.
        Be careful to give a specific enough name to prevent name collisions."""
        pass