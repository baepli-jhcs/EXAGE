#include "Graphics/Context.h"
#include "Vulkan/VulkanContext.h"

namespace exage::Graphics
{
    auto Context::create(ContextCreateInfo& createInfo) noexcept
    -> tl::expected<std::unique_ptr<Context>, Error>
    {
        switch (createInfo.api)
        {
            case API::eVulkan:
                return VulkanContext::create(createInfo);

            default:
                return tl::make_unexpected(ErrorCode::eInvalidAPI);
        }
    }
} // namespace exage::Graphics
