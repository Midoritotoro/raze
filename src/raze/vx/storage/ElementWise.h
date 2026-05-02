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

template <class _Type_, class _Tuple_, class _Function_>
raze_always_inline void __visit_chunk_by_index(_Tuple_& __tuple, i32 __index, _Function_&& __f) noexcept
    requires(__is_simd_tuple<std::remove_cvref_t<_Tuple_>>::value)
{
    [&] <std::size_t... __I> (std::index_sequence<__I...>) raze_always_inline_lambda {
        auto __current = __index;

        ([&]() raze_always_inline_lambda {
            using _Chunk = std::decay_t<decltype(__get<__I>(__tuple))>;
            constexpr auto __chunk_size = sizeof(_Chunk) / sizeof(_Type_);

            if (__current < __chunk_size) {
                __f(__get<__I>(__tuple), __current);
                return true;
            }

            __current -= __chunk_size;
            return false;
        }() || ... );
    }(std::make_index_sequence<__simd_tuple_size<std::remove_cvref_t<_Tuple_>>::value>{});
}

template <arch::ISA _ISA_, class _Type_, class _Tuple_>
raze_always_inline _Type_ __extract_element(const _Tuple_& __tuple, i32 __index) noexcept 
    requires(__is_simd_tuple<std::remove_cvref_t<_Tuple_>>::value)
{
    _Type_ __result {};

    __visit_chunk_by_index<_Type_>(__tuple, __index, [&](const auto& __chunk, i32 __lane) raze_always_inline_lambda {
        __result = _Extract<_ISA_, _Type_>()(__chunk, __lane);
    });

    return __result;
}

template <arch::ISA _ISA_, class _Type_, class _Tuple_>
raze_always_inline void __insert_element(_Tuple_& __tuple, i32 __index, _Type_ __value) noexcept
    requires(__is_simd_tuple<std::remove_cvref_t<_Tuple_>>::value)
{
    __visit_chunk_by_index<_Type_>(__tuple, __index, [&](auto& __chunk, i32 __lane) raze_always_inline_lambda {
        _Insert<_ISA_>()(__chunk, __lane, __value);
    });
}

template <u64 _N_, class _Tuple_, class _Function_, class ... _Args_>
raze_always_inline void __execute_n_times(_Tuple_& __tuple, _Function_&& __f, _Args_&& ... __args) noexcept
    requires(__is_simd_tuple<std::remove_cvref_t<_Tuple_>>::value)
{
    [&] <u64 ... __I> (std::integer_sequence<sizetype, __I...>) raze_always_inline_lambda {
        ([&](auto __current) raze_always_inline_lambda {
            __f(__get<__current>(__tuple), __get<__current>(std::forward<_Args_>(__args))...);
        }(std::integral_constant<std::size_t, __I>{}), ...);
    }(std::make_integer_sequence<sizetype, _N_>{});
}

__RAZE_VX_NAMESPACE_END
