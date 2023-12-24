#pragma once
#include <memory_resource>

#include "exage/Graphics/CommandBuffer.h"
#include "exage/Graphics/Commands.h"
#include "exage/Graphics/Texture.h"
#include "exage/platform/Vulkan/VulkanContext.h"

namespace exage::Graphics
{
    class VulkanGraphicsPipeline;

    class VulkanCommandBuffer final : public CommandBuffer
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
                  uint32_t firstVertex,
                  uint32_t instanceCount,
                  uint32_t firstInstance) noexcept override;

        void drawIndexed(uint32_t indexCount,
                         uint32_t firstIndex,
                         uint32_t vertexOffset,
                         uint32_t instanceCount,
                         uint32_t firstInstance) noexcept override;

        void textureBarrier(std::shared_ptr<Texture> texture,
                            Texture::Layout oldLayout,
                            Texture::Layout newLayout,
                            PipelineStage srcStage,
                            PipelineStage dstStage,
                            Access srcAccess,
                            Access dstAccess,
                            QueueOwnership initialQueue,
                            QueueOwnership finalQueue) noexcept override;

        void bufferBarrier(std::shared_ptr<Buffer> buffer,
                           PipelineStage srcStage,
                           PipelineStage dstStage,
                           Access srcAccess,
                           Access dstAccess,
                           QueueOwnership initialQueue,
                           QueueOwnership finalQueue) noexcept override;

        void blit(std::shared_ptr<Texture> srcTexture,
                  std::shared_ptr<Texture> dstTexture,
                  glm::uvec3 srcOffset,
                  glm::uvec3 dstOffset,
                  uint32_t srcMipLevel,
                  uint32_t dstMipLevel,
                  uint32_t srcFirstLayer,
                  uint32_t dstFirstLayer,
                  uint32_t layerCount,
                  glm::uvec3 srcExtent,
                  glm::uvec3 dstExtent) noexcept override;

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

        void copyBuffer(std::shared_ptr<Buffer> srcBuffer,
                        std::shared_ptr<Buffer> dstBuffer,
                        uint64_t srcOffset,
                        uint64_t dstOffset,
                        uint64_t size) noexcept override;

        void copyBufferToTexture(std::shared_ptr<Buffer> srcBuffer,
                                 std::shared_ptr<Texture> dstTexture,
                                 uint64_t srcOffset,
                                 glm::uvec3 dstOffset,
                                 uint32_t dstMipLevel,
                                 uint32_t dstFirstLayer,
                                 uint32_t layerCount,
                                 glm::uvec3 extent) noexcept override;

        void copyTextureToBuffer(std::shared_ptr<Texture> srcTexture,
                                 std::shared_ptr<Buffer> dstBuffer,
                                 glm::uvec3 srcOffset,
                                 uint32_t srcMipLevel,
                                 uint32_t srcFirstLayer,
                                 uint32_t layerCount,
                                 glm::uvec3 extent,
                                 uint64_t dstOffset) noexcept override;

        void bindGraphicsPipeline(std::shared_ptr<GraphicsPipeline> pipeline) noexcept override;

        void setPushConstant(std::span<const std::byte> data) noexcept override;

        void bindVertexBuffer(std::shared_ptr<Buffer> buffer, uint64_t offset) noexcept override;

        void bindIndexBuffer(std::shared_ptr<Buffer> buffer, uint64_t offset) noexcept override;

        void bindStorageBuffer(std::shared_ptr<Buffer> buffer, uint32_t binding) noexcept override;

        void bindSampler(std::shared_ptr<Sampler> sampler, uint32_t binding) noexcept override;

        void bindSampledTexture(std::shared_ptr<Texture> texture,
                                uint32_t binding,
                                Texture::Aspect) noexcept override;
        void bindStorageTexture(std::shared_ptr<Texture> texture,
                                uint32_t binding,
                                Texture::Aspect aspect) noexcept override;

        void userDefined(std::function<void(CommandBuffer&)> commandFunction) noexcept override;

        [[nodiscard]] auto getCommandBuffer() const noexcept -> vk::CommandBuffer
        {
            return _commandBuffer;
        }

        EXAGE_VULKAN_DERIVED

      private:
        void processCommand(const Commands::GPUCommand& command) noexcept;

        [[nodiscard]] auto getQueueFamilyIndex(QueueOwnership ownership) noexcept -> uint32_t;

        std::reference_wrapper<VulkanContext> _context;
        vk::CommandBuffer _commandBuffer;

        std::vector<Commands::GPUCommand> _commands {};
        std::vector<DataDependency> _dataDependencies {};

        VulkanGraphicsPipeline* _currentPipeline = nullptr;
    };
}  // namespace exage::Graphics
