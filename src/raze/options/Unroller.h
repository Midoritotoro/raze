#pragma once

#include <src/raze/options/Traits.h>

__RAZE_OPTIONS_NAMESPACE_BEGIN

template <class _Traits_>
struct _Unroller {
	template <class _Tag_>
	struct __impl {
		template <class _Function_, class ... _Args_>
		constexpr raze_always_inline auto operator()(sizetype __aligned_size, sizetype __tail_size, _Function_&& __f, _Args_&& ... __args) const noexcept {
			constexpr auto __unrolling = get_unrolling<_Traits_>();
			const auto __guard = vx::make_guard<_Tag_>();

#if defined(raze_cpp_msvc_only)
			if (!__f(_Tag_{}, __aligned_size, __tail_size, std::forward<_Args_>(__args)...)) // Ignore unrolling
				return __f.result();
#else
			if constexpr (__unrolling > 1) {
				 auto __unrolled_size = __aligned_size - (__aligned_size % (sizeof(_Tag_) * __unrolling));

				if (__unrolled_size != 0)
					if (!__f(vx::simd<typename _Tag_::value_type, vx::resize_abi_t<typename _Tag_::abi_type, _Tag_::size()
						* __unrolling>>{}, __unrolled_size, __tail_size, std::forward<_Args_>(__args)...))
						return __f.result();

				__aligned_size -= __unrolled_size;

				if (__aligned_size >= sizeof(_Tag_))
					if (!__f(_Tag_{}, __aligned_size, __tail_size, std::forward<_Args_>(__args)...))
						return __f.result();
			}
			else {
				if (!__f(_Tag_{}, __aligned_size, __tail_size, std::forward<_Args_>(__args)...))
					return __f.result();
			}
#endif // defined(raze_cpp_msvc)

			auto __call = [&] <sizetype ... _Indices_> (std::integer_sequence<sizetype, _Indices_...>) raze_always_inline_lambda -> bool {
				auto __work = [&](auto __i) raze_always_inline_lambda -> bool{ return __f(vx::scalar_tag{}, std::forward<_Args_>(__args)...); };
				return (!__work(_Indices_) && ...);
			};

			while (__call(std::make_integer_sequence<sizetype, __unrolling>{}));
			
			if constexpr (requires { __f.result(); }) return __f.result();
		}

		template <class _Function_, class ... _Args_>
		constexpr raze_always_inline auto operator()(_Function_&& __f, _Args_&& ... __args) const noexcept 
			requires(std::is_same_v<_Tag_, vx::scalar_tag>)
		{
			constexpr auto __unrolling = get_unrolling<_Traits_>();

			auto __call = [&] <sizetype ... _Indices_> (std::integer_sequence<sizetype, _Indices_...>) raze_always_inline_lambda -> bool {
				auto __work = [&](auto __i) raze_always_inline_lambda -> bool{ return __f(_Tag_{}, std::forward<_Args_>(__args)...); };
				return (!__work(_Indices_) && ...);
			};

			while (__call(std::make_integer_sequence<sizetype, __unrolling>{}));
			if constexpr (requires { __f.result(); }) return __f.result();
		}
	};
};

template <class _Traits_, class _Simd_>
static inline constexpr auto __unroller = typename _Unroller<_Traits_>::template __impl<_Simd_>{};

__RAZE_OPTIONS_NAMESPACE_END
