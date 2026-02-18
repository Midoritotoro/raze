#pragma once 

#include <raze/RazeNamespace.h>
#include <type_traits>

#include <src/raze/type_traits/TypeCheck.h>

__RAZE_TYPE_TRAITS_NAMESPACE_BEGIN

template <class _Type_>
constexpr inline bool is_nonbool_integral_v = std::is_integral_v<_Type_> && !std::is_same_v<std::remove_cv_t<_Type_>, bool>;

template <class _Type_>
struct is_character:
	std::false_type
{};

template <>
struct is_character<char>:
	std::true_type
{};

template <>
struct is_character<signed char>:
	std::true_type
{};

template <>
struct is_character<unsigned char>:
	std::true_type
{};

#if defined(__cpp_char8_t)

template <>
struct is_character<char8_t>:
	std::true_type
{};

#endif // defined(__cpp_char8_t)

template <class _Type_>
struct is_character_or_bool: 
	is_character<_Type_>::type
{};

template <>
struct is_character_or_bool<bool>:
	std::true_type
{};

template <class _Type_>
struct is_character_or_byte_or_bool:
	is_character_or_bool<_Type_>::type
{};

#if defined(__cpp_lib_byte)

template <>
struct is_character_or_byte_or_bool<std::byte>:
	std::true_type
{};

#endif // defined(__cpp_lib_byte)

template <class _Type_>
constexpr inline bool is_character_v = is_character<_Type_>::value;

template <class _Type_>
constexpr inline bool is_character_or_bool_v = is_character_or_bool<_Type_>::value;

template <class _Type_>
constexpr inline bool is_character_or_byte_or_bool_v = is_character_or_byte_or_bool<_Type_>::value;

__RAZE_TYPE_TRAITS_NAMESPACE_END

