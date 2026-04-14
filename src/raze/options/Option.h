#pragma once 

#include <src/raze/options/Concepts.h>


__RAZE_OPTIONS_NAMESPACE_BEGIN

template <
    concepts::keyword   _Keyword_,
    class               _Value_>
struct option {
    using stored_value_type = std::decay_t<_Value_>;
    using keyword_type = _Keyword_;

    constexpr raze_always_inline stored_value_type operator()(const keyword_type&) const noexcept {
        return _contents;
    }

    stored_value_type _contents;
};

__RAZE_OPTIONS_NAMESPACE_END
