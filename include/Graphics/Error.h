#pragma once

#include <variant>

#include "Core/Core.h"

namespace exage::Graphics
{
    enum class ErrorCode : uint32_t
    {
        eUnsupportedAPI,
        eSwapchainOutOfDate,
    };

    using Error = std::variant<ErrorCode>;
}  // namespace exage::Graphics
