#pragma once

#include <type_traits>
#include <concepts>

namespace exage::Graphics
{
    template<typename T, typename A, typename B>
    constexpr bool isApiDerivedType = requires()
    {
        std::derived_from<T, B>;
        {
            T::getStaticAPI()
        } -> std::same_as<A>;
    };
} // namespace exage::Graphics

#define EXAGE_BASE_API(apitype, base) \
        [[nodiscard]] virtual auto getAPI() const noexcept -> apitype = 0; \
	template<typename T> \
        requires exage::Graphics::isApiDerivedType<T, apitype, base> \
	[[nodiscard]] auto as() noexcept -> T* \
	{ \
            if (T::getStaticAPI() == getAPI()) \
                return static_cast<T*>(this); \
            return nullptr; \
	}

#define EXAGE_DERIVED_API(apitype, enu) \
    [[nodiscard]] auto getAPI() const noexcept -> apitype override \
    { \
        return apitype::enu; \
    } \
    [[nodiscard]] static auto getStaticAPI() noexcept -> apitype \
    { \
        return apitype::enu; \
    }

#define EXAGE_VULKAN_DERIVED EXAGE_DERIVED_API(API, eVulkan)
