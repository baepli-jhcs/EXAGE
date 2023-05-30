#pragma once

#include <functional>

#include <stdint.h>

namespace exage
{
    inline void hashCombine([[maybe_unused]] size_t& seed) {}

    template<typename T, typename... Rest>
    inline void hashCombine(size_t& seed, const T& v, Rest... rest)
    {
        std::hash<T> hasher;
        seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        hashCombine(seed, rest...);
    }
}  // namespace exage

#define EXAGE_MAKE_HASHABLE(type, ...) \
    namespace std \
    { \
        template<> \
        struct hash<type> \
        { \
            size_t operator()(const type& t) const \
            { \
                size_t ret = 0; \
                exage::hashCombine(ret, __VA_ARGS__); \
                return ret; \
            } \
        }; \
    }
