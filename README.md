# The Corvid Cosim Tool

This document goes over the inidvidual parts of the Corvid Cosim Tool, including environment and how to use, with some explanations on how to build further as well.

## Environment

The Corvid Cosim Tool runs from an Apptainer instance, generated from a series of `.def` files. The `.def` files are contained within the `multi_stage_build` directory.

In order to build your `.sif` file to run within Apptainer, navigate to the `multi_stage_build` directory and run `./build.sh sys` to fully build the environment. Running the script without any options will display a help text to let you know further options.

Once the script is complete, it should generate a file named `rl8_uncc_full_image.sif` that you can run within Apptainer. In order to run it, call something like `apptainer shell --bind <path_to_working_directory> <path_to_sif_directory>/rl8_uncc_full_image.sif` to launch the shell instance.

Furthermore, all actions described from here on assume that you are working from within this Apptainer environment.

## The C++ Code

The `src` directory contains the C++ code that is used to build the HELICS federates for Gridpack. The `src` directory contains a README with greater details, but we will go over the highlights briefly here.

Within this section, every directory is going to be relative to `src`.

### The `scripts` Directory

In order to build the project, we have provided a `build.sh` that has a number of options to build with. Running the script with the `--help` flag will fully display all the options available to you.

To get something built and running with the default options, simply run `./scripts/build.sh release` to build release versions of the executables for highest speed. If you want to build debug executables, run `./scripts/build.sh debug` or simply `./scripts/build.sh` to build the project in debug mode.

Unless you modify the `CMakePresets.json` file, build script will always produce output to the `build` and `install` directories. For usage, the install directory has been deisgned to have a more logical layout and cuts all unecessary build artifacts, leaving you with a directory that only has useful items.

### The `common`, `debug_tools` and `federates` directories

This is the source code directories and should likely not need to be interacted with often. `common` contains the common libraries that are used by the federates, `debug_tools` contains a dumb federate that can link into a helics cosim and do nothing. It was used for sanity checking during development. The `federates` directory is organized by federate source, and then by type. We have `gridlabd` and `gridpack` and a `helics` that contains a queryable helics federate that was meant to monitor the cosim.

Each federate subdirectory contains a README that contains helpful information about useage and setup for the federate. The directory also contains some files that will be used when installing and deploying. Do NOT change these files without changing the source code and deploy code accordingly, since these act as the baseline files for the install and deploy tools. Thankfully, this is all source controlled. If any changes do occur, simply revert the change and you will be back to the baseline status. The federate README's and metadata files are all copied to the install directory as well for you to access there. The same warnings about changing the files there should apply, but rebuilding will revert them back to their orginal state as well.

## The Python Code

The `deploy` directory contains the python scripts that will accept a json file and generate a cosim-ready set of data that uses the built Federates from the `src/install` directory. There are a number of options to it, but changing your directory to `deploy` and running `python -m deploy --help` will list all of the commands available to you, including ways of inspecting existing deployed directories and getting info from them.

As a note, to get the tool to run properly, you will always need to use the `python -m` command with the files. This allows for the python code to be treated like a package and keeps files separated nicely.

Furthermore, within this section, all files will be referenced relative to `deploy`.

### The `plugins` Directory

This directory structure closely mirrors the `src/federates` directory structure. Each Federate implements a plugin interface here, which allows the deploy tool to integrate it into itself without having to modify the deploy capability.

The `plugins/interface.py` file defines an interface class that must be inherited from in order for the system to recognize a new plugin. The `plugins/manager.py` provides a manager class that finds and loads the plugins for you, allowing you to use the plugins within your scripts wihtout having to reimplement the load structure. The manager exposes two functions: `get_plugin_names()` which lists all imported plugins, and a `get(plugin_name)` function that returns an `interface.IDeployable` implementation of a plugin based on the plugin name. Unless you are trying to implement a new plugin, this is about as much detail as you need to know for now, since all of this is hidden through the deploy tool commands.

### The `commands` Directory

This directory contains all of the individual commands of the deploy tool. Running `python -m deploy --help` will list them all for you. Furthermore, if you run `python -m deploy <command_name> --help`, it will list what the command does and include the input options for the command.

The plugin structure itself should mean that you will not need to modify the commands if you add a new model plugin. The specifics of the commands themselves will be discussed later.

### The `tests` Directory

These run unit tests on most of the code within the python directory. If you develop a new plugin and want it to be tested, add the test to this directory. You can look at any given plugin test and get an idea of how to implement it for yourself if desired.

To run the tests, call `python -m test_runner --install-dir=<path_to_install_dir> --write-dir=temp`. The write-dir can be set to any directory you want to write to, it does not have to be temp. Call `python -m test_runner --help` for a full list of options. The `install-dir` is the directory of the installation code you built from C++ code. In particular, you need to point it to `<path_to_src>/install/debug` or `<path_to_src>/install/release` depending on what you have built and want to run.

## Using the Deploy Tool

The Deploy Tool has been built with reusability and automation in mind. Ultimately, in order to deploy a cosim using the tool you only need a handful of inputs: A json configuration that defines the cosim, an installation directory path that points to the source files for the plugins to deploy, and finally a source destination that the cosim is written to. Building the json input can be done by hand, but for large cosims that will be tedious. The `quick_config` command is an example on how to automate that process from a simpler json input. If you are automating your build process using python tooling, you can simply integrate the plugin manager and deploy command directly, and build everything up that way. If you are not using python, the cli command output should be parsable enough to be useful in a programatic way.

### The Deploy JSON Schema

From the deploy perspective, the schema is very simple:

```json
{
    "cosim_name": "The name of the cosim",
    "total_time_seconds": "The amount of simulated time that you want the cosim to run",
    "components": [
        {
            "type": "The name of the federate",
            "options": {
                // The json schema input for the given federate as defined by the specific federate plugin
            },
            "components": [
                // a list of sub components, allowing for continued depth
            ]
        }
    ]
}
```

* `cosim_name`: This is the name of the cosim you want to run.
* `total_time_seconds`: The amount of simulated time that you want the cosim to run
* `components`: A list of json `component` instances
    * A `component` is a json representation of a federate instance from the plugins, and can include other sub-components as well
        * This could allow for grouping of entire systems in the future that could be copied into place as is if the deploy tool has some further developments
    * `component` defintion:
        * `type`: The plugin name that is being represented
        * `options`: The json object definition for the given plugin.
        * `components`: A list of json `component` instances.

### The Quick Config Command

Our tooling currently supports a single Gridapck transmission model that can run alongside one-to-many Gridlabd distribution models within the HELICS cosimulation. If you are trying to build a large cosim by hand, it requires a lot of copy/pasting and is prone to small name typos. To that end, the `quick-config` command was added. It accepts a json file that allows for you to request any of the given federate plugins to be used and setup from one, easy to hand write file.

```json
{
    "total_time_seconds": "The amount of simulated time that you want the cosim to run",
    "bus_ids": [
        // a list of integers for each bus_id that you want to use within the cosim
    ],
    "core_type": "The helics core type that each federate should use",
    "broker_address": "The ip address of your broker",
    "broker_port": "The broker port as an integer",
    "period": "The floating point representation of the time step for each federate",
    "ln_magnitude": "The baseline voltage magnitude, floating point value",
    "gridpack_type": "The plugin name of the gridpack federate plugin you want to use",
    "gridlabd_infos": [
        // A list of gridlabd info objects. You can have multiple gridlabd types supported.
        {
            "gridlabd_type": "The plugin name of the gridlabd federate plugin you want to use",
            "number_instances": "The integer value representation of the number of federate instances you would like"
        }
    ]
}
```

* `total_time_seconds`: The amount of simulated time that you want the cosim to run.
* `bus_ids`: a list of integers for each bus_id that you want to use within the cosim. Gridlabd instances are assigned to a bus in a round-robin style, which consequently creates as even of a distribution of federates to buses as possible.
* `core_type`: The helics core type that each federate should use. For example: `"zmq"` or `"ipc"`.
* `broker_address`: The ip address of your broker.Defaults to `"127.0.0.1"`.
* `broker_port`: The broker port as an integer. HELICS typically uses values in the `23000-25000` range.
* `period`: The floating point representation of the time step for each federate.
* `ln_magnitude`: The baseline voltage magnitude, floating point value.
* `gridpack_type`: The plugin name of the gridpack federate plugin you want to use.
* `gridlabd_infos`: A list of gridlabd info objects. You can have multiple gridlabd types supported.
    * `gridlabd_info` definition:
        * `gridlabd_type`: The plugin name of the gridlabd federate plugin you want to use.
        * `number_instances`: The integer value representation of the number of federate instances you would like.

Note that there are no recursive types, so it is a much shorter input in comparision, with little to no repeated information.

The Quick Config command accepts the input json file, a cosim name, and an output directory that can be created if desired. After the run completes, the output directory will contain a Deploy json file ready to be used to generate the cosim deploy directory.