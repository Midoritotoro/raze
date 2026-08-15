#pragma once 

#include <src/raze/options/Callable.h>
#include <src/raze/options/IgnoreNone.h>
#include <src/raze/vx/Concepts.h>


__RAZE_OPTIONS_NAMESPACE_BEGIN

template <class _Type_>
concept boolean_condition = std::is_same_v<std::remove_cvref_t<_Type_>, bool>;

template <class _ConditionType_>
concept condition_type = vx::simd_mask_type<_ConditionType_> || boolean_condition<_ConditionType_>;

template <class _ConditionType_, class _AlternativeType_>
concept alternative_type = (vx::simd_mask_type<_ConditionType_> && vx::simd_type<_AlternativeType_>) ||
    (boolean_condition<_ConditionType_> && vx::arithmetic_type<_AlternativeType_>);

template <template <class> class _Function_, class _OptionsValues_, class ... _Options_>
struct __conditional_callable:  callable<_Function_, _OptionsValues_, _Options_...> {
    using func_t = _Function_<_OptionsValues_>;
    using base = callable<_Function_, _OptionsValues_, _Options_...>;

    using base::operator[];

    template <condition_type _Condition_, class _Alternative_>
    raze_always_inline constexpr auto operator[](const _Condition_& __condition, const _Alternative_& __source) const noexcept
        requires(alternative_type<_Condition_, _Alternative_>&& requires(const base& __base) { __base[or_(__condition, __source)]; })
    {
        return base::operator[](or_(__condition, __source));
    }

    template <class _Alternative_, condition_type _Condition_>
    raze_always_inline constexpr auto operator[](const _Alternative_& __source, const _Condition_& __condition) const noexcept
        requires(alternative_type<_Condition_, _Alternative_>&& requires(const base& __base) { __base[or_(__condition, __source)]; })
    {
        return base::operator[](or_(__condition, __source));
    }

    template <callable_options __Options_, class _Type_, class ... _Types_>
    constexpr raze_always_inline auto behavior(const __Options_& __options, const _Type_& __first, const _Types_& ... __args) const noexcept {
        return func_t::deferred_call(__options, __first, __args...);
    }
};

struct condition_key_t : as_keyword<condition_key_t> {
    using as_keyword<condition_key_t>::operator=;
};

constexpr inline condition_key_t condition_key = {};

template <class _Type_>
concept conditional_expression = requires(_Type_)  {
    { _Type_::has_alternative } -> std::convertible_to<bool>;
};

struct conditional_option {
    raze_always_inline constexpr auto process(const auto& __base, concepts::exactly<condition_key> auto const& __options) const noexcept {
        return raze::options::merge_prefer_first(__base, options{ __options });
    }

    raze_always_inline constexpr auto process(const auto& __base, raze::vx::simd_mask_type auto const& __option) const noexcept {
        return process(__base, condition_key = if_(__option));
    }

    raze_always_inline constexpr auto process(const auto& __base, raze::options::boolean_condition auto __option) const noexcept {
        return process(__base, condition_key = if_(__option));
    }

    raze_always_inline constexpr auto process(const auto& __base, conditional_expression auto const& __option) const noexcept {
        return process(__base, condition_key = __option);
    }

    raze_always_inline constexpr auto default_to(const auto& __base) const noexcept {
        return raze::options::merge_prefer_first(options{condition_key = ignore_none}, __base);
    }
};

template <template <class> class   _Function_, class _OptionsValues_, class ... _Options_>
struct conditional_callable: __conditional_callable<_Function_, _OptionsValues_, conditional_option, _Options_...> {
    using base_t = __conditional_callable<_Function_, _OptionsValues_, conditional_option, _Options_...>;

    template <callable_options __Options_, class _Type_, class ... _Types_>
    constexpr raze_always_inline auto behavior(const __Options_& __options, const _Type_& __first, const _Types_&... __args) const noexcept {
        return base_t::behavior(__options, __first, __args...);
    }
};

__RAZE_OPTIONS_NAMESPACE_END
