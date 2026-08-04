#pragma once 

#include <type_traits>

#include <raze/RazeNamespace.h>
#include <raze/compatibility/Warnings.h>


__RAZE_TRAITS_NAMESPACE_BEGIN

struct __nonesuch {
	~__nonesuch() = delete;
	__nonesuch(const __nonesuch&) = delete;
	void operator=(const __nonesuch&) = delete;
};

template <class	_Type_, class _Void_, template <class...> class	_Op_, class ... _Args_>
struct __detector {
	using value_t = std::false_type;
	using type = _Type_;
};

template <class _Type_, template <class...> class _Op_, class ... _Args_>
struct __detector<_Type_, std::void_t<_Op_<_Args_...>>, _Op_, _Args_...> {
	using value_t = std::true_type;
	using type = _Op_<_Args_...>;
};

template <template <class...> class	_Op_, class ... _Args_>
using is_detected = typename __detector<__nonesuch, void, _Op_, _Args_...>::value_t;

template <template <class...> class	_Op_, class ... _Args_>
constexpr inline bool is_detected_v = is_detected<_Op_, _Args_...>::value;

raze_warning_push

raze_disable_warning_gcc("-Wold-style-cast");
raze_disable_warning_clang("-Wold-style-cast");

template <class _From_, class _To_>
using __is_virtual_base_conversion_test = decltype((_To_*)std::declval<_From_*>());

raze_warning_pop

template <class _Base_, class _Derived_, class = void>
struct __is_virtual_base_of: std::false_type {};

template <class _Base_, class _Derived_>
struct __is_virtual_base_of<_Base_, _Derived_, std::enable_if_t<
		std::conjunction_v<std::is_base_of<_Base_, _Derived_>,
			is_detected<__is_virtual_base_conversion_test, _Derived_, _Base_>,
		std::negation<is_detected<__is_virtual_base_conversion_test, _Base_, _Derived_>>>>>: 
	std::true_type
{};

template <class _Base_, class _Derived_>
using is_virtual_base_of = __is_virtual_base_of<std::remove_cv_t<_Base_>, std::remove_cv_t<_Derived_>>;

template <class _Base_, class _Derived_>
constexpr inline bool is_virtual_base_of_v = is_virtual_base_of<_Base_, _Derived_>::value;

__RAZE_TRAITS_NAMESPACE_END
