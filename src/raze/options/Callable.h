#pragma once 

#include <src/raze/options/Merge.h>
#include <src/raze/options/Options.h>
#include <src/raze/options/DecoratedWith.h>
#include <src/raze/options/If.h>
#include <src/raze/vx/Concepts.h>


__RAZE_OPTIONS_NAMESPACE_BEGIN

template <template <class> class F, class OptionsValues, class ... Options>
struct callable: decorated_with<OptionsValues, Options...> {
    using base_t = decorated_with<OptionsValues, Options...>;

    raze_always_inline constexpr auto operator[](callable_options auto const& opts) const noexcept {
        auto merged = merge(opts, this->options());
        options<decltype(merged)> new_obj{ merged };
        return F<decltype(new_obj)>{new_obj};
    }

    template <class T>
    raze_always_inline constexpr auto operator[](const T& t) const noexcept
        requires(requires(const base_t& base) { base[t];} || requires(const base_t& base) { base[t()]; })
    {
        if constexpr (requires(const base_t& base) { base[t]; }) 
            return F<decltype(base_t::operator[](t))>{ base_t::operator[](t)};
        else
            return F<decltype(base_t::operator[](t()))>{ base_t::operator[](t())};
    }

    template <class T>
    raze_always_inline void operator[](const T& t) const noexcept
        requires(!callable_options<T> && !requires(const base_t& base) { base[t]; }
            && !requires(const base_t& base) { base[t()]; }) = delete;

    template <class ... Args>
    raze_always_inline constexpr auto behavior(Args&& ... args) const noexcept {
        return F<OptionsValues>::deferred_call(std::forward<Args>(args)...);
    }

    template <class ... Args>
    raze_always_inline constexpr auto retarget(Args&& ... args) const noexcept {
        return F<OptionsValues>::deferred_call(this->options(), std::forward<Args>(args)...);
    }

protected:
    raze_always_inline constexpr F<OptionsValues> const& derived() const noexcept {
        return static_cast<F<OptionsValues>const&>(*this); 
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
