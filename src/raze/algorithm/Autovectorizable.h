#pragma once

#include <raze/options/Options.h>
#include <raze/vx/Config.h>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

#pragma strict_gs_check(off)

struct autovectorizable {};

template <class Work>
concept autovectorizable_kernel = requires(Work& w) { w(autovectorizable{}); };

#if (defined(raze_cpp_clang) && raze_cpp_clang >= 1500) || defined(raze_cpp_gnu)

#if defined(raze_cpp_clang)
#  define RAZE_CLONES_NARROW "avx512bw"
#else
#  define RAZE_CLONES_NARROW "arch=x86-64-v4"
#endif

template <class Work>
struct llvm_invoke_autovec_helper_t {
    using value_type = typename Work::vector_value_type;

    Work& _work;
    llvm_invoke_autovec_helper_t(Work& w) : _work(w) {}

    raze_always_inline void operator()() {
        constexpr bool top = sizeof(value_type) >= 4
            ? vx::has_avx512f<vx::target_isa()>
            : vx::has_avx512bw<vx::target_isa()>;

        if constexpr (top)
            _work(autovectorizable{});
        else if constexpr (vx::has_avx2<vx::target_isa()>)
            from_avx2();
        else if constexpr (vx::has_sse42<vx::target_isa()>)
            from_sse42();
        else
            from_base();
    }

    raze_targets("avx512f", "avx2", "default")
    void from_avx2() requires(sizeof(value_type) >= 4) { _work(autovectorizable{}); }

    raze_targets(RAZE_CLONES_NARROW, "avx2", "default")
    void from_avx2() requires(sizeof(value_type) < 4) { _work(autovectorizable{}); }

    raze_targets("avx512f", "avx2", "sse4.2", "default")
    void from_sse42() requires(sizeof(value_type) >= 4) { _work(autovectorizable{}); }

    raze_targets(RAZE_CLONES_NARROW, "avx2", "sse4.2", "default")
    void from_sse42() requires(sizeof(value_type) < 4) { _work(autovectorizable{}); }

    raze_targets("avx512f", "avx2", "sse4.2", "default")
    void from_base() requires(sizeof(value_type) >= 4) { _work(autovectorizable{}); }

    raze_targets(RAZE_CLONES_NARROW, "avx2", "sse4.2", "default")
    void from_base() requires(sizeof(value_type) < 4) { _work(autovectorizable{}); }
};

#undef RAZE_CLONES_NARROW

template <class Work>
raze_always_inline auto invoke_scalar_autovec_impl(Work& w) {
    auto c = llvm_invoke_autovec_helper_t<Work>(w);
    c();
}

template <class Work>
raze_always_inline constexpr auto invoke_scalar_autovec(Work&& w) {
    if not consteval { invoke_scalar_autovec_impl(w); }
    else { w(); }

    if constexpr (requires { w.result(); }) return w.result();
}

#endif

#pragma strict_gs_check(on)

__RAZE_ALGORITHM_NAMESPACE_END