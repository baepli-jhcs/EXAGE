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

      private:
        std::chrono::time_point<std::chrono::high_resolution_clock> _lastFrameTime;
        std::chrono::time_point<std::chrono::high_resolution_clock> _currentFrameTime;
    };
}  // namespace exage