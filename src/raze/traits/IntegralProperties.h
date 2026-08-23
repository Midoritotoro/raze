#pragma once 

#include <raze/RazeNamespace.h>
#include <type_traits>

#include <src/raze/traits/TypeCheck.h>

__RAZE_TRAITS_NAMESPACE_BEGIN

template <class T>
constexpr inline bool is_nonbool_integral_v = std::is_integral_v<T> && !std::is_same_v<std::remove_cv_t<T>, bool>;

template <class T> struct is_character: std::false_type {};
template <> struct is_character<char>: std::true_type {};
template <> struct is_character<signed char>: std::true_type {};
template <> struct is_character<unsigned char>: std::true_type {};

#if defined(__cpp_char8_t)
  template <> struct is_character<char8_t>: std::true_type {};
#endif // defined(__cpp_char8_t)

template <class T> struct is_character_or_bool: is_character<T>::type {};
template <> struct is_character_or_bool<bool>: std::true_type {};
template <class T> struct is_character_or_byte_or_bool: is_character_or_bool<T>::type {};

#if defined(__cpp_lib_byte)
  template <> struct is_character_or_byte_or_bool<std::byte>: std::true_type {};
#endif // defined(__cpp_lib_byte)

template <class T>
constexpr inline bool is_character_v = is_character<T>::value;

template <class T>
constexpr inline bool is_character_or_bool_v = is_character_or_bool<T>::value;

template <class T>
constexpr inline bool is_character_or_byte_or_bool_v = is_character_or_byte_or_bool<T>::value;

__RAZE_TRAITS_NAMESPACE_END

