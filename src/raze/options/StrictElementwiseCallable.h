#pragma once 

#include <src/raze/options/Concepts.h>
#include <src/raze/options/Fetch.h>
#include <src/raze/options/ConditionalCallable.h>


__RAZE_OPTIONS_NAMESPACE_BEGIN

template <auto _Keyword_, class _Options_, class   _Type_>
concept match_option = concepts::same_as<_Type_, fetch_t<_Keyword_, _Options_>>;

inline constexpr struct { 
    raze_always_inline auto operator()(auto, auto __x) const {
        return __x; 
    } 
} return_2nd = {};

template <template <class> class  _Function_, class _OptionsValues_,  class ... _Options_>
struct strict_elementwise_callable:
    conditional_callable<_Function_, _OptionsValues_, _Options_...>
{
    using base_t = conditional_callable<_Function_, _OptionsValues_, _Options_...>;
    using func_t =  _Function_<_OptionsValues_>;

    template <callable_options __Options_, class _Type_, class ... _Types_>
    constexpr raze_always_inline auto adapt_call(const __Options_& __options, 
        _Type_ __first, _Types_ ... __args) const noexcept
    {
        if constexpr(requires{ func_t::deferred_call(__options, __first, __args...); })
            return func_t::deferred_call(__options, __first, __args...);
        else return ignore{};
    }

    template <callable_options __Options_, class _Type_, class ... _Types_>
    constexpr raze_always_inline auto behaviour(const __Options_& __options, 
        _Type_ __first, _Types_ ... __args) const noexcept
    {
        if constexpr (!__Options_::contains(condition_key) || 
            match_option<condition_key, __Options_, __ignore_none>)
        {
            return adapt_call(__options.drop(condition_key), __first, __args...);
        }
        else {
            auto [__condition, __remove_condition] = __options.extract(condition_key);

            using cond_t =  decltype(__condition);
            _Function_<decltype(__remove_condition)> const f{ __remove_condition };

            return func_t::deferred_call(__condition, __remove_condition, __first, __args...);
        }
    }

    template <class _Type_, class ... _Types_>
    constexpr raze_always_inline auto retarget(auto __arch, _Type_ __first, _Types_...  __args) const noexcept {
        return this->behavior(this->options(), __first, __args...);
    }
};

template <class _Callable_, class ... _Args_>
constexpr raze_no_stack_protector raze_always_inline auto __dispatch_call(
    const _Callable_& __callable, _Args_&& ... __args) noexcept 
{
    using _ReturnType = decltype(__callable(std::forward<_Args_>(__args)...));

    if constexpr (std::is_void_v<_ReturnType>) __callable.behavior(__callable.options(), std::forward<_Args_>(__args)...);
    else return __callable.behavior(__callable.options(), std::forward<_Args_>(__args)...);
}

#if !defined(__raze_add_fold_compatibility)
#  define __raze_add_fold_compatibility(__op) \
    template <arithmetic_type _Type_> \
    raze_nodiscard raze_always_inline _Type_ operator()(_Type_ __x, _Type_ __y) const noexcept { \
        return raze::options::__dispatch_call(*this, __x, __y); \
    } \
    \
    template <class _Value_, arch::ISA _ISA_, intrin_type _First_, class ... _Args_> \
    raze_nodiscard raze_always_inline _First_ operator()(options::as<_Value_> __v, \
        std::integral_constant<arch::ISA, _ISA_> __c, _First_ __first, _Args_&& ... __args) const noexcept \
    { \
        return raze::options::__dispatch_call(*this, __v, __c, __first, __args...); \
    } \
    \
    template <class _Value_, arch::ISA _ISA_, class ... _Args_> \
    static raze_always_inline auto deferred_call(auto __options, options::as<_Value_>, \
        std::integral_constant<arch::ISA, _ISA_>, _Args_&& ... __args) noexcept \
    { \
        return __op<_ISA_, _Value_>()(__args...); \
    } \
    \
    template <arithmetic_type _Type_> \
    static raze_always_inline auto deferred_call(auto __options, _Type_ __x, _Type_ __y) noexcept { \
        return __op<arch::ISA::None, _Type_>()(__x, __y); \
    }
#endif // !defined(__raze_add_fold_compatibility)
__RAZE_OPTIONS_NAMESPACE_END
