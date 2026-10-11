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
 * @ingroup options
 * @brief Option decorator that sets the loop unrolling factor to @p N.
 *
 * Instructs the algorithm to duplicate loop bodies @p N times per iteration to reduce 
 * branch overhead and maximize CPU pipeline saturation.
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
 * // Execute with an unrolling factor of 8
 * raze::algorithm::replace[raze::options::unroll<8>](v, 0, 1);
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
 * @ingroup options
 * @brief Option flag that forces scalar execution mode.
 *
 * When attached to an algorithm, `fscalar` completely disables:
 * 1. Explicit SIMD hardware intrinsics (AVX-512, AVX2, SSE).
 * 2. Autovectorization pragmas and unroll directives.
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
 * @ingroup options
 * @brief Option flag enabling static implementations dispatch.
 *
 * Instructs algorithms to prioritize compile-time properties over runtime dynamic checks and dynamic dispatch logic.
 *
 * ```
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
