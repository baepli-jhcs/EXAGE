#pragma once

#include <vector>

#include "Core/Window.h"

struct GLFWwindow;

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

        [[nodiscard]] auto getRefreshRate() const noexcept -> uint32_t override;
        [[nodiscard]] auto getFullScreenMode() const noexcept -> FullScreenMode override;

        [[nodiscard]] auto getNativeHandle() const noexcept -> void* override;

        void resize(glm::uvec2 extent) noexcept override;
        void setFullScreenMode(FullScreenMode mode) noexcept override;

        [[nodiscard]] auto shouldClose() const noexcept -> bool override;

        [[nodiscard]] auto getGLFWWindow() const noexcept -> GLFWwindow* { return _window; }

        EXAGE_DERIVED_API(WindowAPI, eGLFW);

    private:
        static void resizeCallback(GLFWwindow* window, int width, int height);
        static void keyCallback(
            GLFWwindow* window,
            int key,
            int scancode,
            int action,
            int mods);

        GLFWwindow* _window = nullptr;
        std::string _name;

        glm::uvec2 _extent;
        FullScreenMode _fullScreenMode = FullScreenMode::eWindowed;

        std::vector<ResizeCallback> _resizeCallbacks;
        std::vector<KeyCallback> _keyCallbacks;
    };
} // namespace exage
