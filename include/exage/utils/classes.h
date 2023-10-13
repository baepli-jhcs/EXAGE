#pragma once

#define EXAGE_DELETE_COPY(Class) \
    Class(const Class&) noexcept = delete; \
    auto operator=(const Class&) noexcept -> Class& = delete

#define EXAGE_DELETE_MOVE(Class) \
    Class(Class&&) noexcept = delete; \
    auto operator=(Class&&) noexcept -> Class& = delete

#define EXAGE_DEFAULT_COPY(Class) \
    Class(const Class&) noexcept = default; \
    auto operator=(const Class&) noexcept -> Class& = default

#define EXAGE_DEFAULT_MOVE(Class) \
    Class(Class&&) noexcept = default; \
    auto operator=(Class&&) noexcept -> Class& = default

#define EXAGE_DELETE_ASSIGN(Class) \
    auto operator=(const Class&) noexcept -> Class& = delete; \
    auto operator=(Class&&) noexcept -> Class& = delete

#define EXAGE_DEFAULT_ASSIGN(Class) \
    auto operator=(const Class&) noexcept -> Class& = default; \
    auto operator=(Class&&) noexcept -> Class& = default

#define EXAGE_DELETE_MOVE_COPY_CONSTRUCT(Class) \
    Class(const Class&) noexcept = delete; \
    Class(Class&&) noexcept = delete;

#define EXAGE_DEFAULT_MOVE_COPY_CONSTRUCT(Class) \
    Class(const Class&) noexcept = default; \
    Class(Class&&) noexcept = default;\
