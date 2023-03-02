#include "Graphics/HLPD/ImGui.h"
#include "imgui.h"

namespace exage::Graphics::HLPD
{
    namespace
    {
        ImGuiKey toImGuiKey(KeyCode keyCode)
        {
            switch (keyCode)
            {
                case KeyCode::eSpace:
                    return ImGuiKey_Space;
                case KeyCode::eApostrophe:
                    return ImGuiKey_Apostrophe;
                case KeyCode::eComma:
                    return ImGuiKey_Comma;
                case KeyCode::eMinus:
                    return ImGuiKey_Minus;
                case KeyCode::ePeriod:
                    return ImGuiKey_Period;
                case KeyCode::eSlash:
                    return ImGuiKey_Slash;
                case KeyCode::e0:
                    return ImGuiKey_0;
                case KeyCode::e1:
                    return ImGuiKey_1;
                case KeyCode::e2:
                    return ImGuiKey_2;
                case KeyCode::e3:
                    return ImGuiKey_3;
                case KeyCode::e4:
                    return ImGuiKey_4;
                case KeyCode::e5:
                    return ImGuiKey_5;
                case KeyCode::e6:
                    return ImGuiKey_6;
                case KeyCode::e7:
                    return ImGuiKey_7;
                case KeyCode::e8:
                    return ImGuiKey_8;
                case KeyCode::e9:
                    return ImGuiKey_9;
                case KeyCode::eSemicolon:
                    return ImGuiKey_Semicolon;
                case KeyCode::eEqual:
                    return ImGuiKey_Equal;
                case KeyCode::eA:
                    return ImGuiKey_A;
                case KeyCode::eB:
                    return ImGuiKey_B;
                case KeyCode::eC:
                    return ImGuiKey_C;
                case KeyCode::eD:
                    return ImGuiKey_D;
                case KeyCode::eE:
                    return ImGuiKey_E;
                case KeyCode::eF:
                    return ImGuiKey_F;
                case KeyCode::eG:
                    return ImGuiKey_G;
                case KeyCode::eH:
                    return ImGuiKey_H;
                case KeyCode::eI:
                    return ImGuiKey_I;
                case KeyCode::eJ:
                    return ImGuiKey_J;
                case KeyCode::eK:
                    return ImGuiKey_K;
                case KeyCode::eL:
                    return ImGuiKey_L;
                case KeyCode::eM:
                    return ImGuiKey_M;
                case KeyCode::eN:
                    return ImGuiKey_N;
                case KeyCode::eO:
                    return ImGuiKey_O;
                case KeyCode::eP:
                    return ImGuiKey_P;
                case KeyCode::eQ:
                    return ImGuiKey_Q;
                case KeyCode::eR:
                    return ImGuiKey_R;
                case KeyCode::eS:
                    return ImGuiKey_S;
                case KeyCode::eT:
                    return ImGuiKey_T;
                case KeyCode::eU:
                    return ImGuiKey_U;
                case KeyCode::eV:
                    return ImGuiKey_V;
                case KeyCode::eW:
                    return ImGuiKey_W;
                case KeyCode::eX:
                    return ImGuiKey_X;
                case KeyCode::eY:
                    return ImGuiKey_Y;
                case KeyCode::eZ:
                    return ImGuiKey_Z;
                case KeyCode::eLeftBracket:
                    return ImGuiKey_LeftBracket;
                case KeyCode::eBackslash:
                    return ImGuiKey_Backslash;
                case KeyCode::eRightBracket:
                    return ImGuiKey_RightBracket;
                case KeyCode::eGraveAccent:
                    return ImGuiKey_GraveAccent;
                case KeyCode::eEscape:
                    return ImGuiKey_Escape;
                case KeyCode::eEnter:
                    return ImGuiKey_Enter;
                case KeyCode::eTab:
                    return ImGuiKey_Tab;
                case KeyCode::eBackspace:
                    return ImGuiKey_Backspace;
                case KeyCode::eInsert:
                    return ImGuiKey_Insert;
                case KeyCode::eDelete:
                    return ImGuiKey_Delete;
                case KeyCode::eRight:
                    return ImGuiKey_RightArrow;
                case KeyCode::eLeft:
                    return ImGuiKey_LeftArrow;
                case KeyCode::eDown:
                    return ImGuiKey_DownArrow;
                case KeyCode::ePageUp:
                    return ImGuiKey_PageUp;
                case KeyCode::ePageDown:
                    return ImGuiKey_PageDown;
                case KeyCode::eHome:
                    return ImGuiKey_Home;
                case KeyCode::eEnd:
                    return ImGuiKey_End;
                case KeyCode::eCapsLock:
                    return ImGuiKey_CapsLock;
                case KeyCode::eScrollLock:
                    return ImGuiKey_ScrollLock;
                case KeyCode::eNumLock:
                    return ImGuiKey_NumLock;
                case KeyCode::ePrintScreen:
                    return ImGuiKey_PrintScreen;
                case KeyCode::ePause:
                    return ImGuiKey_Pause;
                case KeyCode::eF1:
                    return ImGuiKey_F1;
                case KeyCode::eF2:
                    return ImGuiKey_F2;
                case KeyCode::eF3:
                    return ImGuiKey_F3;
                case KeyCode::eF4:
                    return ImGuiKey_F4;
                case KeyCode::eF5:
                    return ImGuiKey_F5;
                case KeyCode::eF6:
                    return ImGuiKey_F6;
                case KeyCode::eF7:
                    return ImGuiKey_F7;
                case KeyCode::eF8:
                    return ImGuiKey_F8;
                case KeyCode::eF9:
                    return ImGuiKey_F9;
                case KeyCode::eF10:
                    return ImGuiKey_F10;
                case KeyCode::eF11:
                    return ImGuiKey_F11;
                case KeyCode::eF12:
                    return ImGuiKey_F12;
                case KeyCode::eKP0:
                    return ImGuiKey_Keypad0;
                case KeyCode::eKP1:
                    return ImGuiKey_Keypad1;
                case KeyCode::eKP2:
                    return ImGuiKey_Keypad2;
                case KeyCode::eKP3:
                    return ImGuiKey_Keypad3;
                case KeyCode::eKP4:
                    return ImGuiKey_Keypad4;
                case KeyCode::eKP5:
                    return ImGuiKey_Keypad5;
                case KeyCode::eKP6:
                    return ImGuiKey_Keypad6;
                case KeyCode::eKP7:
                    return ImGuiKey_Keypad7;
                case KeyCode::eKP8:
                    return ImGuiKey_Keypad8;
                case KeyCode::eKP9:
                    return ImGuiKey_Keypad9;
                case KeyCode::eKPDecimal:
                    return ImGuiKey_KeypadDecimal;
                case KeyCode::eKPDivide:
                    return ImGuiKey_KeypadDivide;
                case KeyCode::eKPMultiply:
                    return ImGuiKey_KeypadMultiply;
                case KeyCode::eKPSubtract:
                    return ImGuiKey_KeypadSubtract;
                case KeyCode::eKPAdd:
                    return ImGuiKey_KeypadAdd;
                case KeyCode::eKPEnter:
                    return ImGuiKey_KeypadEnter;
                case KeyCode::eKPEqual:
                    return ImGuiKey_KeypadEqual;
                case KeyCode::eLeftShift:
                    return ImGuiKey_LeftShift;
                case KeyCode::eLeftControl:
                    return ImGuiKey_LeftCtrl;
                case KeyCode::eLeftAlt:
                    return ImGuiKey_LeftAlt;
                case KeyCode::eLeftSuper:
                    return ImGuiKey_LeftSuper;
                case KeyCode::eRightShift:
                    return ImGuiKey_RightShift;
                case KeyCode::eRightControl:
                    return ImGuiKey_RightCtrl;
                case KeyCode::eRightAlt:
                    return ImGuiKey_RightAlt;
                case KeyCode::eRightSuper:
                    return ImGuiKey_RightSuper;
                case KeyCode::eMenu:
                    return ImGuiKey_Menu;
                default:
                    return ImGuiKey_COUNT;
            }           
        }

        void initImGuiVulkan(ImGuiInitInfo& initInfo) noexcept 
        {
            
        }
    }

    
 #ifdef EXAGE_DEBUG
    static bool initialized = false;
 #endif
    
    void initImGui(ImGuiInitInfo& initInfo) noexcept 
    {
    #ifdef EXAGE_DEBUG
        assert(!initialized && "ImGui already initialized!");
        initialized = true;
    #endif
        
        switch (initInfo.context.getAPI())
        {
            case API::eVulkan:
                initImGuiVulkan(initInfo);
                break;
            default:
                break;
        }
    }
}  // namespace exage::Graphics::HLPD