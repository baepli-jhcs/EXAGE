
#include <filesystem>

#include "Editor.h"
#include "exage/Projects/Level.h"

auto main(int /*argc*/, char* /*argv*/[]) -> int
{
    std::cout << "Current path: " << std::filesystem::current_path() << std::endl;

    exage::init();
    exitor::Editor editor;
    editor.run();

    // exage::Projects::Level level {};
    // level.path = "assets/levels/default.exlevel";

    // [[maybe_unused]] auto result = exage::Projects::saveLevel("default.exlevel", level);

    return 0;
}