#pragma once

#include <functional>
#include <string_view>

#include "debugbreak.h"

#ifdef EXAGE_WINDOWS
#    define __declspec(dllexport)
#else
#    define EXAGE_EXPORT
#endif

#ifdef _MSC_VER
#    define EXAGE_MSVC
#endif

#ifdef __GNUC__
#    define EXAGE_GCC
#endif

#ifdef __clang__
#    define EXAGE_CLANG
#endif

#ifdef EXAGE_MSVC
#    define EXAGE_UNREACHABLE __assume(0)
#else
#    define EXAGE_UNREACHABLE __builtin_unreachable()
#endif

#ifdef EXAGE_MSVC
#    define EXAGE_ASSUME(cond) __assume(cond)
#elif defined(EXAGE_CLANG)
#    define EXAGE_ASSUME(cond) __builtin_assume(cond)
#elif defined(EXAGE_GCC)
#    define EXAGE_ASSUME(cond) \
        if (!(cond)) \
        { \
            __builtin_unreachable(); \
        }
#endif

#ifndef EXAGE_USE_ASSERTS
#    ifdef EXAGE_DEBUG
#        define EXAGE_USE_ASSERTS 1
#    else
#        define EXAGE_USE_ASSERTS 0
#    endif
#endif

namespace exage
{
    using AssertHandler = std::function<void(std::string_view message)>;

    void setAssertHandler(AssertHandler handler) noexcept;
    void handleAssertFailure(std::string_view message) noexcept;

    inline void debugAssert([[maybe_unused]] bool condition,
                            [[maybe_unused]] std::string_view message) noexcept
    {
#if EXAGE_USE_ASSERTS
        if (!condition)
        {
            handleAssertFailure(message);
        }
#endif
    }

    inline void debugAssume([[maybe_unused]] bool condition,
                            [[maybe_unused]] std::string_view message) noexcept
    {
#if EXAGE_USE_ASSERTS
        if (!condition)
        {
            handleAssertFailure(message);
        }
#else
        EXAGE_ASSUME(condition);
#endif
    }

}  // namespace exage
