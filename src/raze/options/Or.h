#pragma once 

#include <src/raze/options/Concepts.h>

__RAZE_OPTIONS_NAMESPACE_BEGIN

template <class _Type_>
struct __storage_selector {
    using type = std::conditional_t<sizeof(_Type_) <= 8, _Type_, const _Type_&>;
};

template <class _Type_>
using __storage_selector_t = typename __storage_selector<_Type_>::type;

template <class _Condition_, class _Value_>
struct or_ {
    static constexpr bool has_alternative = true;
    static constexpr bool is_inverted = false;
    static constexpr bool is_complete = false;

    using alternative_type = _Value_;
    using condition_type = _Condition_;
    
    using _ValueStorage_ = __storage_selector_t<_Value_>;
    using _ConditionStorage_ = __storage_selector_t<_Condition_>;

    constexpr or_(const _Condition_& __condition, const _Value_& __value) noexcept:
        _condition(__condition), _alternative(__value)
    {}

    constexpr or_(const or_&) noexcept = default;
    constexpr or_(or_&&) noexcept = default;

    constexpr or_& operator=(const or_&) noexcept = default;
    constexpr or_& operator=(or_&&) noexcept = default;

    constexpr raze_always_inline decltype(auto) alternative() const noexcept {
        return _alternative;
    }

    template <class _Type_>
    raze_always_inline auto mask(const as<_Type_>&) const noexcept {
        return _condition;
    }

    raze_no_unique_address _ConditionStorage_ _condition;
    raze_no_unique_address _ValueStorage_ _alternative;
};

__RAZE_OPTIONS_NAMESPACE_END