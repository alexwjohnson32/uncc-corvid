import helics

import time
import os
import json
import argparse

from dataclasses import dataclass

@dataclass
class SubscriptionDetails:
    name: str
    units: str

class Stopwatch:
    def __init__(self):
        self.start_time = None

    def start(self):
        self.start_time = time.perf_counter()

    def get_elapsed_ms(self):
        current = time.perf_counter()
        elapsed = current - self.start_time
        return elapsed * 1000

def get_helics_federate(config):
    federate_name = config["name"]
    core_type = "zmq"
    if "core_type" in config.keys():
        core_type = config["core_type"]
    core_init_string = "--federates=1"
    if "core_init_string" in config.keys():
        core_init_string = config["core_init_string"]
    time_property = 1.0
    if "time_property" in config.keys():
        time_property = config["time_property"]

    fed_info = helics.helicsCreateFederateInfo()

    helics.helicsFederateInfoSetCoreName(fed_info, federate_name + "_core")
    helics.helicsFederateInfoSetCoreTypeFromString(fed_info, core_type)
    helics.helicsFederateInfoSetCoreInitString(fed_info, core_init_string)
    helics.helicsFederateInfoSetTimeProperty(fed_info, helics.helics_property_time_delta, time_property)

    return helics.helicsCreateMessageFederate(federate_name, fed_info)

def debug_time_query_loop(granted_time, federate):
    loop_watch = Stopwatch()

    output_str = []
    output_str.append("\n###################\n")
    output_str.append(f"Granted Time: {granted_time}\n")

    loop_watch.start()

    debugging_time_query = helics.helicsCreateQuery("root", "global_time_debugging")
    debugging_time_value = helics.helicsQueryExecute(debugging_time_query, federate)
    output_str.append(json.dumps(debugging_time_value))

    loop_execution_ms = loop_watch.get_elapsed_ms()
    output_str.append(f"\nQuery Execution Time: {loop_execution_ms} ms\n")
    output_str.append("###################\n")

    return "".join(output_str)


def discrete_loops(granted_time, federate):
    loop_watch = Stopwatch()

    output_str = []
    output_str.append("\n###################\n")
    output_str.append(f"Granted Time: {granted_time}\n")

    loop_watch.start()

    name_query = helics.helicsCreateQuery("root", "name")
    name_value = helics.helicsQueryExecute(name_query, federate)
    output_str.append(f"Name: {name_value}\n")

    address_query = helics.helicsCreateQuery("root", "address")
    address_value = helics.helicsQueryExecute(address_query, federate)
    output_str.append(f"Address: {address_value}\n")

    is_init_query = helics.helicsCreateQuery("root", "isinit")
    is_init_value = helics.helicsQueryExecute(is_init_query, federate)
    output_str.append(f"IsInit: {is_init_value}\n")

    is_connected_query = helics.helicsCreateQuery("root", "isconnected")
    is_connected_value = helics.helicsQueryExecute(is_connected_query, federate)
    output_str.append(f"IsConnected: {is_connected_value}\n")

    loop_execution_ms = loop_watch.get_elapsed_ms()
    output_str.append(f"Query Execution Time: {loop_execution_ms} ms\n")
    output_str.append("###################\n")

    return "".join(output_str)


def perform_loop(federate, total_time, period):
    with open("query-federate.log", "w") as output_file:
        main_watch = Stopwatch()
        granted_time = 0.0

        main_watch.start()
        while granted_time + period <= total_time:
            granted_time = helics.helicsFederateRequestTime(federate, granted_time + period)

            # output_string = debug_time_query_loop(granted_time, federate)
            output_string = discrete_loops(granted_time, federate)

            output_file.write(output_string)

        main_loop_ms = main_watch.get_elapsed_ms()
        output_str = []
        output_str.append("\n###################\n")
        output_str.append(f"Total Loop Time: {main_loop_ms} ms\n")
        output_str.append("\n###################\n")
        info_string = "".join(output_str)
        output_file.write(info_string)

def execute_federate(json_input):
    if json_input is None or not isinstance(json_input, str):
        return

    path_to_config = os.path.abspath(json_input)
    if not os.path.exists(path_to_config):
        print("Unable to find file '{}'".format(path_to_config))
        print("Returning...")
        return

    with open(path_to_config) as json_file:
        config = json.loads(json_file.read())

    federate = get_helics_federate(config)

    total_time = config["total_time"]
    period = 1.0
    if "period" in config.keys():
        period = config["period"]

    # Sleep for a few seconds to ensure the cosim is fully setup (this is recommended...booo)
    time.sleep(5)

    try:
        helics.helicsFederateEnterExecutingMode(federate)
        perform_loop(federate, total_time, period)
    finally:
        helics.helicsFederateFinalize(federate)
        helics.helicsCloseLibrary()

def main():
    parser = argparse.ArgumentParser(description="A federate to run queries")
    parser.add_argument("json_config_file", help="Path to the json configuration file")

    args = parser.parse_args()

    execute_federate(args.json_config_file)

if __name__ == "__main__":
    main()