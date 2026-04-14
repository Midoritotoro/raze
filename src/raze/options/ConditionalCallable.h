#pragma once 

#include <src/raze/options/Callable.h>
#include <src/raze/options/IgnoreNone.h>
#include <raze/vx/Concepts.h>


__RAZE_OPTIONS_NAMESPACE_BEGIN

template <
    template <class> class  _Function_, 
    class                   _OptionsValues_, 
    class ...               _Options_>
struct __conditional_callable: 
    callable<_Function_, _OptionsValues_, _Options_...>
{
    using func_t =  _Function_<_OptionsValues_>;

    template <
        callable_options    __Options_, 
        class               _Type_,
        class ...           _Types_>
    constexpr raze_always_inline auto behavior(
        auto                __arch, 
        const __Options_&   __options, 
        _Type_              __first,
        _Types_...          __args) const
    {
        return func_t::deferred_call(__arch, __options, __first, __args...);
    }
};

struct condition_key_t : as_keyword<condition_key_t> {
    using as_keyword<condition_key_t>::operator=;
};

constexpr inline condition_key_t condition_key = {};

template <class _Type_>
concept conditional_expression = requires(_Type_)  {
    { _Type_::has_alternative } -> std::convertible_to<bool>;
    { _Type_::is_inverted } -> std::convertible_to<bool>;
    { _Type_::is_complete } -> std::convertible_to<bool>;
};

template <class _Type_> 
concept relative_conditional_expression = conditional_expression<_Type_> && requires(_Type_ __x) {
    { __x.count(as<raze::vx::simd<int, raze::vx::default_abi>>())  };
    { __x.offset(as<raze::vx::simd<int, raze::vx::default_abi>>()) };
    { __x.roffset(as<raze::vx::simd<int, raze::vx::default_abi>>()) };
};

struct relative_conditional_option {
    raze_always_inline constexpr auto process(
        const auto&                             __base, 
        relative_conditional_expression auto    __option) const
    {
        return raze::options::merge_prefer_first(__base, options{condition_key = __option});
    }

    raze_always_inline constexpr auto default_to(const auto& __base) const {
        return raze::options::merge_prefer_first(options{condition_key = ignore_none}, __base);
    }
};

template <
    template<class> class   _Function_, 
    class                   _OptionsValues_, 
    class ...               _Options_>
struct relative_conditional_callable: 
    __conditional_callable<_Function_, _OptionsValues_, 
        relative_conditional_option, _Options_...>
{
    using base_t = __conditional_callable<_Function_,
        _OptionsValues_, relative_conditional_option, _Options_...>;

    template <
        callable_options    __Options_, 
        class               _Type_,
        class ...           _Types_>
    constexpr raze_always_inline auto behavior(
        auto                __arch, 
        const __Options_&   __options, 
        _Type_              __first,
        _Types_...          __args) const noexcept
    {
        return base_t::behavior(__arch, __options, __first, __args...);
    }
};

struct conditional_option {
    raze_always_inline constexpr auto process(
        const auto&                             __base, 
        concepts::exactly<condition_key> auto   __options) const 
    {
        return raze::options::merge_prefer_first(__base, options{ __options });
    }

    raze_always_inline constexpr auto process(
        const auto&                     __base, 
        raze::vx::simd_mask_type auto   __option) const noexcept
    {
        return process(__base, condition_key = if_(__option));
    }

    raze_always_inline constexpr auto process(
        const auto&                 __base, 
        conditional_expression auto __option) const noexcept
    {
        return process(__base, condition_key = __option);
    }

    raze_always_inline constexpr auto default_to(const auto& __base) const  {
        return raze::options::merge_prefer_first(options{condition_key = ignore_none}, __base);
    }
};


template <
    template<class> class   _Function_, 
    class                   _OptionsValues_, 
    class ...               _Options_>
struct conditional_callable: __conditional_callable<_Function_, _OptionsValues_,
    conditional_option, relative_conditional_option, _Options_...>
{
    using base_t = __conditional_callable<_Function_, 
        _OptionsValues_, conditional_option, relative_conditional_option, _Options_...>;

    template <
        callable_options    __Options_, 
        class               _Type_,
        class ...           _Types_>
    constexpr raze_always_inline auto behavior(
        auto                __arch, 
        const __Options_&   __options, 
        _Type_              __first,
        _Types_...          __args) const noexcept
    {
        return base_t::behavior(__arch, __options, __first, __args...);
    }
};

struct ignore { 
    template <class _Type_> operator _Type_() { 
        return _Type_{}; 
    }
};


__RAZE_OPTIONS_NAMESPACE_END
