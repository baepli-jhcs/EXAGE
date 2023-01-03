#define CATCH_CONFIG_RUNNER

#include "catch2/catch_session.hpp"

auto main(int argc, char* argv[]) -> int
{
    int result = Catch::Session().run(argc, argv);
    return result;
}