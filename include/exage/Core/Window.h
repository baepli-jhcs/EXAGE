#pragma once

#include <cstdint>
#include <memory>
#include <optional>
#include <string_view>

#include "exage/Core/Core.h"
#include "exage/Input/KeyCode.h"
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

    struct Monitor
    {
        std::string_view name;
        glm::uvec2 extent;
        uint32_t refreshRate;
    };

    struct WindowInfo
    {
        std::string_view name;
        glm::uvec2 extent;
        bool fullScreen = false;
        bool windowBordered = true;
        uint32_t exclusiveRefreshRate;
        Monitor exclusiveMonitor;
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

        [[nodiscard]] virtual auto isFullScreen() const noexcept -> bool = 0;
        [[nodiscard]] virtual auto isWindowBordered() const noexcept -> bool = 0;

        [[nodiscard]] virtual auto getExclusiveRefreshRate() const noexcept -> uint32_t = 0;
        [[nodiscard]] virtual auto getExclusiveMonitor() const noexcept -> Monitor = 0;

        [[nodiscard]] virtual auto getNativeHandle() const noexcept -> void* = 0;

        virtual void resize(glm::uvec2 extent) noexcept = 0;

        virtual void setFullScreen(bool fullScreen) noexcept = 0;
        virtual void setWindowBordered(bool bordered) noexcept = 0;

        virtual void setExclusiveRefreshRate(uint32_t refreshRate) noexcept = 0;
        virtual void setExclusiveMonitor(Monitor monitorIndex) noexcept = 0;

        [[nodiscard]] virtual auto shouldClose() const noexcept -> bool = 0;
        [[nodiscard]] virtual auto isMinimized() const noexcept -> bool = 0;

        EXAGE_BASE_API(WindowAPI, Window);
        [[nodiscard]] static auto create(const WindowInfo& info, WindowAPI api) noexcept
            -> tl::expected<std::unique_ptr<Window>, WindowError>;
    };

    EXAGE_EXPORT auto getMonitorCount(WindowAPI api) noexcept -> uint32_t;
    EXAGE_EXPORT auto getMonitor(uint32_t index, WindowAPI api) noexcept -> Monitor;
    EXAGE_EXPORT auto getMonitors(WindowAPI api) noexcept -> std::vector<Monitor>;

    inline auto getDefaultMonitor(WindowAPI api) noexcept -> Monitor
    {
        return getMonitor(0, api);
    }

    enum class WindowError
    {
        eInvalidAPI,
        eUnsupportedAPI,
    };
}  // namespace exage
