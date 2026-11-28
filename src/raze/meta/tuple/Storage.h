#pragma once

#include <src/raze/meta/Pack.h>
#include <src/raze/meta/Tags.h>


#if defined(raze_cpp_msvc)
#  define __RAZE_META_EMPTY_BASES __declspec(empty_bases)
#else
#  define __RAZE_META_EMPTY_BASES
#endif

__RAZE_META_NAMESPACE_BEGIN

struct __empty_storage {};

template <bool UseNUA, class ... Ts>
struct __flat_storage;

#define __RAZE_TUP_FIELD_0(T, M) T M;
#define __RAZE_TUP_FIELD_1(T, M) raze_no_unique_address T M;

#define __RAZE_TUP_TP1  class T0
#define __RAZE_TUP_TP2  __RAZE_TUP_TP1,  class T1
#define __RAZE_TUP_TP3  __RAZE_TUP_TP2,  class T2
#define __RAZE_TUP_TP4  __RAZE_TUP_TP3,  class T3
#define __RAZE_TUP_TP5  __RAZE_TUP_TP4,  class T4
#define __RAZE_TUP_TP6  __RAZE_TUP_TP5,  class T5
#define __RAZE_TUP_TP7  __RAZE_TUP_TP6,  class T6
#define __RAZE_TUP_TP8  __RAZE_TUP_TP7,  class T7
#define __RAZE_TUP_TP9  __RAZE_TUP_TP8,  class T8
#define __RAZE_TUP_TP10 __RAZE_TUP_TP9,  class T9
#define __RAZE_TUP_TP11 __RAZE_TUP_TP10, class T10
#define __RAZE_TUP_TP12 __RAZE_TUP_TP11, class T11
#define __RAZE_TUP_TP13 __RAZE_TUP_TP12, class T12
#define __RAZE_TUP_TP14 __RAZE_TUP_TP13, class T13
#define __RAZE_TUP_TP15 __RAZE_TUP_TP14, class T14
#define __RAZE_TUP_TP16 __RAZE_TUP_TP15, class T15

#define __RAZE_TUP_TA1  T0
#define __RAZE_TUP_TA2  __RAZE_TUP_TA1,  T1
#define __RAZE_TUP_TA3  __RAZE_TUP_TA2,  T2
#define __RAZE_TUP_TA4  __RAZE_TUP_TA3,  T3
#define __RAZE_TUP_TA5  __RAZE_TUP_TA4,  T4
#define __RAZE_TUP_TA6  __RAZE_TUP_TA5,  T5
#define __RAZE_TUP_TA7  __RAZE_TUP_TA6,  T6
#define __RAZE_TUP_TA8  __RAZE_TUP_TA7,  T7
#define __RAZE_TUP_TA9  __RAZE_TUP_TA8,  T8
#define __RAZE_TUP_TA10 __RAZE_TUP_TA9,  T9
#define __RAZE_TUP_TA11 __RAZE_TUP_TA10, T10
#define __RAZE_TUP_TA12 __RAZE_TUP_TA11, T11
#define __RAZE_TUP_TA13 __RAZE_TUP_TA12, T12
#define __RAZE_TUP_TA14 __RAZE_TUP_TA13, T13
#define __RAZE_TUP_TA15 __RAZE_TUP_TA14, T14
#define __RAZE_TUP_TA16 __RAZE_TUP_TA15, T15

#define __RAZE_TUP_F1(N)  __RAZE_TUP_FIELD_##N(T0,  _m0)
#define __RAZE_TUP_F2(N)  __RAZE_TUP_F1(N)  __RAZE_TUP_FIELD_##N(T1,  _m1)
#define __RAZE_TUP_F3(N)  __RAZE_TUP_F2(N)  __RAZE_TUP_FIELD_##N(T2,  _m2)
#define __RAZE_TUP_F4(N)  __RAZE_TUP_F3(N)  __RAZE_TUP_FIELD_##N(T3,  _m3)
#define __RAZE_TUP_F5(N)  __RAZE_TUP_F4(N)  __RAZE_TUP_FIELD_##N(T4,  _m4)
#define __RAZE_TUP_F6(N)  __RAZE_TUP_F5(N)  __RAZE_TUP_FIELD_##N(T5,  _m5)
#define __RAZE_TUP_F7(N)  __RAZE_TUP_F6(N)  __RAZE_TUP_FIELD_##N(T6,  _m6)
#define __RAZE_TUP_F8(N)  __RAZE_TUP_F7(N)  __RAZE_TUP_FIELD_##N(T7,  _m7)
#define __RAZE_TUP_F9(N)  __RAZE_TUP_F8(N)  __RAZE_TUP_FIELD_##N(T8,  _m8)
#define __RAZE_TUP_F10(N) __RAZE_TUP_F9(N)  __RAZE_TUP_FIELD_##N(T9,  _m9)
#define __RAZE_TUP_F11(N) __RAZE_TUP_F10(N) __RAZE_TUP_FIELD_##N(T10, _m10)
#define __RAZE_TUP_F12(N) __RAZE_TUP_F11(N) __RAZE_TUP_FIELD_##N(T11, _m11)
#define __RAZE_TUP_F13(N) __RAZE_TUP_F12(N) __RAZE_TUP_FIELD_##N(T12, _m12)
#define __RAZE_TUP_F14(N) __RAZE_TUP_F13(N) __RAZE_TUP_FIELD_##N(T13, _m13)
#define __RAZE_TUP_F15(N) __RAZE_TUP_F14(N) __RAZE_TUP_FIELD_##N(T14, _m14)
#define __RAZE_TUP_F16(N) __RAZE_TUP_F15(N) __RAZE_TUP_FIELD_##N(T15, _m15)

#define __RAZE_TUP_G1  if constexpr (I == 0) return __forward_member<T0, Self>(self._m0);
#define __RAZE_TUP_G2  __RAZE_TUP_G1  else if constexpr (I == 1)  return __forward_member<T1,  Self>(self._m1);
#define __RAZE_TUP_G3  __RAZE_TUP_G2  else if constexpr (I == 2)  return __forward_member<T2,  Self>(self._m2);
#define __RAZE_TUP_G4  __RAZE_TUP_G3  else if constexpr (I == 3)  return __forward_member<T3,  Self>(self._m3);
#define __RAZE_TUP_G5  __RAZE_TUP_G4  else if constexpr (I == 4)  return __forward_member<T4,  Self>(self._m4);
#define __RAZE_TUP_G6  __RAZE_TUP_G5  else if constexpr (I == 5)  return __forward_member<T5,  Self>(self._m5);
#define __RAZE_TUP_G7  __RAZE_TUP_G6  else if constexpr (I == 6)  return __forward_member<T6,  Self>(self._m6);
#define __RAZE_TUP_G8  __RAZE_TUP_G7  else if constexpr (I == 7)  return __forward_member<T7,  Self>(self._m7);
#define __RAZE_TUP_G9  __RAZE_TUP_G8  else if constexpr (I == 8)  return __forward_member<T8,  Self>(self._m8);
#define __RAZE_TUP_G10 __RAZE_TUP_G9  else if constexpr (I == 9)  return __forward_member<T9,  Self>(self._m9);
#define __RAZE_TUP_G11 __RAZE_TUP_G10 else if constexpr (I == 10) return __forward_member<T10, Self>(self._m10);
#define __RAZE_TUP_G12 __RAZE_TUP_G11 else if constexpr (I == 11) return __forward_member<T11, Self>(self._m11);
#define __RAZE_TUP_G13 __RAZE_TUP_G12 else if constexpr (I == 12) return __forward_member<T12, Self>(self._m12);
#define __RAZE_TUP_G14 __RAZE_TUP_G13 else if constexpr (I == 13) return __forward_member<T13, Self>(self._m13);
#define __RAZE_TUP_G15 __RAZE_TUP_G14 else if constexpr (I == 14) return __forward_member<T14, Self>(self._m14);

#define __RAZE_TUP_FLAT_SPEC(USE_NUA, N, K, PREV, LAST)                                           \
template <__RAZE_TUP_TP##K>                                                                       \
struct __flat_storage<USE_NUA, __RAZE_TUP_TA##K> {                                                \
    __RAZE_TUP_F##K(N)                                                                            \
    template <sizetype I, class Self>                                                             \
    raze_always_inline constexpr decltype(auto) get(this Self&& self) noexcept {                  \
        __RAZE_TUP_G##PREV else return __forward_member<T##LAST, Self>(self._m##LAST);            \
    }                                                                                             \
};

#define __RAZE_TUP_DEFINE_FLAT_FAMILY(USE_NUA, N)                                                 \
template <class T0>                                                                               \
struct __flat_storage<USE_NUA, T0> {                                                              \
    __RAZE_TUP_FIELD_##N(T0, _m0)                                                                 \
    template <sizetype I, class Self>                                                             \
    raze_always_inline constexpr decltype(auto) get(this Self&& self) noexcept {                  \
        return __forward_member<T0, Self>(self._m0);                                              \
    }                                                                                             \
};                                                                                                \
__RAZE_TUP_FLAT_SPEC(USE_NUA, N, 2,  1,  1)                                                       \
__RAZE_TUP_FLAT_SPEC(USE_NUA, N, 3,  2,  2)                                                       \
__RAZE_TUP_FLAT_SPEC(USE_NUA, N, 4,  3,  3)                                                       \
__RAZE_TUP_FLAT_SPEC(USE_NUA, N, 5,  4,  4)                                                       \
__RAZE_TUP_FLAT_SPEC(USE_NUA, N, 6,  5,  5)                                                       \
__RAZE_TUP_FLAT_SPEC(USE_NUA, N, 7,  6,  6)                                                       \
__RAZE_TUP_FLAT_SPEC(USE_NUA, N, 8,  7,  7)                                                       \
__RAZE_TUP_FLAT_SPEC(USE_NUA, N, 9,  8,  8)                                                       \
__RAZE_TUP_FLAT_SPEC(USE_NUA, N, 10, 9,  9)                                                       \
__RAZE_TUP_FLAT_SPEC(USE_NUA, N, 11, 10, 10)                                                      \
__RAZE_TUP_FLAT_SPEC(USE_NUA, N, 12, 11, 11)                                                      \
__RAZE_TUP_FLAT_SPEC(USE_NUA, N, 13, 12, 12)                                                      \
__RAZE_TUP_FLAT_SPEC(USE_NUA, N, 14, 13, 13)                                                      \
__RAZE_TUP_FLAT_SPEC(USE_NUA, N, 15, 14, 14)                                                      \
__RAZE_TUP_FLAT_SPEC(USE_NUA, N, 16, 15, 15)

__RAZE_TUP_DEFINE_FLAT_FAMILY(false, 0)
__RAZE_TUP_DEFINE_FLAT_FAMILY(true,  1)

#undef __RAZE_TUP_DEFINE_FLAT_FAMILY
#undef __RAZE_TUP_FLAT_SPEC
#undef __RAZE_TUP_FIELD_0
#undef __RAZE_TUP_FIELD_1
#undef __RAZE_TUP_TP1
#undef __RAZE_TUP_TP2
#undef __RAZE_TUP_TP3
#undef __RAZE_TUP_TP4
#undef __RAZE_TUP_TP5
#undef __RAZE_TUP_TP6
#undef __RAZE_TUP_TP7
#undef __RAZE_TUP_TP8
#undef __RAZE_TUP_TP9
#undef __RAZE_TUP_TP10
#undef __RAZE_TUP_TP11
#undef __RAZE_TUP_TP12
#undef __RAZE_TUP_TP13
#undef __RAZE_TUP_TP14
#undef __RAZE_TUP_TP15
#undef __RAZE_TUP_TP16
#undef __RAZE_TUP_TA1
#undef __RAZE_TUP_TA2
#undef __RAZE_TUP_TA3
#undef __RAZE_TUP_TA4
#undef __RAZE_TUP_TA5
#undef __RAZE_TUP_TA6
#undef __RAZE_TUP_TA7
#undef __RAZE_TUP_TA8
#undef __RAZE_TUP_TA9
#undef __RAZE_TUP_TA10
#undef __RAZE_TUP_TA11
#undef __RAZE_TUP_TA12
#undef __RAZE_TUP_TA13
#undef __RAZE_TUP_TA14
#undef __RAZE_TUP_TA15
#undef __RAZE_TUP_TA16
#undef __RAZE_TUP_F1
#undef __RAZE_TUP_F2
#undef __RAZE_TUP_F3
#undef __RAZE_TUP_F4
#undef __RAZE_TUP_F5
#undef __RAZE_TUP_F6
#undef __RAZE_TUP_F7
#undef __RAZE_TUP_F8
#undef __RAZE_TUP_F9
#undef __RAZE_TUP_F10
#undef __RAZE_TUP_F11
#undef __RAZE_TUP_F12
#undef __RAZE_TUP_F13
#undef __RAZE_TUP_F14
#undef __RAZE_TUP_F15
#undef __RAZE_TUP_F16
#undef __RAZE_TUP_G1
#undef __RAZE_TUP_G2
#undef __RAZE_TUP_G3
#undef __RAZE_TUP_G4
#undef __RAZE_TUP_G5
#undef __RAZE_TUP_G6
#undef __RAZE_TUP_G7
#undef __RAZE_TUP_G8
#undef __RAZE_TUP_G9
#undef __RAZE_TUP_G10
#undef __RAZE_TUP_G11
#undef __RAZE_TUP_G12
#undef __RAZE_TUP_G13
#undef __RAZE_TUP_G14
#undef __RAZE_TUP_G15

template <sizetype I, class T>
struct __leaf {
    raze_no_unique_address T _value;
};

template <sizetype I, class T>
struct __leaf<I, T&> {
    T& _value;

    raze_always_inline constexpr __leaf(T& v) noexcept :
        _value(v)
    {}

    constexpr __leaf(const __leaf&) noexcept = default;
    constexpr __leaf(__leaf&&) noexcept = default;

    constexpr __leaf& operator=(const __leaf& rhs)
        noexcept(std::is_nothrow_copy_assignable_v<T>)
        requires (std::is_copy_assignable_v<T>)
    {
        _value = rhs._value;
        return *this;
    }

    constexpr __leaf& operator=(__leaf&& rhs)
        noexcept(std::is_nothrow_copy_assignable_v<T>)
        requires (std::is_copy_assignable_v<T>)
    {
        _value = rhs._value;
        return *this;
    }
};

template <sizetype I, class T>
struct __leaf<I, T&&> {
    T&& _value;

    raze_always_inline constexpr __leaf(T&& v) noexcept :
        _value(std::forward<T>(v))
    {}

    constexpr __leaf(const __leaf&) noexcept = default;
    constexpr __leaf(__leaf&&) noexcept = default;

    constexpr __leaf& operator=(const __leaf& rhs)
        noexcept(std::is_nothrow_assignable_v<T&, const T&>)
        requires (std::is_assignable_v<T&, const T&>)
    {
        _value = rhs._value;
        return *this;
    }

    constexpr __leaf& operator=(__leaf&& rhs)
        noexcept(std::is_nothrow_assignable_v<T&, T&&>)
        requires (std::is_assignable_v<T&, T&&>)
    {
        _value = std::move(rhs._value);
        return *this;
    }
};

template <class Seq, class ... Ts>
struct __indexed_storage;

template <sizetype ... Is, class ... Ts>
struct __RAZE_META_EMPTY_BASES __indexed_storage<std::index_sequence<Is...>, Ts...> :
    __leaf<Is, Ts>...
{
    static constexpr bool is_leaf_storage = true;

    template <sizetype I, class Self>
    raze_always_inline constexpr decltype(auto) get(this Self&& self) noexcept {
        using elem_t = __nth_type_t<I, Ts...>;
        using leaf_t = std::conditional_t<
            std::is_const_v<std::remove_reference_t<Self>>,
            const __leaf<I, elem_t>,
            __leaf<I, elem_t>>;

        return __forward_member<elem_t, Self>(static_cast<leaf_t&>(self)._value);
    }
};

#undef __RAZE_META_EMPTY_BASES

template <class ... Ts>
consteval auto __select_storage_type() {
    constexpr sizetype n = sizeof...(Ts);

    if constexpr (n == 0)
        return std::type_identity<__empty_storage>{};
    else if constexpr (!__no_refs_v<Ts...>)
        return std::type_identity<__indexed_storage<std::index_sequence_for<Ts...>, Ts...>>{};
    else if constexpr (n <= 16 && !__has_empty_v<Ts...>)
        return std::type_identity<__flat_storage<false, Ts...>>{};
    else if constexpr (n <= 16)
        return std::type_identity<__flat_storage<true, Ts...>>{};
    else
        return std::type_identity<__indexed_storage<std::index_sequence_for<Ts...>, Ts...>>{};
}

template <class ... Ts>
using __storage_t = typename decltype(__select_storage_type<Ts...>())::type;

template <class ... Ts, class ... Args>
raze_always_inline constexpr __storage_t<Ts...> __make_storage(Args&& ... args)
    noexcept((std::is_nothrow_constructible_v<Ts, Args&&> && ...))
{
    using storage_type = __storage_t<Ts...>;

    if constexpr (sizeof...(Ts) == 0) {
        return storage_type{};
    }
    else if constexpr (requires { storage_type::is_leaf_storage; }) {
        return [&] <sizetype ... Is> (std::index_sequence<Is...>) raze_always_inline_lambda {
            return storage_type{ __leaf<Is, Ts>{ std::forward<Args>(args) }... };
        }(std::index_sequence_for<Ts...>{});
    }
    else {
        return storage_type{ std::forward<Args>(args)... };
    }
}

__RAZE_META_NAMESPACE_END
