﻿#pragma once

#include "exage/Graphics/CommandBuffer.h"
#include "exage/Graphics/Utils/BufferTypes.h"
#include "exage/Renderer/Scene/Mesh.h"

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
        ~SceneBuffer();

        [[nodiscard]] auto uploadData(Graphics::CommandBuffer& commandBuffer,
                                      std::span<const std::byte> data,
                                      Graphics::Access access,
                                      Graphics::PipelineStage pipelineStage) noexcept -> size_t;

      private:
        std::reference_wrapper<Graphics::Context> _context;
        std::shared_ptr<Graphics::ResizableBuffer> _buffer;

        // Buddy allocator
        std::vector<std::pair<size_t, size_t>> _freeBlocks;
        std::vector<std::pair<size_t, size_t>> _usedBlocks;
    };

}  // namespace exage::Renderer
