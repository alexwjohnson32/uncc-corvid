# Helics Queryable Federate

This is an observable only federate that performs queries on the cosim.

The executable is named `dummy_federate`, and it accepts a json file as input.

## JSON File Inputs

There are a number of inputs that must be set prior to the launch in order for the federate to be considered usable.

* `federate_name`: The name of the federate. This must be consistent across the entirety of the cosim.
* `fed_info_json`: A separate json object that is the actual HELICS json structure, this will be passed to the federate directly. The HELICS json file itself has many options that can be set, but do NOT define the publications and subscriptions here, that is handled later in the config. You must also configure it to run with an `ipc` core to be compliant on the HPC.
* `total_time`: A double precision value that represents (in seconds) the total simulated time of the cosim.
* `local_log_file`: A path to a local log file to write outputs to, in order to keep a record of the model execution. More useful for debugging purposes than data analysis.

## Running

When launching, this should be one of the last federates to launch since it is querying the system. Meaning, it needs every other federate in place in order to get best results if perfomring metadata queries before offically "starting" the cosimulation.

Call this command to launch the model as a HELICS federate. Change filepaths as necessary.

`./dummy_federate helics.json`

As a side note, if launching as part of a helics runner file, you will probably need to use an exec string like this:

`"/bin/sh -c './dummy_federate helics.json'"`

## IPC Core Compliance

In order to run on an IPC core, you need to set two fields specifically in the HELICS json (the `fed_info_json` value):
* `"coreType": "ipc"`
* `"coreInit": "--brokername=<name_of_broker> --shared_file=/path/to/socketfile"`
    * The `--brokername` is the exact name that you set in the broker's launch call with the `--name` flag.
    * The `--shared_file` is the exact path that you set in the broker's init string `--shared_file` flag.

For a basic example of the json file, refer to the `helics_setup.json` file.

## Purpose and Future Direction

This is just a simple, independent MessageFederate that can be used as a placeholder in a cosim, or you want to just ensure that you can launch a cosim without worrying about external library dependencies. This should not be modified much, if at all, in future development. If you want a tool that can allow you to probe the system, the QueryableFederate is better suited for those needs.