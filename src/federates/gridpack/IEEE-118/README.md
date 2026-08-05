# IEEE-118 Node GridPACK Model

This represents a IEEE-118 Node model implemented in gridpack as a executable built from C++ 17.

The executable is named `ieee-118-gridpack-federate`, and it accepts a json file as input.

## JSON File Inputs

There are a number of inputs that must be set per cosim and model prior to the launch in order for the federate to be considered usable.

* `federate_name`: The name of the federate. This must be consistent across the entirety of the cosim.
* `fed_info_json`: A separate json object that is the actual HELICS json structure, this will be passed to the federate directly. The HELICS json file itself has many options that can be set, but do NOT define the publications and subscriptions here, that is handled later in the config. You must also configure it to run with an `ipc` core to be compliant on the HPC.
* `total_time`: A double precision value that represents (in seconds) the total simulated time of the cosim.
* `local_log_file`: A path to a local log file to write outputs to, in order to keep a record of the model execution. More useful for debugging purposes than data analysis.
* `ln_magnitude`: A double precision measurement of the line's voltag(? needs more explanation and to confirm this is true).
* `gridlabd_infos`: This is a list of objects that represent the Distribution systems attached to this model. It is a list of maps, where a `bus_id` is mapped to a series of distribution system `names`.

## Running

Call this command to launch the model as a HELICS federate. Change filepaths as necessary.

`./ieee-118-gridpack-federate helics_setup.json`

As a side note, if launching as part of a helics runner file, you will probably need to use an exec string like this:

`"/bin/sh -c './ieee-118-gridpack-federate helics_setup.json'"`

## IPC Core Compliance

In order to run on an IPC core, you need to set two fields specifically in the HELICS json (the `fed_info_json` value):
* `"coreType": "ipc"`
* `"coreInit": "--brokername=<name_of_broker> --shared_file=/path/to/socketfile"`
    * The `--brokername` is the exact name that you set in the broker's launch call with the `--name` flag.
    * The `--shared_file` is the exact path that you set in the broker's init string `--shared_file` flag.

For a basic example of the json file, refer to the `helics_setup.json` file.

## Validity Considerations

This was originally built from early IEEE-118 models given to us by UNCC, where one three-phase model was executed as a single federate. For each timestep, the Gridpack app calculates the next voltage for each phase. I don't know if this is correct based off of later truth models given to us later which has each phase execute as it's own federate. In theory, the one-phase app could be used to encapsulate the 118 model if the proper `raw` and `xml` files are generated for it. We are keeping this here though as a historical record to show where development began.

Also, both this and the one-phase apps share the same approach to supporting multiple bus-ids per federate, the validity of which has been untested. The approach was derived from discussions between Corvid and UNCC. Essentially, we compute the voltage per bus id and sum them together and publish that summed value as the output. The truth models provided were only for single bus ids and did not display any approach to approve or counter this design choice, so please be aware of this implementation detail when using the models.