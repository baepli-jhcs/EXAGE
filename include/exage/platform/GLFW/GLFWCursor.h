#pragma once

#include "exage/System/Cursor.h"
#include "exage/utils/classes.h"

class GLFWcursor;

namespace exage::System
{
    class GLFWCursor : public Cursor
    {
      public:
        explicit GLFWCursor(StandardCursor standardCursor) noexcept;
        GLFWCursor(glm::uvec2 extent, const uint8_t* pixels, glm::uvec2 hotspot) noexcept;

        ~GLFWCursor() override;

        EXAGE_DELETE_COPY(GLFWCursor);
        EXAGE_DELETE_MOVE(GLFWCursor);

        static void setCurrent(GLFWCursor* cursor) noexcept;

        EXAGE_DERIVED_API(WindowAPI, eGLFW);

      private:
        GLFWcursor* _cursor = nullptr;
    };
}  // namespace exage::System