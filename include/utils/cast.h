#pragma once

namespace exage
{

    // May change dynamic cast backend in the future
    template<typename R, typename T>
    inline auto dynamicCast(T value) -> R
    {
        return dynamic_cast<R>(value);
    }
}  // namespace exage