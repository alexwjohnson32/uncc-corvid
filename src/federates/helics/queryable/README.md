# Helics Queryable Federate

This is an observable only federate that performs queries on the cosim.

The executable is named `queryable-federate`, and it accepts a json file as input.

## JSON File Inputs

There are a number of inputs that must be set prior to the launch in order for the federate to be considered usable.

* `federate_name`: The name of the federate. This must be consistent across the entirety of the cosim.
* `fed_info_json`: A separate json object that is the actual HELICS json structure, this will be passed to the federate directly. The HELICS json file itself has many options that can be set, but do NOT define the publications and subscriptions here, that is handled later in the config. You must also configure it to run with an `ipc` core to be compliant on the HPC.
* `total_time`: A double precision value that represents (in seconds) the total simulated time of the cosim.
* `local_log_file`: A path to a local log file to write outputs to, in order to keep a record of the model execution. More useful for debugging purposes than data analysis.
* `client_details`: An object that is the location of the listening websocket server that the Query Federate sends messages to. The object consists of a `host`, `port`, and `target`.

## Running

When launching, this should be one of the last federates to launch since it is querying the system. Meaning, it needs every other federate in place in order to get best results if perfomring metadata queries before offically "starting" the cosimulation.

Call this command to launch the model as a HELICS federate. Change filepaths as necessary.

`./queryable-federate query_config.json`

As a side note, if launching as part of a helics runner file, you will probably need to use an exec string like this:

`"/bin/sh -c './queryable-federate query_config.json'"`

## IPC Core Compliance

In order to run on an IPC core, you need to set two fields specifically in the HELICS json (the `fed_info_json` value):
* `"coreType": "ipc"`
* `"coreInit": "--brokername=<name_of_broker> --shared_file=/path/to/socketfile"`
    * The `--brokername` is the exact name that you set in the broker's launch call with the `--name` flag.
    * The `--shared_file` is the exact path that you set in the broker's init string `--shared_file` flag.

For a basic example of the json file, refer to the `helics_setup.json` file.

## Purpose and Future Direction

The original goal of this federate was to provide a way for users to "peek" inside of the cosim while running to see "keep alive" information, potentially even comminucating back to it in order execute requested queries at runtime. Currently, it sends out basic information at each timestep, more of a proof of concept than useful tool.

A lot of the usefulness of this was predicated on being able to utilize a websocket connection that reaches from the HPC to our local machine, but that may not be possible with how the HPC is setup. Instead, the path forward for this tool is more for a "highlights" and post run analysis.

In theory, HELICS provides many metadata queries and enables us to ask questions of the system and respond to particular states. If we have a robust enough configuration and model, we could allow users to specify what subscriptions they want to listen to, and what range of values they want reported. Possibly even with custom messages or commands to execute if a particular value is encountered, potentially even writing data to different files/channels depending on the value received. I believe all of this could be done simply through a json configuration file, so we will not need to recompile the model when changing the cosimulation. The current version of the federate does not support this out of the box, but work could be done (should be done) to push it this direction.