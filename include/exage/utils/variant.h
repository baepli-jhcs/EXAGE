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
    Overload(Ts...) -> Overload<Ts...>;

    template<class F>
    struct YCombinator
    {
        F f;
        template<class... Args>
        decltype(auto) operator()(Args&&... args)
        {
            return f(*this, std::forward<Args>(args)...);
        }
    };

    template<class F>
    YCombinator<std::decay_t<F>> makeYCombinator(F&& f)
    {
        return {std::forward<F>(f)};
    }
}  // namespace exage
