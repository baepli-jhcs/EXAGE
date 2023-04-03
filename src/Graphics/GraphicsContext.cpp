#include "exage/Graphics/Context.h"
#include "exage/platform/Vulkan/VulkanContext.h"

namespace exage::Graphics
{
    auto Context::create(ContextCreateInfo& createInfo) noexcept
        -> tl::expected<std::unique_ptr<Context>, Error>
    {
        switch (createInfo.api)
        {
            case API::eVulkan:
            {
                tl::expected value = VulkanContext::create(createInfo);
                if (!value.has_value())
                {
                    return tl::make_unexpected(value.error());
                }
                return std::unique_ptr<Context>(value->release());
            }
        }

        debugAssume(/*condition=*/false, "Unsupported API");
        return tl::make_unexpected(GraphicsError::eUnsupportedAPI);
    }
}  // namespace exage::Graphics
