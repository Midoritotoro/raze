#pragma once 

#include <src/raze/options/Concepts.h>
#include <src/raze/options/Foldable.h>


__RAZE_OPTIONS_NAMESPACE_BEGIN

template <
    class       _OptionsValues_, 
    class ...   _Options_>
struct decorated_with: 
    _OptionsValues_, _Options_...
{
    using _Options_::process...;

    constexpr decorated_with() noexcept {}
    constexpr decorated_with(_OptionsValues_ __values) noexcept:
        _OptionsValues_(std::move(__values)) 
    {}

    template <class __Options_>
    raze_nodiscard constexpr raze_always_inline auto operator[](__Options_ __options) const noexcept
        requires(requires(const _OptionsValues_& __values) { 
            this->process(__values, __options);
        })
    {
        return process(static_cast<const _OptionsValues_&>(*this), __options);
    }

    raze_nodiscard constexpr raze_always_inline auto options() const noexcept {
        return fold_left(accumulate_decorations{}, std::tuple<_Options_...>{},
            static_cast<const _OptionsValues_&>(*this));
    }
};

__RAZE_OPTIONS_NAMESPACE_END