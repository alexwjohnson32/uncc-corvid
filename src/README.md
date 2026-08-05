# Navigating the Project Layout

The `CMakePresets.json` file holds the default CMake Variables for this project. Look at the `cmake_deps` directory for more dependencies for the project, but those should not be necessary to modify from the build process.

Everything is being built statically currently. If building shared is desired, I think some changes will need to be made to the install process to properly install the shared libs.

Currently, we are not exporting any headers or libs to link to. The current concept of this project is to present a series of executables that all launch HELICS federates. Each federate will have a configurable set of inputs that can be handed to it via a json config file.

## The `scripts` Directory

The `scripts` directory contains a `build.sh`, which by default builds everything in debug mode. To build in Release, simply add `release` after the build steps. To build and run tests, build with the `test` keyword. To remove some build artifacts before building, add the `--fresh` flag.

Besides those keywords, you can add any CMake build flag to the build process by using the form `-D<CMAKE_VAR_NAME>=<Value>`.

If you want to pass specific flags to the `gtest` test runner, first type `--` as a separator, even if you have not specified any other CMake flags. Every flag after the standalone `--` will be interpreted as a gtest runner flag.

If you want to get a fully clean build, the best thing to do is manually `rm -rf` the build and install directories manually. From the `src` directory, the easiest way to do that is to simply run `rm -rf build/* install/*` to get a clean slate as long as you have not specified another install directory location.

## The `install` Directory

The default install directory is going to be `src/install/<build_type>` (meaning debug, release, etc). If you pass an install prefix to the build system, that will be the root of the install directory.

Once built, you will have the following directories:
* `debug_tools`
* `federate`

### `debug_tools`

This contains any tools that may be useful for troubleshooting a helics cosim execution, but is not meant for delivery or truly external use. It contains a dummy server and client for testing the passing of information from an executing helics federate. Currently, the websocket tool is not in use in any federate, but is left within the directory in the event someone may find it useful in the future.

There is also a `dummy_federates` directory which contains a very basic helics federate that does not rely on any external libraries. It exists to simply execute a helics cosim to test that the pieces can connect. You could probably do the same with the queryable federate we will mention later, but this is a dedicated federate for dummy tests that should not change over time.

### `federate`

#### Definitions

For the context of this section, let me define a few terms that I will use when discussing federates to promote understanding and enable future maintainers to be able to continue to grow this capability within this organizational structure.

* `federate type`: When referring to a federate's type, we are specifying that the federate is a GridPACK federate, or a GridLab-D federate (or some other type). Type categorization tells you about the specific library used to define the federate.
* `federate kind`: When referring to a federate's kind, we are specifying that a federate is a Transmission federate, or a Distribution federate, among other things. Kind only reveals the purpose of the federate, but not the specifics or the method.
* `federate model`: When referring to a federate's model, it is the specific Real World Model that is being represented by this federate. The model itself does not necessarly reveal the particulars of how it was implemented, but (to the best of my knowledge) it does reveal the kind of federate that it is.

#### Structure

This directory contains the built federates for the UNCC-COSIM environment. In general, the installed organizational structure is grouping by `federate type -> federate model`. The `federate kind` can be inferred from the specific `federate model`.

The `federate model` itself is a directory that contains all of the files local to it that is necessary for the model to run. They may be c++ executables, gridlabd shell scripts, possibly even python scripts, or any other arbitrary executable type that can be coerced to run as a HELICS federate.

For specific model information and details, one should refer to the `README.md` file within the `federate model` directory.

## Instantiating the HELICS Broker with IPC

In order for a cosim to run, you need to have at least one broker. Currently with the IPC connection, I know how to stand-up a single broker cosim. I believe with our constraints that will be enough since we will have federates launching on multiple nodes, but they all can connect to the broker via the socketfile. If we need to setup a broker hiearchy, some more work will need to be put into this section. I believe it should be possible; in theory we just setup the broker hierarchies, and each "node" broker will have a socketfile connected to it, which is how it will communicate to all of its federates. It will then need to send all of it's data to a separate socketfile (which this may be where a mutli-broker approach could fall apart if this is not supported out-of-the-box).

### HELICS Broker exec string

To launch a helics broker, execute the following:

`helics_broker --federates=<number_of_other_federates> --coretype=ipc --name=<broker_name> --brokerinit="--shared_file=/path/to/socketfile"`

A few notes:
* The `number_of_other_federates` is the total number of federates that you expect to launch, but do NOT count this broker as one. Meaning, if you have the broker plus 2 other federates, you set the flag to `--federates=2`.
* The `broker_name` can be mostly whatever you want. As a rule of thumb, stick to alphanumeric characters and avoid special characters (underscores are allowed and preferred over dashes). The names are case sensitve, and no spaces at all are allowed.
* The `/path/to/socketfile` is a shared path that all federates will have access to. If a federate does not have access to that path, then it cannot communicate with the cosim. This is a path to a FILE, not a directory. However, the file should NOT exist before running the cosim, it is generated by the broker at runtime.

If we have more broker options that we set, it may be beneficial to store the settings as a json file and pass that to the `helics_broker` command.

If we are launching from a HELICS runner file, use this form:

```json
{
    "directory": ".",
    "exec": "helics_broker --federates=<number_of_other_federates> --coretype=ipc --name=<broker_name> --brokerinit=\"--shared_file=/path/to/socketfile\"",
    "host": "localhost",
    "name": "main_broker_fed"
},
```

To be honest, I am not sure if that `"name"` key needs to be different or match the `--name` flag. It seems to work even though they are different. You maybe could make them the same, but treat `--name` as truth.