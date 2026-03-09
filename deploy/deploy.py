import plugins.debug_tools.dummy_federate.dummy_federate_plugin as plugin

import json
import pathlib

def get_install_path() -> pathlib.Path:
    return pathlib.Path(
        "/home",
        "shelf1",
        "compile",
        "lwilliamson",
        "cosim-web-app",
        "server",
        "uncc-corvid",
        "src",
        "install",
        "debug"
    )

def init_deploy_path(model_name: str) -> pathlib.Path:
    deploy_path = pathlib.Path("temp")
    deployed_path = deploy_path / "debug_tools" / "dummy_federate" / model_name
    deployed_path.mkdir(parents=True, exist_ok=True)

    with open(deployed_path / "dummy_federate", "w") as exec_file:
        exec_file.write("exec file")

    with open(deployed_path / "README.md", "w") as readme_file:
        readme_file.write("dummy")

    json_data = dict()
    json_data["federate_name"] = "test_name"
    json_data["local_log_file"] = "local_file.txt"
    json_data["total_time"] = 60.0
    json_data["fed_info_json"] = dict()
    json_data["fed_info_json"]["coreInit"] = "ipc"
    json_data["fed_info_json"]["coreType"] = "--brokername=mainbroker"

    with open(deployed_path / "helics.json", "w") as helics_file:
        json.dump(json_data, helics_file, indent=4)

    return deploy_path

def main():
    install_path = get_install_path()

    model_name = "test_name"
    deploy_path = init_deploy_path(model_name)

    dummy = plugin.DummyFederatePlugin()
    print(f"get_name(): {dummy.get_name()}")
    print(f"get_exec_json(model_name, deploy_root): {json.dumps(dummy.get_exec_json(model_name, str(deploy_path)))}")
    model_files = str.join("\n\t", dummy.get_model_files(model_name, str(deploy_path)))
    print(f"get_model_files(model_name, deploy_root):\n\t{model_files}")
    baseline_files = str.join("\n\t", dummy.get_baseline_files(str(install_path)))
    print(f"get_baseline_files(install_root):\n\t{baseline_files}")

    # deploy_data init
    deploy_data = dict()
    deploy_data["name"] = "dummy_fed_name"
    deploy_data["local_log_file"] = "local_log.txt"
    deploy_data["total_time"] = 1000.0
    deploy_data["core_type"] = "ipc"
    deploy_data["core_init"] = "--brokername=main_broker"

    print("\n\n\nPerfoming Deploy...")
    dummy.deploy(deploy_data, str(deploy_path), str(install_path))
    print("Deploy Complete!")
    print(f"get_exec_json(model_name, deploy_root): {json.dumps(dummy.get_exec_json(model_name, str(deploy_path)))}")
    model_files = str.join("\n\t", dummy.get_model_files(model_name, str(deploy_path)))
    print(f"get_model_files(model_name, deploy_root):\n\t{model_files}")


if __name__ == '__main__':
    main()