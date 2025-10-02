import helics as h

import os
import json
import sys

import socket

from dataclasses import dataclass

HOST = "127.0.0.1"
PORT = 23555

@dataclass
class SubscriptionDetails:
    name: str
    units: str

def get_helics_federate(fed_name, fed_core_type, fed_core_init_string, fed_time_property):
    if not isinstance(fed_name, str):
        return None

    fed_info = h.helicsCreateFederateInfo()

    h.helicsFederateInfoSetCoreName(fed_info, fed_name + "_core")
    h.helicsFederateInfoSetCoreTypeFromString(fed_info, fed_core_type)
    h.helicsFederateInfoSetCoreInitString(fed_info, fed_core_init_string)
    h.helicsFederateInfoSetTimeProperty(fed_info, h.helics_property_time_delta, fed_time_property)

    return h.helicsCreateValueFederate(fed_name, fed_info)

def register_subscriptions(federate, subscription_details = []):
    subscriptions = []

    if federate is None or not isinstance(federate, h.HelicsFederate):
        return subscriptions

    if subscription_details is None or not isinstance(subscription_details, list):
        return subscriptions

    for sub_detail in subscription_details:
        sub_obj = h.helicsFederateRegisterSubscription(federate, sub_detail.name, sub_detail.units)
        subscriptions.append(sub_obj)

    return subscriptions

def perform_loop(federate, total_time, period, helics_subs, s):
    granted_time = 0.0
    with open("tracer.log", "w") as output_file:
        while granted_time + period <= total_time:
            granted_time = h.helicsFederateRequestTime(federate, granted_time + period)

            output_str = []
            output_str.append("\n###################\n")
            output_str.append(f"Granted Time: {granted_time}\n")

            for helics_sub in helics_subs:
                if not h.helicsInputIsUpdated(helics_sub) and not h.helicsInputIsValid(helics_sub):
                    continue

                sub_key = h.helicsInputGetKey(helics_sub)
                sub_value = h.helicsInputGetComplex(helics_sub)
                sub_units = h.helicsInputGetUnits(helics_sub)

                output_str.append(f"Key: {sub_key}\t\tValue: {sub_value} {sub_units}\n")

            output_str.append("###################\n")

            info_string = "".join(output_str)
            output_file.write(info_string)
            s.sendall(info_string.encode("utf-8"))

def execute_federate(json_input, s):
    if json_input is None or not isinstance(json_input, str):
        return

    path_to_config = os.path.abspath(json_input)
    if not os.path.exists(path_to_config):
        print("Unable to find file '{}'".format(path_to_config))
        print("Returning...")
        return

    with open(path_to_config) as json_file:
        config = json.loads(json_file.read())

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

    federate = get_helics_federate(federate_name, core_type, core_init_string, time_property)

    subscription_details = []
    for sub_detail in config["subscriptions"]:
        sub = SubscriptionDetails(sub_detail["name"], sub_detail["units"])
        subscription_details.append(sub)

    helics_subs = register_subscriptions(federate, subscription_details)

    total_time = config["total_time"]
    period = 1.0
    if "period" in config.keys():
        period = config["period"]

    try:
        h.helicsFederateEnterExecutingMode(federate)
        perform_loop(federate, total_time, period, helics_subs, s)
    finally:
        h.helicsFederateFinalize(federate)
        h.helicsCloseLibrary()

def main(argv):
    if len(argv) < 2:
        print("Please provide an input json file.")
        return

    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.connect((HOST, PORT))
        execute_federate(argv[1], s)
    return

if __name__ == "__main__":
    main(sys.argv)