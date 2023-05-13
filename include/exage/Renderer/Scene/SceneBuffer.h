#pragma once

#include "exage/Graphics/CommandBuffer.h"
#include "exage/Graphics/Utils/BufferTypes.h"
#include "exage/Renderer/Scene/Mesh.h"
#include "exage/utils/classes.h"

namespace exage::Renderer
{
    struct SceneBufferCreateInfo
    {
        Graphics::Context& context;
        size_t initialSize = 100 * 1024 * 1024;
    };

    class EXAGE_EXPORT SceneBuffer
    {
      public:
        explicit SceneBuffer(const SceneBufferCreateInfo& createInfo) noexcept;
        ~SceneBuffer() = default;

        EXAGE_DELETE_COPY(SceneBuffer);
        EXAGE_DEFAULT_MOVE(SceneBuffer);

        [[nodiscard]] auto uploadData(Graphics::CommandBuffer& commandBuffer,
                                      std::span<const std::byte> data,
                                      Graphics::Access access,
                                      Graphics::PipelineStage pipelineStage) noexcept -> size_t;

        void freeData(size_t offset, size_t size) noexcept;

      private:
        struct Block
        {
            size_t offset;
            size_t size;
        };

        std::reference_wrapper<Graphics::Context> _context;
        Graphics::ResizableBuffer _buffer;

        std::vector<Block> _freeBlocks;
        std::vector<Block> _usedBlocks;
    };

}  // namespace exage::Renderer
