#pragma once

#ifndef EXAGE_EXPORT
#    define EXAGE_EXPORT __declspec(dllexport)
#endif

#include <stdint.h>

#include "utils/cast.h"
#include "utils/classes.h"
#include "utils/apiType.h"
#include "utils/variant.h"

namespace exage
{
    // Sets up required engine components
    EXAGE_EXPORT void init();
}
