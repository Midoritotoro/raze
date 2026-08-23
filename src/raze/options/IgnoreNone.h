#pragma once 

#include <src/raze/options/Concepts.h>
#include <src/raze/options/As.h>


__RAZE_OPTIONS_NAMESPACE_BEGIN

struct ignore_none_ {
    static constexpr bool has_alternative = false;
    
    ignore_none_() noexcept = default;
    ignore_none_(const ignore_none_&) noexcept = default;
    ignore_none_(ignore_none_&&) noexcept = default;
    ~ignore_none_() noexcept = default;

    ignore_none_& operator=(const ignore_none_&) noexcept = default;
    ignore_none_& operator=(ignore_none_&&) noexcept = default;

    raze_always_inline constexpr bool friend operator==(ignore_none_ const&, ignore_none_ const&) noexcept = default;
};

constexpr inline ignore_none_ ignore_none = {};

__RAZE_OPTIONS_NAMESPACE_END
