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

template <sizetype N>
constexpr inline auto unroll = (unroll_key = index<N>);

struct strategy_key_t : as_keyword<strategy_key_t> {
    template <class Value>
    constexpr auto operator=(const Value& v) const noexcept {
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

struct none_mode {};
constexpr inline auto none = raze::options::flag(none_mode{});

template <class Traits>
constexpr sizetype get_unrolling() {
    return raze::options::fetch_t<(unroll_key | index<1>), Traits>{};
}

struct scalar_mode {};
constexpr inline auto scalar = raze::options::flag(scalar_mode{});

template <class Traits>
constexpr bool always_scalar() {
    return Traits::contains(scalar);
}

struct autovec_mode {};
constexpr inline auto autovec = raze::options::flag(autovec_mode{});

template <class Traits>
constexpr bool is_autovec() {
    return Traits::contains(autovec);
}

constexpr inline auto no_traits = traits();

template <template <class> class F, class Traits>
struct supports_traits {
    using traits_type = Traits;

    raze_always_inline constexpr Traits traits() const noexcept {
        return _traits;
    }

    constexpr supports_traits() {}
    constexpr explicit supports_traits(Traits __traits) noexcept:
        _traits(__traits) 
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
