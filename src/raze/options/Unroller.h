#pragma once

#include <src/raze/options/Traits.h>

__RAZE_OPTIONS_NAMESPACE_BEGIN

template <class _Traits_>
struct _Unroller {
	template <class _Tag_>
	struct __impl {
		template <class _Function_, class ... _Args_>
		constexpr raze_always_inline auto operator()(_Function_&& __f, _Args_&& ... __args) const noexcept {
			constexpr auto __unrolling = get_unrolling<_Traits_>();

			if constexpr (std::is_same_v<_Tag_, vx::scalar_tag>) {
				auto __call = [&] <sizetype ... _Indices_> (std::integer_sequence<sizetype, _Indices_...>) raze_always_inline_lambda -> bool {
					auto __work = [&](auto __i) raze_always_inline_lambda -> bool { return __f(_Tag_{}, std::forward<_Args_>(__args)...); };
					return (!__work(_Indices_) && ...);
				};

				while (__call(std::make_integer_sequence<sizetype, __unrolling>{}));
			}
			else {
				const auto __guard = vx::make_guard<_Tag_>();
#if defined(raze_cpp_msvc)
				__f(_Tag_{}, std::forward<_Args_>(__args)...); // Ignore unrolling
#else
				__f(vx::simd<typename _Tag_::value_type, vx::resize_abi_t<typename _Tag_::abi_type, 
					_Tag_::size() * __unrolling>>{}, std::forward<_Args_>(__args)...);
#endif // defined(raze_cpp_msvc)
			}

			return __f.result();
		}
	};
};

template <class _Traits_, class _Simd_>
constexpr inline auto __unroller = typename _Unroller<_Traits_>::template __impl<_Simd_>{};

__RAZE_OPTIONS_NAMESPACE_END
