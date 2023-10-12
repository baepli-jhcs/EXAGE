#include "exage/platform/Vulkan/VulkanFence.h"

#include "exage/Graphics/Context.h"

namespace exage::Graphics
{
    VulkanFence::VulkanFence(VulkanContext& context) noexcept
        : _context(context)
    {
        vk::FenceCreateInfo createInfo;
        createInfo.flags = vk::FenceCreateFlagBits::eSignaled;

        checkVulkan(_context.get().getDevice().createFence(&createInfo, nullptr, &_fence));
    }

    VulkanFence::~VulkanFence()
    {
        _context.get().getDevice().destroyFence(_fence);
    }

    VulkanFence::VulkanFence(VulkanFence&& old) noexcept
        : _context(old._context)
        , _fence(old._fence)
    {
        old._fence = nullptr;
    }

    auto VulkanFence::operator=(VulkanFence&& old) noexcept -> VulkanFence&
    {
        if (this != &old)
        {
            _context.get().getDevice().destroyFence(_fence);

            _context = old._context;
            _fence = old._fence;
            old._fence = nullptr;
        }

        return *this;
    }

    void VulkanFence::wait() noexcept
    {
        checkVulkan(_context.get().getDevice().waitForFences(1, &_fence, VK_TRUE, UINT64_MAX));
    }

    void VulkanFence::reset() noexcept
    {
        checkVulkan(_context.get().getDevice().resetFences(1, &_fence));
    }

    auto VulkanFence::getState() const noexcept -> State
    {
        vk::Result const result = _context.get().getDevice().getFenceStatus(_fence);

        if (result == vk::Result::eSuccess)
        {
            return State::eSignaled;
        }
        if (result == vk::Result::eNotReady)
        {
            return State::eUnsignaled;
        }

        checkVulkan(result);
        return State::eUnsignaled;
    }

}  // namespace exage::Graphics