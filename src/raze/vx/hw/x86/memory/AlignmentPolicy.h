#pragma once 

#include <raze/RazeNamespace.h>


__RAZE_VX_NAMESPACE_BEGIN

struct __aligned_policy {
    static constexpr bool __alignment = true;
};

struct __unaligned_policy {
    static constexpr bool __alignment = false;
};

template <class _AlignmentPolicy_>
constexpr inline auto __is_aligned_v = std::remove_cvref_t<_AlignmentPolicy_>::__alignment;

__RAZE_VX_NAMESPACE_END
