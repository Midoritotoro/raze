#pragma once 

#include <src/raze/options/Callable.h>
#include <src/raze/algorithm/StrategyBuilder.h>

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
traits(Options&& ... opts) -> traits<decltype(settings(std::forward<Options>(opts) ...))>;

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
 * @brief Option decorator that sets the loop unrolling factor to @p N.
 *
 * Instructs the algorithm to duplicate loop bodies @p N times per iteration.
 *
 * ### Declarations
 * ```cpp
 * template< sizetype N >
 * constexpr inline *unspecified* unroll;
 * ```
 *
 * ### Notes
 * - If not specified, algorithms provide their own default (typically `unroll<4>`).
 * - Setting `unroll<1>` completely disables loop unrolling.
 *
 * ### Example
 * ```cpp
 * // Execute with an unrolling factor of 2
 * raze::algorithm::replace[raze::options::unroll<2>](v, 0, 1);
 * ```
 */
template <sizetype N>
constexpr inline auto unroll = (unroll_key = index<N>);

template <class Traits>
constexpr sizetype get_unrolling() {
    return raze::options::fetch_t<(unroll_key | index<1>), Traits>{};
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
 * SIMD hardware intrinsics (including compiler autovectorization) .
 *
 * Forces the algorithm to execute using only standard scalar instructions and loops.
 *
 * ### Declarations
 * ```cpp
 * constexpr inline *unspecified* fscalar;
 * ```
 *
 * ### Example
 * ```cpp
 * // Disables SIMD; forces scalar iteration
 * raze::algorithm::replace[raze::options::fscalar](vec, 42, 99);
 * ```
 */
constexpr inline auto fscalar = raze::options::flag(fscalar_mode{});

template <class Traits>
constexpr bool is_fscalar() {
    return Traits::contains(fscalar);
}

struct fstatic_mode {};

/**
 * @ingroup raze_options
 * @brief Option flag enabling static implementations dispatch.
 *
 * Instructs algorithms to prioritize compile-time properties over runtime dynamic checks and dynamic dispatch logic.
 *
 * ### Example
 * ```cpp
 * // Static dispatch
 * raze::algorithm::replace[raze::options::fstatic](arr, 0, 1);
 * ```
 */
constexpr inline auto fstatic = raze::options::flag(fstatic_mode{});

template <class Traits>
constexpr bool is_fstatic() {
    return Traits::contains(fstatic);
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
constexpr bool is_strict_types() {
    return Traits::contains(strict_types);
}

constexpr inline auto no_traits = traits();

template <template <class> class F, class Traits>
struct supports_traits {
    using traits_type = Traits;

    raze_always_inline constexpr Traits traits() const noexcept {
        return _traits;
    }

    constexpr supports_traits() {}
    constexpr explicit supports_traits(Traits trs) noexcept:
        _traits(trs)
    {}

    template <class Settings>
    raze_always_inline constexpr auto operator[](raze::options::traits<Settings> tr) const noexcept {
        using SettingsType = decltype(raze::options::merge(tr, _traits));
        auto sum = raze::options::traits<SettingsType>(raze::options::merge(tr, _traits));

        using ReboundType = supports_traits<F, decltype(sum)>;
        return F<ReboundType>(ReboundType(sum));
    }

    template <concepts::option Trait>
    raze_always_inline constexpr auto operator[](Trait tr) const noexcept {
        return operator[](raze::options::traits(tr));
    }
private:
    raze_no_unique_address Traits _traits;
};

template <template <class> class F>
static inline constexpr auto function_with_traits = F<supports_traits<F, decltype(no_traits)>>();

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
 * - **GCC / Clang**: Dynamic dispatch is **only** applied to algorithms with 
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
 * - **GCC / Clang**: Primarily simple map-style algorithms and linear traversals 
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
 * - If `fstatic` is used, dynamic dispatch is disabled, and the algorithm relies 
 *   strictly on the compile-time target ISA.
 *
 * ### See also
 * - @ref mask_semantics
 * - @ref raze_options
 * - `raze::options::strategy_mode::autovec`
*/