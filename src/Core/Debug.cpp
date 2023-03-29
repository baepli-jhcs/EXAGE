#include "exage/Core/Debug.h"
#include <iostream>
#include <mutex>

namespace exage
{
    namespace
    {
        AssertHandler assertHandler {};
        std::mutex assertHandlerMutex {};
    } // namespace

    EXAGE_EXPORT void setAssertHandler(AssertHandler handler) noexcept
    {
        std::lock_guard const lock {assertHandlerMutex};
        assertHandler = std::move(handler);
    }

    EXAGE_EXPORT void handleAssertFailure(std::string_view message) noexcept
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
