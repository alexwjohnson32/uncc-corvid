# Orchestrator Jobs

1. The Orchstrator allows for the reading of a User presented input to allow for the execution of arbitrarily many federates within a helics execution.
    1. Each Federate is a predefined piece that has some modifiable field. Users will not be presenting completely custom federates.
2. The Orchestrator will tell the user which, if any, of the requested federates do not exist.
3. The Orchestrator will tell the user which, if any, of the requested fields on a federate do not exist.
4. The Orchestrator will manage the File I/O of all federates.
5. The Orchestrator will deploy the created structure to a default directory, or a user specified directory if requested.
6. The Orchestrator will launch the helics cosimulation and report relevant lifetime information.

# What has to be done practically

The Orchestrator knows about the installed structure and navigates that to find about know paths and models. The models themselves know what inputs and executables they need and report that back to the Orchestrator. The Orchestrator then creates a deployment manifest that translates the user inputs to the desired number of federates with the custom values. The deployment manifest then is used and the corresponding files are copied and modified into the deployment directory. From there, the helics runner json file is generated, a command process is opened at the deployment directory, and the cosimulation is launched. After helics execution is complete, everything shuts down and execution of the Orchestration is completed.