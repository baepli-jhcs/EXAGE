#pragma once

#ifdef EXAGE_WINDOWS
#    define EXAGE_EXPORT __declspec(dllexport)
#else
#    define EXAGE_EXPORT
#endif

#define ASSERT_USE_MAGIC_ENUM

#include <stdint.h>

#include "assert.hpp"
#include "utils/apiType.h"
#include "utils/cast.h"
#include "utils/classes.h"
#include "utils/flags.h"
#include "utils/variant.h"

namespace exage
{
    // Sets up required engine components
    EXAGE_EXPORT void init();
}  // namespace exage
