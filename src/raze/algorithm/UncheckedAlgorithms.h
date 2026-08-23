#pragma once

#include <raze/options/Options.h>
#include <src/raze/algorithm/VectorizablePredicate.h>
#include <src/raze/algorithm/AlgorithmDebug.h>
#include <src/raze/algorithm/DataSource.h>
#include <src/raze/vx/dispatch/SizedSimdDispatcher.h>
#include <raze/arch/CpuFeature.h>
#include <raze/vx/Config.h>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

#pragma strict_gs_check(off)

struct autovectorizable {};

template <class Work>
concept autovectorizable_kernel = requires(Work& w) {
    w(autovectorizable{});
};

#if (defined(raze_cpp_clang) && raze_cpp_clang >= 1500) || defined(raze_cpp_gnu)

template <class Work>
struct llvm_invoke_autovec_helper_t {
    using value_type = typename Work::vector_value_type;

    llvm_invoke_autovec_helper_t(Work& w) noexcept : _work(w) {}

    raze_targets("avx512f", "avx2", "sse4.2", "default") void operator()() noexcept requires(sizeof(value_type) >= 4) {
        _work(autovectorizable{});
    }

#if defined(raze_cpp_clang)
    raze_targets("avx512bw", "avx2", "sse4.2", "default")
#elif defined(raze_cpp_gnu)
    raze_targets("arch=x86-64-v4", "avx2", "sse4.2", "default")
#endif
    void operator()() noexcept requires(sizeof(value_type) < 4) {
        _work(autovectorizable{});
    }

    Work& _work;
};

template <class Work>
auto invoke_scalar_autovec_impl(Work& w) noexcept {
    auto c = llvm_invoke_autovec_helper_t<Work>(w);
    c();
}

template <class Work>
raze_always_inline constexpr auto invoke_scalar_autovec(Work&& w) noexcept {
    if not consteval { invoke_scalar_autovec_impl(w); }
    else {  w(); }

    if constexpr (requires { w.result(); }) return w.result();
}

#endif // (defined(raze_cpp_clang) && raze_cpp_clang >= 1500) || defined(raze_cpp_gnu)

template <class Work> requires(requires (Work w) { w.result(); })
decltype(std::declval<Work>().result()) get_result_type() noexcept;

template <class Work> requires(!requires (Work w) { w.result(); })
void get_result_type() noexcept;

template <class Function, arch::ISA ... Other>
struct dispatchable {
    template <class ... Args>
    constexpr raze_always_inline auto dispatch(Args&& ... args) const noexcept {
        auto work = typename Function::kernel(std::forward<Args>(args)...);

        using TraitsType = decltype(static_cast<const Function*>(this)->traits());
        using WorkType = decltype(work);
        using Value = typename WorkType::vector_value_type;

        if constexpr (requires { work.exit(); } && requires { work.default_result(); }) {
            if (work.exit()) return work.default_result();
        }

        constexpr auto have_best_isa = vx::has_avx512bw<vx::target_isa()> ||
            (vx::has_avx512f<vx::target_isa()> && sizeof(Value) >= 4);

        constexpr auto use_autovec = options::is_autovec<TraitsType>() ||
            options::get_strategy<TraitsType>().is_autovec();

        if constexpr (!options::always_scalar<TraitsType>() &&
            WorkType::vectorizable() && (options::get_strategy<TraitsType>().is_manual()
            || (have_best_isa && use_autovec)))
        {
            if not consteval {
                using ReturnType = decltype(get_result_type<WorkType>());
                constexpr auto dispatch = vx::dispatch<options::unroller_t<TraitsType>::template __impl, Value, ReturnType, Other...>;

                if constexpr (requires { WorkType::static_size(); })
                    return dispatch(WorkType::static_size(), work);
                else
                    return dispatch(work.size(), work);
            }
        }
#if (defined(raze_cpp_clang) && raze_cpp_clang >= 1500) || defined(raze_cpp_gnu)
        else if constexpr (autovectorizable_kernel<WorkType> && use_autovec) {
            return invoke_scalar_autovec(work);
        }
#endif
        
        return options::unroller<TraitsType, vx::scalar_tag>(work);
    }
};

#pragma strict_gs_check(on)

template <class _Type_>
concept vectorizable_tag = !std::same_as<_Type_, vx::scalar_tag> && vx::simd_type<_Type_>;

template <class _Type_>
concept scalar_tag = std::same_as<_Type_, vx::scalar_tag>;

template <class _Type_>
concept tail_tag = requires(_Type_) {
    typename _Type_::original_type;
} && !vx::simd_type<_Type_>;

__RAZE_ALGORITHM_NAMESPACE_END
