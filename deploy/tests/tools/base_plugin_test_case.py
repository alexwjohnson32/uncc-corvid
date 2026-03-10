import unittest
import pathlib
import abc

class BasePluginTestCase(unittest.TestCase):
    install_dir: pathlib.Path = pathlib.Path()
    deploy_dir: pathlib.Path = pathlib.Path()

    @classmethod
    @abc.abstractmethod
    def _get_install_path(cls) -> pathlib.Path:
        """Return the full path of the expected install directory"""
        pass

    @classmethod
    @abc.abstractmethod
    def _get_safe_deployed_path(cls) -> pathlib.Path:
        """Gets the directory path that should contain the deployed files. This should utilize
        the get_safe_model_name() function and should not be expected for actual paths."""
        pass

    @classmethod
    @abc.abstractmethod
    def _init_deployed_files(cls, deployed_path: pathlib.Path) -> None:
        """Write thin deploy files to the deploy path, which is necessary since some of the
        tested functions depend on a directory to be deployed before call time. Initialize the
        deploy directory with the safe model name given from get_safe_model_name"""
        pass

    @classmethod
    def get_safe_model_name(cls) -> str:
        return f"{cls.__name__}_model_name"

    @classmethod
    def validate_install_dir(cls) -> None:
        install_path = cls._get_install_path()
        print(f"Install Dir: {install_path}")
        if not install_path.exists():
            raise cls.failureException(f"Install Dir does not exist: {install_path}")

    @classmethod
    def validate_deploy_dir(cls) -> None:
        deploy_path = cls._get_safe_deployed_path()
        print(f"Deploy Dir: {deploy_path}")

        deploy_path.mkdir(parents=True, exist_ok=True)
        cls._init_deployed_files(deploy_path)

    @classmethod
    def setUpClass(cls) -> None:
        print("-" * 40)
        print(f"Starting {cls.__name__}")
        print(f"{cls.__name__} Install Dir: {cls.install_dir}")
        cls.validate_install_dir()
        cls.validate_deploy_dir()
        print("-" * 40)