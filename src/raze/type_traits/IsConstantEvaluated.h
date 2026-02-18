#pragma once 

#include <raze/RazeNamespace.h>
#include <type_traits>


__RAZE_TYPE_TRAITS_NAMESPACE_BEGIN

#if defined(__cpp_lib_is_constant_evaluated)
	using std::is_constant_evaluated;
#else
	constexpr bool is_constant_evaluated() noexcept {
#if __has_builtin(__builtin_is_constant_evaluated)
		return __builtin_is_constant_evaluated();
#else
		return false;
#endif // __has_builtin(__builtin_is_constant_evaluated)
	}
#endif // defined(__cpp_lib_is_constant_evaluated)

__RAZE_TYPE_TRAITS_NAMESPACE_END
