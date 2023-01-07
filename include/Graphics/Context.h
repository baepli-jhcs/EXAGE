#pragma once

#include "Core/Core.h"
#include "Core/Window.h"
#include "Swapchain.h"
#include "utils/classes.h"

namespace exage::Graphics
{
    enum class API
    {
        eVulkan,
    };

    enum class ContextError;

    class EXAGE_EXPORT Context
    {
      public:
        Context() = default;
        virtual ~Context() = default;
        EXAGE_DELETE_COPY(Context);
        EXAGE_DEFAULT_MOVE(Context);

        virtual auto createSwapchain(Window& window) -> Swapchain* = 0;

        [[nodiscard]] virtual auto getAPI() const -> API = 0;
        [[nodiscard]] static auto create(API api, WindowAPI windowAPI) noexcept
            -> tl::expected<std::unique_ptr<Context>, ContextError>;
    };

    enum class ContextError
    {
        eInvalidAPI,
        eInvalidWindow,
    };
}  // namespace exage::Graphics
