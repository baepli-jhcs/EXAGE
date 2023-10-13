#pragma once

#include <utility>

#include "exage/Core/Core.h"
#include "exage/Projects/Level.h"
#include "exage/utils/classes.h"

namespace exage::Projects
{
    class ScriptActions
    {
      public:
        ScriptActions() noexcept = default;
        virtual ~ScriptActions() = default;

        EXAGE_DEFAULT_COPY(ScriptActions);
        EXAGE_DEFAULT_MOVE(ScriptActions);

        virtual void loadLevel(std::string_view levelPath) noexcept = 0;
        virtual void unloadLevel() noexcept = 0;

        virtual auto getLevel() noexcept -> Level const* = 0;  // nullptr if no level is loaded
        virtual auto getScene() noexcept -> Scene const* = 0;  // nullptr if no level is loaded
    };

    class Script
    {
      public:
        explicit Script(ScriptActions& actions) noexcept
            : _actions(actions)
        {
        }
        virtual ~Script() = default;

        EXAGE_DEFAULT_MOVE_COPY_CONSTRUCT(Script);
        EXAGE_DELETE_ASSIGN(Script);

        void loadLevel(std::string_view levelPath) noexcept { _actions.loadLevel(levelPath); }
        void unloadLevel() noexcept { _actions.unloadLevel(); }

        virtual void init() noexcept = 0;
        virtual void tick(float deltaTime) noexcept = 0;

        /* UI is rendered on a separate thread, so be careful with shared resources.
            Debug UI is editor-only and within an ImGui window. */
        virtual void drawUI() noexcept = 0;
        virtual void drawDebugUI() noexcept = 0;

        virtual void onLevelLoaded(const exage::Projects::Level& level) noexcept = 0;
        virtual void onLevelUnloaded() noexcept = 0;

      private:
        ScriptActions& _actions;
    };
};  // namespace exage::Projects