#define CATCH_CONFIG_RUNNER

#include "catch2/catch_session.hpp"
#include "exage/Core/Core.h"

auto main(int argc, char* argv[]) -> int
{
    exage::init();
    int const result = Catch::Session().run(argc, argv);
    return result;
}
