#pragma once

#include <string>
namespace exage
{

    inline auto fromU8string(const std::string& s) -> std::string
    {
        return s;
    }
    inline auto fromU8string(std::string&& s) -> std::string
    {
        return std::move(s);
    }

#if defined(__cpp_lib_char8_t)
    inline auto fromU8string(const std::u8string& s) -> std::string
    {
        return {s.begin(), s.end()};
    }
#endif
}  // namespace exage