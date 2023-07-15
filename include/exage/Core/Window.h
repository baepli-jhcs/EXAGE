#pragma once

#include <cstdint>
#include <memory>
#include <optional>
#include <string_view>
#include <variant>

#include "exage/Core/Core.h"
#include "exage/Core/Event.h"
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
        bool hidden = false;
        bool focused = true;
        bool focusOnShow = true;
        bool resizable = true;
    };

    class Window
    {
      public:
        Window() = default;
        virtual ~Window() = default;

        EXAGE_DELETE_COPY(Window);
        EXAGE_DEFAULT_MOVE(Window);

        virtual void close() noexcept = 0;

        [[nodiscard]] virtual auto getName() const noexcept -> std::string_view = 0;

        [[nodiscard]] virtual auto getWidth() const noexcept -> uint32_t = 0;
        [[nodiscard]] virtual auto getHeight() const noexcept -> uint32_t = 0;
        [[nodiscard]] virtual auto getExtent() const noexcept -> glm::uvec2 = 0;
        [[nodiscard]] virtual auto getPosition() const noexcept -> glm::ivec2 = 0;

        [[nodiscard]] virtual auto isFullScreen() const noexcept -> bool = 0;
        [[nodiscard]] virtual auto isWindowBordered() const noexcept -> bool = 0;

        [[nodiscard]] virtual auto getExclusiveRefreshRate() const noexcept -> uint32_t = 0;
        [[nodiscard]] virtual auto getExclusiveMonitor() const noexcept -> Monitor = 0;

        [[nodiscard]] virtual auto isHidden() const noexcept -> bool = 0;

        [[nodiscard]] virtual auto isResizable() const noexcept -> bool = 0;

        [[nodiscard]] virtual auto getID() const noexcept -> uint32_t = 0;
        [[nodiscard]] virtual auto getNativeHandle() const noexcept -> void* = 0;

        virtual void resize(glm::uvec2 extent) noexcept = 0;

        virtual void setFullScreen(bool fullScreen) noexcept = 0;
        virtual void setWindowBordered(bool bordered) noexcept = 0;

        virtual void setExclusiveRefreshRate(uint32_t refreshRate) noexcept = 0;
        virtual void setExclusiveMonitor(Monitor monitor) noexcept = 0;

        virtual void setHidden(bool hidden) noexcept = 0;

        virtual void setResizable(bool resizable) noexcept = 0;

        [[nodiscard]] virtual auto shouldClose() const noexcept -> bool = 0;
        [[nodiscard]] virtual auto isIconified() const noexcept -> bool = 0;

        EXAGE_BASE_API(WindowAPI, Window);
        [[nodiscard]] static auto create(const WindowInfo& info, WindowAPI api) noexcept
            -> tl::expected<std::unique_ptr<Window>, WindowError>;
    };

    [[nodiscard]] auto getWindowByID(uint32_t id, WindowAPI api) noexcept -> Window*;

    [[nodiscard]] auto getMonitorCount(WindowAPI api) noexcept -> uint32_t;
    [[nodiscard]] auto getMonitor(uint32_t index, WindowAPI api) noexcept -> Monitor;
    [[nodiscard]] auto getMonitors(WindowAPI api) noexcept -> std::vector<Monitor>;

    [[nodiscard]] inline auto getDefaultMonitor(WindowAPI api) noexcept -> Monitor
    {
        return getMonitor(0, api);
    }

    enum class WindowError
    {
        eInvalidAPI,
        eUnsupportedAPI,
    };

    void pollEvents(WindowAPI api) noexcept;
    void waitEvent(WindowAPI api) noexcept;

    [[nodiscard]] auto nextEvent(WindowAPI api) noexcept -> std::optional<Event>;

    //// revamp of event system
    // struct CloseEvent
    //{
    // };

    // struct KeyEvent
    //{
    //     KeyCode key;
    //     KeyAction action;
    // };

    // struct ResizeEvent
    //{
    //     glm::uvec2 extent;
    // };

    // using WindowEvent = std::variant<CloseEvent, KeyEvent, ResizeEvent>;
    // struct EventCallback
    //{
    //     void* data;
    //     void (*callback)(void* data, Window& window, const WindowEvent& event);
    // };

    //[[nodiscard]] auto pollEvent(WindowAPI api) noexcept ->
    // std::optional<WindowEvent>;
    //[[nodiscard]] auto waitEvent(WindowAPI api) noexcept ->
    // std::optional<WindowEvent>;
    //
}  // namespace exage
