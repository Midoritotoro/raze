#pragma once

#include <raze/options/Options.h>
#include <src/raze/algorithm/VectorizablePredicate.h>
#include <src/raze/algorithm/AlgorithmDebug.h>
#include <src/raze/vx/dispatch/SizedSimdDispatcher.h>

#if !defined(__raze_define_unchecked)
#  define __raze_define_unchecked(__ret) \
	template <class _Size_, class _Iterator_, class ... _Args_> \
	raze_nodiscard constexpr raze_always_inline __ret __unchecked_dispatch(_Size_ __bytes, _Iterator_ __first, _Args_&& ... __args) const noexcept { \
		using _TraitsType_ = decltype(this->traits()); \
		using _Value_ = std::iter_value_t<_Iterator_>; \
		using _WorkType_ = decltype(__kernel(__first, std::forward<_Args_>(__args)...)); \
		auto __work = __kernel(__first, std::forward<_Args_>(__args)...); \
		if constexpr (!options::always_scalar<_TraitsType_>() && _WorkType_::can_enable_vectorization) {	\
			if not consteval {	\
				return raze::vx::__dispatch_sized_impl<traits_unroller_t(_TraitsType_), _Value_, __ret>(__bytes, __work);	\
			} \
		} \
		return raze::options::__unroller<_TraitsType_, raze::vx::scalar_tag>(__work); \
	}
#endif // !defined(__raze_define_unchecked)

#if !defined(__raze_unchecked_call)
#  define __raze_unchecked_call(...) __unchecked_dispatch(__VA_ARGS__)
#endif // !defined(__raze_unchecked_call)
