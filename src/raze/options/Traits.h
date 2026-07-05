#pragma once 

#include <src/raze/options/Callable.h>

__RAZE_OPTIONS_NAMESPACE_BEGIN

template <class _Settings_>
struct traits : _Settings_ {
    template <concepts::option ... _Options_>
    constexpr explicit traits(_Options_ && ... __options): 
        _Settings_(std::forward<_Options_>(__options)...) 
    {}

    template <class ... _Options_>
    constexpr traits(const settings<_Options_...>& __options) :
        _Settings_(__options)
    {}
};

template <concepts::option ... _Options_>
traits(_Options_&& ... __options) -> traits<decltype(settings(std::forward<_Options_>(__options) ...))>;

struct unroll_key_t: as_keyword<unroll_key_t> {
    template <class _Value_> 
    constexpr auto operator=(const _Value_&) const noexcept {
        return option<unroll_key_t, _Value_>{};
    }
};
  
constexpr inline unroll_key_t unroll_key;

template <sizetype _N_>
constexpr inline auto index = std::integral_constant<sizetype, _N_>{};

template <sizetype _N_>
constexpr inline auto unroll = (unroll_key = index<_N_>);

template <class _Traits_>
constexpr sizetype get_unrolling() {
    return raze::options::fetch_t<(unroll_key | index<1>), _Traits_>{};
}

struct scalar_mode {};
constexpr inline auto scalar = raze::options::flag(scalar_mode{});

template <class _Traits_>
constexpr bool always_scalar() {
    return _Traits_::contains(scalar);
}

constexpr inline auto no_traits = traits();

template <template <class> class _Function_, class _Traits_>
struct __supports_traits {
    using traits_type = _Traits_;

    constexpr _Traits_ traits() const noexcept { 
        return _traits;
    }

    constexpr __supports_traits() {}
    constexpr explicit __supports_traits(_Traits_ __traits) noexcept:
        _traits(__traits) 
    {}

    template <class _Settings_>
    constexpr auto operator[](raze::options::traits<_Settings_> __traits) const noexcept {
        using _SettingsType = decltype(raze::options::merge(__traits, _traits));
        auto __sum = raze::options::traits<_SettingsType>(raze::options::merge(__traits, _traits));

        using _ReboundType = __supports_traits<_Function_, decltype(__sum)>;
        return _Function_<_ReboundType>(_ReboundType(__sum));
    }

    template <concepts::option _Trait_>
    constexpr auto operator[](_Trait_ __trait) const noexcept {
        return operator[](raze::options::traits(__trait));
    }
private:
    _Traits_ _traits;
};

template <template <class> class _Function_>
constexpr auto function_with_traits = _Function_<__supports_traits<_Function_, decltype(no_traits)>>();

__RAZE_OPTIONS_NAMESPACE_END
