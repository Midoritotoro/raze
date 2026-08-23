#pragma once 

#include <src/raze/options/Concepts.h>


__RAZE_OPTIONS_NAMESPACE_BEGIN

struct unknown_key { 
    using type = unknown_key; 
};

template <concepts::option ... Options> 
struct aggregator:
    Options...
{
    constexpr aggregator(const Options& ... opts) noexcept:
        Options(opts)... 
    {}

    using Options::operator()...;

    template <concepts::keyword Keyword> 
    constexpr raze_always_inline auto operator()(const Keyword&) const noexcept {
        return unknown_key{};
    }
};


__RAZE_OPTIONS_NAMESPACE_END
