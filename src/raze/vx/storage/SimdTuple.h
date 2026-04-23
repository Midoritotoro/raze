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

    template <
        class _NewHead_, 
        class _NewTail_>
    _Simd_tuple_node(
        _NewHead_&& __head, 
        _NewTail_&& __tail) noexcept: 
            _head(std::forward<_NewHead_>(__head)), 
            _tail(std::forward<_NewTail_>(__tail))
    {}
};

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

template <
    std::size_t _Index_, 
    class       _Head_,
    class       _Tail_>
constexpr auto& __get(_Simd_tuple_node<_Head_, _Tail_>& __node) noexcept {
    if constexpr (_Index_ == 0)
        return __node._head;
    else
        return __get<_Index_ - 1>(__node._tail);
}

template <
    std::size_t _Index_, 
    class       _Head_,
    class       _Tail_>
constexpr const auto& __get(const _Simd_tuple_node<_Head_, _Tail_>& __node) noexcept {
    if constexpr (_Index_ == 0)
        return __node._head;
    else
        return __get<_Index_ - 1>(__node._tail);
}

template <
    std::size_t _Index_,
    class       _Type_>
constexpr const auto& __get(const _Type_& __value) noexcept {
    return __value;
}


template <
    std::size_t _Index_,
    class       _Type_>
constexpr auto& __get(_Type_& __value) noexcept {
    return __value;
}

template <
    class   _Type_,
    class   _Abi_,
    int     _Remaining_>
struct best_chunk {
    static constexpr auto __total_bytes = _Remaining_ * sizeof(_Type_);
    
    static constexpr int __max_isa_width = 
        __has_avx512f_support_v<_Abi_::isa> ? 512 :
        __has_avx_support_v<_Abi_::isa> ? 256 :
        __has_sse2_support_v<_Abi_::isa> ? 128 : 0;

    static constexpr int __data_width = 
        (__total_bytes >= 64) ? 512 :
        (__total_bytes >= 32) ? 256 :
        (__total_bytes >= 16) ? 128 : 0;

    static constexpr int __width = (__data_width < __max_isa_width) ? __data_width : __max_isa_width;
    
    using type = std::conditional_t<(__width == 0), _Type_, type_traits::__deduce_simd_vector_type<_Type_, __width>>;
};

template <
    class   _Type_, 
    class   _Abi_,
    int     _Remaining_>
struct __build_tuple {
    using chunk_type = typename best_chunk<_Type_, _Abi_, _Remaining_>::type;

    static constexpr auto __chunk_elems = sizeof(chunk_type) / sizeof(_Type_);
    static constexpr auto __next = (_Remaining_ > __chunk_elems) ? (_Remaining_ - __chunk_elems) : 0;
    
    using type = _Simd_tuple_node<chunk_type, typename __build_tuple<_Type_, _Abi_, __next>::type>;
};

template <class _Type_, class _Abi_> 
struct __build_tuple<_Type_, _Abi_, 0> {
    using type = _Simd_tuple_nil; 
};

template <
    class _Type_, 
    class _Abi_>
using _Simd_fallback_tuple_type = typename __build_tuple<_Type_, _Abi_, _Abi_::size>::type;

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

template <
    class _Type_, 
    class _Tuple_, 
    class _Func_>
raze_always_inline void __visit_chunk_by_index(
    _Tuple_&        __tuple, 
    int32           __index, 
    _Func_&&        __function) noexcept 
        requires(__is_simd_tuple<std::remove_cvref_t<_Tuple_>>::value)
{
    [&] <std::size_t... __I> (std::index_sequence<__I...>) raze_always_inline_lambda {
        auto __current = __index;

        ([&]() raze_always_inline_lambda {
            using _Chunk = std::decay_t<decltype(__get<__I>(__tuple))>;
            constexpr auto __chunk_size = sizeof(_Chunk) / sizeof(_Type_);

            if (__current < __chunk_size) {
                __function(__get<__I>(__tuple), __current);
                return true;
            }

            __current -= __chunk_size;
            return false;
        }() || ... );
    }(std::make_index_sequence<__simd_tuple_size<std::remove_cvref_t<_Tuple_>>::value>{});
}

template <
    arch::ISA   _ISA_,
    class       _Type_,
    class       _Tuple_>
raze_always_inline _Type_ __extract_element(
    const _Tuple_&  __tuple, 
    int32           __index) noexcept 
        requires(__is_simd_tuple<std::remove_cvref_t<_Tuple_>>::value)
{
    _Type_ __result {};

    __visit_chunk_by_index<_Type_>(__tuple, __index, [&](const auto& __chunk, int32 __lane) raze_always_inline_lambda {
        __result = _Extract<_ISA_, _Type_>()(__chunk, __lane);
    });

    return __result;
}

template <
    arch::ISA _ISA_,
    class _Type_, 
    class _Tuple_>
raze_always_inline void __insert_element(
    _Tuple_&        __tuple, 
    int32           __index,
    _Type_          __value) noexcept
        requires(__is_simd_tuple<std::remove_cvref_t<_Tuple_>>::value)
{
    __visit_chunk_by_index<_Type_>(__tuple, __index, [&](auto& __chunk, int32 __lane) raze_always_inline_lambda {
        __chunk = _Insert<_ISA_, _Type_>()(__chunk, __lane, __value);
    });
}

template <
    sizetype    _N_,
    class       _Tuple_,
    class       _Function_,
    class ...   _Args_>
raze_always_inline void __execute_n_times(
    _Tuple_&        __x,
    _Function_&&    __function,
    _Args_&& ...    __args) noexcept
        requires(__is_simd_tuple<std::remove_cvref_t<_Tuple_>>::value)
{
    [&] <sizetype ... __I> (std::integer_sequence<sizetype, __I...>) raze_always_inline_lambda {
        ([&](auto __current) raze_always_inline_lambda {
            __function(__get<__current>(__x), __get<__current>(std::forward<_Args_>(__args))...);
        }(std::integral_constant<std::size_t, __I>{}), ...);
    }(std::make_integer_sequence<sizetype, _N_>{});
}

__RAZE_VX_NAMESPACE_END
