#pragma once 

#include <src/raze/options/Concepts.h>
#include <src/raze/options/Settings.h>


__RAZE_OPTIONS_NAMESPACE_BEGIN

template <
    class                   _Key_,
    concepts::keyword ...   _Keywors_>
struct filter {
    using type = keys<_Keywors_...>;

    template <class _Type_> constexpr auto operator+(const keys<_Type_>&) const {
        using kw_t = typename _Type_::keyword_type;

        if constexpr (!concepts::same_as<_Key_, typename kw_t::tag_type>)
            return filter<_Key_, _Keywors_..., kw_t>{};
        else
            return *this;
    }
};

template <
    class _Key_,
    class _Select_>
struct select_keys;

template <
    class                   _Key_,
    concepts::option ...    _Options_>
struct select_keys<_Key_, settings<_Options_...>> :
    decltype((filter<typename _Key_::tag_type>{} + ... + keys<_Options_>{}))
{};

__RAZE_OPTIONS_NAMESPACE_END
