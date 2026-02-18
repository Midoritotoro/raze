#pragma once 

#include <type_traits>

#include <raze/RazeNamespace.h>
#include <raze/compatibility/Warnings.h>


__RAZE_TYPE_TRAITS_NAMESPACE_BEGIN

struct __nonesuch {
	~__nonesuch() = delete;
	__nonesuch(const __nonesuch&) = delete;
	void operator=(const __nonesuch&) = delete;
};

template <
	typename						_Type_,
	typename						_Void_,
	template <typename...> class	_Op_,
	typename...						_Args_>
struct __detector 
{
	using value_t = std::false_type;
	using type = _Type_;
};

template <
	typename						_Type_,
	template <typename...> class	_Op_,
	typename...						_Args_>
struct __detector<
	_Type_,
	std::void_t<_Op_<_Args_...>>,
	_Op_,
	_Args_...> 
{
	using value_t = std::true_type;
	using type = _Op_<_Args_...>;
};


template <
	template <typename...> class	_Op_,
	typename...						_Args_>
using is_detected = typename __detector<
	__nonesuch, void,
	_Op_, _Args_...>::value_t;

template <
	template <typename...> class	_Op_,
	typename...						_Args_>
constexpr inline bool is_detected_v = is_detected<_Op_, _Args_...>::value;


raze_warning_push

raze_disable_warning_gcc("-Wold-style-cast");
raze_disable_warning_clang("-Wold-style-cast");

template <
	typename From,
	typename To>
using __is_virtual_base_conversion_test = decltype((To*)std::declval<From*>());

raze_warning_push

template <
	typename Base,
	typename Derived,
	typename = void>
struct __is_virtual_base_of:
	std::false_type
{};

template <
	typename Base,
	typename Derived>
struct __is_virtual_base_of <
	Base, Derived,
	std::enable_if_t<
		std::conjunction_v<
			std::is_base_of<Base, Derived>,
			is_detected<__is_virtual_base_conversion_test, Derived, Base>,
		std::negation<
			is_detected<__is_virtual_base_conversion_test, Base, Derived>
			>
		>
	>	
>: 
	std::true_type
{};

template <
	typename Base,
	typename Derived>
using is_virtual_base_of = __is_virtual_base_of<
	std::remove_cv_t<Base>,
	std::remove_cv_t<Derived>>;

template <
	typename Base,
	typename Derived>
constexpr inline bool is_virtual_base_of_v = is_virtual_base_of<Base, Derived>::value;

__RAZE_TYPE_TRAITS_NAMESPACE_END
