#pragma once

#include <chrono>

namespace exage
{
    class Timer
    {
      public:
        Timer() noexcept { reset(); }

        void reset() noexcept
        {
            _lastFrameTime = std::chrono::high_resolution_clock::now();
            _currentFrameTime = std::chrono::high_resolution_clock::now();
        }
        [[nodiscard]] auto nextFrame() noexcept -> float
        {
            _lastFrameTime = _currentFrameTime;
            _currentFrameTime = std::chrono::high_resolution_clock::now();

            return std::chrono::duration<float>(_currentFrameTime - _lastFrameTime).count();
        }

        /* Returns time from start of the program in seconds */
        [[nodiscard]] static auto getTimeFromStart() noexcept -> double;

      private:
        static void init() noexcept;

        std::chrono::time_point<std::chrono::high_resolution_clock> _lastFrameTime;
        std::chrono::time_point<std::chrono::high_resolution_clock> _currentFrameTime;

        friend void init() noexcept;
    };
}  // namespace exage