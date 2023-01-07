#include "Graphics/Context.h"
#include "Vulkan/VulkanContext.h"

namespace exage::Graphics
{
    auto Context::create(API api, WindowAPI windowAPI) noexcept
        -> tl::expected<std::unique_ptr<Context>, ContextError>
    {
        switch (api)
        {
            case API::eVulkan:
                try
                {
                    return std::make_unique<VulkanContext>(windowAPI);
                }
                catch (const std::exception& e)
                {
                    return tl::make_unexpected(ContextError::eInvalidWindow);
                }

            default:
                return tl::make_unexpected(ContextError::eInvalidAPI);
        }
    }
}  // namespace exage::Graphics