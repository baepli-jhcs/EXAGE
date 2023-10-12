#pragma once

#include "exage/Graphics/Buffer.h"
#include "exage/Graphics/CommandBuffer.h"
#include "exage/Graphics/Context.h"

namespace exage::Renderer
{
    inline auto createBufferAndUploadData(Graphics::Context& context,
                                          Graphics::CommandBuffer& commandBuffer,
                                          Graphics::PipelineStage pipelineStage,
                                          Graphics::Access access,
                                          std::span<const std::byte> data,
                                          bool cached = false) noexcept
        -> std::shared_ptr<Graphics::Buffer>
    {
        Graphics::BufferCreateInfo bufferCreateInfo {
            .size = data.size(),
            .mapMode = Graphics::Buffer::MapMode::eIfOptimal,
            .cached = cached,
        };

        std::shared_ptr<Graphics::Buffer> buffer = context.createBuffer(bufferCreateInfo);

        if (buffer->isMapped())
        {
            buffer->write(data, 0);
        }
        else
        {
            bufferCreateInfo.mapMode = Graphics::Buffer::MapMode::eMapped;
            std::shared_ptr<Graphics::Buffer> stagingBuffer =
                context.createBuffer(bufferCreateInfo);

            stagingBuffer->write(data, 0);

            commandBuffer.copyBuffer(stagingBuffer, buffer, 0, 0, data.size());

            commandBuffer.bufferBarrier(buffer,
                                        Graphics::PipelineStageFlags::eTransfer,
                                        pipelineStage,
                                        Graphics::AccessFlags::eTransferWrite,
                                        access,
                                        Graphics::QueueOwnership::eUndefined,
                                        Graphics::QueueOwnership::eUndefined);
        }

        return buffer;
    }
}  // namespace exage::Renderer