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
			// Ignore unrolling
			if (!__f(_Tag_{}, __aligned_size, __tail_size, std::forward<_Args_>(__args)...)) {
				if constexpr (requires { __f.result(); }) return __f.result();
				else return;
			}
#else
			if constexpr (__unrolling > 1) {
				 auto __unrolled_size = __aligned_size - (__aligned_size % (sizeof(_Tag_) * __unrolling));

				 if (__unrolled_size != 0) {
					 if (!__f(vx::simd<typename _Tag_::value_type, vx::resize_abi_t<typename _Tag_::abi_type, _Tag_::size()
						 * __unrolling>>{}, __unrolled_size, __tail_size, std::forward<_Args_>(__args)...)) 
					 {
						 if constexpr (requires { __f.result(); }) return __f.result();
						 else return;
					 }
				 }

				__aligned_size -= __unrolled_size;

				if (__aligned_size >= sizeof(_Tag_)) {
					if (!__f(_Tag_{}, __aligned_size, __tail_size, std::forward<_Args_>(__args)...)) {
						if constexpr (requires { __f.result(); }) return __f.result();
						else return;
					}
				}
			}
			else {
				if (!__f(_Tag_{}, __aligned_size, __tail_size, std::forward<_Args_>(__args)...)) {
					if constexpr (requires { __f.result(); }) return __f.result();
					else return;
				}
			}
#endif // defined(raze_cpp_msvc)

			while (!__f(vx::scalar_tag{}, std::forward<_Args_>(__args)...));
			if constexpr (requires { __f.result(); }) return __f.result();
		}

		template <class _Function_, class ... _Args_>
		constexpr raze_always_inline auto operator()(_Function_&& __f, _Args_&& ... __args) const noexcept 
			requires(std::is_same_v<_Tag_, vx::scalar_tag>)
		{
			while (!__f(vx::scalar_tag{}, std::forward<_Args_>(__args)...));
			if constexpr (requires { __f.result(); }) return __f.result();
		}
	};
};

template <class _Traits_, class _Simd_>
static inline constexpr auto __unroller = typename _Unroller<_Traits_>::template __impl<_Simd_>{};

__RAZE_OPTIONS_NAMESPACE_END
