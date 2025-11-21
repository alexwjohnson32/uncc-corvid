#pragma once

#include <fstream>
#include <type_traits>
#include <vector>
#include <array>
#include <string>
#include <filesystem>

#include <boost/json.hpp>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

namespace json_templates
{
/**
 * @brief Get the underlying type of an enum value, whether that is scoped or non-scoped.
 * @tparam T is passed in enum type.
 * @param value is the passed in enum value.
 * @return constexpr std::enable_if<std::is_enum<T>::value, typename std::underlying_type<T>::type>::type
 *         - Looks like a monster but is saying two things. Only compile if this template is called with
 *           an enum passed in, and then this template is expected to return the found underlying type of
 *           the given enum type.
 */
template <typename T>
constexpr typename std::enable_if<std::is_enum<T>::value, typename std::underlying_type<T>::type>::type
get_underlying_type(const T &value)
{
    return static_cast<typename std::underlying_type<T>::type>(value);
}

/**
 * @brief Given a boost::json::object instance and a std::string key, set argument t to the
 *        value if it exists within obj. If it does not exist, set t to an instance of T
 *        instantiated with the empty constructor.
 *
 * @tparam T is the expected type of the retrieved value within boost::json::object.
 * @param obj is the boost::json::object we expect to retrieve data from.
 * @param key is a std::string that maps the internal value to t.
 * @param t is a public member of some class/struct T that is mapped to a boost::json::object by
 *          the given key.
 */
template <class T> void extract(boost::json::object const &obj, const std::string &key, T &t)
{
    const boost::json::value *found_value = obj.if_contains(key);
    if (found_value)
    {
        t = boost::json::value_to<T>(*found_value);
    }
    else
    {
        t = T();
    }
}

/**
 * @brief Given a boost::json::object instance and a std::string key, set argument t to the
 *        expected enum value of the underlying type value if the key exists within obj.
 *        If it does not exist, set t to the default_enum value.
 *
 *        To future maintainers of this template:
 *        I am sure there is a way to declare this so that it will only compile
 *        if it is used when type T is an enum. I was not sure how to write that and instead of
 *        specializing the previous "extract" template, chose to differentiate by template names.
 *
 *        Do note that I am cannot simply take the value of obj[key], but I must cast that to the
 *        given enum type before setting it to t.
 *
 * @tparam T is the expected type the retrieved value within boost::json::object can be cast to.
 * @param obj is the boost::json::object we expect to retrieve data from.
 * @param key is a std::string that maps the internal value to t.
 * @param t is a public member of some class/struct T that is an enum type and is mapped to a
 *          boost::json::object by the given key.
 * @param default_enum is the default value to set to t if no value can be found within the obj.
 */
template <typename T>
void extract_enum(boost::json::object const &obj, const std::string &key, T &t, const T default_enum)
{
    // This is the value from the json string, expected to be an enum
    const boost::json::value *found_value = obj.if_contains(key);

    if (found_value)
    {
        // Next, T describes the enum type, which we now need to find the underlying type of
        // in order to give it to the boost::json::value_to call.
        t = static_cast<T>(boost::json::value_to<typename std::underlying_type<T>::type>(*found_value));
    }
    else
    {
        t = default_enum;
    }
}

/**
 * @brief This simply maps a C-style array to a std::vector. The json components do not play nicely
 *        with C-style arrays, so this mapping places it into a usable type. Fun fact, C-style arrays
 *        are not allowed to be returned directly from a function. That's why this template exists.
 *
 *        To future maintainers of this template:
 *        I am sure there is a way to constrain this to only compile given a C-style array, but that
 *        was not immediately evident to me at the time, so instead of a "to_vector" call, we are
 *        differentiating by template name.
 *
 * @tparam T is some C-style array type (example, int[], double[], etc.).
 * @param input_array is the C-style array instance we need to map to a std::vector
 * @return std::vector<typename std::remove_all_extents<T>::type> - Looks scary but is simply the
 *         returned std::vector of the type that the C-style input array indexed type is. Meaning,
 *         given a double[], we return a std::vector<double> and so on.
 */
template <typename T> std::vector<typename std::remove_all_extents<T>::type> array_to_vector(T &input_array)
{
    return std::vector<typename std::remove_all_extents<T>::type>(
        input_array, input_array + sizeof(input_array) / sizeof(input_array[0]));
}

/**
 * @brief Given a boost::json::object instance and a std::string key, set argument t to
 *        a C-style array instance generated from the found std::vector value. Because we
 *        have to repackage the C-style array to a std::vector, we must reverse that process
 *        when extracting data.
 *
 *        To future maintainers of this template:
 *        I am sure there is a way to constrain this to only compile given a C-style array, but that
 *        was not immediately evident to me at the time, so instead of being a specialized version of
 *        "extract", I differentiated by template name.
 *
 *        The only "tricky" part of this template is the 'typename std::remove_all_extents<T>::type>' that
 *        is used a couple of times. This is simply giving the types of the indvidual objects within the
 *        vector in order for us to cast to the C-style array correctly. Meaning, if given std::vector<double>,
 *        that type call will return to us "double".
 *
 *        Also note that C-style arrays are always initialized with their given size, so we can assume the array
 *        has been initialized properly. We could always do a check before setting to an index if we want to be 100%
 *        safe, however. But given the current use case, these should not be out of step.
 *
 * @tparam T is some C-style array type (example, int[], double[], etc.).
 * @param obj is the boost::json::object we expect to retrieve data from.
 * @param key is a std::string that maps the internal value to t.
 * @param t is a public member of some class/struct T that is C-style array type and is mapped to a
 *          boost::json::object by the given key.
 */
template <class T> void extract_array(boost::json::object const &obj, const std::string &key, T &t)
{
    const boost::json::value *found_value = obj.if_contains(key);
    if (found_value)
    {
        std::vector<typename std::remove_all_extents<T>::type> found_vector =
            boost::json::value_to<std::vector<typename std::remove_all_extents<T>::type>>(*found_value);

        for (uint i = 0; i < found_vector.size(); i++)
        {
            t[i] = found_vector[i];
        }
    }
}

/**
 * @brief This simply maps a std::array to a std::vector. The json components do not play nicely
 *        with std::arrays, so this mapping places it into a usable type.
 *
 *        To future maintainers of this template:
 *        I am sure there is a way to constrain this to only compile given a std::array, but that
 *        was not immediately evident to me at the time, so instead of a "to_vector" call, we are
 *        differentiating by template name.
 *
 * @tparam T is some std::array type (example, int[], double[], etc.).
 * @param input_array is the std::array instance we need to map to a std::vector
 * @return constexpr std::vector<typename std::remove_reference<decltype(*std::begin(std::declval<T&>()))>::type>
 *         - This is quite awful to look at, and honestly I don't fully understand all of the pieces, but this
 *         is doing the same thing our "typename std::remove_all_extents<T>::type" calls above were doing, gettting
 *         the underlying type of the given std::array. We would be using the same expression if it actually worked,
 *         but std::arrays, for whatever reason, are not compatible with that simpler statement.
 */
template <typename T>
constexpr std::vector<typename std::remove_reference<decltype(*std::begin(std::declval<T &>()))>::type>
std_array_to_vector(T &input_array)
{
    return std::vector<typename std::remove_reference<decltype(*std::begin(std::declval<T &>()))>::type>(
        input_array.begin(), input_array.end());
}

/**
 * @brief Given a boost::json::object instance and a std::string key, set argument t to
 *        a std::array instance generated from the found std::vector value. Because we
 *        have to repackage the std::array to a std::vector, we must reverse that process
 *        when extracting data.
 *
 *        To future maintainers of this template:
 *        I am sure there is a way to constrain this to only compile given a std::array, but that
 *        was not immediately evident to me at the time, so instead of being a specialized version of
 *        "extract", I differentiated by template name.
 *
 *        The only "tricky" part of this template is the
 *        'typename std::remove_reference<decltype(*std::begin(std::declval<T&>()))>::type'
 *        that is used a couple of times. This is simply giving the types of the indvidual objects within the
 *        vector in order for us to cast to the std::array correctly. Meaning, if given std::vector<double>,
 *        that type call will return to us "double".
 *
 *        Also note that std::arrays are always initialized with their given size, so we can assume the std::array
 *        has been initialized properly. We could always do a check before setting to an index if we want to be 100%
 *        safe, however. But given the current use case, these should not be out of step.
 *
 * @tparam T is some std::array type (example, int[], double[], etc.).
 * @param obj is the boost::json::object we expect to retrieve data from.
 * @param key is a std::string that maps the internal value to t.
 * @param t is a public member of some class/struct T that is std::array type and is mapped to a
 *          boost::json::object by the given key.
 */
template <class T> void extract_std_array(boost::json::object const &obj, const std::string &key, T &t)
{
    const boost::json::value *found_value = obj.if_contains(key);
    if (found_value)
    {
        std::vector<typename std::remove_reference<decltype(*std::begin(std::declval<T &>()))>::type> found_vector =
            boost::json::value_to<
                std::vector<typename std::remove_reference<decltype(*std::begin(std::declval<T &>()))>::type>>(
                *found_value);

        for (uint i = 0; i < found_vector.size(); i++)
        {
            t[i] = found_vector[i];
        }
    }
}

/**
 * @brief Given some object that can be written to JSON, write the object to a string.
 *
 * @tparam T is some type that can be written to a Json string.
 * @param input is the instance that we want to convert to a Json string.
 * @return The converted object as a Json string.
 */
template <typename T> std::string ToJsonString(const T &input)
{
    return boost::json::serialize(boost::json::value_from(input));
}

/**
 * @brief Given some object that can be written to JSON, write the object to the
 *        given path. The existence of the file does not matter, and it will
 *        overwrite any file that already exists. It will not create any new
 *        directories, and it will fail if it is given a directory that does not exist.
 *
 * @tparam T is some type that can be written to a Json file.
 * @param input is the instance that we want to write to Json.
 * @param output_file is the full filepath of the json file.
 * @return true if we could successfully write to the json file.
 * @return false if we could not write to the json file.
 */
template <typename T> bool ToJsonFile(const T &input, const std::string &output_file)
{
    bool was_write_successful = false;
    std::filesystem::path json_file_path = std::filesystem::absolute(output_file.c_str());

    // I don't care if the json file exists yet, but I do care if we have a valid directory path
    if (std::filesystem::exists(json_file_path.parent_path()))
    {
        std::ofstream json_stream(json_file_path);
        json_stream << ToJsonString(input);
        json_stream.close();

        was_write_successful = true;
    }

    return was_write_successful;
}

/**
 * @brief Given some propery json string that can be mapped to an
 *        object of type T, instantiate an instance of T created using
 *        this particular string.
 * @tparam T is some type that can be created from a Json string.

 */
template <typename T> T FromJsonString(const std::string &json_string)
{
    return boost::json::value_to<T>(boost::json::parse(json_string));
}

/**
 * @brief Given some json file that can be mapped to object of type T,
 *        instantiate an instance of T created using the particular file.
 *
 *        We could probably do more to make this safer to use from a file
 *        access perspective.
 *
 * @tparam T is some type that can be created from a Json file.
 * @param input_file is the full filepath of the json file.
 * @return T is the newly created instance from the given file.
 */
template <typename T> T FromJsonFile(const std::string &input_file)
{
    auto size = std::filesystem::file_size(input_file);
    std::string json_content(size, '\0');
    std::ifstream in(input_file);
    in.read(&json_content[0], size);

    return FromJsonString<T>(json_content);
}

inline std::string GetPrettyJsonString(const std::string &raw_json)
{
    std::stringstream ss;

    try
    {
        ss << raw_json;
        boost::property_tree::ptree tree;
        boost::property_tree::read_json(ss, tree);

        ss.str("");
        boost::property_tree::write_json(ss, tree, true);
    }
    catch (const std::exception &e)
    {
        ss << "Error parsing JSON with ptree: " << e.what() << "\nRaw Content:\n" << raw_json << "\n";
    }

    return ss.str();
}

inline std::string GetPrettyJsonFileAsString(const std::string &json_file)
{
    std::stringstream ss;

    try
    {
        boost::property_tree::ptree tree;
        boost::property_tree::read_json(json_file, tree);

        boost::property_tree::write_json(ss, tree, true);
    }
    catch (const std::exception &e)
    {
        ss << "Error reading or parsing JSON file: " << e.what() << "\nFilepatht: " << json_file << "\n";
    }

    return ss.str();
}
} // namespace json_templates