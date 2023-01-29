#pragma once

#define EXAGE_DELETE_COPY(Class) \
    Class(const Class&) noexcept = delete; \
    auto operator=(const Class&) noexcept ->Class& = delete

#define EXAGE_DELETE_MOVE(Class) \
    Class(Class&&) noexcept = delete; \
    auto operator=(Class&&) noexcept -> Class& = delete

#define EXAGE_DEFAULT_COPY(Class) \
    Class(const Class&) noexcept = default; \
    auto operator=(const Class&) noexcept -> Class& = default

#define EXAGE_DEFAULT_MOVE(Class) \
    Class(Class&&) noexcept = default; \
    auto operator=(Class&&) noexcept -> Class& = default
