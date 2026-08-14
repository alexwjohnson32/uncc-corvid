import datetime
import pathlib

def get_ieee_8500_model_config(input_data: dict) -> dict:
    return {
        "broker": "main_broker",
        "broker_port": 23500,
        "coreInit": input_data["core_init"],
        "coreType": input_data["core_type"],
        "name": input_data["name"],
        "offset": 0.0,
        "period": 1.0,
        "uninterruptible": True,
        "logfile": input_data["local_log_file"],
        "log_level": "debug",
        "publications": [
            {
                "global": True,
                "key": f"{input_data['name']}/Sa",
                "type": "complex",
                "unit": "VA",
                "info": {
                    "object": "HVMV_Sub_HSB",
                    "property": "measured_power_A"
                }
            },
            {
                "global": True,
                "key": f"{input_data['name']}/Sb",
                "type": "complex",
                "unit": "VA",
                "info": {
                    "object": "HVMV_Sub_HSB",
                    "property": "measured_power_B"
                }
            },
            {
                "global": True,
                "key": f"{input_data['name']}/Sc",
                "type": "complex",
                "unit": "VA",
                "info": {
                    "object": "HVMV_Sub_HSB",
                    "property": "measured_power_C"
                }
            }
        ],
        "subscriptions": [
            {
                "required": True,
                "key": f"{input_data['subscription_name']}_a/Va",
                "type": "complex",
                "unit": "V",
                "info": {
                    "object": "HVMV_Sub_HSB",
                    "property": "voltage_A"
                }
            },
            {
                "required": True,
                "key": f"{input_data['subscription_name']}_b/Vb",
                "type": "complex",
                "unit": "V",
                "info": {
                    "object": "HVMV_Sub_HSB",
                    "property": "voltage_B"
                }
            },
            {
                "required": True,
                "key": f"{input_data['subscription_name']}_c/Vc",
                "type": "complex",
                "unit": "V",
                "info": {
                    "object": "HVMV_Sub_HSB",
                    "property": "voltage_C"
                }
            }
        ]
    }

def get_model_glm(baseline_file_path: pathlib.Path, name: str, total_time_seconds: float) -> str:
    """This is literally copy pasted from the class itself, so this needs to be updated if
        that function is updated. Just copy it again for simplicity. """
    # get datetime setup
    now_obj = datetime.datetime.now()
    start_time_obj = now_obj.replace(year=(now_obj.year - 1), minute=0, second=0)
    start_time_str = start_time_obj.strftime("%Y-%m-%d %H:%M:%S")
    stop_time_obj = start_time_obj + datetime.timedelta(seconds=total_time_seconds)
    stop_time_str = stop_time_obj.strftime("%Y-%m-%d %H:%M:%S")

    return f"""#include "{baseline_file_path}"

object helics_msg {{
    name {name};
    configure IEEE_8500node.json;
}}

clock {{
    timezone CST+6CDT;
    starttime '{start_time_str}';
    stoptime '{stop_time_str}';
}}"""