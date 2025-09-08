# Scalability Tests

Right now we don't have the models, but I want to explore what a potential scaling tool would look like once we have those models. This is super exploratory and highly a *prototype*.

## General Ideas

We got three source directories: `gridlabd, gridpack, orchestrator`. Each are for a different purpose (and are probably pretty explanatory).

* `gridlabd`: This is for the gridlabd model. Depending on how things are delivered to us, scaling it may be weird. I am going to assume that they will provide one model, and it's our job to make the json file for the helics connection. The provided glm model may need to be modified on our part to support scaling.
* `gridpack`: This is for the gridpack model. Should be a pretty simple federate that can be created given an input. The federate should basically contain NO hard coded paths.
* `orchestrator`: Provides the main function that uses the gridpack model federate, and through a newly defined input file, connects the gridpack model to the gridlab models. More generally, this connects transmission systems to distribution systems.

When building, everything needs to be installed to an external location (meaning `build/install` for the time being). We can assume a particular structure of all the executables, models, and files within this install directory. The inputs that we provide to our executable will be using the `install` directories layouts as the reference. The install directory needs to be writable by the caller of the executable.

## Running the project

* Accept some input from the user (probably a json file).
* Using the inputs, generate the necessary files for the models, and choose the correpsonding federates.
    * NOTE: This does not mean instantiae federates, this means choose the federates that will be requested.
* Using the chosen federates, write the connection logic among correpsonding federates.
* Write all the federates to a runnable helics json file.
* Call `helics run --path=path_to_json`.


## Investigations

1. Can I launch helics and make federates just in time and then choose to launch the federation?
2. Can I build a federate that can spawn federates?


# Explanation of Current State (08/15/2025)

## Directories

* gridlabd
    * Contains the single gridlabd instance, not split out for multiples in any way. I am not pushing towards that though because Alex Johnson has already provided a viable solution for deployment.
* gridpack
    * Contains the code to build and execute the gridpack federate. This is currently in development to support multiple gridlabd instances at the same time. The federate reads from a json input, parses it, and assigns pubs and subs from the input file. Then, at each time step, we serially perform the powerflow operation on each gridlabd federate subscription. In theory. It is not working quite yet, but that is the approach. More will be stated later.
* corvid_helics_lib
    * A lib that allows for common utilities across the other libraries. Only contains some json helper template functions for now, but it is used in gridpack and the orchestrator.
* orchestrator
    * The main executable. The way it works is it accepts a deploy directory and uses the existing orchestrator::IModel classes to setup the desired cosimulation, and then deploys it to the given location. Everything except for the deploy location is currently hardcoded. This is still very much a prototype, and is trying to solve some basic deploy and execute logic, with an assumption that we can "Read inputs and correctly configure the corresponding models". Good news though, is that with Alex J's deploy directory code, most of this is not going to be immediately relevant, since we can assume that the deployed directory exists and that models are correctly configured to work together.

## What needs to be done Right Now

Since Alex J has created a way to create a deplyed cosim area that we can assume also has configured the cosimulation inputs to work together, we can focus completely on getting a gridpack federate to work with multiple gridlabd federates. But practically, how do we test this?

I have been updating this directory since I have complete control over everything within it, and the example that I am working on is still trying to run a one-to-one example, but with the gridpack federate containing internal logic to run one-to-n. As soon as you get the gridpack federate to work with the one-to-one example, immediately start using Alex J's delpoyed directory structure to reduce your effort.

### Goals of the Gridpack Federate Update

I have already updated the `pf_app` and `pf_factory` files and classes to handle a more general case, and I do not anticipate any additional work needs to be done there to get it to work. Why? Because realistically all I changed was that instead of hard-coding a bus id to run with, it uses a provided bus-id instead.

The `pf_input` files and classes are all about supporting the json input to the powerflow application. I am using the `boost/json.hpp` library for this, as the `tag_invoke` method of reading and writing json files is fairly intuitive to me, and it all can operate in the background. Look at the `JsonTemplate.hpp` file for the shortcut functions I have already provided for creating and reading files and strings into objects. I believe you should be able to follow along fairly well. At any rate, these files only need to change if you need to change, add, or remove the input fields for the gridpack federate.

When you do try to run within Alex J's deploy directory, you will need to create a json file with the following information (assuming you do not change anything in these classes):
* gridpack_name: whatever his gridlabd isntances report as the gridlab name
* total_time: A double that represents the total simulated time
* ln_magnitude: A double voltage value that represents....whatever it does. Its the value that is multiplied by a magic number to get the corresponding bus. This also assumes all gridlabd isntances are using different buses with the same voltages. This could be modified if you find you need something else to do.
* A vector of gridlabd_infos: The infos consist of the following
    * name: The name of the gridlabd object. That can be found in the deploy structure for each gridlabd instance
    * bus_id: You need to look in the 118.raw file and find a corresponding available bus to use.

You can either hand the json file to the powerflow_ex.x application at execution time, or place that created json file next to raw and xml files and configure the powerflow app to look for a hardcoded file name. I have a couple of commits pushed already that implement either option.