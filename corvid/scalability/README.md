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
