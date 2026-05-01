#pragma once 

#include <src/raze/vx/storage/Node.h>

__RAZE_VX_NAMESPACE_BEGIN

template <class _Type_>
struct __simd_tuple_size;

template <>
struct __simd_tuple_size<_Simd_tuple_nil>: 
    std::integral_constant<std::size_t, 0> 
{};

template <
    class _Head_, 
    class _Tail_>
struct __simd_tuple_size<_Simd_tuple_node<_Head_, _Tail_>>: 
    std::integral_constant<std::size_t, 1 + __simd_tuple_size<_Tail_>::value> 
{};


__RAZE_VX_NAMESPACE_END