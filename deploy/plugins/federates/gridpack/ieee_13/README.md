# ieee_13

Within the deploy tool, the required inputs to use this should go within the component's `options` tag.

## Json Configuration

```json
{
    "name": "The model name. Required",
    "local_log_file": "Path to the log file to write to. Required",
    "core_type": "The HELICS core type. Required",
    "core_init": "The HELICS core init string. Required",
    "broker": "The name of the broker to connect to. Required",
    "broker_port": "The port id of the broker to connect to. Required",
    "log_level": "The HELICS Log Level. Required",
    "ln_magnitude": "The Voltage Magnitude. Required",
    "gridlabd_infos": [
        // Required: List of gridlabd_info instances to connect to.
        {
            "bus_id": "Integer Bus Id Value. Required",
            "names" [
                "A string list of gridlabd model names for the bus_id."
            ]
        }
    ],
    "period": "The timestep of the simulation, floating point. Optional, defaults to 1.0"
}
```

## Implementation Notes

This currently uses three instances of the `one-phase-gridpack-federate` under the hood, creating three separate federates that act as a single model. This interface abstracts this away though. However, the model name that you provide will not appear directly. Instead, the model name will appear throughout the individual federates as `<model_name>_a`, `<model_name>_b`, and `<model_name>_c`.

Even with this implementation, the `IDeployable` interface implementation holds true exactly as the system expects it to.