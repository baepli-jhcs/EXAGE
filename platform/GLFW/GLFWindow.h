#pragma once

#include <vector>

#include "Core/Window.h"

struct GLFWwindow;

namespace exage
{
    class EXAGE_EXPORT GLFWindow final : public Window
    {
      public:
        explicit GLFWindow(const WindowInfo& info);
        ~GLFWindow() override;

        EXAGE_DELETE_COPY(GLFWindow);
        EXAGE_DEFAULT_MOVE(GLFWindow);

        void update() override;
        void close() override;

        void addResizeCallback(const ResizeCallback& callback) override;
        void removeResizeCallback(const ResizeCallback& callback) override;

        void addKeyCallback(const KeyCallback& callback) override;
        void removeKeyCallback(const KeyCallback& callback) override;

        auto getName() const -> std::string_view override { return _name; }

        auto getWidth() const -> uint32_t override { return _extent.x; }
        auto getHeight() const -> uint32_t override { return _extent.y; }
        auto getExtent() const -> glm::uvec2 override { return _extent; }

        auto getRefreshRate() const -> uint32_t override;
        auto getFullScreenMode() const -> FullScreenMode override;

        void resize(glm::uvec2 extent) override;
        void setFullScreenMode(FullScreenMode mode) override;

        auto shouldClose() const -> bool override;

        auto getGLFWWindow() const -> GLFWwindow* { return _window; }
        auto getAPI() const -> WindowAPI override { return WindowAPI::eGLFW; }

      private:
        static void resizeCallback(GLFWwindow* window, int width, int height);
        static void keyCallback(
            GLFWwindow* window, int key, int scancode, int action, int mods);

        GLFWwindow* _window = nullptr;
        std::string _name;

        glm::uvec2 _extent;
        FullScreenMode _fullScreenMode = FullScreenMode::eWindowed;

        std::vector<ResizeCallback> _resizeCallbacks;
        std::vector<KeyCallback> _keyCallbacks;
    };
}  // namespace exage