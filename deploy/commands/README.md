# Deploy Tool Commands

This is a brief overview of the public facing functions from each command file within the `commands` directory. This is NOT an API for the actual CLI calls (that can be found within the cli commands using the `--help` option). Instead this lists each command in the event you were to integrate the commands directly into another Python application.

## `list-names`

List the names of the found `IDeployable` plugins from a given `PluginManager` instance.

This is a convenience tool for formatting the names for cli outputs. If what you really want is a list of the names of the plugins, simply call `get_plugin_names()` on the `PluginManager` itself.

### Inputs

* `manager`
    * Type: `plugins.manager.PluginManager`
    * Description: An initialized instance of the `PluginManager` that is supposed to contain any number of `IDeployable` plugin instances.

### Outputs

* Type: `str`
* Description: A string that contains every found `IDeployable` name found within the `PluginManager` instance. The names all appear on separate lines and are prepended with the tab special character. If no names are found, it returns an empty string.

### Raises

There should be no raised exceptions.

## `get-exec`

Attempts to find a specific model-name within a given deployed directory.

### Inputs

* `manager`
    * Type: `plugins.manager.PluginManager`
    * Description: An initialized instance of the `PluginManager` that is supposed to contain any number of `IDeployable` plugin instances.
* `model_name`
    * Type: `str`
    * Description: The name of the model that you want to get the definition of.
* `deploy_root`
    * Type: `pathlib.Path`
    * Description: The path to the root of the deployed directory to search within.
* `plugin_name`
    * Type: `str`
    * Default: `""`
    * Description: An optional parameter to limit the searches to only plugins of a given type.

### Outputs

* Type: `str`
* Description: A json string that is ready to be used in a HELICS runner file to launch this as a federate. If no name can be found, a description of why it could not be found is provided instead.

### Raises

* `ValueError` if the `deploy_root` does not exist.
* `KeyError` if a `plugin_name` is specified and the plugin does not exist.

## `list-source-files`

Attempts to list the given source files for a plugin located within the install directory.

This is a convenience tool for formatting the file names for cli outputs.

### Inputs

* `manager`
    * Type: `plugins.manager.PluginManager`
    * Description: An initialized instance of the `PluginManager` that is supposed to contain any number of `IDeployable` plugin instances.
* `plugin_name`
    * Type: `str`
    * Description: The plugin that you want to get the source files for.
* `install_dir`
    * Type: `pathlib.Path`
    * Description: The path to the root of the install directory to search within.

### Outputs

* Type: `str`
* Description: A string that contains every found source file for a particular `IDeployable` instance found within the `PluginManager` instance. The files all appear on separate lines and are prepended with the tab special character. If no files are found, it returns an empty string.

### Raises

* `ValueError` if the `install_dir` does not exist.
* `KeyError` if a `plugin_name` does not exist.

## `list-model-files`

Attempts to list the given model files for a plugin and model name located within the deploy directory.

This is a convenience tool for formatting the file names for cli outputs.

### Inputs

* `manager`
    * Type: `plugins.manager.PluginManager`
    * Description: An initialized instance of the `PluginManager` that is supposed to contain any number of `IDeployable` plugin instances.
* `model_name`
    * Type: `str`
    * Description: The name of the model that you want to get the definition of.
* `deploy_dir`
    * Type: `pathlib.Path`
    * Description: The path to the root of the deployed directory to search within.
* `plugin_name`
    * Type: `str`
    * Default: `""`
    * Description: An optional parameter to limit the searches to only plugins of a given type.

### Outputs

* Type: `str`
* Description: A string that contains every found model file for a particular model within a given deployed directory. The files all appear on separate lines and are prepended with the tab special character. If no files are found, it returns a string that contains the message of why it could not find the model.

### Raises

* `ValueError` if the `deploy_dir` does not exist.
* `KeyError` if a `plugin_name` is specified and the plugin does not exist.

## `list-model-names`

Attempts to list the model names found within a given deploy directory.

This is a convenience tool for finding all of the model names within a deploy directory as a string.

### Inputs

* `manager`
    * Type: `plugins.manager.PluginManager`
    * Description: An initialized instance of the `PluginManager` that is supposed to contain any number of `IDeployable` plugin instances.
* `deploy_dir`
    * Type: `pathlib.Path`
    * Description: The path to the root of the deployed directory to search within.
* `plugin_name`
    * Type: `str`
    * Default: `""`
    * Description: An optional parameter to limit the searches to only plugins of a given type.

### Outputs

* Type: `str`
* Description: A string that contains every found model name within a given deployed directory. If a specific plugin name has been specified, then only model names by that plugin are listed. The names all appear on separate lines. If no files are found, it returns an empty string.

### Raises

* `ValueError` if the `deploy_dir` does not exist.
* `KeyError` if a `plugin_name` is specified and the plugin does not exist.

## `deploy`

Attempts to deploy a given configuration to a given directory.

This is the tool to use to perform a deploy out of the box using the json configuration file.

### Inputs

* `manager`
    * Type: `plugins.manager.PluginManager`
    * Description: An initialized instance of the `PluginManager` that is supposed to contain any number of `IDeployable` plugin instances.
* `install_root`
    * Type: `pathlib.Path`
    * Description: The path to the root of the install directory to read from.
* `deploy_root`
    * Type: `pathlib.Path`
    * Description: The path to the root of the deployed directory to write to.
* `json_file`
    * Type: `pathlib.Path`
    * Description: The path to the json configuration file to read deploy information from.

### Outputs

* Type: `str`
* Description: A successful deploy will return a message that contains the path to the created HELICS runner file.

### Raises

* `ValueError` if any given path does not exist, cannot be created, or any other path issues arise preventing the deploy operation.

## `quick-config`

Attempts to generate a cosim definition based on the given input.

This is a convenience tool for generating a large deploy json configuration from a sparse input json.

### Inputs

* `json_file`
    * Type: `pathlib.Path`
    * Description: The path to the json configuration file to read deploy information from.
* `cosim_name`
    * Type: `str`
    * Description: The name of the cosim to be deployed.
* `output_dir`
    * Type: `pathlib.Path`
    * Description: The path to the directory to write the config to.
* `create_output_dir`
    * Type: `bool`
    * Description: Tells the function if it needs to create the directory.


### Outputs

* Type: `str`
* Description: A successful quick deploy will return a message that contains the path to the created deploy json file. A failure will return an error message detailing the cause of the failure.

### Raises

There should be no raised exceptions.
