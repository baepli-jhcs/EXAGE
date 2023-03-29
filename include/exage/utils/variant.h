#pragma once

namespace exage
{
    // Overload struct for variant
    template<typename... Ts>
    struct Overload : Ts...
    {
        using Ts::operator()...;
    };

    template<typename... Ts>
    Overload(Ts ...) -> Overload<Ts...>;
} // namespace exage
