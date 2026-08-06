#pragma once

#include <raze/options/Options.h>
#include <src/raze/algorithm/VectorizablePredicate.h>
#include <src/raze/algorithm/AlgorithmDebug.h>
#include <src/raze/algorithm/DataSource.h>
#include <src/raze/vx/dispatch/SizedSimdDispatcher.h>

#if !defined(__raze_define_kernel_dispatch)
#  define __raze_define_kernel_dispatch() \
    template <source _Source_, class ... _Args_> \
    raze_nodiscard constexpr raze_always_inline auto \
    __unchecked_kernel_dispatch(_Source_&& __src, _Args_&& ... __args) const noexcept { \
        using _TraitsType_ = decltype(this->traits()); \
        using _Value_ = std::iter_value_t<typename std::remove_cvref_t<_Source_>::iterator_type>; \
        auto __work = __kernel(std::forward<_Source_>(__src), std::forward<_Args_>(__args)...); \
        using _WorkType_ = decltype(__work); \
        using _ReturnType_ = decltype(__work.result()); \
        if constexpr (!options::always_scalar<_TraitsType_>() && _WorkType_::vectorizable()) { \
            if not consteval { \
                if constexpr (requires { _WorkType_::static_size(); }) { \
                    return raze::vx::__dispatch_sized_impl<traits_unroller_t(_TraitsType_), _Value_, _ReturnType_>(_WorkType_::static_size(), __work); \
                } \
                else { \
                    return raze::vx::__dispatch_sized_impl<traits_unroller_t(_TraitsType_), _Value_, _ReturnType_>(__work.size(), __work); \
                } \
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
