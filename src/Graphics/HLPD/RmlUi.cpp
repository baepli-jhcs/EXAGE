#include <cstddef>

#include "exage/Graphics/HLPD/RmlUi.h"

#include <RmlUi/Core/Context.h>
#include <RmlUi/Core/Input.h>
#include <RmlUi/Core/StringUtilities.h>
#include <glm/gtc/type_ptr.hpp>

#include "exage/Core/Timer.h"
#include "exage/System/Clipboard.h"
#include "exage/System/Cursor.h"
#include "exage/utils/variant.h"

namespace exage::Graphics::RmlUi
{

    void RenderInterface::RenderGeometry(Rml::Vertex* vertices,
                                         int num_vertices,
                                         int* indices,
                                         int num_indices,
                                         Rml::TextureHandle texture,
                                         const Rml::Vector2f& translation)
    {
        RenderInstruction instruction;
        instruction.numVertices = num_vertices;
        instruction.numIndices = num_indices;
        instruction.baseVertex = _vertices.size();
        instruction.baseIndex = _indices.size();
        instruction.texture = texture;
        instruction.translation = translation;

        _vertices.insert(_vertices.end(), vertices, vertices + num_vertices);
        _indices.insert(_indices.end(), indices, indices + num_indices);
        _instructions.push_back(instruction);
    }

    auto RenderInterface::CompileGeometry(Rml::Vertex* vertices,
                                          int num_vertices,
                                          int* indices,
                                          int num_indices,
                                          Rml::TextureHandle texture) -> Rml::CompiledGeometryHandle
    {
        CompileInstruction instruction {};
        instruction.numVertices = num_vertices;
        instruction.numIndices = num_indices;
        instruction.baseVertex = _vertices.size();
        instruction.baseIndex = _indices.size();
        instruction.texture = texture;
        instruction.compiledGeometryHandle = getNextCompiledGeometry();

        _vertices.insert(_vertices.end(), vertices, vertices + num_vertices);
        _indices.insert(_indices.end(), indices, indices + num_indices);
        _instructions.push_back(instruction);

        return instruction.compiledGeometryHandle;
    }

    void RenderInterface::RenderCompiledGeometry(Rml::CompiledGeometryHandle geometry,
                                                 const Rml::Vector2f& translation)
    {
        RenderCompiledInstruction instruction;
        instruction.geometry = geometry;
        instruction.translation = translation;

        _instructions.push_back(instruction);
    }

    void RenderInterface::ReleaseCompiledGeometry(Rml::CompiledGeometryHandle geometry)
    {
        ReleaseCompiledInstruction instruction;
        instruction.geometry = geometry;

        _instructions.push_back(instruction);
    }

    void RenderInterface::EnableScissorRegion(bool enable)
    {
        EnableScissorInstruction instruction;
        instruction.enable = enable;

        _instructions.push_back(instruction);
    }

    void RenderInterface::SetScissorRegion(int x, int y, int width, int height)
    {
        SetScissorInstruction instruction;
        instruction.x = x;
        instruction.y = y;
        instruction.width = width;
        instruction.height = height;

        _instructions.push_back(instruction);
    }

    bool RenderInterface::LoadTexture(Rml::TextureHandle& texture_handle,
                                      Rml::Vector2i& texture_dimensions,
                                      const Rml::String& source)
    {
        // TODO: implement
        return false;
    }

    bool RenderInterface::GenerateTexture(Rml::TextureHandle& texture_handle,
                                          const Rml::byte* source,
                                          const Rml::Vector2i& source_dimensions)
    {
        texture_handle = getNextTexture();

        GenerateTextureInstruction instruction;
        instruction.textureHandle = texture_handle;
        instruction.sourceDimensions = source_dimensions;
        instruction.baseIndex = _textureData.size();

        size_t const numPixels =
            static_cast<const size_t>(source_dimensions.x) * source_dimensions.y;

        _textureData.insert(_textureData.end(), source, source + numPixels * 4);
        _instructions.push_back(instruction);

        return true;
    }

    void RenderInterface::ReleaseTexture(Rml::TextureHandle texture_handle)
    {
        ReleaseTextureInstruction instruction {};
        instruction.textureHandle = texture_handle;

        _instructions.push_back(instruction);
    }

    void RenderInterface::SetTransform(const Rml::Matrix4f* transform)
    {
        SetTransformInstruction instruction {};
        if (transform != nullptr)
        {
            instruction.transform = glm::make_mat4x4(transform->data());
        }
        else
        {
            instruction.transform = glm::mat4(1.0f);
        }

        _instructions.push_back(instruction);
    }

    void RenderInterface::beginNewFrame() noexcept
    {
        _instructions.clear();
        _vertices.clear();
        _indices.clear();
        _textureData.clear();
    }

    SystemInterface::SystemInterface(System::Window& window) noexcept
        : _window(&window)
        , _windowAPI(window.getAPI())
    {
        _arrow = System::Cursor::create(System::StandardCursor::eArrow);
        _pointer = System::Cursor::create(System::StandardCursor::eHand);
        _crosshair = System::Cursor::create(System::StandardCursor::eCrosshair);
        _text = System::Cursor::create(System::StandardCursor::eIBeam);
    }

    auto SystemInterface::GetElapsedTime() -> double
    {
        return Timer::getTimeFromStart();
    }

    void SystemInterface::SetMouseCursor(const Rml::String& cursor_name)
    {
        System::Cursor* cursor = nullptr;
        if (cursor_name.empty() || cursor_name == "arrow")
        {
            cursor = _arrow.get();
        }
        else if (cursor_name == "move")
        {
            cursor = _pointer.get();
        }
        else if (cursor_name == "pointer")
        {
            cursor = _pointer.get();
        }
        else if (cursor_name == "resize")
        {
            cursor = _pointer.get();
        }
        else if (cursor_name == "cross")
        {
            cursor = _crosshair.get();
        }
        else if (cursor_name == "text")
        {
            cursor = _text.get();
        }
        else if (cursor_name == "unavailable")
        {
            cursor = nullptr;
        }
        else if (Rml::StringUtilities::StartsWith(cursor_name, "rmlui-scroll"))
        {
            cursor = _pointer.get();
        }

        System::setCursor(cursor);
    }

    void SystemInterface::SetClipboardText(const Rml::String& text)
    {
        System::setClipboard(_windowAPI, text);
    }

    void SystemInterface::GetClipboardText(Rml::String& text)
    {
        text = System::getClipboard(_windowAPI);
    }

    auto registerEvent(Rml::Context& context, uint32_t windowID, const System::Event& event) -> bool
    {
        using namespace System::Events;

        bool result = true;

#define EXAGE_VERIFY_WINDOW_ID() \
    if (event.pertainingID != windowID) \
    { \
        return; \
    }

        std::visit(
            Overload {
                [&](const WindowResized& data)
                {
                    EXAGE_VERIFY_WINDOW_ID();
                    context.SetDimensions(
                        {static_cast<int>(data.extent.x), static_cast<int>(data.extent.y)});
                },
                [&](const MouseButtonPressed& data)
                {
                    EXAGE_VERIFY_WINDOW_ID();
                    result = !context.ProcessMouseButtonDown(toRmlUiMouseButton(data.button),
                                                             toRmlUiModifiers(data.modifiers));
                },
                [&](const MouseButtonReleased& data)
                {
                    EXAGE_VERIFY_WINDOW_ID();
                    result = !context.ProcessMouseButtonUp(toRmlUiMouseButton(data.button),
                                                           toRmlUiModifiers(data.modifiers));
                },
                [&](const MouseMoved& data)
                {
                    EXAGE_VERIFY_WINDOW_ID();
                    result = !context.ProcessMouseMove(
                        data.position.x, data.position.y, toRmlUiModifiers(data.modifiers));
                },
                [&](const MouseScrolled& data)
                {
                    EXAGE_VERIFY_WINDOW_ID();
                    result = !context.ProcessMouseWheel(static_cast<float>(data.offset.y),
                                                        toRmlUiModifiers(data.modifiers));
                },
                [&](const KeyPressed& data)
                {
                    EXAGE_VERIFY_WINDOW_ID();
                    result = !context.ProcessKeyDown(toRmlUiKey(data.key),
                                                     toRmlUiModifiers(data.modifiers));
                    if (data.key.code == KeyCode::eEnter || data.key.code == KeyCode::eKPEnter)
                    {
                        result |= !context.ProcessTextInput('\n');  // TODO: Verify
                    }
                },
                [&](const KeyRepeated& data)
                {
                    EXAGE_VERIFY_WINDOW_ID();
                    result = context.ProcessKeyDown(toRmlUiKey(data.key),
                                                    toRmlUiModifiers(data.modifiers));
                    if (data.key.code == KeyCode::eEnter || data.key.code == KeyCode::eKPEnter)
                    {
                        result |= !context.ProcessTextInput('\n');
                    }
                },
                [&](const KeyReleased& data)
                {
                    EXAGE_VERIFY_WINDOW_ID();
                    result = !context.ProcessKeyUp(toRmlUiKey(data.key),
                                                   toRmlUiModifiers(data.modifiers));
                },
                [&](const CodepointInput& data)
                {
                    EXAGE_VERIFY_WINDOW_ID();
                    result = !context.ProcessTextInput(static_cast<Rml::Character>(data.codepoint));
                },
                [&](const MouseLeft&)
                {
                    EXAGE_VERIFY_WINDOW_ID();
                    result = !context.ProcessMouseLeave();
                },
                [&](const auto&) {

                }},
            event.data);

        return result;

#undef EXAGE_VERIFY_WINDOW_ID
    }

    auto toRmlUiKey(KeyCode key) noexcept -> Rml::Input::KeyIdentifier
    {
        switch (key.code)
        {
            case KeyCode::eSpace:
                return Rml::Input::KI_SPACE;
            case KeyCode::eApostrophe:
                return Rml::Input::KI_OEM_7;
            case KeyCode::eComma:
                return Rml::Input::KI_OEM_COMMA;
            case KeyCode::eMinus:
                return Rml::Input::KI_OEM_MINUS;
            case KeyCode::ePeriod:
                return Rml::Input::KI_OEM_PERIOD;
            case KeyCode::eSlash:
                return Rml::Input::KI_OEM_2;
            case KeyCode::eD0:
                return Rml::Input::KI_0;
            case KeyCode::eD1:
                return Rml::Input::KI_1;
            case KeyCode::eD2:
                return Rml::Input::KI_2;
            case KeyCode::eD3:
                return Rml::Input::KI_3;
            case KeyCode::eD4:
                return Rml::Input::KI_4;
            case KeyCode::eD5:
                return Rml::Input::KI_5;
            case KeyCode::eD6:
                return Rml::Input::KI_6;
            case KeyCode::eD7:
                return Rml::Input::KI_7;
            case KeyCode::eD8:
                return Rml::Input::KI_8;
            case KeyCode::eD9:
                return Rml::Input::KI_9;
            case KeyCode::eSemicolon:
                return Rml::Input::KI_OEM_1;
            case KeyCode::eEqual:
                return Rml::Input::KI_OEM_PLUS;
            case KeyCode::eA:
                return Rml::Input::KI_A;
            case KeyCode::eB:
                return Rml::Input::KI_B;
            case KeyCode::eC:
                return Rml::Input::KI_C;
            case KeyCode::eD:
                return Rml::Input::KI_D;
            case KeyCode::eE:
                return Rml::Input::KI_E;
            case KeyCode::eF:
                return Rml::Input::KI_F;
            case KeyCode::eG:
                return Rml::Input::KI_G;
            case KeyCode::eH:
                return Rml::Input::KI_H;
            case KeyCode::eI:
                return Rml::Input::KI_I;
            case KeyCode::eJ:
                return Rml::Input::KI_J;
            case KeyCode::eK:
                return Rml::Input::KI_K;
            case KeyCode::eL:
                return Rml::Input::KI_L;
            case KeyCode::eM:
                return Rml::Input::KI_M;
            case KeyCode::eN:
                return Rml::Input::KI_N;
            case KeyCode::eO:
                return Rml::Input::KI_O;
            case KeyCode::eP:
                return Rml::Input::KI_P;
            case KeyCode::eQ:
                return Rml::Input::KI_Q;
            case KeyCode::eR:
                return Rml::Input::KI_R;
            case KeyCode::eS:
                return Rml::Input::KI_S;
            case KeyCode::eT:
                return Rml::Input::KI_T;
            case KeyCode::eU:
                return Rml::Input::KI_U;
            case KeyCode::eV:
                return Rml::Input::KI_V;
            case KeyCode::eW:
                return Rml::Input::KI_W;
            case KeyCode::eX:
                return Rml::Input::KI_X;
            case KeyCode::eY:
                return Rml::Input::KI_Y;
            case KeyCode::eZ:
                return Rml::Input::KI_Z;
            case KeyCode::eLeftBracket:
                return Rml::Input::KI_OEM_4;
            case KeyCode::eBackslash:
                return Rml::Input::KI_OEM_5;
            case KeyCode::eRightBracket:
                return Rml::Input::KI_OEM_6;
            case KeyCode::eGraveAccent:
                return Rml::Input::KI_OEM_3;
            case KeyCode::eWorld1:
                return Rml::Input::KI_UNKNOWN;  // no equivalent
            case KeyCode::eWorld2:
                return Rml::Input::KI_UNKNOWN;  // no equivalent
            case KeyCode::eEscape:
                return Rml::Input::KI_ESCAPE;
            case KeyCode::eEnter:
                return Rml::Input::KI_RETURN;
            case KeyCode::eTab:
                return Rml::Input::KI_TAB;
            case KeyCode::eBackspace:
                return Rml::Input::KI_BACK;
            case KeyCode::eInsert:
                return Rml::Input::KI_INSERT;
            case KeyCode::eDelete:
                return Rml::Input::KI_DELETE;
            case KeyCode::eRight:
                return Rml::Input::KI_RIGHT;
            case KeyCode::eLeft:
                return Rml::Input::KI_LEFT;
            case KeyCode::eDown:
                return Rml::Input::KI_DOWN;
            case KeyCode::eUp:
                return Rml::Input::KI_UP;
            case KeyCode::ePageUp:
                return Rml::Input::KI_PRIOR;
            case KeyCode::ePageDown:
                return Rml::Input::KI_NEXT;
            case KeyCode::eHome:
                return Rml::Input::KI_HOME;
            case KeyCode::eEnd:
                return Rml::Input::KI_END;
            case KeyCode::eCapsLock:
                return Rml::Input::KI_CAPITAL;
            case KeyCode::eScrollLock:
                return Rml::Input::KI_SCROLL;
            case KeyCode::eNumLock:
                return Rml::Input::KI_NUMLOCK;
            case KeyCode::ePrintScreen:
                return Rml::Input::KI_SNAPSHOT;
            case KeyCode::ePause:
                return Rml::Input::KI_PAUSE;
            case KeyCode::eF1:
                return Rml::Input::KI_F1;
            case KeyCode::eF2:
                return Rml::Input::KI_F2;
            case KeyCode::eF3:
                return Rml::Input::KI_F3;
            case KeyCode::eF4:
                return Rml::Input::KI_F4;
            case KeyCode::eF5:
                return Rml::Input::KI_F5;
            case KeyCode::eF6:
                return Rml::Input::KI_F6;
            case KeyCode::eF7:
                return Rml::Input::KI_F7;
            case KeyCode::eF8:
                return Rml::Input::KI_F8;
            case KeyCode::eF9:
                return Rml::Input::KI_F9;
            case KeyCode::eF10:
                return Rml::Input::KI_F10;
            case KeyCode::eF11:
                return Rml::Input::KI_F11;
            case KeyCode::eF12:
                return Rml::Input::KI_F12;
            case KeyCode::eF13:
                return Rml::Input::KI_F13;
            case KeyCode::eF14:
                return Rml::Input::KI_F14;
            case KeyCode::eF15:
                return Rml::Input::KI_F15;
            case KeyCode::eF16:
                return Rml::Input::KI_F16;
            case KeyCode::eF17:
                return Rml::Input::KI_F17;
            case KeyCode::eF18:
                return Rml::Input::KI_F18;
            case KeyCode::eF19:
                return Rml::Input::KI_F19;
            case KeyCode::eF20:
                return Rml::Input::KI_F20;
            case KeyCode::eF21:
                return Rml::Input::KI_F21;
            case KeyCode::eF22:
                return Rml::Input::KI_F22;
            case KeyCode::eF23:
                return Rml::Input::KI_F23;
            case KeyCode::eF24:
                return Rml::Input::KI_F24;
            case KeyCode::eF25:
                return Rml::Input::KI_UNKNOWN;  // no equivalent
            case KeyCode::eKP0:
                return Rml::Input::KI_NUMPAD0;
            case KeyCode::eKP1:
                return Rml::Input::KI_NUMPAD1;
            case KeyCode::eKP2:
                return Rml::Input::KI_NUMPAD2;
            case KeyCode::eKP3:
                return Rml::Input::KI_NUMPAD3;
            case KeyCode::eKP4:
                return Rml::Input::KI_NUMPAD4;
            case KeyCode::eKP5:
                return Rml::Input::KI_NUMPAD5;
            case KeyCode::eKP6:
                return Rml::Input::KI_NUMPAD6;
            case KeyCode::eKP7:
                return Rml::Input::KI_NUMPAD7;
            case KeyCode::eKP8:
                return Rml::Input::KI_NUMPAD8;
            case KeyCode::eKP9:
                return Rml::Input::KI_NUMPAD9;
            case KeyCode::eKPDecimal:
                return Rml::Input::KI_DECIMAL;
            case KeyCode::eKPDivide:
                return Rml::Input::KI_DIVIDE;
            case KeyCode::eKPMultiply:
                return Rml::Input::KI_MULTIPLY;
            case KeyCode::eKPSubtract:
                return Rml::Input::KI_SUBTRACT;
            case KeyCode::eKPAdd:
                return Rml::Input::KI_ADD;
            case KeyCode::eKPEnter:
                return Rml::Input::KI_NUMPADENTER;
            case KeyCode::eKPEqual:
                return Rml::Input::KI_OEM_NEC_EQUAL;
            case KeyCode::eLeftShift:
                return Rml::Input::KI_LSHIFT;
            case KeyCode::eLeftControl:
                return Rml::Input::KI_LCONTROL;
            case KeyCode::eLeftAlt:
                return Rml::Input::KI_LMENU;
            case KeyCode::eLeftSuper:
                return Rml::Input::KI_LWIN;
            case KeyCode::eRightShift:
                return Rml::Input::KI_RSHIFT;
            case KeyCode::eRightControl:
                return Rml::Input::KI_RCONTROL;
            case KeyCode::eRightAlt:
                return Rml::Input::KI_RMENU;
            case KeyCode::eRightSuper:
                return Rml::Input::KI_RWIN;
            case KeyCode::eMenu:
                return Rml::Input::KI_APPS;
            case KeyCode::eUnknown:
                return Rml::Input::KI_UNKNOWN;
        }

        return Rml::Input::KI_UNKNOWN;
    }

    auto toRmlUiModifiers(Modifiers modifiers) -> int
    {
        int result = 0;
        if (modifiers.any(ModifierFlags::eShift))
        {
            result |= Rml::Input::KM_SHIFT;
        }

        if (modifiers.any(ModifierFlags::eControl))
        {
            result |= Rml::Input::KM_CTRL;
        }

        if (modifiers.any(ModifierFlags::eAlt))
        {
            result |= Rml::Input::KM_ALT;
        }

        if (modifiers.any(ModifierFlags::eSuper))
        {
            result |= Rml::Input::KM_META;
        }

        if (modifiers.any(ModifierFlags::eCapsLock))
        {
            result |= Rml::Input::KM_CAPSLOCK;
        }

        if (modifiers.any(ModifierFlags::eNumLock))
        {
            result |= Rml::Input::KM_NUMLOCK;
        }

        return result;
    }

    auto toRmlUiMouseButton(MouseButton button) noexcept -> int
    {
        switch (button.code)
        {
            case MouseButton::eLeft:
                return 0;
            case MouseButton::eRight:
                return 1;
            case MouseButton::eMiddle:
                return 2;
        }

        return 3;
    }
}  // namespace exage::Graphics::RmlUi