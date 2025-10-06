import helics

import os
import json
import argparse
import http.client

from dataclasses import dataclass

@dataclass
class SubscriptionDetails:
    name: str
    units: str

class PostHelper:
    def __init__(self, hostname, port, path):
        self._hostname = hostname
        self._port = port
        self._path = path
        self._connection = http.client.HTTPConnection(self._hostname, self._port)
        with open("tracer_output.log", "w") as output_log:
            output_log.write("PostHelper")

    def __enter__(self):
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        self.close()
        return False # re-raise exceptions

    def post(self, content):
        if content is None or not isinstance(content, str):
            return

        with open("tracer_output.log", "a") as output_log:
            headers = {
                "Content-Type": "application/json",
                "Content-Length": str(len(content))
            }

            output_log.write(f"Sending Request Body: {content}\nend body")
            self._connection.request("POST", self._path, body=content, headers=headers)
            output_log.write("Sent request, awaiting response...")
            response = self._connection.getresponse()
            output_log.write(f"Status: {response.status}")
            output_log.write(f"Response: {response.read().decode()}")

    def close(self):
        if self._connection is not None:
            self._connection.close()


def get_helics_federate(fed_name, fed_core_type, fed_core_init_string, fed_time_property):
    if not isinstance(fed_name, str):
        return None

    fed_info = helics.helicsCreateFederateInfo()

    helics.helicsFederateInfoSetCoreName(fed_info, fed_name + "_core")
    helics.helicsFederateInfoSetCoreTypeFromString(fed_info, fed_core_type)
    helics.helicsFederateInfoSetCoreInitString(fed_info, fed_core_init_string)
    helics.helicsFederateInfoSetTimeProperty(fed_info, helics.helics_property_time_delta, fed_time_property)

    return helics.helicsCreateValueFederate(fed_name, fed_info)

def register_subscriptions(federate, subscription_details = []):
    subscriptions = []

    if federate is None or not isinstance(federate, helics.HelicsFederate):
        return subscriptions

    if subscription_details is None or not isinstance(subscription_details, list):
        return subscriptions

    for sub_detail in subscription_details:
        sub_obj = helics.helicsFederateRegisterSubscription(federate, sub_detail.name, sub_detail.units)
        subscriptions.append(sub_obj)

    return subscriptions

def perform_loop(federate, total_time, period, helics_subs, post_helper):
    granted_time = 0.0
    with open("tracer.log", "w") as output_file:
        while granted_time + period <= total_time:
            granted_time = helics.helicsFederateRequestTime(federate, granted_time + period)

            output_str = []
            output_str.append("\n###################\n")
            output_str.append(f"Granted Time: {granted_time}\n")

            for helics_sub in helics_subs:
                if not helics.helicsInputIsUpdated(helics_sub) and not helics.helicsInputIsValid(helics_sub):
                    continue

                sub_key = helics.helicsSubscriptionGetKey(helics_sub)
                sub_value = helics.helicsInputGetComplex(helics_sub)
                sub_units = helics.helicsInputGetUnits(helics_sub)

                output_str.append(f"Key: {sub_key}\t\tValue: {sub_value}\t{sub_units}\n")

            output_str.append("###################\n")

            info_string = "".join(output_str)
            output_file.write(info_string)

            post_helper.post(info_string)

def execute_federate(json_input, post_helper):
    if json_input is None or not isinstance(json_input, str):
        return

    if post_helper is None or not isinstance(post_helper, PostHelper):
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
        helics.helicsFederateEnterExecutingMode(federate)
        perform_loop(federate, total_time, period, helics_subs, post_helper)
    finally:
        helics.helicsFederateFinalize(federate)
        helics.helicsCloseLibrary()

def main():
    parser = argparse.ArgumentParser(description="A federate to output desired cosimulation runtime information.")
    parser.add_argument("json_config_file", help="Path to the json configuration file")
    parser.add_argument("--hostname", dest="hostname", help="Hostname to connect to", default="127.0.0.1")
    parser.add_argument("--port", dest="port", help="Port to connect to", default="23555")
    parser.add_argument("--path", dest="path", help="Http request path to send to", default="/ws/logs")

    args = parser.parse_args()

    try:
        with PostHelper(args.hostname, args.port, args.path) as post_helper:
            execute_federate(args.json_config_file, post_helper)
    except ConnectionRefusedError:
        print(f"Connection refused. Ensure the server is running on {args.hostname}:{args.port}.")
    except Exception as e:
        print(f"An error occurred: {e}")

    # with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    #     s.connect((args.hostname, args.port))
    #     execute_federate(argv[1], s)
    return

if __name__ == "__main__":
    main()