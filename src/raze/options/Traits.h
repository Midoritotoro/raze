#pragma once 

#include <src/raze/options/Callable.h>
#include <src/raze/algorithm/StrategyBuilder.h>
#include <src/raze/vx/hw/configurable/memory/AlignedOption.h>

__RAZE_OPTIONS_NAMESPACE_BEGIN

template <class Settings>
struct traits : Settings {
    template <concepts::option ... Options>
    constexpr explicit traits(Options && ... opts) noexcept: 
        Settings(std::forward<Options>(opts)...) 
    {}

    template <class ... Options>
    constexpr traits(const settings<Options...>& opts) noexcept:
        Settings(opts)
    {}
};

template <concepts::option ... Options>
traits(Options&& ... opts) -> traits<raze::options::settings<std::decay_t<Options>...>>;

struct unroll_key_t: as_keyword<unroll_key_t> {
    template <class Value> 
    constexpr auto operator=(const Value&) const noexcept {
        return option<unroll_key_t, Value>{};
    }
};
  
constexpr inline unroll_key_t unroll_key;

template <sizetype N>
constexpr inline auto index = std::integral_constant<sizetype, N>{};

/**
 * @ingroup raze_options
 * @brief Option hint that sets the loop unrolling factor for manual SIMD loops to @p N.
 *
 * Instructs the algorithm to duplicate the manual SIMD loop body @p N times per iteration.
 *
 * ### Declarations
 * ```cpp
 * template< sizetype N >
 * constexpr inline *unspecified* unroll;
 * ```
 *
 * ### Notes
 * - Applies as a hint only to explicit (manual) SIMD loops; it has no effect on scalar
 *   or compiler auto-vectorized (`autovec`) execution paths.
 * - If not specified, algorithms provide their own default (typically `unroll<4>`).
 * - Setting `unroll<1>` disables manual SIMD loop unrolling.
 *
 * ### Example
 * ```cpp
 * // Execute with a manual SIMD unrolling factor of 2
 * raze::algorithm::replace[raze::options::unroll<2>](v, 0, 1);
 * ```
 */
template <sizetype N>
constexpr inline auto unroll = (unroll_key = index<N>);

template <class Traits>
constexpr sizetype get_unrolling() noexcept {
    return raze::options::fetch_t<(unroll_key | index<1>), Traits>{};
}

struct aligned_key_t : as_keyword<aligned_key_t> {
    template <class Value>
    constexpr auto operator=(const Value&) const noexcept {
        return option<aligned_key_t, Value>{};
    }
};

constexpr inline aligned_key_t aligned_key;

/**
 * @ingroup raze_options
 * @brief Option decorator declaring that contiguous range pointers are aligned to at least @p N bytes.
 *
 * Allows SIMD kernels to use aligned loads/stores and skip alignment peeling whenever the
 * guaranteed alignment @p N satisfies the active ISA register alignment (`N >= alignof(Tag)`).
 *
 * ### Declarations
 * ```cpp
 * template< sizetype N >
 *   requires(N != 0 && (N & (N - 1)) == 0)
 * constexpr inline *unspecified* aligned;
 * ```
 *
 * ### Notes
 * - @p N must be a non-zero power of two (e.g., `16`, `32`, `64`).
 * - **Safe with dynamic dispatch**: if runtime CPU dispatch selects a wider ISA than @p N
 *   (for example, `AVX-512` requiring 64-byte alignment when `aligned<32>` was specified),
 *   the kernel automatically uses unaligned accesses for that ISA instead of faulting.
 * - In compiler auto-vectorized (`autovec`) paths, enables alignment assumptions
 *   (e.g., via `std::assume_aligned<N>`).
 * - **Precondition**: the underlying data pointer(s) of the contiguous range(s) must actually be
 *   aligned to at least @p N bytes; otherwise, the behavior is undefined.
 *
 * ### Example
 * ```cpp
 * alignas(32) std::array<int, 256> src{};
 * alignas(32) std::array<int, 256> dst{};
 *
 * // Safe even if runtime dispatch selects AVX-512 (64-byte vectors)
 * raze::algorithm::copy[raze::options::aligned<32>](src, dst.begin());
 * ```
 */
template <sizetype N> requires (N != 0 && (N & (N - 1)) == 0)
constexpr inline auto aligned = (aligned_key = index<N>);

template <class Traits>
constexpr sizetype get_alignment() noexcept {
    return raze::options::fetch_t<(aligned_key | index<1>), Traits>{};
}

struct strategy_key_t : as_keyword<strategy_key_t> {
    template <class Value>
    constexpr auto operator=(const Value&) const noexcept {
        return option<strategy_key_t, Value>{};
    }
};

constexpr inline strategy_key_t strategy_key;

template <algorithm::strategy Strategy>
constexpr inline auto strategy = (strategy_key = Strategy);

template <class Traits>
constexpr auto get_strategy() noexcept {
    return raze::options::fetch_t<(strategy_key | algorithm::strategy{}), Traits>{};
}
 
struct fscalar_mode {};

/**
 * @ingroup raze_options
 * @brief Option flag that forces scalar execution mode.
 *
 * When attached to an algorithm, `fscalar` completely disables all
 * SIMD hardware intrinsics (including compiler auto-vectorization via compiler pragmas).
 *
 * Forces the algorithm to execute using only standard scalar instructions and loops.
 *
 * ### Declarations
 * ```cpp
 * constexpr inline *unspecified* fscalar;
 * ```
 *
 * ### Notes
 * - Guarantees strict sequential in-order execution when used in single-threaded mode.
 * - When combined with `par` (`[par][fscalar]`), executes in parallel across threads while
 *   keeping each thread's inner loop strictly scalar (similar to `std::execution::par`).
 *
 * ### Example
 * ```cpp
 * // Disables SIMD; forces scalar iteration
 * raze::algorithm::replace[raze::options::fscalar](vec, 42, 99);
 * ```
 */
constexpr inline auto fscalar = raze::options::flag(fscalar_mode{});

template <class Traits>
constexpr bool is_fscalar() noexcept {
    return Traits::contains(fscalar);
}

struct fautovec_mode {};

/**
 * @ingroup raze_options
 * @brief Option flag that opts into compiler auto-vectorization (`autovec`) without `raze::vx::simd` probing.
 *
 * Instructs the algorithm to execute via its compiler auto-vectorizable loop
 * whenever supported, bypassing `raze::vx::simd` callable probing. If the algorithm does not
 * provide an auto-vectorizable kernel, or the input is non-contiguous, execution falls back
 * to the scalar path.
 *
 * ### Declarations
 * ```cpp
 * constexpr inline *unspecified* fautovec;
 * ```
 *
 * ### Notes
 * - Useful for scalar-typed callables, branching logic (e.g., `if (x > 0)`), `const` references,
 *   or custom projections that cannot be instantiated with `raze::vx::simd` yet are safe for
 *   unsequenced compiler vectorization.
 * - **Execution contract**: by specifying `fautovec`, the caller guarantees unsequenced (`unseq`)
 *   semantics — no loop-carried dependencies and no reliance on mutating internal functor state
 *   across iterations. Exact vectorization and codegen may vary across compilers.
 * - When combined with `par` (`[par][fautovec]`), runs compiler auto-vectorization within each
 *   parallel chunk.
 * - In constant-evaluated contexts (`if consteval`), falls back to the scalar path.
 * - Should not be combined with `fscalar`.
 *
 * ### Example
 * ```cpp
 * // Uses compiler auto-vectorization
 * // even with an explicit scalar parameter and conditional branch
 * raze::algorithm::for_each[raze::options::fautovec](v, [](int& x) {
 *     if (x > 0) x *= 2;
 * });
 * ```
 */
constexpr inline auto fautovec = raze::options::flag(fautovec_mode{});

template <class Traits>
constexpr bool is_fautovec() noexcept {
    return Traits::contains(fautovec);
}

struct fstatic_mode {};

/**
 * @ingroup raze_options
 * @brief Option flag enabling static implementation dispatch.
 *
 * Instructs algorithms to disable runtime ISA dynamic dispatch and rely strictly on the
 * compile-time target ISA.
 *
 * ### Declarations
 * ```cpp
 * constexpr inline *unspecified* fstatic;
 * ```
 *
 * ### Example
 * ```cpp
 * // Static dispatch using the compile-time target ISA
 * raze::algorithm::replace[raze::options::fstatic](arr, 0, 1);
 * ```
 */
constexpr inline auto fstatic = raze::options::flag(fstatic_mode{});

template <class Traits>
constexpr bool is_fstatic() noexcept {
    return Traits::contains(fstatic);
}

struct par_mode {};

/**
 * @ingroup raze_options
 * @brief Option flag that enables multi-threaded parallel execution.
 *
 * Splits eligible ranges into chunks and executes the algorithm kernel concurrently across
 * multiple worker threads.
 *
 * ### Declarations
 * ```cpp
 * constexpr inline *unspecified* par;
 * ```
 *
 * ### Notes
 * - Orthogonal to vectorization and dispatch options:
 *   - `[par]` — parallel across threads with SIMD / `autovec` inside each chunk (when vectorizable).
 *   - `[par][fscalar]` — parallel across threads, strictly scalar inside each chunk.
 *   - `[par][fautovec]` — parallel across threads with compiler auto-vectorization inside each chunk.
 *   - `[par][fstatic]` — parallel across threads without runtime ISA dynamic dispatch.
 * - Partitioning granularity and concurrency can be tuned with `grain_size<N>` and `threads<N>`.
 * - **Execution contract**: elements and chunks may be processed concurrently in arbitrary order.
 *   User-provided callables, predicates, and projections must be thread-safe and free of data races
 *   and loop-carried dependencies.
 * - In constant-evaluated contexts (`if consteval`), or for ranges too small to split according to
 *   the active grain size, execution runs sequentially on the calling thread.
 *
 * ### Example
 * ```cpp
 * // Parallel vectorized execution with custom grain size and thread limit
 * raze::algorithm::for_each[raze::options::par]
 *                          [raze::options::grain_size<4096>]
 *                          [raze::options::threads<4>](
 *     v, [](auto& x) { x *= 2; });
 * ```
 */
constexpr inline auto par = raze::options::flag(par_mode{});

template <class Traits>
constexpr bool is_par() noexcept {
    return Traits::contains(par);
}

struct grain_size_key_t : as_keyword<grain_size_key_t> {
    template <class Value>
    constexpr auto operator=(const Value&) const noexcept {
        return option<grain_size_key_t, Value>{};
    }
};

constexpr inline grain_size_key_t grain_size_key;

/**
 * @ingroup raze_options
 * @brief Option decorator that sets the minimum parallel chunk size (in elements) to @p N.
 *
 * Controls work partitioning granularity when parallel execution (`par`) is enabled.
 * Ranges smaller than the threshold execute on the calling thread to avoid synchronization overhead.
 *
 * ### Declarations
 * ```cpp
 * template< sizetype N >
 *   requires(N > 0)
 * constexpr inline *unspecified* grain_size;
 * ```
 *
 * ### Notes
 * - Has an effect only when combined with `raze::options::par`.
 * - Larger values reduce scheduling overhead for lightweight element operations; smaller values
 *   improve load balancing for expensive callables or uneven workloads.
 * - If not specified, `get_grain_size()` returns `0`, instructing the algorithm to use its
 *   default partitioning heuristic.
 *
 * ### Example
 * ```cpp
 * // Require at least 8192 elements per parallel chunk
 * raze::algorithm::transform[raze::options::par]
 *                           [raze::options::grain_size<8192>](
 *     src, dst.begin(), [](auto x) { return x + 1; });
 * ```
 */
template <sizetype N>
    requires (N > 0)
constexpr inline auto grain_size = (grain_size_key = index<N>);

template <class Traits>
constexpr sizetype get_grain_size() noexcept {
    return raze::options::fetch_t<(grain_size_key | index<0>), Traits>{};
}

struct threads_key_t : as_keyword<threads_key_t> {
    template <class Value>
    constexpr auto operator=(const Value&) const noexcept {
        return option<threads_key_t, Value>{};
    }
};

constexpr inline threads_key_t threads_key;

/**
 * @ingroup raze_options
 * @brief Option decorator that sets the maximum number of threads for parallel execution to @p N.
 *
 * Constrains the degree of concurrency used when parallel execution (`par`) is enabled.
 *
 * ### Declarations
 * ```cpp
 * template< sizetype N >
 *   requires(N > 0)
 * constexpr inline *unspecified* threads;
 * ```
 *
 * ### Notes
 * - Has an effect only when combined with `raze::options::par`.
 * - Fewer than @p N threads may be used if the input size and `grain_size` yield fewer chunks.
 * - Setting `threads<1>` restricts execution to a single thread.
 * - If not specified, `get_threads()` returns `0`, deferring the thread count to the runtime
 *   or hardware concurrency.
 *
 * ### Example
 * ```cpp
 * // Parallelize across at most 4 threads
 * raze::algorithm::for_each[raze::options::par]
 *                          [raze::options::threads<4>](
 *     v, [](auto& x) { x *= 2; });
 * ```
 */
template <sizetype N> requires (N > 0)
constexpr inline auto threads = (threads_key = index<N>);

template <class Traits>
constexpr sizetype get_threads() noexcept {
    return raze::options::fetch_t<(threads_key | index<0>), Traits>{};
}

struct strict_types_mode {};

/**
 * @ingroup raze_options
 * @brief Option flag that enforces strict input type matching for SIMD vectorization.
 *
 * By default, mixed-type binary algorithms (e.g., `transform`) allow SIMD 
 * vectorization of heterogeneous inputs (such as `int32_t` and `float`) by 
 * promoting both operands to their `std::common_type_t` before invoking the 
 * user-provided function. This mirrors standard scalar usual arithmetic conversions.
 *
 * However, if the user-provided functor inspects the exact static type of its 
 * arguments (e.g., via `decltype`, `sizeof`, or type-specific overloads), this 
 * implicit common-type promotion may alter the intended semantics.
 *
 * Attaching `strict_types` disables this promotion. The algorithm will only 
 * utilize the vectorized SIMD path if both input types are strictly identical. 
 * If the types differ, it safely falls back to the scalar execution path, 
 * guaranteeing 100% scalar-equivalent semantics.
 *
 * ### Declarations
 * ```cpp
 * constexpr inline *unspecified* strict_types;
 * ```
 *
 * ### Example
 * ```cpp
 * // Default: int32_t and float are promoted to float, and then vectorized.
 * raze::algorithm::transform(a_int, b_float, out, [](auto x, auto y) { 
 *     return x + y; 
 * });
 *
 * // Strict: int32_t and float differ, so it falls back to scalar iteration.
 * // Ideal for lambdas that behave differently for int32_t and float.
 * raze::algorithm::transform[raze::options::strict_types](
 *     a_int, b_float, out, [](auto x, auto y) { 
 *         if constexpr (std::is_same_v<decltype(x), int32_t>) return x; 
 *         else return 0.0f; 
 *     });
 * ```
 */
constexpr inline auto strict_types = raze::options::flag(strict_types_mode{});

template <class Traits>
constexpr bool is_strict_types() noexcept {
    return Traits::contains(strict_types);
}

constexpr inline auto no_traits = traits<settings<>>();

template <template <class> class F, class Traits>
struct supports_traits {
    using traits_type = Traits;

    raze_always_inline constexpr Traits traits() const {
        return _traits;
    }

    constexpr supports_traits() {}
    constexpr explicit supports_traits(Traits trs):
        _traits(trs)
    {}

    template <class Settings>
    raze_always_inline constexpr auto operator[](raze::options::traits<Settings> tr) const {
        using SettingsType = decltype(raze::options::merge(tr, _traits));
        auto sum = raze::options::traits<SettingsType>(raze::options::merge(tr, _traits));

        using ReboundType = supports_traits<F, decltype(sum)>;
        return F<ReboundType>(ReboundType(sum));
    }

    template <concepts::option Trait>
    raze_always_inline constexpr auto operator[](Trait tr) const {
        return operator[](raze::options::traits<raze::options::settings<Trait>>(tr));
    }
private:
    raze_no_unique_address Traits _traits;
};

template <template <class> class F>
static inline constexpr auto function_with_traits = F<supports_traits<F, decltype(no_traits)>>();

template <class Traits>
consteval sizetype alignment_of() {
    if constexpr (requires { typename Traits::traits_type; }) return get_alignment<typename Traits::traits_type>();
    else return get_alignment<Traits>();
}

template <vx::simd_type V, sizetype N>
constexpr inline auto aligned_for = []() consteval {
    if constexpr (N >= alignof(V) && (N % alignof(V) == 0)) return vx::aligned;
    else return ignore_none;
}();

__RAZE_OPTIONS_NAMESPACE_END

/**
 * @page dynamic_dispatch Dynamic dispatch in `raze::algorithm`
 *
 * Algorithms in `raze::algorithm` 
 * can adapt to the host CPU instruction set at runtime. However, the dispatch 
 * mechanism heavily depends on the target compiler and the available ISA.
 *
 * ### Static Dispatch Override
 *
 * Regardless of the compiler, if the target ISA at compile time (via `-march`, 
 * `/arch:`, or specific `-misa*` flags) already satisfies the reference ISA 
 * (the maximum ISA required by the algorithm's implementation), the algorithm 
 * will **always** use static dispatch. The runtime check is completely elided.
 *
 * ### Mechanism
 *
 * If the reference ISA is not guaranteed at compile time, the fallback behavior 
 * differs by compiler:
 *
 * - **MSVC**: **All** algorithms use a hand-written explicit runtime dispatcher. 
 *   CPU feature detection (e.g., via CPUID) is performed **at program startup**.
 *   The generated dispatcher then forwards execution to the most optimized 
 *   implementation based on the detected features.
 *   There is no automatic compiler-driven dynamic dispatch for loops. To force 
 *   static dispatch (using the compile-time target ISA), you must explicitly 
 *   pass the `raze::options::fstatic` option.
 *
 * - **GCC / Clang / AOCC / nvc++ / icpx **: Dynamic dispatch is **only** applied to algorithms with 
 *   loops that are easily auto-vectorizable by the compiler. For these, 
 *   `__attribute__((target_clones))` is emitted, generating multiple versions 
 *   of the loop. The runtime resolver selects the best version. More complex 
 *   algorithms (e.g., those with complex dependencies or hard-to-vectorize 
 *   predicates) may not receive dynamic dispatch and will rely on the 
 *   compile-time ISA.
 *
 * ### Affected Algorithms
 *
 * - **MSVC**: Every algorithm supports runtime dispatch.
 * - **GCC / Clang / AOCC / nvc++ / icpx **: Primarily simple map-style algorithms and linear traversals 
 *   where the hot loop is entirely inlined and easily cloned:
 *   - `for_each`
 *   - `transform`
 *   - `replace`
 *   - and similar element-wise range operations.
 *
 * ### Performance Characteristics
 *
 * - **Zero API changes**: Call the algorithm as usual.
 * - **Overhead**: The runtime resolution adds a negligible one-time cost 
 *   (or a highly predictable branch per call, depending on the compiler). 
 *   For very short ranges, the dispatch overhead may outweigh the SIMD benefit.
 * - **Code bloat**: Expect the binary size to increase slightly, as multiple 
 *   copies of the inner loop exist for different ISAs.
 *
 * ### Interplay with Options
 *
 * - If `fscalar` is used, the vectorized paths are bypassed entirely, 
 *   degenerating to a scalar implementation regardless of the CPU capabilities.
 * - If `fautovec` is used, `raze::vx::simd` probing is bypassed and the compiler
 *   auto-vectorized (`target_clones`) path is selected when available (falling back
 *   to scalar otherwise).
 * - If `fstatic` is used, dynamic dispatch is disabled, and the algorithm relies 
 *   strictly on the compile-time target ISA.
 * - If `par` is used, dynamic dispatch selects the ISA kernel executed inside each
 *   parallel chunk (unless `fstatic` or `fscalar` is also applied).
 *
 * ### See also
 * - @ref mask_semantics
 * - @ref raze_options
 * - `raze::options::strategy_mode::autovec`
*/