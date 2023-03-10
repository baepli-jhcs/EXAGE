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

        [[nodiscard]] virtual auto begin() noexcept -> std::optional<Error> = 0;
        [[nodiscard]] virtual auto end() noexcept -> std::optional<Error> = 0;

        [[nodiscard]] virtual void submitCommand(GPUCommand command) noexcept = 0;
        [[nodiscard]] virtual void insertDataDependency(DataDependency dependency) noexcept = 0;

        EXAGE_BASE_API(API, CommandBuffer);
    };

    struct QueueCommandRepoCreateInfo
    {
        Context& context;
        Queue& queue;
    };

    class EXAGE_EXPORT QueueCommandRepo
    {
      public:
        [[nodiscard]] static auto create(QueueCommandRepoCreateInfo& createInfo) noexcept
            -> tl::expected<QueueCommandRepo, Error>;
        ~QueueCommandRepo() = default;

        EXAGE_DELETE_COPY(QueueCommandRepo);
        EXAGE_DEFAULT_MOVE(QueueCommandRepo);
        
        [[nodiscard]] auto current() noexcept -> CommandBuffer&;
        [[nodiscard]] auto current() const noexcept -> const CommandBuffer&;

      private:
        QueueCommandRepo(QueueCommandRepoCreateInfo& createInfo) noexcept;
        [[nodiscard]] auto init(Context& context) noexcept -> std::optional<Error>;

        std::reference_wrapper<Queue> _queue;
        std::vector<std::unique_ptr<CommandBuffer>> _commandBuffers;
    };
}  // namespace exage::Graphics
