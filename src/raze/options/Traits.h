#pragma once 

#include <src/raze/options/Callable.h>
#include <src/raze/algorithm/StrategyBuilder.h>
#include <src/raze/utility/Assert.h>
#include <src/raze/vx/hw/configurable/memory/AlignedOption.h>
#include <memory>
#include <cstdint>

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

template <class ... Options>
traits(const settings<Options...>& opts) -> traits<settings<Options...>>;

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

template <sizetype N>
concept valid_alignment = (N != 0) && ((N & (N - 1)) == 0);

template <sizetype... Ns>
struct alignment_pack {
    static constexpr sizetype count = sizeof...(Ns);
    static constexpr sizetype values[count] = { Ns... };

    template <sizetype Index>
        requires (count == 1 || Index < count)
    static consteval sizetype at() noexcept {
        if constexpr (count == 1) return values[0];
        else return values[Index];
    }

    static consteval bool is_unaligned() noexcept {
        return ((Ns == 1) && ...);
    }
};

template <class T>
struct is_alignment_pack : std::false_type {};

template <sizetype... Ns>
struct is_alignment_pack<alignment_pack<Ns...>> : std::true_type {};

template <class T>
constexpr inline bool is_alignment_pack_v = is_alignment_pack<std::remove_cvref_t<T>>::value;

struct aligned_key_t : as_keyword<aligned_key_t> {
    using as_keyword<aligned_key_t>::operator=;

    template <class T>
    static raze_always_inline constexpr bool check() noexcept {
        return is_alignment_pack_v<T>;
    }
};

constexpr inline aligned_key_t aligned_key;

/**
 * @ingroup raze_options
 * @brief Alignment sentinel indicating no alignment guarantee beyond the element type's natural alignment.
 *
 * Evaluates to `1` and is intended for use as a readable placeholder in multi-range
 * `raze::options::aligned<Ns...>` specifications when only a subset of the algorithm's
 * ranges are over-aligned.
 *
 * ### Declarations
 * ```cpp
 * constexpr inline sizetype unaligned = 1;
 * ```
 *
 * ### Notes
 * - Equivalent to passing `1` directly in `raze::options::aligned<...>`.
 * - Instructs the algorithm to keep unaligned SIMD memory accesses and omit
 *   `std::assume_aligned` for the corresponding range.
 *
 * ### Example
 * ```cpp
 * alignas(64) std::array<float, 256> a{};
 * std::vector<float> b(256); // standard allocation, alignment not guaranteed
 * alignas(32) std::array<float, 256> out{};
 *
 * raze::algorithm::transform[
 *     raze::options::aligned<64, raze::options::unaligned, 32>
 * ](a, b, out.begin(), [](auto x, auto y) { return x + y; });
 * ```
*/
constexpr inline sizetype unaligned = 1;

/**
 * @ingroup raze_options
 * @brief Option decorator declaring minimum byte alignment(s) for contiguous range pointers.
 *
 * Allows SIMD kernels to emit aligned loads/stores and enables compiler alignment assumptions
 * (`std::assume_aligned`) whenever a range's guaranteed alignment satisfies the active ISA
 * register alignment (`N >= alignof(V)`).
 *
 * Supports two forms:
 * - **Broadcast form (`aligned<N>`)**: declares that *every* contiguous range/iterator
 *   passed to the algorithm is aligned to at least @p N bytes.
 * - **Positional form (`aligned<N1, N2, ...>`)**: declares an independent alignment for each
 *   range/iterator argument in exact parameter order (e.g., `<In, Out>` for binary copy/transform
 *   algorithms, `<In1, In2>` for two-input comparisons, `<In1, In2, Out>` for binary `transform`).
 *   Use `1` or `raze::options::unaligned` for arguments without extra alignment guarantees.
 *
 * ### Declarations
 * ```cpp
 * template< sizetype... Ns >
 *   requires (sizeof...(Ns) > 0 && ((Ns != 0 && (Ns & (Ns - 1)) == 0) && ...))
 * constexpr inline *unspecified* aligned;
 * ```
 *
 * ### Notes
 * - Every value in @p Ns must be a non-zero power of two (e.g., `1`, `16`, `32`, `64`).
 * - **Compile-time arity check**: `sizeof...(Ns)` must be either `1` or equal to the number of
 *   range/iterator arguments accepted by the algorithm; otherwise compilation fails with a
 *   `static_assert` (regardless of `fscalar`, `fautovec`, or iterator category).
 * - **Safe with dynamic dispatch & mixed types**: each buffer's alignment is checked independently
 *   against its own SIMD register type. If runtime CPU dispatch selects a wider ISA than @p N
 *   (e.g., AVX-512 requiring 64-byte alignment when `aligned<32>` was specified), or if a narrowing
 *   conversion breaks per-iteration alignment, that buffer automatically falls back to unaligned
 *   accesses instead of faulting.
 * - **Autovectorization**: in `autovec` paths, applies `std::assume_aligned<N>` per buffer
 *   whenever `N > alignof(T)`, helping the compiler eliminate alignment-peeling prologues.
 * - **Non-alignable access patterns**: operations whose memory accesses cannot preserve
 *   start-of-range alignment (such as variable-stride output writes in `remove_copy_if`/`copy_if`,
 *   offset loads in `search`/`adjacent_find`/`is_sorted`, or reverse scans starting from `end()`
 *   in `copy_backward`/`find_last`) safely ignore alignment for those specific accesses.
 * - **Overriding**: chaining multiple `aligned<...>` decorators replaces the previous one
 *   (`alg[aligned<32>][aligned<64, unaligned>]` resolves to `<64, 1>`).
 * - **Precondition**: for every contiguous range with `N > 1`, its initial data pointer must
 *   actually be aligned to at least `N` bytes; otherwise, the behavior is undefined.
 *
 * ### Example
 * ```cpp
 * // 1) Broadcast form: all contiguous buffers are aligned to at least 32 bytes
 * alignas(32) std::array<int, 256> src{};
 * alignas(32) std::array<int, 256> dst{};
 * raze::algorithm::copy[raze::options::aligned<32>](src, dst.begin());
 *
 * // 2) Binary positional form: aligned input, unaligned output
 * std::vector<int> unaligned_dst(256);
 * raze::algorithm::replace_copy_if[
 *     raze::options::aligned<32, raze::options::unaligned>
 * ](src, unaligned_dst.begin(), [](auto x) { return x < 0; }, 0);
 *
 * // 3) Ternary positional form: independent alignments for (in1, in2, out)
 * alignas(64) std::array<float, 256> lhs{};
 * std::vector<float> rhs(256);
 * alignas(32) std::array<float, 256> out{};
 * raze::algorithm::transform[
 *     raze::options::aligned<64, raze::options::unaligned, 32>
 * ](lhs, rhs, out.begin(), [](auto x, auto y) { return x * y; });
 * ```
*/
template <sizetype... Ns>
    requires (sizeof...(Ns) > 0 && (valid_alignment<Ns> && ...))
constexpr inline auto aligned = (aligned_key = alignment_pack<Ns...>{});

template <class Traits>
constexpr sizetype get_alignment() noexcept {
    return raze::options::fetch_t<(aligned_key | alignment_pack<1>{}), Traits>::template at<0>();
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
 * @brief Option flag that bypasses `raze::vx::simd` probing and selects compiler auto-vectorization (`autovec`) when available.
 *
 * Primarily intended as a drop-in migration aid when moving existing code from `std::` / `std::ranges`
 * algorithms: attaching `fautovec` lets existing generic (`auto`) lambdas with scalar-only logic
 * (such as `if` branches or scalar-only operations) compile without rewriting them for `raze::vx::simd`,
 * while still gaining compiler auto-vectorization and runtime CPUID dynamic dispatch on supported compilers.
 *
 * ### Declarations
 * ```cpp
 * constexpr inline *unspecified* fautovec;
 * ```
 *
 * ### Notes
 * - For callables where manual `raze::vx` vectorization is already inapplicable (e.g., explicit scalar
 *   parameter types like `[](int& x)` on contiguous ranges), supported compilers (GCC, Clang, ICPX, AOCC)
 *   may automatically select the `autovec` path even without `fautovec`.
 * - On MSVC — and for algorithms without an `autovec` kernel or non-contiguous ranges — `fautovec`
 *   effectively degrades to scalar execution (equivalent to `fscalar`, without `raze::vx::simd` probing).
 * - When combined with `par` (`[par][fautovec]`), runs the `autovec` (or scalar fallback) path inside
 *   each parallel chunk.
 * - In constant-evaluated contexts (`if consteval`), falls back to the scalar path.
 * - Should not be combined with `fscalar`.
 *
 * ### Example
 * ```cpp
 * // Drop-in migration of a generic `std::ranges` lambda with scalar branching:
 * // avoids `vx::simd` probing errors while still getting autovec + dynamic dispatch on GCC/Clang/ICPX
 * raze::algorithm::for_each[raze::options::fautovec](v, [](auto& x) {
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
struct unwrap_traits {
    using type = Traits;
};

template <class Traits>
    requires requires { typename Traits::traits_type; }
struct unwrap_traits<Traits> {
    using type = typename Traits::traits_type;
};

template <class Traits>
using unwrap_traits_t = typename unwrap_traits<std::remove_cvref_t<Traits>>::type;

template <class Traits>
using alignment_pack_of = fetch_t < (aligned_key | alignment_pack<1>{}), unwrap_traits_t<Traits>>;

template <class Traits, sizetype Index = 0, sizetype Arity = 1>
consteval sizetype alignment_of() noexcept {
    using pack = alignment_pack_of<Traits>;
    static_assert(pack::count == 1 || pack::count == Arity, "raze::options::aligned<Ns...>: argument count must be 1 or match the number of ranges.");
    static_assert(Index < Arity, "Alignment index out of range.");
    return pack::template at<Index>();
}

template <vx::simd_type V, sizetype N>
constexpr inline auto aligned_for = []() consteval {
    if constexpr (N >= alignof(V) && (sizeof(V) % alignof(V) == 0)) return vx::aligned;
    else return options<>{};
}();

template <class Traits, sizetype Arity>
struct alignments {
    using pack_type = alignment_pack_of<Traits>;

    static constexpr sizetype arity = Arity;

    static_assert(pack_type::count == 1 || pack_type::count == Arity,
        "raze::options::aligned<Ns...>: argument count must be 1 or match the number of ranges.");

    template <sizetype Index> requires (Index < Arity)
    static constexpr sizetype get = pack_type::template at<Index>();

    template <sizetype Index, vx::simd_type V> requires (Index < Arity)
    static constexpr auto policy_for = aligned_for<V, get<Index>>;

    template <sizetype Index, class T>
    raze_nodiscard static raze_always_inline constexpr T* assume(T* ptr) noexcept
        requires (Index < Arity)
    {
        if constexpr (get<Index> > alignof(T)) return std::assume_aligned<get<Index>>(ptr);
        else return ptr;
    }

    template <sizetype Index, class Arg>
    static raze_always_inline constexpr void verify(const Arg& arg) noexcept
        requires (Index < Arity)
    {
        if !consteval {
            if constexpr (get<Index> > 1) {
                if (!is_empty_arg(arg))
                    verify_one<Index>(arg);
            }
        }
    }

    template <class... Args>
    static raze_always_inline constexpr void verify(const Args&... args) noexcept
        requires (sizeof...(Args) == Arity)
    {
        if !consteval {
            if constexpr (!pack_type::is_unaligned()) {
                if ((is_empty_arg(args) || ...))
                    return;

                [&]<sizetype... Is>(std::index_sequence<Is...>) {
                    (verify_one<Is>(args), ...);
                }(std::make_index_sequence<Arity>{});
            }
        }
    }

private:
    template <class Arg>
    static raze_always_inline constexpr bool is_empty_arg(const Arg& arg) noexcept {
        if constexpr (requires { { arg.empty() } -> std::convertible_to<bool>; })
            return arg.empty();
        else
            return false;
    }

    template <sizetype Index, class T>
    static raze_always_inline void verify_ptr(const T* ptr) noexcept {
        if constexpr (get<Index> > 1)
            raze_debug_assert(reinterpret_cast<std::uintptr_t>(ptr) % get<Index> == 0);
    }

    template <sizetype Index, class Arg>
    static raze_always_inline void verify_one(const Arg& arg) noexcept {
        if constexpr (get<Index> > 1) {
            if constexpr (std::is_pointer_v<Arg>) {
                verify_ptr<Index>(arg);
            }
            else if constexpr (std::contiguous_iterator<Arg>) {
                verify_ptr<Index>(std::to_address(arg));
            }
            else if constexpr (requires { std::remove_cvref_t<Arg>::to_raw_range(arg.ubegin(), arg.uend()); }) {
                verify_ptr<Index>(std::remove_cvref_t<Arg>::to_raw_range(arg.ubegin(), arg.uend()).first);
            }
            else if constexpr (requires { arg.ubegin(); }) {
                if constexpr (std::contiguous_iterator<std::remove_cvref_t<decltype(arg.ubegin())>>)
                    verify_ptr<Index>(std::to_address(arg.ubegin()));
            }
        }
    }
};

template <class Settings>
raze_nodiscard constexpr raze_always_inline auto drop_alignment(const traits<Settings>& tr) noexcept {
    if constexpr (traits<Settings>::contains(aligned_key)) return traits(drop(aligned_key, tr));
    else return tr;
}

template <sizetype... Ns, class Settings>
raze_nodiscard constexpr raze_always_inline auto rebind_alignment(const traits<Settings>& tr) noexcept 
    requires (sizeof...(Ns) > 0 && (valid_alignment<Ns> && ...))
{
    if constexpr (!traits<Settings>::contains(aligned_key)) return tr;  
    else if constexpr (((Ns == 1) && ...)) return traits(drop(aligned_key, tr));
    else return traits(merge(settings(aligned<Ns...>), tr));
}

template <sizetype Arity, sizetype... Indices, class Settings>
raze_nodiscard constexpr raze_always_inline auto select_alignment(const traits<Settings>& tr) noexcept
    requires (sizeof...(Indices) > 0 && ((Indices < Arity) && ...))
{
    return rebind_alignment<alignment_of<traits<Settings>, Indices, Arity>()...>(tr);
}

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