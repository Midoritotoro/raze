#pragma once 

#include <src/raze/options/Concepts.h>
#include <src/raze/options/Foldable.h>
#include <src/raze/options/Fetch.h>
#include <src/raze/options/IgnoreNone.h>

__RAZE_OPTIONS_NAMESPACE_BEGIN

template <class OptionsValues, class ... Options>
struct decorated_with: 
    OptionsValues, Options...
{
    using Options::process...;

    constexpr decorated_with() noexcept {}
    constexpr decorated_with(const OptionsValues& values) noexcept:
        OptionsValues(values) 
    {}

    raze_nodiscard constexpr raze_always_inline auto operator[](auto const& opts) const noexcept
        requires(requires(const OptionsValues& values) { 
            this->process(values, opts);
        })
    {
        auto values = static_cast<const OptionsValues&>(*this);
        return process(values, opts);
    }

    raze_nodiscard constexpr raze_always_inline auto options() const noexcept {
        return static_cast<const OptionsValues&>(*this);
    }
};

__RAZE_OPTIONS_NAMESPACE_END