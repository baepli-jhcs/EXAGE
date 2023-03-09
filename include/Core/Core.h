#pragma once

#ifdef EXAGE_WINDOWS
#    define EXAGE_EXPORT __declspec(dllexport)
#else
#    define EXAGE_EXPORT
#endif

#include <stdint.h>

#include "utils/cast.h"
#include "utils/classes.h"
#include "utils/apiType.h"
#include "utils/variant.h"
#include "utils/flags.h"

namespace exage
{
    // Sets up required engine components
    EXAGE_EXPORT void init();
}
