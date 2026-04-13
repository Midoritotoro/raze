#pragma once 

#include <src/raze/options/Concepts.h>
#include <src/raze/options/As.h>


__RAZE_OPTIONS_NAMESPACE_BEGIN

struct __ignore_none {
    static constexpr bool has_alternative = false;
    static constexpr bool is_inverted = true;
    static constexpr bool is_complete = true;

    template <class _Value_>
    constexpr raze_always_inline auto else_(_Value_ __value) const noexcept {
        return *this;  
    }

    template <class T> 
    raze_always_inline auto mask(const as<T>& __target) const noexcept {
        // return detail::to_logical(*this, __target);
    }

    template <class _Type_> 
    constexpr raze_always_inline std::ptrdiff_t offset(const as<_Type_>&) const noexcept {
        return 0;
    }

    template <class _Type_>
    constexpr raze_always_inline std::ptrdiff_t roffset(const as<_Type_>&) const noexcept {
        return 0;
    }

    template <class _Type_>
    constexpr raze_always_inline std::ptrdiff_t count(const as<_Type_>&) const noexcept {
        return _Type_::size();
    }

    constexpr bool friend operator==(ignore_none_ const&, ignore_none_ const&) noexcept = default;
};

constexpr inline __ignore_none ignore_none = {};


__RAZE_OPTIONS_NAMESPACE_END
