#pragma once

#include <array>
#include <optional>
#include <variant>

#include <glm/glm.hpp>

#include "exage/Core/Core.h"
#include "exage/Input/KeyCode.h"

namespace exage::System
{
    enum class WindowAPI
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

        struct JoystickConnected
        {
        };

        struct JoystickDisconnected
        {
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
                                   Events::JoystickConnected,
                                   Events::JoystickDisconnected>;

    struct Event
    {
        uint32_t pertainingID = std::numeric_limits<uint32_t>::max();
        EventData data;
    };

    void pollEvents(WindowAPI api) noexcept;
    void waitEvent(WindowAPI api) noexcept;

    [[nodiscard]] auto nextEvent(WindowAPI api) noexcept -> std::optional<Event>;
}  // namespace exage::System