#include "local_log_helper.hpp"

// --- LocalLogHelper Implementation ---

utils::LocalLogHelper::LocalLogHelper(const std::string &output_file) : m_output_stream(output_file) {}

utils::LocalLogHelper::~LocalLogHelper()
{
    if (IsOpen()) m_output_stream.close();
}

bool utils::LocalLogHelper::IsOpen() const { return m_output_stream.is_open(); }

void utils::LocalLogHelper::SetOnWriteCallback(std::function<void(const std::string &)> on_write)
{
    m_on_write = on_write;
}

void utils::LocalLogHelper::SetOutputFile(const std::string &output_file)
{
    if (IsOpen())
    {
        m_output_stream.close();
    }

    if (!output_file.empty())
    {
        m_output_stream = std::ofstream(output_file);
    }
}

void utils::LocalLogHelper::AppendManip(std::ostream &(*manip)(std::ostream &))
{
    if (m_output_stream.is_open()) m_output_stream << manip;
    m_formatting_stream << manip;
}

void utils::LocalLogHelper::FlushToCallback()
{
    if (m_on_write) m_on_write(m_formatting_stream.str());

    m_formatting_stream.str(std::string());
    m_formatting_stream.clear();
}

// Note: The return type LogStream must be qualified as it is nested inside the class and namespace
utils::LocalLogHelper::LogStream utils::LocalLogHelper::operator<<(StreamManipulator manip)
{
    LogStream proxy(*this);
    proxy << manip;
    return proxy;
}

// --- LogStream Proxy Implementation ---

utils::LocalLogHelper::LogStream::LogStream(LocalLogHelper &parent) : m_parent(parent), m_should_flush(true) {}

utils::LocalLogHelper::LogStream::LogStream(LogStream &&other) noexcept
    : m_parent(other.m_parent), m_should_flush(other.m_should_flush)
{
    other.m_should_flush = false;
}

utils::LocalLogHelper::LogStream::~LogStream()
{
    if (m_should_flush) m_parent.FlushToCallback();
}

utils::LocalLogHelper::LogStream &utils::LocalLogHelper::LogStream::operator<<(StreamManipulator manip)
{
    m_parent.AppendManip(manip);
    return *this;
}