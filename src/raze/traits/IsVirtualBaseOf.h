#pragma once 

#include <type_traits>

#include <raze/RazeNamespace.h>
#include <raze/compatibility/Warnings.h>


__RAZE_TRAITS_NAMESPACE_BEGIN

struct nonesuch {
	~nonesuch() = delete;
	nonesuch(const nonesuch&) = delete;
	void operator=(const nonesuch&) = delete;
};

template <class	T, class Void, template <class...> class Op, class ... Args>
struct detector_impl {
	using value_t = std::false_type;
	using type = T;
};

template <class T, template <class...> class Op, class ... Args>
struct detector_impl<T, std::void_t<Op<Args...>>, Op, Args...> {
	using value_t = std::true_type;
	using type = Op<Args...>;
};

template <template <class...> class	Op, class ... Args>
using is_detected = typename detector_impl<nonesuch, void, Op, Args...>::value_t;

template <template <class...> class	Op, class ... Args>
constexpr inline bool is_detected_v = is_detected<Op, Args...>::value;

raze_warning_push

raze_disable_warning_gcc("-Wold-style-cast");
raze_disable_warning_clang("-Wold-style-cast");

template <class From, class To>
using is_virtual_base_conversion_test = decltype((To*)std::declval<From*>());

raze_warning_pop

template <class Base, class Derived, class = void>
struct is_virtual_base_of_impl: std::false_type {};

template <class Base, class Derived>
struct is_virtual_base_of_impl<Base, Derived, std::enable_if_t<
		std::conjunction_v<std::is_base_of<Base, Derived>,
			is_detected<is_virtual_base_conversion_test, Derived, Base>,
		std::negation<is_detected<is_virtual_base_conversion_test, Base, Derived>>>>>: 
	std::true_type
{};

template <class Base, class Derived>
using is_virtual_base_of = is_virtual_base_of_impl<std::remove_cv_t<Base>, std::remove_cv_t<Derived>>;

template <class Base, class Derived>
constexpr inline bool is_virtual_base_of_v = is_virtual_base_of<Base, Derived>::value;

__RAZE_TRAITS_NAMESPACE_END
