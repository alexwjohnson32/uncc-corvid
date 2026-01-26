#pragma once

#include <fstream>
#include <type_traits>
#include <vector>
#include <array>
#include <string>
#include <filesystem>
#include <string_view>

#include <boost/json.hpp>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

namespace utils
{

/**
 * @brief Simple wrapper to signal that we want the raw JSON serialized as a string.
 */
struct raw_json
{
    std::string &value;
    explicit raw_json(std::string &s) : value(s) {}
};

namespace detail
{
// Detect std::array
template <typename T> struct is_std_array : std::false_type
{
};
template <typename T, std::size_t N> struct is_std_array<std::array<T, N>> : std::true_type
{
};
template <typename T> constexpr bool is_std_array_v = is_std_array<std::remove_cv_t<T>>::value;

// Element type extraction
template <typename T> struct element_type
{
    using type = std::remove_all_extents_t<T>;
};
template <typename T, std::size_t N> struct element_type<std::array<T, N>>
{
    using type = T;
};
template <typename T> using element_type_t = typename element_type<std::remove_cv_t<T>>::type;

// Boost JSON Serialization Detection
// These traits check f boost::json::value_from or value_to can be called for type T.
template <typename T, typename = void> struct is_json_writable : std::false_type
{
};

template <typename T>
struct is_json_writable<T, std::void_t<decltype(boost::json::value_from(std::declval<const T &>()))>> : std::true_type
{
};

template <typename T, typename = void> struct is_json_readable : std::false_type
{
};

template <typename T>
struct is_json_readable<T, std::void_t<decltype(boost::json::value_to<T>(std::declval<boost::json::value>()))>>
    : std::true_type
{
};

} // namespace detail

/**
 * @brief Modernized helper to get the underlying type of an enum.
 */
template <typename T>
constexpr auto get_underlying_type(const T &value) -> std::enable_if_t<std::is_enum_v<T>, std::underlying_type_t<T>>
{
    return static_cast<std::underlying_type_t<T>>(value);
}

/**
 * @brief Unified function to map C-style arrays or std::array to a std::vector.
 */
template <typename T> auto to_vector(T &input)
{
    static_assert(std::is_array_v<T> || detail::is_std_array_v<T>,
                  "to_vector(T&) requires a C-style array or std::array.");
    return std::vector<detail::element_type_t<T>>(std::begin(input), std::end(input));
}

/**
 * @brief Comprehensive extract function.
 * Supports: Raw Serialization (via raw_json), C-Arrays, Enums, and standard Boost types.
 */
template <typename T> void extract(const boost::json::object &obj, const std::string &key, T &&t)
{
    const boost::json::value *found = obj.if_contains(key);

    // Use decay for branch logic, but remove_reference for array operations
    using DecayedT = std::decay_t<T>;
    using BaseT = std::remove_reference_t<T>;

    // 1. RAW SERIALIZATION BRANCH
    if constexpr (std::is_same_v<DecayedT, raw_json>)
    {
        t.value = found ? boost::json::serialize(*found) : "";
    }
    // 2. C-STYLE ARRAY BRANCH
    else if constexpr (std::is_array_v<BaseT>)
    {
        using Elem = detail::element_type_t<BaseT>;
        if (found && found->is_array())
        {
            auto const &jarr = found->as_array();
            constexpr std::size_t N = std::extent_v<BaseT>;
            for (std::size_t i = 0; i < std::min(N, jarr.size()); ++i)
            {
                t[i] = boost::json::value_to<Elem>(jarr[i]);
            }
        }
        else
        {
            for (auto &item : t) item = Elem();
        }
    }
    // 3. ENUM BRANCH
    else if constexpr (std::is_enum_v<DecayedT>)
    {
        try
        {
            if (found)
            {
                // To handle enums without a custom tag_invoke, convert to the
                // underlying integer type first, then cast to the enum type.
                using Underlying = std::underlying_type_t<DecayedT>;
                t = static_cast<DecayedT>(boost::json::value_to<Underlying>(*found));
            }
            else
            {
                t = static_cast<DecayedT>(0);
            }
        }
        catch (...)
        {
            t = static_cast<DecayedT>(0);
        }
    }
    // 4. STANDARD TYPES (std::vector, std::array, int, std::string, etc.)
    else
    {
        static_assert(detail::is_json_readable<DecayedT>::value,
                      "Type T cannot be created from boost::json. Ensure you have defined 'tag_invoke' "
                      "for this type or that it is a standard type supported by Boost JSON.");
        try
        {
            t = found ? boost::json::value_to<DecayedT>(*found) : DecayedT();
        }
        catch (...)
        {
            t = DecayedT();
        }
    }
}

/**
 * @brief Takes a JSON string and formats it with indentations and newlines.
 */
inline std::string FormatPretty(std::string_view raw_json)
{
    std::stringstream input(std::string{ raw_json });
    std::stringstream output;
    try
    {
        boost::property_tree::ptree pt;
        boost::property_tree::read_json(input, pt);
        boost::property_tree::write_json(output, pt, true);
        return output.str();
    }
    catch (const std::exception &e)
    {
        return "/* Error Formatting JSON: " + std::string(e.what()) + " */\n" + std::string(raw_json);
    }
}

/**
 * @brief Converts any compatible type to a serialized JSON string.
 */
template <typename T> std::string ToJsonString(const T &input, bool pretty = false)
{
    static_assert(detail::is_json_writable<T>::value,
                  "Type T is not serializable by boost::json. Ensure you have defined 'tag_invoke' "
                  "for this type or that it is a standard type supported by Boost JSON.");

    std::string str = boost::json::serialize(boost::json::value_from(input));
    return pretty ? FormatPretty(str) : str;
}

/**
 * @brief Parses a JSON string into an object of type T.
 */
template <typename T> T FromJsonString(std::string_view json_string)
{
    static_assert(detail::is_json_readable<T>::value,
                  "Type T cannot be created from boost::json. Ensure you have defined 'tag_invoke' "
                  "for this type or that it is a standard type supported by Boost JSON.");

    boost::json::string_view bv{ json_string.data(), json_string.size() };
    return boost::json::value_to<T>(boost::json::parse(bv));

    // return boost::json::value_to<T>(boost::json::parse(json_string));
}

/**
 * @brief Reads a JSON file into a string, optionally formatting it.
 */
inline std::string ReadJsonFileAsString(const std::filesystem::path &path, bool pretty = false)
{
    if (!std::filesystem::exists(path)) return "";

    std::ifstream ifs(path, std::ios::in | std::ios::binary);
    std::stringstream ss;
    ss << ifs.rdbuf();

    std::string json_str = ss.str();
    if (json_str.empty()) return "";
    return pretty ? FormatPretty(json_str) : json_str;
}

/**
 * @brief Reads a JSON file and parses it into an object of type T.
 */
template <typename T> T FromJsonFile(const std::filesystem::path &path)
{
    static_assert(detail::is_json_readable<T>::value,
                  "Type T cannot be created from boost::json. Ensure you have defined 'tag_invoke' "
                  "for this type or that it is a standard type supported by Boost JSON.");

    std::string content = ReadJsonFileAsString(path);
    if (content.empty())
    {
        throw std::runtime_error("JSON file is empty or does not exist: " + path.string());
    }
    return FromJsonString<T>(content);
}

/**
 * @brief Writes a compatible type to a file as JSON.
 */
template <typename T> bool ToJsonFile(const T &input, const std::filesystem::path &path, bool pretty = false)
{
    static_assert(detail::is_json_writable<T>::value,
                  "Type T is not serializable by boost::json. Ensure you have defined 'tag_invoke' "
                  "for this type or that it is a standard type supported by Boost JSON.");

    try
    {
        std::filesystem::path parent = path.parent_path();
        if (!parent.empty() && !std::filesystem::exists(parent))
        {
            return false;
        }

        std::ofstream ofs(path, std::ios::out | std::ios::trunc);
        if (!ofs.is_open()) return false;

        ofs << ToJsonString(input, pretty);
        return true;
    }
    catch (...)
    {
        return false;
    }
}

} // namespace utils