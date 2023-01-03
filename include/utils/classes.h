#pragma once

#define EXAGE_DELETE_COPY(Class) \
    Class(const Class&) = delete; \
    auto operator=(const Class&)->Class& = delete

#define EXAGE_DELETE_MOVE(Class) \
    Class(Class&&) = delete; \
    auto operator=(Class&&)->Class& = delete

#define EXAGE_DEFAULT_COPY(Class) \
    Class(const Class&) = default; \
    auto operator=(const Class&)->Class& = default

#define EXAGE_DEFAULT_MOVE(Class) \
    Class(Class&&) = default; \
    auto operator=(Class&&)->Class& = default