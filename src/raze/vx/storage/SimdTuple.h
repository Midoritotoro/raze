#pragma once 


#include <src/raze/type_traits/TypeTraits.h>


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
    class   _Type_,
    class   _Abi_,
    int     _Remaining_>
struct best_chunk {
    static constexpr auto width = []() {
        constexpr auto __total_bytes = _Remaining_ * sizeof(_Type_);
        if constexpr (__total_bytes >= 64)
            return 512;
        else if constexpr (__total_bytes >= 32)
            return 256;
        else
            return 128;
    }();
    
    using type = type_traits::__deduce_simd_vector_type<_Type_, width>;
};

template <
    class   _Type_, 
    class   _Abi_,
    int     _Remaining_>
struct __build_tuple {
    using chunk_type = typename best_chunk<_Type_, _Abi_, _Remaining_>::type;

    static constexpr auto __chunk_elems = sizeof(chunk_type) / sizeof(_Type_);
    static constexpr auto __next        = _Remaining_ - __chunk_elems;

    using type = _Simd_tuple_node<chunk_type, typename __build_tuple<_Type_, _Abi_, __next>::type>;
};

template <class _Type_, class _Abi_> 
struct __build_tuple<_Type_, _Abi_, 0> {
    using type = _Simd_tuple_nil; 
};

template <class _Type_, class _Abi_>
struct __build_tuple<_Type_, _Abi_, 1> {
    using type = _Simd_tuple_node<_Type_, _Simd_tuple_nil>;
};

template <class _Type_, class _Abi_>
struct __build_tuple<_Type_, _Abi_, 2> {
    using type = _Simd_tuple_node<_Type_, _Simd_tuple_node<_Type_, _Simd_tuple_nil>>;
};

template <class _Type_, class _Abi_>
struct __build_tuple<_Type_, _Abi_, 3> {
    using type = _Simd_tuple_node<_Type_, _Simd_tuple_node<_Type_, _Simd_tuple_node<_Type_, _Simd_tuple_nil>>>;
};

template <class _Type_, class _Abi_>
struct __build_tuple<_Type_, _Abi_, 4> {
    using type = _Simd_tuple_node<_Type_, _Simd_tuple_node<_Type_, _Simd_tuple_node<_Type_, _Simd_tuple_node<_Type_, _Simd_tuple_nil>>>>;
};

template <class _Type_, class _Abi_>
struct __build_tuple<_Type_, _Abi_, 5> {
    using type = _Simd_tuple_node<_Type_, _Simd_tuple_node<_Type_, _Simd_tuple_node<_Type_, _Simd_tuple_node<_Type_, _Simd_tuple_node<_Type_, _Simd_tuple_nil>>>>>;
};

template <class _Type_, class _Abi_>
struct __build_tuple<_Type_, _Abi_, 6> {
    using type = _Simd_tuple_node<_Type_, _Simd_tuple_node<_Type_, _Simd_tuple_node<_Type_, _Simd_tuple_node<_Type_, _Simd_tuple_node<_Type_, _Simd_tuple_node<_Type_, _Simd_tuple_nil>>>>>>;
};

template <class _Type_, class _Abi_>
struct __build_tuple<_Type_, _Abi_, 7> {
    using type = _Simd_tuple_node<_Type_, _Simd_tuple_node<_Type_, _Simd_tuple_node<_Type_, _Simd_tuple_node<_Type_, _Simd_tuple_node<_Type_, _Simd_tuple_node<_Type_, _Simd_tuple_node<_Type_, _Simd_tuple_nil>>>>>>>;
};

template <class _Type_, class _Abi_>
struct __build_tuple<_Type_, _Abi_, 8> {
    using type = _Simd_tuple_node<_Type_, _Simd_tuple_node<_Type_, _Simd_tuple_node<_Type_, _Simd_tuple_node<_Type_, _Simd_tuple_node<_Type_, _Simd_tuple_node<_Type_, _Simd_tuple_node<_Type_, _Simd_tuple_node<_Type_, _Simd_tuple_nil>>>>>>>>;
};

template <class _Type_, class _Abi_>
struct __build_tuple<_Type_, _Abi_, 9> {
    using type = _Simd_tuple_node<_Type_, _Simd_tuple_node<_Type_, _Simd_tuple_node<_Type_, _Simd_tuple_node<_Type_, _Simd_tuple_node<_Type_, _Simd_tuple_node<_Type_, _Simd_tuple_node<_Type_, _Simd_tuple_node<_Type_, _Simd_tuple_node<_Type_, _Simd_tuple_nil>>>>>>>>>;
};

template <class _Type_, class _Abi_>
struct __build_tuple<_Type_, _Abi_, 10> {
    using type = _Simd_tuple_node<_Type_, _Simd_tuple_node<_Type_, _Simd_tuple_node<_Type_, _Simd_tuple_node<_Type_, _Simd_tuple_node<_Type_, _Simd_tuple_node<_Type_, _Simd_tuple_node<_Type_, _Simd_tuple_node<_Type_, _Simd_tuple_node<_Type_, _Simd_tuple_node<_Type_, _Simd_tuple_nil>>>>>>>>>>;
};

template <class _Type_, class _Abi_>
struct __build_tuple<_Type_, _Abi_, 11> {
    using type = _Simd_tuple_node<_Type_, _Simd_tuple_node<_Type_, _Simd_tuple_node<_Type_, _Simd_tuple_node<_Type_, _Simd_tuple_node<_Type_, _Simd_tuple_node<_Type_, _Simd_tuple_node<_Type_, _Simd_tuple_node<_Type_, _Simd_tuple_node<_Type_, _Simd_tuple_node<_Type_, _Simd_tuple_node<_Type_, _Simd_tuple_nil>>>>>>>>>>>;
};

template <class _Type_, class _Abi_>
struct __build_tuple<_Type_, _Abi_, 12> {
    using type = _Simd_tuple_node<_Type_, _Simd_tuple_node<_Type_, _Simd_tuple_node<_Type_, _Simd_tuple_node<_Type_, _Simd_tuple_node<_Type_, _Simd_tuple_node<_Type_, _Simd_tuple_node<_Type_, _Simd_tuple_node<_Type_, _Simd_tuple_node<_Type_, _Simd_tuple_node<_Type_, _Simd_tuple_node<_Type_, _Simd_tuple_node<_Type_, _Simd_tuple_nil>>>>>>>>>>>>;
};

template <class _Type_, class _Abi_>
struct __build_tuple<_Type_, _Abi_, 13> {
    using type = _Simd_tuple_node<_Type_, _Simd_tuple_node<_Type_, _Simd_tuple_node<_Type_, _Simd_tuple_node<_Type_, _Simd_tuple_node<_Type_, _Simd_tuple_node<_Type_, _Simd_tuple_node<_Type_, _Simd_tuple_node<_Type_, _Simd_tuple_node<_Type_, _Simd_tuple_node<_Type_, _Simd_tuple_node<_Type_, _Simd_tuple_node<_Type_, _Simd_tuple_node<_Type_, _Simd_tuple_nil>>>>>>>>>>>>>;
};

template <class _Type_, class _Abi_>
struct __build_tuple<_Type_, _Abi_, 14> {
    using type = _Simd_tuple_node<_Type_, _Simd_tuple_node<_Type_, _Simd_tuple_node<_Type_, _Simd_tuple_node<_Type_, _Simd_tuple_node<_Type_, _Simd_tuple_node<_Type_, _Simd_tuple_node<_Type_, _Simd_tuple_node<_Type_, _Simd_tuple_node<_Type_, _Simd_tuple_node<_Type_, _Simd_tuple_node<_Type_, _Simd_tuple_node<_Type_, _Simd_tuple_node<_Type_, _Simd_tuple_node<_Type_, _Simd_tuple_nil>>>>>>>>>>>>>>;
};

template <class _Type_, class _Abi_>
struct __build_tuple<_Type_, _Abi_, 15> {
    using type = _Simd_tuple_node<_Type_, _Simd_tuple_node<_Type_, _Simd_tuple_node<_Type_, _Simd_tuple_node<_Type_, _Simd_tuple_node<_Type_, _Simd_tuple_node<_Type_, _Simd_tuple_node<_Type_, _Simd_tuple_node<_Type_, _Simd_tuple_node<_Type_, _Simd_tuple_node<_Type_, _Simd_tuple_node<_Type_, _Simd_tuple_node<_Type_, _Simd_tuple_node<_Type_, _Simd_tuple_node<_Type_, _Simd_tuple_node<_Type_, _Simd_tuple_nil>>>>>>>>>>>>>>>;
};

template <
    class _Type_, 
    class _Abi_>
using _Simd_fallback_tuple_type = typename __build_tuple<_Type_, _Abi_, _Abi_::size>::type;

template <
    sizetype    _N_,
    class       _Tuple_,
    class       _Function_,
    class ...   _Args_>
raze_always_inline void __execute_n_times(
    _Tuple_&        __tuple,
    _Function_&&    __function,
    _Args_&& ...    __args) noexcept 
{
    [&] <sizetype ... __I> (std::integer_sequence<sizetype, __I...>) noexcept {
        ([&](auto __current) noexcept {
            __function(__get<__current>(__tuple), std::forward<_Args_>(__args)...);
        }(std::integral_constant<std::size_t, __I>{}), ...);
    }(std::make_integer_sequence<sizetype, _N_>{});
}


__RAZE_VX_NAMESPACE_END
