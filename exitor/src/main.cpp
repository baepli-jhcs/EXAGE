
#include <filesystem>

#include "Editor.h"
#include "exage/Projects/Level.h"

void mainFunction()
{
    std::cout << "Current path: " << std::filesystem::current_path() << std::endl;

    exage::init();
    exitor::Editor editor;
    editor.run();

    // exage::Projects::Level level {};
    // level.path = "assets/levels/default.exlevel";

    // [[maybe_unused]] auto result = exage::Projects::saveLevel("default.exlevel", level);
}

#ifdef EXAGE_WINDOWS
#    include <Windows.h>
auto WINAPI WinMain(HINSTANCE /*unused*/, HINSTANCE /*unused*/, PSTR /*unused*/, int /*unused*/)
    -> int
{
    mainFunction();
    return 0;
}
#else
auto main(int /*argc*/, char* /*argv*/[]) -> int
{
    mainFunction();
    return 0;
}
#endif