#pragma once 

#include <raze/compatibility/FunctionAttributes.h>
#include <raze/compatibility/Inline.h>

#include <raze/Types.h>

__RAZE_ALGORITHM_NAMESPACE_BEGIN


raze_declare_const_function raze_always_inline bool __always_true(
    const void*,
    const void*) noexcept
{
    return true;
}

raze_declare_const_function raze_always_inline bool __memcmp1(
    const void* __first,
    const void* __second) noexcept
{
    return static_cast<const char*>(__first)[0] == static_cast<const char*>(__second)[0];
}

raze_declare_const_function raze_always_inline bool __memcmp2(
    const void* __first,
    const void* __second) noexcept
{
    const auto __first_loaded    = *reinterpret_cast<const uint16*>(__first);
    const auto __second_loaded   = *reinterpret_cast<const uint16*>(__second);

    return __first_loaded == __second_loaded;
}

raze_declare_const_function raze_always_inline bool __memcmp3(
    const void* __first,
    const void* __second) noexcept
{
    const auto __first_loaded   = *reinterpret_cast<const uint32*>(__first);
    const auto __second_loaded  = *reinterpret_cast<const uint32*>(__second);

    return (__first_loaded & 0x00ffffff) == (__second_loaded & 0x00ffffff);
}

raze_declare_const_function raze_always_inline bool __memcmp4(
    const void* __first,
    const void* __second) noexcept
{
    const auto __first_loaded   = *reinterpret_cast<const uint32*>(__first);
    const auto __second_loaded  = *reinterpret_cast<const uint32*>(__second);

    return __first_loaded == __second_loaded;
}

raze_declare_const_function raze_always_inline bool __memcmp5(
    const void* __first,
    const void* __second) noexcept
{
    const auto __first_loaded   = *reinterpret_cast<const uint64*>(__first);
    const auto __second_loaded  = *reinterpret_cast<const uint64*>(__second);

    return ((__first_loaded ^ __second_loaded) & 0x000000fffffffffflu) == 0;
}

raze_declare_const_function raze_always_inline bool __memcmp6(
    const void* __first,
    const void* __second) noexcept
{
    const auto __first_loaded   = *reinterpret_cast<const uint64*>(__first);
    const auto __second_loaded  = *reinterpret_cast<const uint64*>(__second);

    return ((__first_loaded ^ __second_loaded) & 0x0000fffffffffffflu) == 0;
}

raze_declare_const_function raze_always_inline bool __memcmp7(
    const void* __first,
    const void* __second) noexcept
{
    const auto __first_loaded   = *reinterpret_cast<const uint64*>(__first);
    const auto __second_loaded  = *reinterpret_cast<const uint64*>(__second);

    return ((__first_loaded ^ __second_loaded) & 0x00fffffffffffffflu) == 0;
}

raze_declare_const_function raze_always_inline bool __memcmp8(
    const void* __first,
    const void* __second) noexcept
{
    const auto __first_loaded   = *reinterpret_cast<const uint64*>(__first);
    const auto __second_loaded  = *reinterpret_cast<const uint64*>(__second);

    return __first_loaded == __second_loaded;
}

raze_declare_const_function raze_always_inline bool __memcmp9(
    const void* __first,
    const void* __second) noexcept
{
    const auto __first_loaded   = *reinterpret_cast<const uint64*>(__first);
    const auto __second_loaded  = *reinterpret_cast<const uint64*>(__second);

    return (__first_loaded == __second_loaded) & (static_cast<const char*>(__first)[8] == static_cast<const char*>(__second)[8]);
}

raze_declare_const_function raze_always_inline bool __memcmp10(
    const void* __first,
    const void* __second) noexcept
{
    const auto __first_qword    = *reinterpret_cast<const uint64*>(__first);
    const auto __second_qword   = *reinterpret_cast<const uint64*>(__second);

    const auto __first_word     = *reinterpret_cast<const uint16*>(static_cast<const char*>(__first) + 8);
    const auto __second_word    = *reinterpret_cast<const uint16*>(static_cast<const char*>(__second) + 8);

    return (__first_qword == __second_qword) & (__first_word == __second_word);
}

raze_declare_const_function raze_always_inline bool __memcmp11(
    const void* __first,
    const void* __second) noexcept
{
    const auto __first_qword    = *reinterpret_cast<const uint64*>(__first);
    const auto __second_qword   = *reinterpret_cast<const uint64*>(__second);

    const auto __first_dword    = *reinterpret_cast<const uint32*>(static_cast<const char*>(__first) + 8);
    const auto __second_dword   = *reinterpret_cast<const uint32*>(static_cast<const char*>(__second) + 8);

    return (__first_qword == __second_qword) & ((__first_dword & 0x00ffffff) == (__second_dword & 0x00ffffff));
}

raze_declare_const_function raze_always_inline bool __memcmp12(
    const void* __first,
    const void* __second) noexcept
{
    const auto __first_qword    = *reinterpret_cast<const uint64*>(__first);
    const auto __second_qword   = *reinterpret_cast<const uint64*>(__second);

    const auto __first_dword    = *reinterpret_cast<const uint32*>(static_cast<const char*>(__first) + 8);
    const auto __second_dword   = *reinterpret_cast<const uint32*>(static_cast<const char*>(__second) + 8);

    return (__first_qword == __second_qword) & (__first_dword == __second_dword);
}

using __memcmp_function = bool(*)(const void*, const void*);

template <sizetype _NeedleSizeInBytes_>
struct __memcmp_selector {
    using type                  = void*;
    static constexpr type value = nullptr;
};

template <>
struct __memcmp_selector<0> {
    using type                  = __memcmp_function;
    static constexpr type value = &__always_true;
};

template <>
struct __memcmp_selector<1> {
    using type                  = __memcmp_function;
    static constexpr type value = &__memcmp1;
};

template <>
struct __memcmp_selector<2> {
    using type                  = __memcmp_function;
    static constexpr type value = &__memcmp2;
};

template <>
struct __memcmp_selector<3> {
    using type                  = __memcmp_function;
    static constexpr type value = &__memcmp3;
};

template <>
struct __memcmp_selector<4> {
    using type                  = __memcmp_function;
    static constexpr type value = &__memcmp4;
};

template <>
struct __memcmp_selector<5> {
    using type                  = __memcmp_function;
    static constexpr type value = &__memcmp5;
};

template <>
struct __memcmp_selector<6> {
    using type                  = __memcmp_function;
    static constexpr type value = &__memcmp6;
};

template <>
struct __memcmp_selector<7> {
    using type                  = __memcmp_function;
    static constexpr type value = &__memcmp7;
};

template <>
struct __memcmp_selector<8> {
    using type                  = __memcmp_function;
    static constexpr type value = &__memcmp8;
};

template <>
struct __memcmp_selector<9> {
    using type                  = __memcmp_function;
    static constexpr type value = &__memcmp9;
};

template <>
struct __memcmp_selector<10> {
    using type                  = __memcmp_function;
    static constexpr type value = &__memcmp10;
};

template <>
struct __memcmp_selector<11> {
    using type                  = __memcmp_function;
    static constexpr type value = &__memcmp11;
};

template <>
struct __memcmp_selector<12> {
    using type                  = __memcmp_function;
    static constexpr type value = &__memcmp12;
};

template <sizetype _Bytes_>
using __choose_fixed_memcmp_function = __memcmp_selector<_Bytes_>;


template <class _Type_>
raze_always_inline bool __is_all_bits_zero(const _Type_& __value) noexcept {
    static_assert(std::is_scalar_v<_Type_> && !std::is_member_pointer_v<_Type_>);

    if constexpr (std::is_same_v<_Type_, std::nullptr_t>)
        return true;

    constexpr auto __zero = _Type_{};
    
    if constexpr (sizeof(_Type_) <= 12)
        return __choose_fixed_memcmp_function<sizeof(_Type_)>::value(&__value, &__zero);
    else 
        return memcmp(&__value, &__zero, sizeof(_Type_)) == 0;
}

__RAZE_ALGORITHM_NAMESPACE_END

