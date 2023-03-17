#pragma once
#include <memory_resource>

#include "Graphics/CommandBuffer.h"
#include "Graphics/Commands.h"
#include "Vulkan/VulkanContext.h"

namespace exage::Graphics
{
    class EXAGE_EXPORT VulkanCommandBuffer final : public CommandBuffer
    {
      public:
        explicit VulkanCommandBuffer(VulkanContext& context) noexcept;
        ~VulkanCommandBuffer() override;

        EXAGE_DELETE_COPY(VulkanCommandBuffer);

        VulkanCommandBuffer(VulkanCommandBuffer&& old) noexcept;
        auto operator=(VulkanCommandBuffer&& old) noexcept -> VulkanCommandBuffer&;

        void begin() noexcept override;
        void end() noexcept override;

        void insertDataDependency(DataDependency dependency) noexcept override;

        void draw(uint32_t vertexCount,
                  uint32_t instanceCount,
                  uint32_t firstVertex,
                  uint32_t firstInstance) noexcept override;

        void drawIndexed(uint32_t indexCount,
                         uint32_t instanceCount,
                         uint32_t firstIndex,
                         uint32_t vertexOffset,
                         uint32_t firstInstance) noexcept override;

        void textureBarrier(std::shared_ptr<Texture> texture,
                            Texture::Layout newLayout,
                            PipelineStage srcStage,
                            PipelineStage dstStage,
                            Access srcAccess,
                            Access dstAccess) noexcept override;

        void blit(std::shared_ptr<Texture> srcTexture,
                  std::shared_ptr<Texture> dstTexture,
                  glm::uvec3 srcOffset,
                  glm::uvec3 dstOffset,
                  uint32_t srcMipLevel,
                  uint32_t dstMipLevel,
                  uint32_t srcFirstLayer,
                  uint32_t dstFirstLayer,
                  uint32_t layerCount,
                  glm::uvec3 extent) noexcept override;

        void setViewport(glm::uvec2 offset, glm::uvec2 extent) noexcept override;
        void setScissor(glm::uvec2 offset, glm::uvec2 extent) noexcept override;

        void clearTexture(std::shared_ptr<Texture> texture,
                          glm::vec4 color,
                          uint32_t mipLevel,
                          uint32_t firstLayer,
                          uint32_t layerCount) noexcept override;

        void beginRendering(std::shared_ptr<FrameBuffer> frameBuffer,
                            std::vector<ClearColor> clearColors,
                            ClearDepthStencil clearDepth) noexcept override;
        void endRendering() noexcept override;

        void userDefined(std::function<void(CommandBuffer&)> commandFunction) noexcept override;

        [[nodiscard]] auto getCommandBuffer() const noexcept -> vk::CommandBuffer
        {
            return _commandBuffer;
        }

        EXAGE_VULKAN_DERIVED

      private:
        void processCommand(const GPUCommand& command) noexcept;

        std::reference_wrapper<VulkanContext> _context;
        vk::CommandPool _commandPool;
        vk::CommandBuffer _commandBuffer;

        std::vector<GPUCommand> _commands {};
        std::vector<DataDependency> _dataDependencies {};
        std::unique_ptr<std::mutex> _commandsMutex = std::make_unique<std::mutex>();
        std::unique_ptr<std::mutex> _dataDependenciesMutex = std::make_unique<std::mutex>();
    };
}  // namespace exage::Graphics
