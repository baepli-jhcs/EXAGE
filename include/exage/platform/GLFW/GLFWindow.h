#pragma once

#include <vector>

#include "exage/Core/Window.h"

struct GLFWwindow;
struct GLFWmonitor;

namespace exage
{
    class EXAGE_EXPORT GLFWindow final : public Window
    {
      public:
        explicit GLFWindow(const WindowInfo& info) noexcept;
        ~GLFWindow() noexcept override;

        EXAGE_DELETE_COPY(GLFWindow);
        EXAGE_DEFAULT_MOVE(GLFWindow);

        void update() noexcept override;
        void close() noexcept override;

        void addResizeCallback(const ResizeCallback& callback) noexcept override;
        void removeResizeCallback(const ResizeCallback& callback) noexcept override;

        void addKeyCallback(const KeyCallback& callback) noexcept override;
        void removeKeyCallback(const KeyCallback& callback) noexcept override;

        [[nodiscard]] auto getName() const noexcept -> std::string_view override { return _name; }

        [[nodiscard]] auto getWidth() const noexcept -> uint32_t override { return _extent.x; }
        [[nodiscard]] auto getHeight() const noexcept -> uint32_t override { return _extent.y; }
        [[nodiscard]] auto getExtent() const noexcept -> glm::uvec2 override { return _extent; }

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

        [[nodiscard]] auto getNativeHandle() const noexcept -> void* override;

        void resize(glm::uvec2 extent) noexcept override;

        void setFullScreen(bool fullScreen) noexcept override;
        void setWindowBordered(bool bordered) noexcept override;

        void setExclusiveRefreshRate(uint32_t refreshRate) noexcept override;
        void setExclusiveMonitor(Monitor monitorIndex) noexcept override;

        [[nodiscard]] auto shouldClose() const noexcept -> bool override;
        [[nodiscard]] auto isMinimized() const noexcept -> bool override;

        [[nodiscard]] auto getGLFWWindow() const noexcept -> GLFWwindow* { return _window; }

        static auto getMonitorCount() noexcept -> uint32_t;
        static auto getMonitor(uint32_t index) noexcept -> Monitor;
        static auto getMonitors() noexcept -> std::vector<Monitor>;

        EXAGE_DERIVED_API(WindowAPI, eGLFW);

      private:
        static void resizeCallback(GLFWwindow* window, int width, int height);
        static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

        [[nodiscard]] auto exclusiveMonitor() const noexcept -> GLFWmonitor*;

        GLFWwindow* _window = nullptr;
        std::string _name;

        glm::uvec2 _extent;

        bool _fullScreen;
        bool _windowBordered;

        uint32_t _exclusiveRefreshRate;
        Monitor _exclusiveMonitor;

        std::vector<ResizeCallback> _resizeCallbacks;
        std::vector<KeyCallback> _keyCallbacks;
    };
}  // namespace exage
