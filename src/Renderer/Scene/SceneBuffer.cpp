#include "exage/Renderer/Scene/SceneBuffer.h"

#include <vcruntime.h>

#include "exage/Graphics/Context.h"
#include "exage/Graphics/Utils/BufferTypes.h"

namespace exage::Renderer
{

    SceneBuffer::SceneBuffer(const SceneBufferCreateInfo& createInfo) noexcept
        : _context(createInfo.context)
        , _buffer(Graphics::ResizableBufferCreateInfo {
              {createInfo.initialSize, Graphics::Buffer::MapMode::eIfOptimal, false},
              createInfo.context})
    {
        Block block {};
        block.offset = 0;
        block.size = createInfo.initialSize;
        _freeBlocks.push_back(block);
    }

}  // namespace exage::Renderer