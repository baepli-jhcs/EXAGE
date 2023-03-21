#pragma once

#include <memory_resource>
#include <optional>
#include <thread>

#include "Commands.h"
#include "Core/Core.h"
#include "Error.h"
#include "Graphics/Context.h"

namespace exage::Graphics
{
    class EXAGE_EXPORT CommandBuffer
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
                          uint32_t instanceCount,
                          uint32_t firstVertex,
                          uint32_t firstInstance) noexcept = 0;

        virtual void drawIndexed(uint32_t indexCount,
                                 uint32_t instanceCount,
                                 uint32_t firstIndex,
                                 uint32_t vertexOffset,
                                 uint32_t firstInstance) noexcept = 0;

        virtual void textureBarrier(std::shared_ptr<Texture> texture,
                                    Texture::Layout newLayout,
                                    PipelineStage srcStage,
                                    PipelineStage dstStage,
                                    Access srcAccess,
                                    Access dstAccess) noexcept = 0;

        virtual void blit(std::shared_ptr<Texture> srcTexture,
                          std::shared_ptr<Texture> dstTexture,
                          glm::uvec3 srcOffset,
                          glm::uvec3 dstOffset,
                          uint32_t srcMipLevel,
                          uint32_t dstMipLevel,
                          uint32_t srcFirstLayer,
                          uint32_t dstFirstLayer,
                          uint32_t layerCount,
                          glm::uvec3 extent) noexcept = 0;

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

        virtual void userDefined(std::function<void(CommandBuffer&)> commandFunction) noexcept = 0;

        EXAGE_BASE_API(API, CommandBuffer);
    };
}  // namespace exage::Graphics
