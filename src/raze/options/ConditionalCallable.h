#pragma once 

#include <src/raze/options/Callable.h>
#include <src/raze/options/IgnoreNone.h>
#include <src/raze/vx/Concepts.h>


__RAZE_OPTIONS_NAMESPACE_BEGIN

template <class T>
concept boolean_condition = std::is_same_v<std::remove_cvref_t<T>, bool>;

template <class T>
concept condition_type = vx::simd_mask_type<T> || boolean_condition<T>;

template <class Condition, class Alternative>
concept alternative_type = (vx::simd_mask_type<Condition> && vx::simd_type<Alternative>) ||
    (boolean_condition<Condition> && vx::arithmetic_type<Alternative>);

template <template <class> class F, class OptionsValues, class ... Options>
struct conditional_callable_impl:  callable<_Function_, _OptionsValues_, _Options_...> {
    using func_t = _Function_<_OptionsValues_>;
    using base_t = callable<_Function_, _OptionsValues_, _Options_...>;

    using base_t::operator[];

    template <condition_type Condition, class Alternative>
    raze_always_inline constexpr auto operator[](const Condition& condition, const Alternative& source) const noexcept
        requires(alternative_type<Condition, Alternative> && requires(const base_t& base) { base[or_(condition, source)]; })
    {
        return base_t::operator[](or_(condition, source));
    }

    template <class Alternative, condition_type Condition>
    raze_always_inline constexpr auto operator[](const Alternative& source, const Condition& condition) const noexcept
        requires(alternative_type<Condition, Alternative> && requires(const base_t& base) { base[or_(condition, source)]; })
    {
        return base_t::operator[](or_(condition, source));
    }

    template <class T, class ... Ts>
    constexpr raze_always_inline auto behavior(callable_options auto const& opts,
         const T& x1, const Ts& ... xs) const noexcept 
    {
        return func_t::deferred_call(opts, x1, xs...);
    }
};

struct condition_key_t : as_keyword<condition_key_t> {
    using as_keyword<condition_key_t>::operator=;
};

constexpr inline condition_key_t condition_key = {};

template <class T>
concept conditional_expression = requires(T)  {
    { T::has_alternative } -> std::convertible_to<bool>;
};

struct conditional_option {
    raze_always_inline constexpr auto process(const auto& base, concepts::exactly<condition_key> auto const& opts) const noexcept {
        return merge_prefer_first(base, options{ opts });
    }

    raze_always_inline constexpr auto process(const auto& base, vx::simd_mask_type auto const& o) const noexcept {
        return process(base, condition_key = if_(o));
    }

    raze_always_inline constexpr auto process(const auto& base, boolean_condition auto o) const noexcept {
        return process(base, condition_key = if_(o));
    }

    raze_always_inline constexpr auto process(const auto& base, conditional_expression auto const& o) const noexcept {
        return process(base, condition_key = o);
    }

    raze_always_inline constexpr auto default_to(const auto& base) const noexcept {
        return merge_prefer_first(options{condition_key = ignore_none}, base);
    }
};

template <template <class> class F, class OptionsValues, class ... Options>
struct conditional_callable: conditional_callable_impl<F, OptionsValues, conditional_option, Options...> {
    using base_t = conditional_callable_impl<F, OptionsValues, conditional_option, Options...>;

    template <class T, class ... Ts>
    constexpr raze_always_inline auto behavior(callable_options auto const& opts, 
        const T& x1, const Ts&... xs) const noexcept 
    {
        return base_t::behavior(opts, x1, xs...);
    }
};

__RAZE_OPTIONS_NAMESPACE_END
