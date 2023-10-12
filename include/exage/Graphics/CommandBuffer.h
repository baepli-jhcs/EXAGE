#pragma once

#include <memory_resource>
#include <optional>
#include <thread>
#include <utility>

#include "Commands.h"
#include "Error.h"
#include "exage/Core/Core.h"
#include "exage/Graphics/Context.h"

namespace exage::Graphics
{
    class CommandBuffer
    {
      public:
        CommandBuffer() noexcept = default;
        virtual ~CommandBuffer() = default;

        EXAGE_DEFAULT_COPY(CommandBuffer);
        EXAGE_DEFAULT_MOVE(CommandBuffer);

        virtual void begin() noexcept = 0;
        virtual void end() noexcept = 0;

        virtual void insertDataDependency(DataDependency dependency) noexcept = 0;

        virtual void draw(uint32_t vertexCount,
                          uint32_t firstVertex,
                          uint32_t instanceCount,
                          uint32_t firstInstance) noexcept = 0;

        virtual void drawIndexed(uint32_t indexCount,
                                 uint32_t firstIndex,
                                 uint32_t vertexOffset,
                                 uint32_t instanceCount,
                                 uint32_t firstInstance) noexcept = 0;

        virtual void textureBarrier(std::shared_ptr<Texture> texture,
                                    Texture::Layout oldLayout,
                                    Texture::Layout newLayout,
                                    PipelineStage srcStage,
                                    PipelineStage dstStage,
                                    Access srcAccess,
                                    Access dstAccess,
                                    QueueOwnership initialQueue,
                                    QueueOwnership finalQueue) noexcept = 0;

        virtual void bufferBarrier(std::shared_ptr<Buffer> buffer,
                                   PipelineStage srcStage,
                                   PipelineStage dstStage,
                                   Access srcAccess,
                                   Access dstAccess,
                                   QueueOwnership initialQueue,
                                   QueueOwnership finalQueue) noexcept = 0;

        virtual void blit(std::shared_ptr<Texture> srcTexture,
                          std::shared_ptr<Texture> dstTexture,
                          glm::uvec3 srcOffset,
                          glm::uvec3 dstOffset,
                          uint32_t srcMipLevel,
                          uint32_t dstMipLevel,
                          uint32_t srcFirstLayer,
                          uint32_t dstFirstLayer,
                          uint32_t layerCount,
                          glm::uvec3 srcExtent,
                          glm::uvec3 dstExtent) noexcept = 0;

        virtual void setViewport(glm::uvec2 offset, glm::uvec2 extent) noexcept = 0;
        virtual void setScissor(glm::uvec2 offset, glm::uvec2 extent) noexcept = 0;

        virtual void clearTexture(std::shared_ptr<Texture> texture,
                                  glm::vec4 color,
                                  uint32_t mipLevel,
                                  uint32_t firstLayer,
                                  uint32_t layerCount) noexcept = 0;

        virtual void beginRendering(std::shared_ptr<FrameBuffer> frameBuffer,
                                    std::vector<ClearColor> clearColors,
                                    ClearDepthStencil clearDepth) noexcept = 0;
        virtual void endRendering() noexcept = 0;

        virtual void copyBuffer(std::shared_ptr<Buffer> srcBuffer,
                                std::shared_ptr<Buffer> dstBuffer,
                                uint64_t srcOffset,
                                uint64_t dstOffset,
                                uint64_t size) noexcept = 0;

        virtual void copyBufferToTexture(std::shared_ptr<Buffer> srcBuffer,
                                         std::shared_ptr<Texture> dstTexture,
                                         uint64_t srcOffset,
                                         glm::uvec3 dstOffset,
                                         uint32_t dstMipLevel,
                                         uint32_t dstFirstLayer,
                                         uint32_t layerCount,
                                         glm::uvec3 extent) noexcept = 0;

        virtual void copyTextureToBuffer(std::shared_ptr<Texture> srcTexture,
                                         std::shared_ptr<Buffer> dstBuffer,
                                         glm::uvec3 srcOffset,
                                         uint32_t srcMipLevel,
                                         uint32_t srcFirstLayer,
                                         uint32_t layerCount,
                                         glm::uvec3 extent,
                                         uint64_t dstOffset) noexcept = 0;

        virtual void bindPipeline(std::shared_ptr<Pipeline> pipeline) noexcept = 0;

        virtual void setPushConstant(std::span<const std::byte> data) noexcept = 0;

        virtual void bindVertexBuffer(std::shared_ptr<Buffer> buffer, uint64_t offset) noexcept = 0;

        virtual void bindIndexBuffer(std::shared_ptr<Buffer> buffer, uint64_t offset) noexcept = 0;

        virtual void bindStorageBuffer(std::shared_ptr<Buffer> buffer,
                                       uint32_t binding) noexcept = 0;

        virtual void bindSampler(std::shared_ptr<Sampler> sampler, uint32_t binding) noexcept = 0;

        virtual void bindSampledTexture(std::shared_ptr<Texture> texture,
                                        uint32_t binding,
                                        Texture::Aspect aspect) noexcept = 0;
        virtual void bindStorageTexture(std::shared_ptr<Texture> texture,
                                        uint32_t binding,
                                        Texture::Aspect aspect) noexcept = 0;

        virtual void userDefined(std::function<void(CommandBuffer&)> commandFunction) noexcept = 0;

        template<typename T>
        void setPushConstant(const T& data) noexcept
        {
            setPushConstant(std::as_bytes(std::span {&data, 1}));
        }

        EXAGE_BASE_API(API, CommandBuffer);
    };
}  // namespace exage::Graphics
