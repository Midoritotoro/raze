#pragma once 

#include <src/raze/vx/storage/Node.h>

__RAZE_VX_NAMESPACE_BEGIN

template <class>
struct simd_tuple_size_impl;

template <>
struct simd_tuple_size_impl<_Simd_tuple_nil>:
    std::integral_constant<sizetype, 0>
{};

template <class H, class T>
struct simd_tuple_size_impl<_Simd_tuple_node<H, T>>:
    std::integral_constant<sizetype, 1 + simd_tuple_size_impl<T>::value>
{};

template <class T>
struct simd_tuple_size:
    simd_tuple_size_impl<std::remove_cvref_t<T>>
{};

__RAZE_VX_NAMESPACE_END