#include "Core/Debug.h"
#include <iostream>
#include <mutex>

namespace exage
{
    static AssertHandler assertHandler {};
    static std::mutex assertHandlerMutex {};

    EXAGE_EXPORT void setAssertHandler(AssertHandler handler) noexcept
    {
        std::lock_guard lock {assertHandlerMutex};
        assertHandler = handler;
    }

    EXAGE_EXPORT void handleAssertFailure(std::string_view message) noexcept
    {
        std::lock_guard lock {assertHandlerMutex};
        
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

    
}  // namespace exage::Graphics