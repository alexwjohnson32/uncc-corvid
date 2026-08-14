# buses_3

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
    "subscription_name": "The model name to subscribe to. Required. See Notes.",
    "is_three_part": "A boolean to note if this is subscribed to a three part federate. Optional, defaults to True. See Notes",
    "period": "The timestep of the simulation, floating point. Optional, defaults to 1.0"
}
```

## Notes

The `is_three_part` and `subscription_name` tags work together. When `is_threee_part` is set to True, the subscription names are appended with `"_a"`, `"_b"`, `"_c"`, allowing the name to work with models that are implemented as three separate federates. If `is_three_part` is set to False, then the subscription name is used as presented directly on all three phases.

I admit, this is what I think of as the only cheat in this whole setup, since it requires that knowledge outside of this plugin in needed in order to implement. I could have had three separate tags instead: `subscription_name_a`, `subscription_name_b`, and `subscription_name_c`, which would be more encapsulated. Its also more verbose. But if changes are made to this in the future, remove the `is_three_part` and `subscription_name` tags and implement the `subscription_name_a`, `subscription_name_b`, and `subscription_name_c` tags instead.