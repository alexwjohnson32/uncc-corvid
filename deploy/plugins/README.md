# `plugins`

This directory directly contains the `IDeployable` interface class and the `PluginManager` class. Subdirectories will contain the actual plugin implementations.

The subdirectories created within this have arbitrary names, and if you want to add your own plugin in a new directory, ensure that the directories all contain an `__init__.py` file within each directory within the path to find your plugin. This is a python limitation and further understanding of the why of it all can be learned elsewhere.

## `interface.py`

### `IDeployable`

The `IDeployable` interface allows for the `deploy` tool to interact with an arbitrary federate without requiring the `deploy` tool to build special code around individual federates. Furthermore, the `IDeployable` interface allows for logical grouping of abstract implementations, allowing cosim creators to think better along the lines of "which models do I want" over thinking "what pieces do I need in order to use the models?"

### `deploy`

This is the major call and the most intricate function of the interface because this call takes the baseline inputs, transforms them with a Json configuration, and writes the modified data to a ready to be used output directory. This function should NOT modify the actual baseline files in any way. Instead, it should read them and/or copy them and modify the copied files or modify the read data in memory before writing back out to a new location. This should throw standard exceptions if the deployment cannot be completed.

* `json_config`: A dictionary that is derived from a Json config file, containing the specific information for this plugin.
* `total_time_seconds`: The total time, in seconds, that is to be simulated. A floating-point value.
* `deploy_root`: The root path to the directory that is to be written to.
* `install_root`: The root path to the directory that contains the installed information about the plugin (generated from the installed `src` code).

This function does not return anything. A deploy is considered successful on no exception throws.

### `get_baseline_files`

Returns a `list` of all the files that are used to generate the deployed directory. These files should NOT be modified by the user.

* `install_root`: The root path to the directory that contains the installed information about the plugin (generated from the installed `src` code).

If the `install_root` does not exist, throw a `ValueError` describing the issue. Otherwise, return the list of source file paths.

### `get_model_files`

Returns a `list` of all the files that have been generated to create the specific model request by name. You may make changes to these files at your own risk of breaking the specific cosim. This function should not throw any exceptions.

* `model_name`: The specific model name to search for within the deployment directory.
* `deploy_root`: The deploy directory to search within for the model.

If the model name cannot be found or the directory does not exist, return an empty `list`. Otherwise, return all the generated file paths found within the directory.

### `get_exec_json`

Returns a dictionary object that represents a HELICS federate object that can be used within a HELICS runner file. This function should not throw any exceptions.

* `model_name`: The specific model name to search for within the deployment directory.
* `deploy_root`: The deploy directory to search within for the model.

If the model name cannot be found or the directory does not exist, return an empty `dict`. Otherwise, the `dict` returned must follow this definition:
```json
{
    "directory": "<relative_working_directory derived from deploy_root and model_name>",
    "exec": "<specific exec command to launch the federate with any options>",
    "host": "localhost or whatever the host of the federate needs to be",
    "name": "<model_name>
}
```

### `get_name`

Returns the name of the Plugin (NOT the model). This is the unique identifier for the plugin. Be careful to give a specific enough name to prevent name collisions. This function should not throw any exceptions.

This will return the name of the plugin, with path parts separated using the `"/"` character regardless of operating system.

### `list_model_names`

Returns a `list` of the model names that are contained within this specific deploy directory. This function should not throw any exceptions.

* `deploy_root`: The deploy directory to search within for the model names.

If the deploy root does not exist or no names can be found, return an empty `list`. Otherwise, return a list of all the model names found.


## `manager.py`

### `PluginManager`

The `PluginManager` is a helper class that accepts the path to the root of the plugins directory and recursively searches through it to find all classes that inherit the `IDeployable` interface, importing them and saving an instance of each within its internal `dict`. The instances are exposed through a `get` function that safely checks for name existence for you, leaving you to only have to check that the returned value is not `None`.

### `get`

Returns an `IDeployable` instance with the given name, or `None` if the name does not exist.

* `name`: The name of the `IDeployable` that is being requested.

### `get_plugin_names`

Returns a list of all the names of the found `IDeployable` instances imported into memory.
