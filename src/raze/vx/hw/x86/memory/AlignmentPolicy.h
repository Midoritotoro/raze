#pragma once 

#include <raze/RazeNamespace.h>


__RAZE_VX_NAMESPACE_BEGIN

struct aligned_policy {
    static constexpr bool alignment = true;
};

struct unaligned_policy {
    static constexpr bool alignment = false;
};

template <class Policy>
constexpr inline auto is_aligned_v = std::remove_cvref_t<Policy>::alignment;

__RAZE_VX_NAMESPACE_END
