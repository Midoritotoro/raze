#pragma once

#include <raze/options/Options.h>
#include <raze/vx/Config.h>
#include <raze/math/Math.h>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

#if defined(raze_cpp_msvc)
#pragma strict_gs_check(off)
#endif

struct autovectorizable {};

template <class Work>
concept autovectorizable_kernel = requires(Work& w) { w(autovectorizable{}); };

template <std::contiguous_iterator It, std::sentinel_for<It> Sent, class Func>
raze_always_inline constexpr void for_noalias(It& first, Sent last, Func func) {
    auto sz = last - first;
    for (auto i = 0; i < sz; ++i)
        func(i);
    first += sz;
}

template <std::contiguous_iterator It1, std::sentinel_for<It1> Sent1,
    std::contiguous_iterator It2, std::sentinel_for<It2> Sent2, class Func>
raze_always_inline constexpr void for_noalias2(It1& first1, Sent1 last1,
    It2& first2, Sent2 last2, Func func)
{
    auto sz = math::min(last1 - first1, last2 - first2);
    for (auto i = 0; i < sz; ++i)
        func(i);
    first1 += sz;
    first2 += sz;
}

template <std::input_iterator It, std::sentinel_for<It> Sent,
    std::weakly_incrementable Out, class Func>
raze_always_inline constexpr void for_novec(It& first, Sent last, Out& out, Func func)
{
#if defined(raze_cpp_msvc_only)
    if consteval {
        for (; first != last; ++first, ++out)
            *out = func(*first);
        return;
    }
#endif
    raze_disable_unrolling
        for (; first != last; ++first, ++out)
            *out = func(*first);
}

template <std::input_iterator It, std::sentinel_for<It> Sent, class Func>
raze_always_inline constexpr void for_novec(It& first, Sent last, Func func)
{
#if defined(raze_cpp_msvc_only)
    if consteval {
        for (; first != last; ++first)
            func(*first);
        return;
    }
#endif
    raze_disable_unrolling
        for (; first != last; ++first)
            func(*first);
}

template <std::input_iterator It1, std::sentinel_for<It1> Sent1,
    std::input_iterator It2, std::sentinel_for<It2> Sent2, class Func>
raze_always_inline constexpr void for_novec2(It1& first1, Sent1 last1,
    It2& first2, Sent2 last2, Func func)
{
#if defined(raze_cpp_msvc_only)
    if consteval {
        for (; first1 != last1 && first2 != last2; ++first1, ++first2)
            func(first1, first2);
        return;
    }
#endif
    raze_disable_unrolling
    for (; first1 != last1 && first2 != last2; ++first1, ++first2)
        func(first1, first2);
}

template <std::input_iterator It1, std::sentinel_for<It1> Sent1,
    std::input_iterator It2, std::sentinel_for<It2> Sent2,
    std::weakly_incrementable Out, class Func>
raze_always_inline constexpr void for_novec2(It1& first1, Sent1 last1,
    It2& first2, Sent2 last2, Out& out, Func func)
{
#if defined(raze_cpp_msvc_only)
    if consteval {
        for (; first1 != last1 && first2 != last2; ++first1, ++first2, ++out)
            *out = func(*first1, *first2);
        return;
    }
#endif
    raze_disable_unrolling
        for (; first1 != last1 && first2 != last2; ++first1, ++first2, ++out)
            *out = func(*first1, *first2);
}

#if (defined(raze_cpp_clang) && raze_cpp_clang >= 1500 && RAZE_HAS_CLANG_RT_BUILTINS) || (defined(raze_cpp_gnu) && !defined(raze_os_windows))

#if defined(raze_cpp_clang)
#  define RAZE_CLONES_NARROW "avx512bw"
#else
#  define RAZE_CLONES_NARROW "arch=x86-64-v4"
#endif

template <class Work>
struct invoke_autovec_helper_t {
    using value_type = typename Work::vector_value_type;

    Work& _work;
    invoke_autovec_helper_t(Work& w) : _work(w) {}

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
    auto c = invoke_autovec_helper_t<Work>(w);
    c();
}

template <class Work>
raze_always_inline constexpr auto invoke_scalar_autovec(Work&& w) {
    if not consteval { invoke_scalar_autovec_impl(w); }
    else { w(); }

    if constexpr (requires { w.result(); }) return w.result();
}

#endif

#if defined(raze_cpp_msvc)
#pragma strict_gs_check(on)
#endif

__RAZE_ALGORITHM_NAMESPACE_END