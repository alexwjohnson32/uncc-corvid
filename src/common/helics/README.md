# `helics`

This is the area for all common utils and HELICS only tools (not necessarily utilities).

## Linking to the library in CMake

NOTE: This is all done under the assumption of static linkage. If we have shared libraries, some of this guidance may change, or may require extra steps. As of the time of this writing though, we only build statically.

The `helics` directory builds a library that can be referenced throughout CMake by the variable `CORVID_HELICS_LIB` (referenced in CMake like `${CORVID_HELICS_LIB}`). This library publically links all of the libraries and headers in order to have your library compile with HELICS libraries. It also links the `CORVID_UTILS_LIB` which exposes the utils and the Boost library, although you may still need to specify that you are linking against `CORVID_UTILS_LIB` as well within your CMake.

## Using the library in C++

All classes and functions found within this library exist under the `common::helics::` namespace.

We will briefly describe each file, but this is not an API document. If you want to see something more along those lines, look at the header files themselves. This is a purpose overview.

### `tools.hpp`

This contains a few helper classes and functions that seem like they may be universally useful for powerflow applications, but are actually helics specific operations. At the time of this writing, we only really have the `IEEE-118` Transmission system to make assumptions off of, so this may need to be moved to be implementation specific classes later down the line.

Do note that the `LimitPower` function divides each of the `common::helics::ThreePhaseValues` by `1e8` before computing. This might be important for the reader to know.

### `helics_input.hpp`

This is the common input object that each federate uses as a baseline currently. Most of the federates use an object that is a subclass of this one, and add more fields to this. This utilizes the `Boost/Json.hpp` `boost::json::value_from_tag` and `boost::json::value_to_tag<T>` methods to promote object serialization and deserialization.

If you have a class that wants to inherit from this one and implement your own `value_from_tag` and `value_to_tag`, use this as a guideline:

```C++
void other::ns::tag_invoke(boost::json::value_from_tag, boost::json::value &json_value,
                          const other::ns::ChildClass &data)
{
    common::helics::tag_invoke(boost::json::value_from_tag(), json_value,
                               static_cast<const common::helics::HelicsInput &>(data));

    boost::json::object &obj = json_value.as_object();
    // Note the usage of boost::json::value_from, that os because this particular field
    // is an object like a std::vector or another custom serializable class that has a
    // a serializable form implemented.
    obj["child_vector"] = boost::json::value_from(data.child_field);
    obj["child_field"] = data.child_field;
}

other::ns::ChildClass other::ns::tag_invoke(boost::json::value_to_tag<other::ns::ChildClass>,
                                            const boost::json::value &json_value)
{
    common::helics::HelicsInput base_data = boost::json::value_to<common::helics::HelicsInput>(json_value);

    const boost::json::object &obj = json_value.as_object();
    other::ns::ChildClass data{ base_data };

    common::utils::extract(obj, "child_vector", data.child_vector);
    common::utils::extract(obj, "child_field", data.child_field);

    return data;
}
```