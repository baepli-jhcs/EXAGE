#include <chrono>

#include "exage/Core/Timer.h"

namespace exage
{
    namespace
    {
        std::chrono::time_point<std::chrono::high_resolution_clock> startTime;
    }

    void Timer::init() noexcept
    {
        startTime = std::chrono::high_resolution_clock::now();
    }

    auto Timer::getTimeFromStart() noexcept -> double
    {
        return std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - startTime)
            .count();
    }
}  // namespace exage