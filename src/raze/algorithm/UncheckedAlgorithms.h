#pragma once

#include <raze/options/Options.h>
#include <src/raze/algorithm/VectorizablePredicate.h>
#include <src/raze/algorithm/AlgorithmDebug.h>
#include <src/raze/vx/dispatch/SizedSimdDispatcher.h>

#if !defined(__raze_define_kernel_dispatch)
#  define __raze_define_kernel_dispatch(...) \
    template <std::ranges::input_range _Range_, class ... _Args_> \
    raze_nodiscard constexpr raze_always_inline auto \
    __unchecked_kernel_dispatch(_Range_&& __r, _Args_&& ... __args) const noexcept { \
        return __unchecked_kernel_dispatch_impl(std::forward<_Range_>(__r), std::forward<_Args_>(__args)...); \
    } \
    template <std::input_iterator _Iterator_, std::sentinel_for<_Iterator_> _Sentinel_, class ... _Args_> \
    raze_nodiscard constexpr raze_always_inline auto \
    __unchecked_kernel_dispatch(_Iterator_ __first, _Sentinel_ __last, _Args_&& ... __args) const noexcept { \
        return __unchecked_kernel_dispatch_impl(traits::__uiter<_Sentinel_>(std::move(__first)), \
            traits::__usent<_Iterator_>(std::move(__last)), std::forward<_Args_>(__args)...); \
    } \
    template <std::input_iterator _Iterator_, std::sentinel_for<_Iterator_> _Sentinel_, class ... _Args_> \
    raze_nodiscard constexpr raze_always_inline auto \
    __unchecked_kernel_dispatch_impl(_Iterator_ __first, _Sentinel_ __last, _Args_&& ... __args) const noexcept { \
        using _TraitsType_ = decltype(this->traits()); \
        using _Value_ = std::iter_value_t<_Iterator_>; \
        auto __work = __kernel(__first, __last, std::forward<_Args_>(__args)...); \
        using _WorkType_ = decltype(__work); \
        using _ReturnType_ = decltype(__work.result()); \
        if constexpr (!options::always_scalar<_TraitsType_>() && _WorkType_::vectorizable) { \
            if not consteval { \
                constexpr auto __offset = __VA_OPT__(__VA_ARGS__) + 0; \
                auto __bytes = __bytes_distance(__first, __last); \
                raze_debug_assert(__bytes >= __offset, "The range size is smaller than the offset."); \
                __bytes -= __offset; \
                return raze::vx::__dispatch_sized_impl<traits_unroller_t(_TraitsType_), _Value_, _ReturnType_>(__bytes, __work); \
            } \
        } \
        return raze::options::__unroller<_TraitsType_, raze::vx::scalar_tag>(__work); \
    } \
    template <std::ranges::input_range _Range_, class ... _Args_> \
    raze_nodiscard constexpr raze_always_inline auto \
    __unchecked_kernel_dispatch_impl(_Range_&& __r, _Args_&& ... __args) const noexcept { \
        auto __first = traits::__ubegin(__r); \
        auto __last = traits::__uend(__r); \
        using _Iterator_ = decltype(__first); \
        using _TraitsType_ = decltype(this->traits()); \
        using _Value_ = std::iter_value_t<_Iterator_>; \
        auto __work = __kernel(__first, __last, std::forward<_Args_>(__args)...); \
        using _WorkType_ = decltype(__work); \
        using _ReturnType_ = decltype(__work.result()); \
        if constexpr (!options::always_scalar<_TraitsType_>() && _WorkType_::vectorizable) { \
            if not consteval { \
                raze_debug_assert(__bytes_distance(__r) >= __offset, "The range size is smaller than the offset."); \
                return raze::vx::__dispatch_sized_impl<traits_unroller_t(_TraitsType_), _Value_, _ReturnType_>(__bytes_distance(__r), __work); \
            } \
        } \
        return raze::options::__unroller<_TraitsType_, raze::vx::scalar_tag>(__work); \
    }
#endif

#if !defined(__raze_kernel_dispatch_call)
#  define __raze_kernel_dispatch_call(...) __unchecked_kernel_dispatch(__VA_ARGS__)
#endif // !defined(__raze_kernel_dispatch_call)

__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <class _Type_>
concept vectorizable_tag = !options::concepts::same_as<_Type_, vx::scalar_tag>;

template <class _Type_>
concept scalar_tag = options::concepts::same_as<_Type_, vx::scalar_tag>;

__RAZE_ALGORITHM_NAMESPACE_END
