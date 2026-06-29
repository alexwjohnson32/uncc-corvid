# One Phase Node GridPACK Model

This represents a single phase of a transmission model implemented in gridpack as a executable built from C++ 17.

The executable is named `one-phase-gridpack-federate`, and it accepts a json file as input.

## JSON File Inputs

There are a number of inputs that must be set per cosim and model prior to the launch in order for the federate to be considered usable.

* `federate_name`: The name of the federate. This must be consistent across the entirety of the cosim.
* `fed_info_json`: A separate json object that is the actual HELICS json structure, this will be passed to the federate directly. The HELICS json file itself has many options that can be set, but do NOT define the publications and subscriptions here, that is handled later in the config. You must also configure it to run with an `ipc` core to be compliant on the HPC.
* `total_time`: A double precision value that represents (in seconds) the total simulated time of the cosim.
* `local_log_file`: A path to a local log file to write outputs to, in order to keep a record of the model execution. More useful for debugging purposes than data analysis.
* `ln_magnitude`: A double precision measurement of the line's voltag(? needs more explanation and to confirm this is true).
* `phase_name`: The name of the phase being modeled.
* `publication_field`: The name of the publication that this model will publish. The project will prepend the field with `federate_name/`.
* `subscription_field`: The name of the subscription that this model will listen for. The project will prepend the field with each name found within `gridlabd_infos`, so multiple subscriptions can occur. The fields will be prepended with `gridlabd_name/`.
* `gridlabd_infos`: This is a list of objects that represent the Distribution systems attached to this model. It is a list of maps, where a `bus_id` is mapped to a series of distribution system `names`.

## Running

Call this command to launch the model as a HELICS federate. Change filepaths as necessary.

`./one-phase-gridpack-federate helics_setup.json`

As a side note, if launching as part of a helics runner file, you will probably need to use an exec string like this:

`"/bin/sh -c './one-phase-gridpack-federate helics_setup.json'"`

## IPC Core Compliance

In order to run on an IPC core, you need to set two fields specifically in the HELICS json (the `fed_info_json` value):
* `"coreType": "ipc"`
* `"coreInit": "--brokername=<name_of_broker> --shared_file=/path/to/socketfile"`
    * The `--brokername` is the exact name that you set in the broker's launch call with the `--name` flag.
    * The `--shared_file` is the exact path that you set in the broker's init string `--shared_file` flag.

For a basic example of the json file, refer to the `helics_setup.json` file.