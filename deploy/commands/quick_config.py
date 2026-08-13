import json
import pathlib
import typing
import itertools

T = typing.TypeVar("T")

class GridlabdInput:
    def __init__(self, input: dict):
        self.gridlabd_type: str = ""
        self.number_instances: int = 0

        self.gridlabd_type = _validate_config(input, "gridlabd_type", str)
        self.number_instances = _validate_config(input, "number_instances", int)

class GridpackGridlabdInfo:
    def __init__(self, name: str, bus_id: int):
        self.name: str = name
        self.bus_id: int = bus_id

class GridpackInstance:
    def __init__(self):
        self.gridpack_type: str = ""
        self.name: str = ""
        self.core_type: str = ""
        self.broker_port: int = 0
        self.broker_address: str = ""
        self.period: float = 0.0
        self.ln_magnitude: float = 0.0
        self.gridlabds: list[GridpackGridlabdInfo] = []

class GridlabdInstance:
    def __init__(self):
        self.gridlabd_type: str = ""
        self.name: str = ""
        self.core_type: str = ""
        self.broker_port: int = 0
        self.broker_address: str = ""
        self.period: float = 0.0
        self.gridpack_name: str = ""

def quick_deploy(json_file: pathlib.Path, cosim_name: str, output_dir: pathlib.Path, create_output_dir: bool) -> str:
    if not json_file.exists():
        return f"Given json file {json_file if json_file is not None else 'NONE'} does not exist!"

    if not cosim_name:
        return "No cosim name was given!"

    if output_dir is None:
        return "Given output directory is NONE!"

    if not create_output_dir:
        if not output_dir.exists():
            return f"Given output directory {output_dir} does not exist and create_output_dir is set to false!"

        if not output_dir.is_dir():
            return f"Given output directory {output_dir} is not a directory!"
    else:
        output_dir.mkdir(parents=True, exist_ok=True)

        if not output_dir.is_dir():
            return f"Given output directory was created but {output_dir} is not a directory!"

    with open(json_file, "r") as f:
        input = json.load(f)

    try:
        total_time_seconds = _validate_config(input, "total_time_seconds", float)

        bus_ids = _validate_config_list(input, "bus_ids", int)
        core_type = _validate_config(input, "core_type", str)
        broker_address = _validate_config(input, "broker_address", str)
        broker_port = _validate_config(input, "broker_port", int)
        period = _validate_config(input, "period", float)
        ln_magnitude = _validate_config(input, "ln_magnitude", float)

        gridpack_instance = GridpackInstance()
        gridpack_instance.gridpack_type = _validate_config(input, "gridpack_type", str)
        gridpack_instance.name = "GP1"
        gridpack_instance.core_type = core_type
        gridpack_instance.broker_address = broker_address
        gridpack_instance.broker_port = broker_port
        gridpack_instance.period = period
        gridpack_instance.ln_magnitude = ln_magnitude

        gridlabd_instances = list[GridlabdInstance]()
        gridlabd_inputs = _validate_config_list(input, "gridlabd_infos", dict)
        for gridlabd_input in gridlabd_inputs:
            glabd = GridlabdInput(gridlabd_input)

            for i in range(1, glabd.number_instances + 1):
                glabd_instance = GridlabdInstance()
                gridlabd_instance_number = len(gridlabd_instances) + 1

                glabd_instance.gridlabd_type = glabd.gridlabd_type
                glabd_instance.name = f"GLD{gridlabd_instance_number}"
                glabd_instance.core_type = core_type
                glabd_instance.broker_port = broker_port
                glabd_instance.broker_address = broker_address
                glabd_instance.period = period
                glabd_instance.gridpack_name = gridpack_instance.name

                gridlabd_instances.append(glabd_instance)

        # Create Gridlabd Components
        gridlabd_component_list = list[dict]()
        bus_ids_cycle = itertools.cycle(bus_ids) # This will continually loop over the list, yielding one item at a time.
        for gridlabd_instance in gridlabd_instances:
            # First, add instance to the gridpack list
            gridpack_instance.gridlabds.append(GridpackGridlabdInfo(gridlabd_instance.name, next(bus_ids_cycle)))

            # Next, setup the options for the instance
            options = dict()
            options["name"] = gridlabd_instance.name
            options["local_log_file"] = f"{options['name']}.log"
            options["core_init"] = "--federates=1"
            options["core_type"] = gridlabd_instance.core_type
            options["broker"] = gridlabd_instance.broker_address
            options["broker_port"] = gridlabd_instance.broker_port
            options["period"] = gridlabd_instance.period
            options["subscription_name"] = gridlabd_instance.gridpack_name
            options["is_three_part"] = True

            # Next, add instance to the component list
            component = dict()
            component["type"] = gridlabd_instance.gridlabd_type
            component["options"] = options

            # Finally, add it back to the components list
            gridlabd_component_list.append(component)

        # Create list of Gridpack Gridlabd Infos
        gridpack_gridlabd_infos_map = dict()
        for info in gridpack_instance.gridlabds:
            if info.bus_id not in gridpack_gridlabd_infos_map:
                gridpack_gridlabd_infos_map[info.bus_id] = list()

            gridpack_gridlabd_infos_map[info.bus_id].append(info.name)
        gridlabd_infos_list = list[dict]()
        for bus_id, names in gridpack_gridlabd_infos_map.items():
            gridlabd_infos_list.append({"bus_id": bus_id, "names": names})

        # Create Gridpack Component
        gridpack_options = dict()
        gridpack_options["name"] = gridpack_instance.name
        gridpack_options["local_log_file"] = f"{gridpack_options['name']}.log"
        gridpack_options["core_init"] = "--federates=1"
        gridpack_options["core_type"] = gridpack_instance.core_type
        gridpack_options["broker"] = gridpack_instance.broker_address
        gridpack_options["broker_port"] = gridpack_instance.broker_port
        gridpack_options["period"] = gridpack_instance.period
        gridpack_options["log_level"] = "debug"
        gridpack_options["ln_magnitude"] = gridpack_instance.ln_magnitude
        gridpack_options["gridlabd_infos"] = gridlabd_infos_list

        # Create top-level component
        top_level_component = dict()
        top_level_component["type"] = gridpack_instance.gridpack_type
        top_level_component["options"] = gridpack_options
        top_level_component["components"] = gridlabd_component_list

        # Create deploy json
        deploy = dict()
        deploy["cosim_name"] = cosim_name
        deploy["total_time_seconds"] = total_time_seconds
        deploy["components"] = [top_level_component]

        output_file = output_dir / f"{deploy['cosim_name']}.json"
        with open(output_file, "w") as f:
            json.dump(deploy, f, indent=4)

        return f"Wrote output to {str(output_file)}."

    except RuntimeError as e:
        return str(e)

def _validate_config(json_dict: dict, key: str, expected_type: typing.Type[T]) -> T:
    value = expected_type()

    if not key in json_dict:
        raise RuntimeError(f"Key {key} not found!")
    elif not isinstance(json_dict[key], expected_type):
        raise RuntimeError(f"Key {key} found, but the type is incorrect. Expected '{expected_type.__name__}', Actual: '{type(json_dict[key]).__name__}'")
    else:
        value = json_dict[key]

    return value

def _validate_config_list(json_dict: dict, key: str, expected_inner_type: typing.Type[T]) -> list[T]:
    value = list[expected_inner_type]()

    if not key in json_dict:
        raise RuntimeError(f"Key {key} not found!")

    value = json_dict[key]
    if not isinstance(value, list):
        raise RuntimeError(f"Key {key} found but was expected to be a list!")

    if not value:
        raise RuntimeError(f"Key {key} found as a list but it should not be empty!")

    for v in value:
        if not isinstance(v, expected_inner_type):
            raise RuntimeError(f"Key {key} found, but the list contains an incorrect type. Expected '{expected_inner_type.__name__}', Actual: '{type(v).__name__}'")

    return value
