#pragma once 

#include <src/raze/vx/storage/Size.h>


__RAZE_VX_NAMESPACE_BEGIN

template <std::size_t _Index_, class _Head_, class _Tail_>
constexpr auto& __get(_Simd_tuple_node<_Head_, _Tail_>& __node) noexcept {
    if constexpr (_Index_ == 0) return __node._head;
    else return __get<_Index_ - 1>(__node._tail);
}

template <std::size_t _Index_, class _Head_, class _Tail_>
constexpr const auto& __get(const _Simd_tuple_node<_Head_, _Tail_>& __node) noexcept {
    if constexpr (_Index_ == 0)  return __node._head;
    else return __get<_Index_ - 1>(__node._tail);
}

template <std::size_t _Index_, class _Type_>
constexpr const auto& __get(const _Type_& __value) noexcept {
    return __value;
}

template <std::size_t _Index_, class _Type_>
constexpr auto& __get(_Type_& __value) noexcept {
    return __value;
}

template <class _Tuple_, class _Function_>
raze_always_inline void __visit_chunk_by_index(_Tuple_& __tuple, i32 __index, _Function_&& __f) noexcept
    requires(__is_simd_tuple<std::remove_cvref_t<_Tuple_>>::value)
{
    [&] <std::size_t... __I> (std::index_sequence<__I...>) raze_always_inline_lambda {
        auto __current = __index;

        ([&] () raze_always_inline_lambda {
            using _Chunk = std::decay_t<decltype(__get<__I>(__tuple))>;

            if (__current < _Chunk::size) {
                __f(__get<__I>(__tuple), __current);
                return true;
            }

            __current -= _Chunk::size;
            return false;
        }() || ... );
    }(std::make_index_sequence<__simd_tuple_size<std::remove_cvref_t<_Tuple_>>::value>{});
}

template <class _Tuple_, class _Function_, class ... _Args_>
raze_always_inline void __for_each_tuple(_Tuple_& __tuple, _Function_&& __f, _Args_&& ... __args) noexcept
    requires(__is_simd_tuple<std::remove_cvref_t<_Tuple_>>::value)
{
    [&] <u64 ... __I> (std::integer_sequence<sizetype, __I...>) raze_always_inline_lambda {
        ([&] (auto __current) raze_always_inline_lambda {
            __f(__get<__current>(__tuple), __get<__current>(std::forward<_Args_>(__args))...);
        }(std::integral_constant<std::size_t, __I>{}), ...);
    }(std::make_integer_sequence<sizetype, __simd_tuple_size<std::remove_cvref_t<_Tuple_>>::value>{});
}

template <class _Tuple_, class _Function_, class ... _Args_>
raze_always_inline bool __for_each_tuple_all_of(_Tuple_& __tuple, _Function_&& __f, _Args_&& ... __args) noexcept
    requires(__is_simd_tuple<std::remove_cvref_t<_Tuple_>>::value)
{
    return [&] <u64 ... __I> (std::integer_sequence<sizetype, __I...>) raze_always_inline_lambda {
        return ([&] (auto __current) raze_always_inline_lambda {
            return __f(__get<__current>(__tuple), __get<__current>(std::forward<_Args_>(__args))...);
        }(std::integral_constant<std::size_t, __I>{}) && ...);
    }(std::make_integer_sequence<sizetype, __simd_tuple_size<std::remove_cvref_t<_Tuple_>>::value>{});
}

template <class _Tuple_, class _Function_, class ... _Args_>
raze_always_inline bool __for_each_tuple_any_of(_Tuple_& __tuple, _Function_&& __f, _Args_&& ... __args) noexcept
    requires(__is_simd_tuple<std::remove_cvref_t<_Tuple_>>::value)
{
    return [&] <u64 ... __I> (std::integer_sequence<sizetype, __I...>) raze_always_inline_lambda {
        return ([&] (auto __current) raze_always_inline_lambda {
            return __f(__get<__current>(__tuple), __get<__current>(std::forward<_Args_>(__args))...);
        }(std::integral_constant<std::size_t, __I>{}) || ...);
    }(std::make_integer_sequence<sizetype, __simd_tuple_size<std::remove_cvref_t<_Tuple_>>::value>{});
}

template <class _Tuple_, class _Function_, class ... _Args_>
raze_always_inline void __for_each_tuple_reverse(_Tuple_& __tuple, _Function_&& __f, _Args_&& ... __args) noexcept
    requires(__is_simd_tuple<std::remove_cvref_t<_Tuple_>>::value)
{
    constexpr sizetype __N = __simd_tuple_size<std::remove_cvref_t<_Tuple_>>::value;
    [&] <u64 ... __I> (std::integer_sequence<sizetype, __I...>) raze_always_inline_lambda {
        ([&] (auto __current) raze_always_inline_lambda {
            __f(__get<__current>(__tuple), __get<__current>(std::forward<_Args_>(__args))...);
        }(std::integral_constant<std::size_t, __N - 1 - __I>{}), ...);
    }(std::make_integer_sequence<sizetype, __N>{});
}

template <class _Tuple_, class _Function_, class ... _Args_>
raze_always_inline bool __for_each_tuple_all_of_reverse(_Tuple_& __tuple, _Function_&& __f, _Args_&& ... __args) noexcept
    requires(__is_simd_tuple<std::remove_cvref_t<_Tuple_>>::value)
{
    constexpr sizetype __N = __simd_tuple_size<std::remove_cvref_t<_Tuple_>>::value;
    return [&] <u64 ... __I> (std::integer_sequence<sizetype, __I...>) raze_always_inline_lambda {
        return ([&] (auto __current) raze_always_inline_lambda {
            return __f(__get<__current>(__tuple), __get<__current>(std::forward<_Args_>(__args))...);
        }(std::integral_constant<std::size_t, __N - 1 - __I>{}) && ...);
    }(std::make_integer_sequence<sizetype, __N>{});
}

template <class _Tuple_, class _Function_, class ... _Args_>
raze_always_inline bool __for_each_tuple_any_of_reverse(_Tuple_& __tuple, _Function_&& __f, _Args_&& ... __args) noexcept
    requires(__is_simd_tuple<std::remove_cvref_t<_Tuple_>>::value)
{
    constexpr sizetype __N = __simd_tuple_size<std::remove_cvref_t<_Tuple_>>::value;
    return [&] <u64 ... __I> (std::integer_sequence<sizetype, __I...>) raze_always_inline_lambda {
        return ([&] (auto __current) raze_always_inline_lambda {
            return __f(__get<__current>(__tuple), __get<__current>(std::forward<_Args_>(__args))...);
        }(std::integral_constant<std::size_t, __N - 1 - __I>{}) || ...);
    }(std::make_integer_sequence<sizetype, __N>{});
}
__RAZE_VX_NAMESPACE_END
