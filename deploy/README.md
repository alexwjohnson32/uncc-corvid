# The `deploy` Directory

The `deploy` directory contains the python scripts that will accept a json file and generate a cosim ready set of data that uses the built Federates from the `src/install` directory. There are a number of options to it, but changing your directory to `deploy` and running `python -m deploy --help` will list all of the commands available to you, including ways of inspecting existing deployed directories and getting info from them.

As a note, to get the tool to run properly, you will always need to use the `python -m` command with the files. This allows for the python code to be treated like a package and keeps files separated nicely.

Furthermore, within this section, all files will be referenced relative to `deploy`.

## The `plugins` Directory

This directory structure closely mirrors the `src/federates` directory structure. Each Federate implements a plugin interface here, which allows the deploy tool to integrate it into itself without having to modify the deploy capability.

The `plugins/interface.py` file defines an interface class that must be inherited from in order for the system to recognize a new plugin. The `plugins/manager.py` provides a manager class that finds and loads the plugins for you, allowing you to use the plugins within your scripts without having to reimplement the load structure. The manager exposes two functions: `get_plugin_names()` which lists all imported plugins, and a `get(plugin_name)` function that returns an `interface.IDeployable` implementation of a plugin based on the plugin name. Unless you are trying to implement a new plugin, this is about as much detail as you need to know for now, since all of this is hidden through the deploy tool commands.

## The `commands` Directory

This directory contains all of the individual commands of the deploy tool. Running `python -m deploy --help` will list them all for you. Furthermore, if you run `python -m deploy <command_name> --help`, it will list what the command does and include the input options for the command.

The plugin structure itself should mean that you will not need to modify the commands if you add a new model plugin. The specifics of the commands themselves will be discussed later.

## The `tests` Directory

These run unit tests on most of the code within the python directory. If you develop a new plugin and want it to be tested, add the test to this directory. You can look at any given plugin test and get an idea of how to implement it for yourself if desired.

To run the tests, call `python -m test_runner --install-dir=<path_to_install_dir> --write-dir=temp`. The write-dir can be set to any directory you want to write to, it does not have to be temp. Call `python -m test_runner --help` for a full list of options. The `install-dir` is the directory of the installation code you built from C++ code. In particular, you need to point it to `<path_to_src>/install/debug` or `<path_to_src>/install/release` depending on what you have built and want to run.

# Using the Deploy Tool

The Deploy Tool has been built with reusability and automation in mind. Ultimately, in order to deploy a cosim using the tool you only need a handful of inputs: A json configuration that defines the cosim, an installation directory path that points to the source files for the plugins to deploy, and finally a source destination that the cosim is written to. Building the json input can be done by hand, but for a large cosim that will be tedious. The `quick_config` command is an example on how to automate that process from a simpler json input. If you are automating your build process using python tooling, you can simply integrate the plugin manager and deploy command directly, and build everything up that way. If you are not using python, the cli command output should be parsable enough to be useful in a programmatic way.

## The Deploy JSON Schema

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
    * `component` definition:
        * `type`: The plugin name that is being represented
        * `options`: The json object definition for the given plugin.
        * `components`: A list of json `component` instances.

## The Quick Config Command

Our tooling currently supports a single Gridpack transmission model that can run alongside one-to-many Gridlabd distribution models within the HELICS cosimulation. If you are trying to build a large cosim by hand, it requires a lot of copy/pasting and is prone to small name typos. To that end, the `quick-config` command was added. It accepts a json file that allows for you to request any of the given federate plugins to be used and set up from one, easy to hand write file.

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
* `core_type`: The HELICS core type that each federate should use. For example: `"zmq"` or `"ipc"`.
* `broker_address`: The ip address of your broker.Defaults to `"127.0.0.1"`.
* `broker_port`: The broker port as an integer. HELICS typically uses values in the `23000-25000` range.
* `period`: The floating point representation of the time step for each federate.
* `ln_magnitude`: The baseline voltage magnitude, floating point value.
* `gridpack_type`: The plugin name of the gridpack federate plugin you want to use.
* `gridlabd_infos`: A list of gridlabd info objects. You can have multiple gridlabd types supported.
    * `gridlabd_info` definition:
        * `gridlabd_type`: The plugin name of the gridlabd federate plugin you want to use.
        * `number_instances`: The integer value representation of the number of federate instances you would like.

Note that there are no recursive types, so it is a much shorter input in comparison, with little to no repeated information.

The Quick Config command accepts the input json file, a cosim name, and an output directory that can be created if desired. After the run completes, the output directory will contain a Deploy json file ready to be used to generate the cosim deploy directory.