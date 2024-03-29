#pragma once

#include <array>
#include <optional>
#include <variant>

#include <glm/glm.hpp>

#include "exage/Core/Core.h"
#include "exage/Input/KeyCode.h"

namespace exage::System
{
    enum class API
    {
        eGLFW,
        eSDL  // TODO: Implement SDL
    };

    enum class WindowError
    {
        eInvalidAPI,
        eUnsupportedAPI,
    };

    namespace Events
    {
        struct WindowMoved
        {
            glm::ivec2 position;
        };

        struct WindowResized
        {
            glm::uvec2 extent;
        };

        struct WindowClosed
        {
        };

        struct WindowFocused
        {
        };

        struct WindowLostFocus
        {
        };

        struct WindowIconified
        {
        };

        struct WindowRestored
        {
        };

        struct WindowMaximized
        {
        };

        struct WindowUnmaximized
        {
        };

        struct WindowScaleChanged
        {
            float scale;
        };

        struct MouseButtonPressed
        {
            Modifiers modifiers;
            MouseButton button;
        };

        struct MouseButtonReleased
        {
            Modifiers modifiers;
            MouseButton button;
        };

        struct MouseMoved
        {
            Modifiers modifiers;
            glm::ivec2 position;
        };

        struct MouseScrolled
        {
            Modifiers modifiers;
            glm::vec2 offset;
        };

        struct MouseEntered
        {
        };

        struct MouseLeft
        {
        };

        struct KeyPressed
        {
            Modifiers modifiers;
            KeyCode key;
        };

        struct KeyReleased
        {
            Modifiers modifiers;
            KeyCode key;
        };

        struct KeyRepeated
        {
            Modifiers modifiers;
            KeyCode key;
        };

        struct CodepointInput
        {
            char32_t codepoint;
        };

        struct FileDropped
        {
            std::string path;
        };

        struct MonitorConnected
        {
        };

        struct MonitorDisconnected
        {
        };

        struct GamepadConnected
        {
        };

        struct GamepadDisconnected
        {
        };

        struct GamepadButtonPressed
        {
            GamepadButton button;
        };

        struct GamepadButtonReleased
        {
            GamepadButton button;
        };
    }  // namespace Events

    using EventData = std::variant<Events::WindowMoved,
                                   Events::WindowResized,
                                   Events::WindowClosed,
                                   Events::WindowFocused,
                                   Events::WindowLostFocus,
                                   Events::WindowIconified,
                                   Events::WindowRestored,
                                   Events::WindowMaximized,
                                   Events::WindowUnmaximized,
                                   Events::WindowScaleChanged,
                                   Events::MouseButtonPressed,
                                   Events::MouseButtonReleased,
                                   Events::MouseMoved,
                                   Events::MouseScrolled,
                                   Events::MouseEntered,
                                   Events::MouseLeft,
                                   Events::KeyPressed,
                                   Events::KeyReleased,
                                   Events::KeyRepeated,
                                   Events::CodepointInput,
                                   Events::FileDropped,
                                   Events::MonitorConnected,
                                   Events::MonitorDisconnected,
                                   Events::GamepadConnected,
                                   Events::GamepadDisconnected,
                                   Events::GamepadButtonPressed,
                                   Events::GamepadButtonReleased>;

    struct Event
    {
        uint32_t pertainingID = std::numeric_limits<uint32_t>::max();
        EventData data;
    };

    void pollEvents(API api) noexcept;
    void waitEvent(API api) noexcept;

    [[nodiscard]] auto nextEvent(API api) noexcept -> std::optional<Event>;
}  // namespace exage::System