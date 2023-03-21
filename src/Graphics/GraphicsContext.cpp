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
            {
                tl::expected value = VulkanContext::create(createInfo);
                if (!value.has_value())
                {
                    return tl::make_unexpected(value.error());
                }
                return std::make_unique<VulkanContext>(std::move(value.value()));
            }
        }

        debugAssume(/*condition=*/false, "Unsupported API");
        return tl::make_unexpected(ErrorCode::eUnsupportedAPI);
    }
}  // namespace exage::Graphics
