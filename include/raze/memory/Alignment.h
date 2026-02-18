#pragma once 

#include <raze/Types.h>

#include <src/raze/algorithm/MsvcIteratorUnwrap.h>
#include <raze/compatibility/SystemDetection.h>

#include <raze/memory/PointerToIntegral.h>


__RAZE_MEMORY_NAMESPACE_BEGIN

raze_always_inline bool is_alignment(sizetype __value) noexcept {
    return (__value > 0) && ((__value & (__value - 1)) == 0);
}

raze_always_inline bool is_aligned(
    const void* __pointer,
    sizetype    __alignment) noexcept
{
    raze_debug_assert(is_alignment(__alignment));
    return (pointer_to_integral(__pointer) & (__alignment - 1)) == 0;
}

raze_always_inline void* align_down(
    void*       __pointer,
    sizetype    __alignment) noexcept
{
    raze_debug_assert(is_alignment(__alignment));
    return reinterpret_cast<void*>(~(__alignment - 1) & pointer_to_integral(__pointer));

}

raze_always_inline void* align_up(
    void*       __pointer,
    sizetype    __alignment) noexcept
{
    raze_debug_assert(is_alignment(__alignment));
    return reinterpret_cast<void*>(~(__alignment - 1) & (pointer_to_integral(__pointer) + __alignment - 1));
}

template <sizetype _Size_>
struct is_alignment_constant : 
    std::integral_constant<bool, (_Size_ > 0) && ((_Size_& (_Size_ - 1)) == 0)>
{};

__RAZE_MEMORY_NAMESPACE_END
