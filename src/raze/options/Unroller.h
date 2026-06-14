#pragma once

#include <src/raze/options/Traits.h>
#include <src/raze/vx/Concepts.h>

__RAZE_OPTIONS_NAMESPACE_BEGIN

template <class _Traits_>
struct _Unroller {
	template <class _Simd_>
	struct __impl {
		template <class _Traits_, template <class, class ...> class _Function_, class ... _Args_>
		raze_always_inline constexpr void operator()(sizetype __aligned, sizetype __tail,
			_Function_<_Simd_, _Args_...>&& __f, _Args_ ... __args) const noexcept 
		{
			constexpr auto __unrolling = get_unrolling<_Traits_>();

			auto __call = [&] <sizetype ... _Indices_> (std::integer_sequence<sizetype, _Indices_...>) raze_always_inline_lambda -> bool {
				auto __work = [&](auto __i) raze_always_inline_lambda -> bool{ return __f(_Simd_{}, __aligned, __tail, __args...); };
				return (!__work(_Indices_) && ...);
			};

			while (__call(std::make_integer_sequence<sizetype, __unrolling>{}));
		}

		template <class _Traits_, template <class ...> class _Function_, class ... _Args_>
		raze_always_inline constexpr void operator()(_Function_<_Args_...>&& __f, _Args_ ... __args) const noexcept {
			constexpr auto __unrolling = get_unrolling<_Traits_>();

			auto __call = [&] <sizetype ... _Indices_> (std::integer_sequence<sizetype, _Indices_...>) raze_always_inline_lambda -> bool {
				auto __work = [&] (auto __i) raze_always_inline_lambda -> bool { return __f(__args...); };
				return (!__work(_Indices_) && ...);
			};

			while (__call(std::make_integer_sequence<sizetype, __unrolling>{}));
		}
	};
};

template <class _Traits_, class _Simd_>
constexpr inline auto __unroller = typename _Unroller<_Traits_>::__impl<_Simd_>();

__RAZE_OPTIONS_NAMESPACE_END
