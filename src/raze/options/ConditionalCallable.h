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
    constexpr inline auto behavior(
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
concept conditional_expr = requires(_Type_)  {
    { _Type_::has_alternative };
    { _Type_::is_inverted };
    { _Type_::is_complete };
};

template <class _Type_> 
concept relative_conditional_expr = conditional_expr<_Type_> && requires(_Type_ __x) {
    { __x.count(as<raze::vx::simd<int, raze::vx::default_abi>>())  };
    { __x.offset(as<raze::vx::simd<int, raze::vx::default_abi>>()) };
    { __x.roffset(as<raze::vx::simd<int, raze::vx::default_abi>>()) };
};

struct relative_conditional_option {
    inline constexpr auto process(
        const auto&                     __base, 
        relative_conditional_expr auto  __option) const 
    {
        return raze::options::merge_prefer_first(__base, options{condition_key = __option});
    }

    inline constexpr auto default_to(const auto& __base) const {
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
    constexpr inline auto behavior(
        auto                __arch, 
        const __Options_&   __options, 
        _Type_              __first,
        _Types_...          __args) const noexcept
    {
        return base_t::behavior(__arch, __options, __first, __args...);
    }
};

template <
    class _Condition_,
    class _Value_>
struct or_: _Condition_ {
    static constexpr bool has_alternative = true;

    using alternative_type = _Value_;
    using conditional_type = _Condition_;

    constexpr or_(const _Condition_& __condition, const _Value_& __value) noexcept: 
        _Condition_(__condition), _alternative(__value) 
    {}

    constexpr _Condition_ base() const {
        return *this; 
    }

    template <class _Type_> 
    constexpr auto rebase(_Type_ __value) const {
        return or_<_Condition_, _Type_>{static_cast<const _Condition_&>(*this), __value};
    }

    constexpr _Condition_ drop_alternative() const { 
        return *this; 
    }

    constexpr auto map_alternative(auto __operation) const {
        auto __mapped = __operation(_alternative);
        _Condition_ __condition = *this;

        return or_<_Condition_, decltype(__mapped)>{__condition, __mapped};
    }

    _Value_ _alternative;
};

template <class _Condition_>
struct if_
  {
    static constexpr bool has_alternative = false;
    static constexpr bool is_inverted     = false;
    static constexpr bool is_complete     = false;

    if_(_Condition_ __condition) noexcept:
        _condition(__condition) 
    {}

    template <class _Value_>
    raze_always_inline auto else_(const _Value_& __value) const noexcept {
        return or_(*this, __value);
    }

    template <class _Type_>
    raze_always_inline auto mask(const as<_Type_>&) const noexcept {
        return _condition;
    }

    _Condition_ _condition;
  };

struct conditional_option {
    inline constexpr auto process(
        const auto&                             __base, 
        concepts::exactly<condition_key> auto   __options) const 
    {
        return raze::options::merge_prefer_first(__base, options{ __options });
    }

    inline constexpr auto process(
        const auto&                     __base, 
        raze::vx::simd_mask_type auto   __option) const 
    {
        return process(__base, condition_key = if_(__option));
    }

    inline constexpr auto process(const auto& __base, conditional_expr auto __option) const {
        return process(__base, condition_key = __option);
    }

    inline constexpr auto default_to(const auto& __base) const  {
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
    constexpr inline auto behavior(
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
