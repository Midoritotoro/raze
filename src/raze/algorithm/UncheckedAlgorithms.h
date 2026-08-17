#pragma once

#include <raze/options/Options.h>
#include <src/raze/algorithm/VectorizablePredicate.h>
#include <src/raze/algorithm/AlgorithmDebug.h>
#include <src/raze/algorithm/DataSource.h>
#include <src/raze/vx/dispatch/SizedSimdDispatcher.h>
#include <raze/arch/CpuFeature.h>

#if !defined(__raze_define_kernel_dispatch)
#  define __raze_define_kernel_dispatch(...) \
    template <class ... _Args_> \
    raze_nodiscard constexpr raze_always_inline auto \
    __unchecked_kernel_dispatch(_Args_&& ... __args) const noexcept { \
        using _TraitsType_ = decltype(this->traits()); \
        auto __work = __kernel(std::forward<_Args_>(__args)...); \
        using _WorkType_ = decltype(__work); \
        using _Value_ = typename _WorkType_::vector_value_type; \
        using _ReturnType_ = decltype(__work.result()); \
        if constexpr (requires { __work.exit(); } && requires { __work.default_result(); }) { \
            if (__work.exit()) { return __work.default_result(); } \
        } \
        if constexpr (!options::always_scalar<_TraitsType_>() && _WorkType_::vectorizable()) { \
            if not consteval { \
                if constexpr (requires { _WorkType_::static_size(); }) { \
                    return raze::vx::__dispatch_sized_impl<traits_unroller_t(_TraitsType_), _Value_, _ReturnType_ __VA_ARGS__>(_WorkType_::static_size(), __work); \
                } \
                else { \
                    return raze::vx::__dispatch_sized_impl<traits_unroller_t(_TraitsType_), _Value_, _ReturnType_ __VA_ARGS__>(__work.size(), __work); \
                } \
            } \
        } \
        return raze::options::__unroller<_TraitsType_, raze::vx::scalar_tag>(__work); \
    }
#endif

#if !defined(__raze_kernel_dispatch_call)
#  define __raze_kernel_dispatch_call(...) __unchecked_kernel_dispatch(__VA_ARGS__)
#endif // !defined(__raze_kernel_dispatch_call)


#if !defined(raze_kernel_scalar_paths)
#  define raze_kernel_scalar_paths(__code) \
    raze_always_inline constexpr void operator()() noexcept { \
        raze_disable_unrolling \
        __code \
    } \
    raze_always_inline void autovec_run() noexcept { \
        __code \
    }
#endif // !defined(raze_kernel_scalar_paths)

__RAZE_ALGORITHM_NAMESPACE_BEGIN

#pragma strict_gs_check(off)

#if (defined(raze_cpp_clang) && raze_cpp_clang >= 1500) || defined(raze_cpp_gnu)

template <class _Work_>
struct __llvm_invoke_autovec_helper_t {
    using value_type = typename _Work_::vector_value_type;

    __llvm_invoke_autovec_helper_t(_Work_ __work) noexcept :
        _work(__work) {}

    raze_targets("avx512f", "avx2", "default") void operator()() noexcept requires(sizeof(value_type) >= 4) {
        _work.autovec_run();
    }

#if defined(raze_cpp_clang)
    raze_targets("avx512bw", "avx2", "default")
#elif defined(raze_cpp_gnu)
    raze_targets("arch=x86-64-v4", "avx2", "default")
#endif
    void operator()() noexcept requires(sizeof(value_type) < 4) {
        _work.autovec_run();
    }

    _Work_ _work;
};

template <class _Work_>
auto __invoke_scalar_autovec_impl(_Work_& __work) noexcept {
    auto __c = __llvm_invoke_autovec_helper_t<_Work_>(__work);
    __c();
}

template <class _Work_>
raze_always_inline auto __invoke_scalar_autovec(_Work_&& __work) noexcept {
    __invoke_scalar_autovec_impl(__work);
    if constexpr (requires { __work.result(); }) return __work.result();
}

#endif // (defined(raze_cpp_clang) && raze_cpp_clang >= 1500) || defined(raze_cpp_gnu)

template <class _Function_, arch::ISA _Default_ = arch::ISA::None, arch::ISA ... _Other_>
struct dispatchable {
    template <class ... _Args_>
    constexpr raze_always_inline auto dispatch(_Args_&& ... __args) const noexcept {
        using _TraitsType_ = decltype(static_cast<const _Function_*>(this)->traits());
        auto __work = typename _Function_::__kernel(std::forward<_Args_>(__args)...);
        using _WorkType_ = decltype(__work);
        using _Value_ = typename _WorkType_::vector_value_type;
        using _ReturnType_ = decltype(__work.result());

        if constexpr (requires { __work.exit(); } && requires { __work.default_result(); }) {
            if (__work.exit()) {
                return __work.default_result();
            }
        }

        if constexpr (options::is_autovec<_TraitsType_>()) {
#if (defined(raze_cpp_clang) && raze_cpp_clang >= 1500) || defined(raze_cpp_gnu)
            return __invoke_scalar_autovec(__work);
#else 
            return raze::options::__unroller<_TraitsType_, raze::vx::scalar_tag>(__work);
#endif
        }
      
        if constexpr (!raze::options::always_scalar<_TraitsType_>() && _WorkType_::vectorizable()) {
            if not consteval {
                if constexpr (requires { _WorkType_::static_size(); }) {
                    return raze::vx::__dispatch_sized_impl<raze::options::_Unroller<_TraitsType_>::
                        template __impl, _Value_, _ReturnType_>(_WorkType_::static_size(), __work);
                }
                else {
                    return raze::vx::__dispatch_sized_impl<raze::options::_Unroller<_TraitsType_>::
                        template __impl, _Value_, _ReturnType_>(__work.size(), __work);
                }
            }
        }

        return raze::options::__unroller<_TraitsType_, raze::vx::scalar_tag>(__work);
    }
};

#pragma strict_gs_check(on)

template <class _Type_>
concept vectorizable_tag = !options::concepts::same_as<_Type_, vx::scalar_tag> && vx::simd_type<_Type_>;

template <class _Type_>
concept scalar_tag = options::concepts::same_as<_Type_, vx::scalar_tag>;

template <class _Type_>
concept tail_tag = requires(_Type_) {
    typename _Type_::original_type;
} && !vx::simd_type<_Type_>;

__RAZE_ALGORITHM_NAMESPACE_END
