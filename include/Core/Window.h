#pragma once

#include <cstdint>
#include <string_view>

#include "Input/KeyCode.h"
#include "glm/glm.hpp"
#include "utils/classes.h"

#ifndef EXAGE_EXPORT
#    define EXAGE_EXPORT __declspec(dllexport)
#endif

namespace exage
{
    enum class WindowAPI
    {
        eGLFW,
        eSDL  // TODO: Implement SDL
    };

    enum class FullScreenMode
    {
        eWindowed,
        eBorderless,
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

        virtual void update() = 0;
        virtual void close() = 0;

        virtual void addResizeCallback(const ResizeCallback& callback) = 0;
        virtual void removeResizeCallback(const ResizeCallback& callback) = 0;

        virtual void addKeyCallback(const KeyCallback& callback) = 0;
        virtual void removeKeyCallback(const KeyCallback& callback) = 0;

        virtual auto getName() const -> std::string_view = 0;

        virtual auto getWidth() const -> uint32_t = 0;
        virtual auto getHeight() const -> uint32_t = 0;
        virtual auto getExtent() const -> glm::uvec2 = 0;

        virtual auto getRefreshRate() const -> uint32_t = 0;
        virtual auto getFullScreenMode() const -> FullScreenMode = 0;

        virtual void resize(glm::uvec2 extent) = 0;
        virtual void setFullScreenMode(FullScreenMode mode) = 0;

        virtual auto shouldClose() const -> bool = 0;

        virtual auto getAPI() const -> WindowAPI = 0;
        static auto create(const WindowInfo& info, WindowAPI api) -> Window*;
    };
}  // namespace exage
