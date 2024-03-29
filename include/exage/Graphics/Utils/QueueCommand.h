﻿#pragma once

#include "exage/Core/Core.h"
#include "exage/Graphics/CommandBuffer.h"
#include "exage/Graphics/Context.h"
#include "exage/Graphics/Queue.h"

namespace exage::Graphics
{
    struct QueueCommandRepoCreateInfo
    {
        Context& context;
    };

    class QueueCommandRepo
    {
      public:
        explicit QueueCommandRepo(QueueCommandRepoCreateInfo& createInfo) noexcept;
        ~QueueCommandRepo() = default;

        EXAGE_DELETE_COPY(QueueCommandRepo);
        EXAGE_DEFAULT_MOVE(QueueCommandRepo);

        [[nodiscard]] auto current() noexcept -> CommandBuffer&;
        [[nodiscard]] auto current() const noexcept -> const CommandBuffer&;

      private:
        std::reference_wrapper<Queue> _queue;
        std::array<std::unique_ptr<CommandBuffer>, MAX_FRAMES_IN_FLIGHT> _commandBuffers;
    };
}  // namespace exage::Graphics
