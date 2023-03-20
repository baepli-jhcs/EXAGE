#pragma once

#include <cstdint>
#include <memory>
#include <string_view>

#include "Core/Core.h"
#include "Input/KeyCode.h"
#include "glm/glm.hpp"
#include "tl/expected.hpp"

namespace exage
{
    enum class WindowAPI
    {
        eGLFW,
        eSDL  // TODO: Implement SDL
    };

    enum class WindowError;

    enum class FullScreenMode
    {
        eWindowed,
        eWindowedBorderless,
        eExclusive
    };

    struct WindowInfo
    {
        glm::uvec2 extent;
        std::string_view name;
        FullScreenMode fullScreenMode = FullScreenMode::eWindowed;
        uint32_t refreshRate = 0;  // 0 = Use monitor refresh rate
    };

    struct ResizeCallback
    {
        void* data;
        void (*callback)(void* data, glm::uvec2 extent);
    };

    struct KeyCallback
    {
        void* data;
        void (*callback)(void* data, KeyCode key, KeyAction action);
    };

    class EXAGE_EXPORT Window
    {
      public:
        Window() = default;
        virtual ~Window() = default;
        EXAGE_DELETE_COPY(Window);
        EXAGE_DEFAULT_MOVE(Window);

        virtual void update() noexcept = 0;
        virtual void close() noexcept = 0;

        virtual void addResizeCallback(const ResizeCallback& callback) noexcept = 0;
        virtual void removeResizeCallback(const ResizeCallback& callback) noexcept = 0;

        virtual void addKeyCallback(const KeyCallback& callback) noexcept = 0;
        virtual void removeKeyCallback(const KeyCallback& callback) noexcept = 0;

        [[nodiscard]] virtual auto getName() const noexcept -> std::string_view = 0;

        [[nodiscard]] virtual auto getWidth() const noexcept -> uint32_t = 0;
        [[nodiscard]] virtual auto getHeight() const noexcept -> uint32_t = 0;
        [[nodiscard]] virtual auto getExtent() const noexcept -> glm::uvec2 = 0;

        [[nodiscard]] virtual auto getRefreshRate() const noexcept -> uint32_t = 0;
        [[nodiscard]] virtual auto getFullScreenMode() const noexcept -> FullScreenMode = 0;

        [[nodiscard]] virtual auto getNativeHandle() const noexcept -> void* = 0;

        virtual void resize(glm::uvec2 extent) noexcept = 0;

        virtual void setRefreshRate(uint32_t refreshRate) noexcept = 0;
        virtual void setFullScreenMode(FullScreenMode mode) noexcept = 0;

        [[nodiscard]] virtual auto shouldClose() const noexcept -> bool = 0;
        [[nodiscard]] virtual auto isMinimized() const noexcept -> bool = 0;

        EXAGE_BASE_API(WindowAPI, Window);
        [[nodiscard]] static auto create(const WindowInfo& info, WindowAPI api) noexcept
            -> tl::expected<std::unique_ptr<Window>, WindowError>;
    };

    enum class WindowError
    {
        eInvalidAPI,
        eUnsupportedAPI,
    };
}  // namespace exage
