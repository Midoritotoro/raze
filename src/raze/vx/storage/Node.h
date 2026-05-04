#pragma once 


#include <src/raze/type_traits/TypeTraits.h>
#include <src/raze/vx/hw/Access.h>


__RAZE_VX_NAMESPACE_BEGIN

struct _Simd_tuple_nil 
{};

template <
    class _Head_, 
    class _Tail_ = _Simd_tuple_nil>
struct _Simd_tuple_node {
    _Head_ _head;
    _Tail_ _tail;

    _Simd_tuple_node() noexcept = default;
    _Simd_tuple_node(const _Simd_tuple_node&) noexcept = default;
    _Simd_tuple_node(_Simd_tuple_node&&) noexcept = default;

    ~_Simd_tuple_node() = default;

    _Simd_tuple_node& operator=(const _Simd_tuple_node&) noexcept = default;
    _Simd_tuple_node& operator=(_Simd_tuple_node&&) noexcept = default;

    template <class _NewHead_, class _NewTail_>
    _Simd_tuple_node(_NewHead_&& __head, _NewTail_&& __tail) noexcept: 
        _head(std::forward<_NewHead_>(__head)), 
        _tail(std::forward<_NewTail_>(__tail))
    {}
};

template <class _T_> 
struct __is_simd_tuple: 
    std::false_type 
{};

template <
    class _H_,
    class _T_> 
struct __is_simd_tuple<_Simd_tuple_node<_H_, _T_>>: 
    std::true_type 
{};

template <> 
struct __is_simd_tuple<_Simd_tuple_nil>:
    std::true_type 
{};

template <class _Type_>
concept wrapped_type = requires(const _Type_& __x_with_data) { __x_with_data.data(); };

__RAZE_VX_NAMESPACE_END
