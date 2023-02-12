#pragma once

#include <optional>

#include "Error.h"
#include "Core/Core.h"
#include "Graphics/Context.h"

namespace exage::Graphics
{
    enum class CommandBufferType
    {
        eQueue,
        eTemporary,
    };

    class EXAGE_EXPORT CommandBuffer
    {
    public:
        CommandBuffer() noexcept = default;
        virtual ~CommandBuffer() = default;

        EXAGE_DEFAULT_COPY(CommandBuffer);
        EXAGE_DEFAULT_MOVE(CommandBuffer);
    };


    struct QueueCommandBufferCreateInfo { };

    class EXAGE_EXPORT QueueCommandBuffer : public CommandBuffer
    {
    public:
        using CommandBuffer::CommandBuffer;
        ~QueueCommandBuffer() override = default;

        EXAGE_DEFAULT_COPY(QueueCommandBuffer);
        EXAGE_DEFAULT_MOVE(QueueCommandBuffer);

        virtual std::optional<Error> beginFrame() noexcept = 0;
        virtual std::optional<Error> endFrame() noexcept = 0;

        EXAGE_BASE_API(API, QueueCommandBuffer);
    };

    struct TemporaryCommandBufferCreateInfo {};

    class EXAGE_EXPORT TemporaryCommandBuffer : public CommandBuffer
    {
    public:
        using CommandBuffer::CommandBuffer;
        ~TemporaryCommandBuffer() override = default;

        EXAGE_DEFAULT_COPY(TemporaryCommandBuffer);
        EXAGE_DEFAULT_MOVE(TemporaryCommandBuffer);

        EXAGE_BASE_API(API, TemporaryCommandBuffer);
    };
} // namespace exage::Graphics
