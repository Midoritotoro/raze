#pragma once 

#include <raze/RazeNamespace.h>

#include <type_traits>

#include <optional>
#include <variant>


__RAZE_TYPE_TRAITS_NAMESPACE_BEGIN

template <
	typename,
	typename = void>
struct __is_container: 
	std::false_type
{};

template <typename _Type_>
struct __is_container<_Type_, std::void_t<
	typename _Type_::value_type,
	std::is_convertible<decltype(
		std::declval<_Type_>().begin() != std::declval<_Type_>().end()), bool>>
>: 
	std::true_type 
{};

template <typename _Type_>
constexpr inline bool __is_container_v = __is_container<_Type_>::value;

template <
	typename,
	typename = void>
struct __has_operator_equal: 
	std::false_type
{};

template <typename _Type_>
struct __has_operator_equal<_Type_, std::void_t<decltype(bool(std::declval<const _Type_&>() == std::declval<const _Type_&>()))>>: 
	std::true_type
{};

template <
	typename	_Type_,
	bool	=	__is_container_v<_Type_>>
struct __expand_operator_equal_container;

template <typename _Type_>
struct __expand_operator_equal_tuple;

template <typename _Type_>
using __expand_operator_equal = __expand_operator_equal_container<_Type_>;

template <
	typename _Type_,
	bool>
struct __expand_operator_equal_container: 
	__expand_operator_equal_tuple<_Type_>
{};

template <typename _Type_>
struct __expand_operator_equal_container<_Type_, true>:
	std::conjunction<
		std::disjunction<
			std::is_same<_Type_, typename _Type_::value_type>,
			__expand_operator_equal<typename _Type_::value_type>
	>,	__expand_operator_equal_tuple<_Type_>>
{};

template <typename ... _Type_>
using __expand_operator_equal_recursive = std::conjunction<__expand_operator_equal<_Type_>...>;

template <typename _Type_>
struct __expand_operator_equal_tuple:
	__has_operator_equal<_Type_>
{};

template <typename _Type_>
struct __expand_operator_equal_tuple<std::optional<_Type_>>:
	__expand_operator_equal_recursive<_Type_>
{};

template <
	typename __FirstType_,
	typename _SecondType_>
struct __expand_operator_equal_tuple<std::pair<__FirstType_, _SecondType_>>:
	__expand_operator_equal_recursive<__FirstType_, _SecondType_>
{};

template <typename ... _Type_>
struct __expand_operator_equal_tuple<std::tuple<_Type_...>>:
	__expand_operator_equal_recursive<_Type_...>
{};

template<typename ... _Type_>
struct __expand_operator_equal_tuple<std::variant<_Type_...>>:
	__expand_operator_equal_recursive<_Type_...>
{};

template <
	typename,
	typename = void>
struct __has_operator_less_than:
	std::false_type
{};

template <typename _Type_>
struct __has_operator_less_than<_Type_, std::void_t<decltype(bool(std::declval<const _Type_&>() < std::declval<const _Type_&>()))>>:
	std::true_type
{};

template <
	typename	_Type_,
	bool =		__is_container_v<_Type_>>
struct __expand_operator_less_than_container;

template <typename _Type_>
struct __expand_operator_less_than_tuple;

template <typename _Type_>
using __expand_operator_less_than = __expand_operator_less_than_container<_Type_>;

template <
	typename _Type_,
	bool>
struct __expand_operator_less_than_container: 
	__expand_operator_less_than_tuple<_Type_>
{};

template <typename _Type_>
struct __expand_operator_less_than_container<_Type_, true>:
	std::conjunction<
		std::disjunction<
			std::is_same<_Type_, typename _Type_::value_type>,
			__expand_operator_less_than<typename _Type_::value_type>
	>, __expand_operator_less_than_tuple<_Type_>
> 
{};

template <typename ... _Type_>
using __expand_operator_less_than_recursive = std::conjunction<__expand_operator_less_than<_Type_>...>;

template <typename _Type_>
struct __expand_operator_less_than_tuple:
	__has_operator_less_than<_Type_>
{};

template <typename _Type_>
struct __expand_operator_less_than_tuple<std::optional<_Type_>>:
	__expand_operator_less_than_recursive<_Type_>
{};

template <
	typename __FirstType_,
	typename _SecondType_>
struct __expand_operator_less_than_tuple<std::pair<__FirstType_, _SecondType_>>:
	__expand_operator_less_than_recursive<__FirstType_, _SecondType_>
{};

template <typename ... _Type_>
struct __expand_operator_less_than_tuple<std::tuple<_Type_...>>:
	__expand_operator_less_than_recursive<_Type_...>
{};

template <typename ... _Type_>
struct __expand_operator_less_than_tuple<std::variant<_Type_...>>:
	__expand_operator_less_than_recursive<_Type_...>
{};

template <
	typename	_Type_,
	typename =	void>
struct __is_dereferenceable: 
	std::false_type 
{};

template<typename _Type_>
struct __is_dereferenceable<_Type_, std::void_t<decltype(std::declval<_Type_>().operator->())>>:
	std::true_type
{};

template <typename _Type_>
inline constexpr bool is_dereferenceable_v = __is_dereferenceable<_Type_>::value;

template <typename _Type_>
struct has_operator_equal: 
	__expand_operator_equal<_Type_>
{};

template <typename _Type_>
inline constexpr bool has_operator_equal_v = has_operator_equal<_Type_>::value;

template <
	typename _Container_,
	typename _Type_>
using __has_operator_equal_container = 
	std::disjunction<
		std::is_base_of<
			_Container_, _Type_>,
	has_operator_equal<_Type_>>;

template <typename _Type_>
struct has_operator_less_than: 
	__expand_operator_less_than<_Type_> 
{};

template <typename _Type_>
inline constexpr bool has_operator_less_than_v = has_operator_less_than<_Type_>::value;


template <
	typename _Container_,
	typename _Type_>
using has_operator_less_than_container = 
	std::disjunction<
		std::is_base_of<
			_Container_, _Type_>,
	has_operator_less_than<_Type_>>;

template <typename ... _Type_>
using compare_eq_result = std::enable_if_t<
	std::conjunction_v<
	has_operator_equal<_Type_>...>,
	bool>;

template <
	typename		_Container_,
	typename ...	_Type_>
using compare_eq_result_container = std::enable_if_t<
	std::conjunction_v<
		__has_operator_equal_container<
			_Container_, _Type_>...>, 
	bool>;

template <typename ... _Type_>
using compare_lt_result = std::enable_if_t<
	std::conjunction_v<
	has_operator_less_than<_Type_>...>,
	bool>;

template <
	typename		_Container_,
	typename ...	_Type_>
using compare_lt_result_container = std::enable_if_t<
	std::conjunction_v<
		has_operator_less_than_container<
			_Container_, _Type_>...>,
	bool>;

__RAZE_TYPE_TRAITS_NAMESPACE_END

