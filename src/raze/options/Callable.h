#pragma once 

#include <src/raze/options/Merge.h>
#include <src/raze/options/Options.h>
#include <src/raze/options/DecoratedWith.h>
#include <src/raze/options/If.h>
#include <raze/vx/Concepts.h>


__RAZE_OPTIONS_NAMESPACE_BEGIN

struct __decorator 
{};

template <class _ID_>
concept decorator = std::derived_from<_ID_, __decorator>;

template <
    template<class> class   _Functor_,
    class                   _OptionsValues_, 
    class ...               _Options_>
struct callable:
    decorated_with<_OptionsValues_, _Options_...>
{
    using base = decorated_with<_OptionsValues_, _Options_...>;

    template <callable_options __Options_> 
    raze_always_inline constexpr auto operator[](const __Options_& __options) const noexcept {
        auto __merged = raze::options::merge(__options, this->options());
        raze::options::options<decltype(__merged)> __new_options{__merged};

        return _Functor_<decltype(__new_options)>{__new_options};
    }

    template <class _Type_>
    raze_always_inline constexpr auto operator[](_Type_ __t) const noexcept
        requires(requires(const base& __base) { __base[__t];}) 
    {
        auto __new_traits = base::operator[](__t);
        return _Functor_<decltype(__new_traits)>{__new_traits};
    }

    template <
        vx::simd_mask_type  _Condition_, 
        vx::simd_type       _Alternative_>
    raze_always_inline constexpr auto operator[](_Condition_ __condition, _Alternative_ __source) const noexcept
        requires(requires(const base& __base) { __base[or_(if_(__condition), __source)]; })
    {
        auto __new_traits = base::operator[](or_(if_(__condition), __source));
        return _Functor_<decltype(__new_traits)>{__new_traits};
    }

    template <class _Type_>
    raze_always_inline void operator[](const _Type_& __t) const noexcept
        requires(!callable_options<_Type_> && 
            !requires(const base& __base) { __base[__t]; } &&
            !decorator<_Type_>) = delete;

    template <class ... Args>
    raze_always_inline constexpr auto behavior(auto __arch, Args&& ... __args) const noexcept {
        return _Functor_<_OptionsValues_>::deferred_call(__arch, std::forward<Args>(__args)...);
    }

    template <class ... Args>
    raze_always_inline constexpr auto retarget(auto __arch, Args&& ... __args) const noexcept {
        return _Functor_<_OptionsValues_>::deferred_call(__arch, this->options(), std::forward<Args>(__args)...);
    }

protected:
    raze_always_inline constexpr _Functor_<_OptionsValues_> const& derived() const noexcept {
        return static_cast<_Functor_<_OptionsValues_>const&>(*this); 
    }

    template <
        template<class> class   F2, 
        typename                OV2, 
        typename...             O2>
    friend raze_always_inline constexpr bool operator==(const callable&, const callable<F2,OV2,O2...>&) noexcept {
        return false; 
    }

    friend raze_always_inline constexpr bool operator==(const callable&, const callable&) noexcept {
        return true;  
    }
};


__RAZE_OPTIONS_NAMESPACE_END
