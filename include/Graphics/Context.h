#pragma once

#include "Core/Window.h"
#include "Swapchain.h"
#include "utils/classes.h"

namespace exage::Graphics
{
    enum class API
    {
        eVulkan,
    };

    class EXAGE_EXPORT Context
    {
      public:
        Context() = default;
        virtual ~Context() = default;
        EXAGE_DELETE_COPY(Context);
        EXAGE_DEFAULT_MOVE(Context);

        virtual auto createSwapchain(Window& window) -> Swapchain* = 0;

        virtual auto getAPI() const -> API = 0;
        static auto create(API api, WindowAPI windowAPI) -> Context*;
    };
}  // namespace exage::Graphics