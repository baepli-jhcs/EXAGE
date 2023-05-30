#include <iostream>
#include <mutex>

#include "exage/Core/Debug.h"

namespace exage
{
    namespace
    {
        AssertHandler assertHandler {};
        std::mutex assertHandlerMutex {};
    }  // namespace

    void setAssertHandler(AssertHandler handler) noexcept
    {
        std::lock_guard const lock {assertHandlerMutex};
        assertHandler = std::move(handler);
    }

    void handleAssertFailure(std::string_view message) noexcept
    {
        std::lock_guard const lock {assertHandlerMutex};

        if (assertHandler)
        {
            assertHandler(message);
        }
        else
        {
            std::cerr << "Assertion failed: " << message << std::endl;
            debug_break();
            std::abort();
        }
    }

}  // namespace exage
