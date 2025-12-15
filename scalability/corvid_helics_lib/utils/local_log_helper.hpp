#pragma once

#include <fstream>
#include <ostream>
#include <string>
#include <functional>
#include <iostream>
#include <sstream>

namespace utils
{
class LocalLogHelper
{
  private:
    const std::string &m_output_file;
    std::ofstream m_output_stream{};

    // This is an internal buffer for accumulating message parts
    std::stringstream m_formatting_stream{};
    std::function<void(const std::string &)> m_on_write;

    // Proxy helpers

    /**
     * Appends dat to both the file (if open) and internal buffer
     */
    template <typename T> void Append(const T &msg)
    {
        if (m_output_stream.is_open()) m_output_stream << msg;
        m_formatting_stream << msg;
    }

    /**
     * Appends manipulators (like std::endl) to file (if open) and internal buffer
     */
    void AppendManip(std::ostream &(*manip)(std::ostream &))
    {
        if (m_output_stream.is_open()) manip(m_output_stream);
        manip(m_formatting_stream);
    }

    /**
     * Collected by the Proxy destructor to fire the callback once
     */
    void FlushToCallback()
    {
        if (m_on_write) m_on_write(m_formatting_stream.str());

        // Reset the text content but preserve formatting flags (hex, precision, etc.)
        m_formatting_stream.str(std::string());
        m_formatting_stream.clear();
    }

  public:
    LocalLogHelper(const std::string &output_file) : m_output_file(output_file), m_output_stream(m_output_file) {}
    ~LocalLogHelper()
    {
        if (IsOpen()) m_output_stream.close();
    }

    std::string OutputFile() const { return m_output_file; }
    bool IsOpen() const { return m_output_stream.is_open(); }

    void SetOnWriteCallback(std::function<void(const std::string &)> on_write) { m_on_write = on_write; }

    // Alias for a function pointer for stream manipulation
    using StreamManipulator = std::ostream &(*)(std::ostream &);

    // --- PROXY CLASS DEFINITION ---
    // This temporary object lives only for the duration of the logging statement.
    class LogStream
    {
      private:
        LocalLogHelper &m_parent;
        bool m_should_flush{};

      public:
        explicit LogStream(LocalLogHelper &parent) : m_parent(parent), m_should_flush(true) {}

        /**
         * Move Constructor
         * When the proxy is returned from operator<<, C++ might move it. We must
         * ensure only the final surviving proxy triggers the callback
         */
        LogStream(LogStream &&other) noexcept : m_parent(other.m_parent), m_should_flush(other.m_should_flush)
        {
            other.m_should_flush = false;
        }

        // Disable copying
        LogStream(const LogStream &) = delete;
        LogStream &operator=(const LogStream &) = delete;

        /**
         * Destructor
         * Triggers the one existing callback
         */
        ~LogStream()
        {
            if (m_should_flush) m_parent.FlushToCallback();
        }

        // Chainable operators for the proxy
        template <typename T> LogStream &operator<<(const T &msg)
        {
            m_parent.Append(msg);
            return *this;
        }

        LogStream &operator<<(StreamManipulator manip)
        {
            m_parent.AppendManip(manip);
            return *this;
        }
    };

    // Entry Point Operators

    // These create the intial LogStream proxy
    template <typename T> LogStream operator<<(const T &msg)
    {
        LogStream proxy(*this);
        proxy << msg;
        return proxy;
    }

    LogStream operator<<(StreamManipulator manip)
    {
        LogStream proxy(*this);
        proxy << manip;
        return proxy;
    }
};
} // namespace utils