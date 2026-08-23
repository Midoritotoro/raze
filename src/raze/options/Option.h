#pragma once 

#include <src/raze/options/Concepts.h>


__RAZE_OPTIONS_NAMESPACE_BEGIN

template <concepts::keyword Keyword, class Value>
struct option {
    using stored_value_type = std::decay_t<Value>;
    using keyword_type = Keyword;

    constexpr raze_always_inline stored_value_type operator()(const keyword_type&) const noexcept {
        return _contents;
    }

    raze_no_unique_address stored_value_type _contents;
};

__RAZE_OPTIONS_NAMESPACE_END
