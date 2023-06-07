#pragma once

#include <vector>

#include <stdint.h>

#include "exage/Core/Window.h"

struct GLFWwindow;
struct GLFWmonitor;

namespace exage
{
    class GLFWindow final : public Window
    {
      public:
        explicit GLFWindow(const WindowInfo& info) noexcept;
        ~GLFWindow() noexcept override;

        EXAGE_DELETE_COPY(GLFWindow);
        EXAGE_DEFAULT_MOVE(GLFWindow);

        void close() noexcept override;

        [[nodiscard]] auto getName() const noexcept -> std::string_view override { return _name; }

        [[nodiscard]] auto getWidth() const noexcept -> uint32_t override { return _extent.x; }
        [[nodiscard]] auto getHeight() const noexcept -> uint32_t override { return _extent.y; }
        [[nodiscard]] auto getExtent() const noexcept -> glm::uvec2 override { return _extent; }
        [[nodiscard]] auto getPosition() const noexcept -> glm::ivec2 override;

        [[nodiscard]] auto isFullScreen() const noexcept -> bool override { return _fullScreen; }
        [[nodiscard]] auto isWindowBordered() const noexcept -> bool override
        {
            return _windowBordered;
        }

        [[nodiscard]] auto getExclusiveRefreshRate() const noexcept -> uint32_t override
        {
            return _exclusiveRefreshRate;
        }
        [[nodiscard]] auto getExclusiveMonitor() const noexcept -> Monitor override
        {
            return _exclusiveMonitor;
        }

        [[nodiscard]] auto isHidden() const noexcept -> bool override;

        [[nodiscard]] auto getID() const noexcept -> uint32_t override;
        [[nodiscard]] auto getNativeHandle() const noexcept -> void* override;

        void resize(glm::uvec2 extent) noexcept override;

        void setFullScreen(bool fullScreen) noexcept override;
        void setWindowBordered(bool bordered) noexcept override;

        void setExclusiveRefreshRate(uint32_t refreshRate) noexcept override;
        void setExclusiveMonitor(Monitor monitor) noexcept override;

        void setHidden(bool hidden) noexcept override;

        [[nodiscard]] auto shouldClose() const noexcept -> bool override;
        [[nodiscard]] auto isIconified() const noexcept -> bool override;

        [[nodiscard]] auto getGLFWWindow() const noexcept -> GLFWwindow* { return _window; }

        static void init() noexcept;

        static void pollEvents() noexcept;
        static void waitEvents() noexcept;

        static auto getMonitorCount() noexcept -> uint32_t;
        static auto getMonitor(uint32_t index) noexcept -> Monitor;
        static auto getMonitors() noexcept -> std::vector<Monitor>;

        static auto getWindowByID(uint32_t id) noexcept -> GLFWindow*;

        static auto nextEvent() noexcept -> std::optional<Event>;

        EXAGE_DERIVED_API(WindowAPI, eGLFW);

      private:
        [[nodiscard]] auto exclusiveMonitor() const noexcept -> GLFWmonitor*;

        uint32_t _id;

        GLFWwindow* _window = nullptr;
        std::string _name;

        glm::uvec2 _extent;

        bool _fullScreen;
        bool _windowBordered;

        uint32_t _exclusiveRefreshRate;
        Monitor _exclusiveMonitor;
    };
}  // namespace exage
