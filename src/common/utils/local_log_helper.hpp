#pragma once

#include <fstream>
#include <ios>
#include <ostream>
#include <string>
#include <functional>
#include <sstream>
#include <type_traits>

namespace utils
{
class LocalLogHelper
{
  private:
    std::ofstream m_output_stream{};
    std::stringstream m_formatting_stream{};
    std::function<void(const std::string &)> m_on_write;

    /**
     * Appends data to both the file (if open) and internal buffer.
     * Must remain in header because it is a template.
     */
    template <typename T> void Append(const T &msg)
    {
        if (m_output_stream.is_open()) m_output_stream << msg;
        m_formatting_stream << msg;
    }

    void AppendManip(std::ostream &(*manip)(std::ostream &));
    void FlushToCallback();

  public:
    LocalLogHelper();
    LocalLogHelper(const std::string &output_file);
    ~LocalLogHelper();

    bool IsOpen() const;
    void SetOnWriteCallback(std::function<void(const std::string &)> on_write);
    void SetOutputFile(const std::string &output_file);

    using StreamManipulator = std::ostream &(*)(std::ostream &);

    // --- PROXY CLASS DEFINITION ---
    class LogStream
    {
      private:
        LocalLogHelper &m_parent;
        bool m_should_flush{};

      public:
        explicit LogStream(LocalLogHelper &parent);
        LogStream(LogStream &&other) noexcept;

        LogStream(const LogStream &) = delete;
        LogStream &operator=(const LogStream &) = delete;

        ~LogStream();

        /**
         * Must remain in header because it is a template.
         */
        template <typename T> LogStream &operator<<(const T &msg)
        {
            m_parent.Append(msg);
            return *this;
        }

        LogStream &operator<<(StreamManipulator manip);
    };

    /**
     * Entry point operator.
     * Must remain in header because it is a template.
     */
    template <typename T> LogStream operator<<(const T &msg)
    {
        static_assert(!std::is_base_of_v<std::ios_base, T>,
                      "Compilation Error: You cannot pass a stream object to the logger.");
        LogStream proxy(*this);
        proxy << msg;
        return proxy;
    }

    LogStream operator<<(StreamManipulator manip);
};
} // namespace utils