#pragma once

#include <memory_resource>
#include <optional>
#include <thread>

#include "Error.h"
#include "Core/Core.h"
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

        EXAGE_BASE_API(API, CommandBuffer);
    };

    struct QueueCommandRepoCreateInfo
    {
        Context& context;
        Queue& queue;
    };

    //class EXAGE_EXPORT QueueCommandRepo
    //{
    //public:
    //    QueueCommandRepo(QueueCommandRepoCreateInfo& createInfo) noexcept;
    //    virtual ~QueueCommandRepo() = default;

    //    EXAGE_DELETE_COPY(QueueCommandRepo);
    //    EXAGE_DEFAULT_MOVE(QueueCommandRepo);

    //    [[nodiscard]] virtual auto beginFrame() noexcept -> std::optional<Error>;
    //    [[nodiscard]] virtual auto endFrame() noexcept -> std::optional<Error>;

    //    [[nodiscard]] virtual auto current() noexcept -> CommandBuffer&;
    //    [[nodiscard]] virtual auto current() const noexcept -> const CommandBuffer&;

    //private:
    //    std::vector<std::unique_ptr<CommandBuffer>> _commandBuffers;
    //};
} // namespace exage::Graphics
