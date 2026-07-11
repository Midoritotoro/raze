#pragma once 

#include <src/raze/options/Concepts.h>
#include <src/raze/options/As.h>


__RAZE_OPTIONS_NAMESPACE_BEGIN

struct __ignore_none {
    static constexpr bool has_alternative = false;
    
    __ignore_none() noexcept = default;
    __ignore_none(const __ignore_none&) noexcept = default;
    __ignore_none(__ignore_none&&) noexcept = default;
    ~__ignore_none() noexcept = default;

    __ignore_none& operator=(const __ignore_none&) noexcept = default;
    __ignore_none& operator=(__ignore_none&&) noexcept = default;

    template <class _Value_>
    constexpr raze_always_inline auto else_(_Value_ __value) const noexcept {
        return *this;  
    }

    template <class _Type_> 
    raze_always_inline _Type_ mask(const as<_Type_>& __target) const noexcept {
        return _Type_{};
    }

    raze_always_inline constexpr bool friend operator==(__ignore_none const&, __ignore_none const&) noexcept = default;
};

constexpr inline __ignore_none ignore_none = {};


__RAZE_OPTIONS_NAMESPACE_END
