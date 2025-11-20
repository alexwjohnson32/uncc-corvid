#pragma once

#include <chrono>
#include <ratio>
namespace utils
{
class Stopwatch
{
  private:
    std::chrono::high_resolution_clock::time_point m_start_time;

  public:
    void start() { m_start_time = std::chrono::high_resolution_clock::now(); }
    double elapsedMilliseconds() const
    {
        auto now = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> elapsed = now - m_start_time;
        return elapsed.count();
    }
};
} // namespace utils