#pragma once

#include <raze/options/Options.h>
#include <raze/vx/Config.h>
#include <raze/math/Math.h>
#include <raze/arch/ProcessorFeatures.h>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

#if defined(raze_cpp_msvc)
#pragma strict_gs_check(off)
#endif

/**
 * @ingroup raze_algo_dispatch
 * @brief Tag type passed to `kernel::operator()` to select the compiler auto-vectorizable (`for_noalias`) path.
*/
struct autovectorizable_tag {};

/**
 * @ingroup raze_algo_dispatch
 * @brief Tag type passed to `kernel::operator()` to select a custom native/library fast path (e.g., `memcpy`).
*/
struct native_tag {};

template <class Work>
concept autovectorizable_kernel = requires(Work& w) { w(autovectorizable_tag{}); };

template <class Work>
concept native_kernel = requires(Work & w) { w(native_tag{}); };

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

#if defined(raze_cpp_msvc_only)
#define raze_novec_loop(header, body) \
    if !consteval {                   \
        raze_do_pragma(loop(no_vector)) \
        for header body               \
        return;                       \
    }                                 \
    raze_disable_unrolling            \
    for header body
#else
#define raze_novec_loop(header, body) \
    raze_disable_unrolling            \
    for header body
#endif

template <std::input_iterator It, std::sentinel_for<It> Sent,
    std::weakly_incrementable Out, class Func>
raze_always_inline constexpr void for_novec(It& first, Sent last, Out& out, Func func)
{
#if defined(raze_cpp_msvc_only)
    if !consteval {
        raze_do_pragma(loop(no_vector))
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
    if !consteval {
        raze_do_pragma(loop(no_vector))
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
    if !consteval {
        raze_do_pragma(loop(no_vector))
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
    if !consteval {
        raze_do_pragma(loop(no_vector))
        for (; first1 != last1 && first2 != last2; ++first1, ++first2, ++out)
            *out = func(*first1, *first2);
        return;
    }
#endif
    raze_disable_unrolling
    for (; first1 != last1 && first2 != last2; ++first1, ++first2, ++out)
        *out = func(*first1, *first2);
}

#if defined(raze_cpp_clang) || defined(raze_cpp_gnu)

#define RAZE_AUTOVEC_TARGET(isa_str) __attribute__((target(isa_str), noinline))

template <class Work>
struct invoke_autovec_helper_t {
    using value_type = typename Work::vector_value_type;

    static constexpr bool is_wide = []() consteval {
        if constexpr (std::same_as<value_type, void>) return false;
        else return sizeof(value_type) >= 4;
    }();

    Work& _work;
    constexpr explicit invoke_autovec_helper_t(Work& w) noexcept : _work(w) {}

    static consteval bool compile_has_avx512() noexcept {
        return is_wide ? vx::has_avx512f<vx::target_isa()>
            : vx::has_avx512bw<vx::target_isa()>;
    }

    static raze_always_inline bool runtime_has_avx512(i32 all) noexcept {
        if constexpr (is_wide) return arch::ProcessorFeatures::has<arch::features::AVX512F>(all);
        else return arch::ProcessorFeatures::has<arch::features::AVX512F>(all) &&
            arch::ProcessorFeatures::has<arch::features::AVX512BW>(all);
    }

    RAZE_AUTOVEC_TARGET("avx512f") void run_avx512f() { _work(autovectorizable_tag{}); }
    RAZE_AUTOVEC_TARGET("avx512f,avx512bw") void run_avx512bw() { _work(autovectorizable_tag{}); }
    RAZE_AUTOVEC_TARGET("avx2") void run_avx2() { _work(autovectorizable_tag{}); }
    RAZE_AUTOVEC_TARGET("sse4.2") void run_sse42() { _work(autovectorizable_tag{}); }

    raze_always_inline void operator()() {
        if constexpr (compile_has_avx512()) {
            _work(autovectorizable_tag{});
        }
        else {
            const i32 all = arch::ProcessorFeatures::all();

            if (runtime_has_avx512(all)) {
                if constexpr (is_wide) run_avx512f();
                else run_avx512bw();
                return;
            }

            if constexpr (vx::has_avx2<vx::target_isa()>) {
                _work(autovectorizable_tag{});
            }
            else {
                if (arch::ProcessorFeatures::has<arch::features::AVX2>(all)) {
                    run_avx2();
                    return;
                }

                if constexpr (vx::has_sse42<vx::target_isa()>) {
                    _work(autovectorizable_tag{});
                }
                else {
                    if (arch::ProcessorFeatures::has<arch::features::SSE42>(all)) {
                        run_sse42();
                        return;
                    }

                    _work(autovectorizable_tag{});
                }
            }
        }
    }
};

#undef RAZE_AUTOVEC_TARGET

template <class Work>
raze_always_inline auto invoke_scalar_autovec_impl(Work& w) {
    auto c = invoke_autovec_helper_t<Work>(w);
    c();
}

template <bool IsFstatic = false, class Work>
raze_always_inline constexpr auto invoke_scalar_autovec(Work&& w) {
    if not consteval {
        if constexpr (IsFstatic) {
            w(autovectorizable_tag{});
        }
        else {
            invoke_scalar_autovec_impl(w);
        }
    }
    else { w(); }

    if constexpr (requires { w.result(); }) return w.result();
}

#endif

#if defined(raze_cpp_msvc)
#pragma strict_gs_check(on)
#endif

__RAZE_ALGORITHM_NAMESPACE_END