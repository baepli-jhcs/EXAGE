#pragma once
#include <memory>

namespace exage
{
    // May change dynamic cast backend in the future
    template<typename R, typename T>
    auto dynamicCast(T value) -> R
    {
        return dynamic_cast<R>(value);
    }

    template<typename R, typename T>
    auto dynamicUniqueCast(std::unique_ptr<T>& value) noexcept -> std::unique_ptr<R>
    {
        auto result = dynamic_cast<R*>(value.get());
        if (result != nullptr)
        [[likely]]
        {
            value.release();
            return std::unique_ptr<R>(result);
        }
        return nullptr;
    }
} // namespace exage
