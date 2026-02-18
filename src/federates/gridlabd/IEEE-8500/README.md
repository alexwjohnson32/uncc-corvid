# IEEE-8500 Node Gridlab-D Model

This represents a IEEE-8500 Node model implemented in gridlabd, utilizing a base model file and an inherited file, as well as a HELICS json configuration file.

The thinking behind separating the base and child files is so that multiple instances of the gridlabd model can all share the one huge base file, and only need to modify the much smaller child file that is where the "instance" settings exist.

As far as structure goes, the `IEEE_8500node.glm` file is the one you are wanting to use. It largely does not matter where the baseline or json file lives as long as you configure the details of the `IEEE_8500node.glm` file to reference the correct paths.

## Model Inputs

There are a number of inputs that must be set per cosim and model prior to the federate to be considered usable.

* a `#include` path to the `baseline_IEEE_8500.glm` file.
* The `clock` must be set with a given start and stop time that exists for the intended cosim simulated time.
* The `helics_msg` that defines the helics name (using `name`) and calls `configure` with the correct path to the corresponding input HELICS json file.

The HELICS json file itself has many options that can be set, but make sure that you define all the publications and subscriptions for the object.

You must also configure it to run with an `ipc` core to be compliant on the HPC.

## Running

Call this command to launch the model as a HELICS federate. Change filepaths as necessary.

`gridladbd.sh IEEE_8500node.glm`

## IPC Core Compliance

In order to run on an IPC core, you need to set two fields specifically in the HELICS json:
* `"coreType": "ipc"`
* `"coreInit": "--brokername=<name_of_broker> --shared_file=/path/to/socketfile"`
    * The `--brokername` is the exact name that you set in the broker's launch call with the `--name` flag.
    * The `--shared_file` is the exact path that you set in the broker's init string `--shared_file` flag.

For a basic example of the json file, refer to the `IEEE_8500node.json` file.