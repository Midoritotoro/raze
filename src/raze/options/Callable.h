#pragma once 

#include <src/raze/options/Merge.h>
#include <src/raze/options/Options.h>
#include <src/raze/options/DecoratedWith.h>
#include <src/raze/options/If.h>
#include <src/raze/vx/Concepts.h>


__RAZE_OPTIONS_NAMESPACE_BEGIN

template <template <class> class _Functor_, class _OptionsValues_, class ... _Options_>
struct callable:
    decorated_with<_OptionsValues_, _Options_...>
{
    using base = decorated_with<_OptionsValues_, _Options_...>;

    template <callable_options __Options_> 
    raze_always_inline constexpr auto operator[](const __Options_& __options) const noexcept {
        auto __merged = merge(__options, this->options());
        options<decltype(__merged)> __new{ __merged };
        return _Functor_<decltype(__new)>{__new};
    }

    template <class _Type_>
    raze_always_inline constexpr auto operator[](const _Type_& __t) const noexcept
        requires(requires(const base& __base) { __base[__t];} || requires(const base & __base) { __base[__t()]; })
    {
        if constexpr (requires(const base & __base) { __base[__t]; }) 
            return _Functor_<decltype(base::operator[](__t))>{ base::operator[](__t)};
        else
            return _Functor_<decltype(base::operator[](__t()))>{ base::operator[](__t())};
    }

    template <class _Type_>
    raze_always_inline void operator[](const _Type_& __t) const noexcept
        requires(!callable_options<_Type_> && !requires(const base& __base) { __base[__t]; }
            && !requires(const base& __base) { __base[__t()]; }) = delete;

    template <class ... Args>
    raze_always_inline constexpr auto behavior(Args&& ... __args) const noexcept {
        return _Functor_<_OptionsValues_>::deferred_call(std::forward<Args>(__args)...);
    }

    template <class ... Args>
    raze_always_inline constexpr auto retarget(Args&& ... __args) const noexcept {
        return _Functor_<_OptionsValues_>::deferred_call(this->options(), std::forward<Args>(__args)...);
    }

protected:
    raze_always_inline constexpr _Functor_<_OptionsValues_> const& derived() const noexcept {
        return static_cast<_Functor_<_OptionsValues_>const&>(*this); 
    }

    template <template <class> class F2, class OV2, class ... O2>
    friend raze_always_inline constexpr bool operator==(const callable&, const callable<F2,OV2,O2...>&) noexcept {
        return false; 
    }

    friend raze_always_inline constexpr bool operator==(const callable&, const callable&) noexcept {
        return true;  
    }
};


__RAZE_OPTIONS_NAMESPACE_END
