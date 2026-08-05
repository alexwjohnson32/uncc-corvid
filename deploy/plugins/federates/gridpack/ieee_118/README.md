# ieee_118_gridpack_fed

Within the deploy tool, the required inputs to use this should go within the component's `options` tag.

## Json Configuration

```json
{
    "name": "The model name. Required",
    "local_log_file": "Path to the log file to write to. Required",
    "core_type": "The HELICS core type. Required",
    "core_init": "The HELICS core init string. Required",
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
    ]
}
```