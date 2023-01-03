#include "Graphics/Context.h"
#include "Vulkan/VulkanContext.h"

namespace exage::Graphics
{
    auto Context::create(API api, WindowAPI windowAPI) -> Context*
    {
        switch (api)
        {
            case API::eVulkan:
                return new VulkanContext(windowAPI);
            default:
                return nullptr;
        }
    }
}  // namespace exage::Graphics