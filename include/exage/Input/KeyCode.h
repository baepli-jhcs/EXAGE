#pragma once

#include "exage/Core/Core.h"

namespace exage
{
    enum class KeyAction : uint32_t
    {
        eRelease,
        ePress,
        eRepeat
    };

    enum class KeyCode : uint32_t
    {
        eUnknown,
        eSpace,
        eApostrophe, /* ' */
        eComma, /* , */
        eMinus, /* - */
        ePeriod, /* . */
        eSlash, /* / */
        e0,
        e1,
        e2,
        e3,
        e4,
        e5,
        e6,
        e7,
        e8,
        e9,
        eSemicolon, /* ; */
        eEqual, /* = */
        eA,
        eB,
        eC,
        eD,
        eE,
        eF,
        eG,
        eH,
        eI,
        eJ,
        eK,
        eL,
        eM,
        eN,
        eO,
        eP,
        eQ,
        eR,
        eS,
        eT,
        eU,
        eV,
        eW,
        eX,
        eY,
        eZ,
        eLeftBracket, /* [ */
        eBackslash, /* \ */
        eRightBracket, /* ] */
        eGraveAccent, /* ` */
        eWorld1, /* non-US #1 */
        eWorld2, /* non-US #2 */
        eEscape,
        eEnter,
        eTab,
        eBackspace,
        eInsert,
        eDelete,
        eRight,
        eLeft,
        eDown,
        eUp,
        ePageUp,
        ePageDown,
        eHome,
        eEnd,
        eCapsLock,
        eScrollLock,
        eNumLock,
        ePrintScreen,
        ePause,
        eF1,
        eF2,
        eF3,
        eF4,
        eF5,
        eF6,
        eF7,
        eF8,
        eF9,
        eF10,
        eF11,
        eF12,
        eF13,
        eF14,
        eF15,
        eF16,
        eF17,
        eF18,
        eF19,
        eF20,
        eF21,
        eF22,
        eF23,
        eF24,
        eF25,
        eKP0,
        eKP1,
        eKP2,
        eKP3,
        eKP4,
        eKP5,
        eKP6,
        eKP7,
        eKP8,
        eKP9,
        eKPDecimal,
        eKPDivide,
        eKPMultiply,
        eKPSubtract,
        eKPAdd,
        eKPEnter,
        eKPEqual,
        eLeftShift,
        eLeftControl,
        eLeftAlt,
        eLeftSuper,
        eRightShift,
        eRightControl,
        eRightAlt,
        eRightSuper,
        eMenu,
    };
}  // namespace exage
