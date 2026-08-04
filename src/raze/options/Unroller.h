#pragma once

#include <src/raze/options/Traits.h>
#include <src/raze/options/SameAs.h>

__RAZE_OPTIONS_NAMESPACE_BEGIN

template <class _Traits_>
struct _Unroller {
	template <class _Tag_>
	struct __impl {
		constexpr __impl() noexcept = default;
		constexpr ~__impl() noexcept = default;

		template <class _Function_, class ... _Args_>
		constexpr raze_always_inline auto operator()(sizetype __aligned_size, sizetype __tail_size, _Function_ __f, _Args_&& ... __args) const noexcept
			requires(!std::is_same_v<_Tag_, vx::scalar_tag>) 
		{
			constexpr auto __has_early_exit = concepts::same_as<decltype(__f(_Tag_{}, __aligned_size, std::forward<_Args_>(__args)...)), bool>;
			constexpr auto __unrolling = get_unrolling<_Traits_>();
			const auto __guard = vx::make_guard<_Tag_>();

#if defined(raze_cpp_msvc_only)
			// Ignore unrolling
			if constexpr (__has_early_exit) {
				if (!__f(_Tag_{}, __aligned_size, std::forward<_Args_>(__args)...)) {
					if constexpr (requires { __f.result(); }) return __f.result();
					else return;
				}
			}
			else {
				__f(_Tag_{}, __aligned_size, std::forward<_Args_>(__args)...);
			}
#else
			if constexpr (__unrolling > 1) {
				 auto __unrolled_size = __aligned_size - (__aligned_size % (sizeof(_Tag_) * __unrolling));

				 if (__unrolled_size != 0) {
					 if constexpr (__has_early_exit) {
						 if (!__f(vx::simd<typename _Tag_::value_type, vx::resize_abi_t<typename _Tag_::abi_type, _Tag_::size()
							 * __unrolling>>{}, __unrolled_size, std::forward<_Args_>(__args)...))
						 {
							 if constexpr (requires { __f.result(); }) return __f.result();
							 else return;
						 }
					 }
					 else {
						 __f(vx::simd<typename _Tag_::value_type, vx::resize_abi_t<typename _Tag_::abi_type, _Tag_::size()
							 * __unrolling>>{}, __unrolled_size, std::forward<_Args_>(__args)...);
					 }
				 }

				__aligned_size -= __unrolled_size;

				if (__aligned_size >= sizeof(_Tag_)) {
					if constexpr (__has_early_exit) {
						if (!__f(_Tag_{}, __aligned_size, std::forward<_Args_>(__args)...)) {
							if constexpr (requires { __f.result(); }) return __f.result();
							else return;
						}
					}
					else {
						__f(_Tag_{}, __aligned_size, std::forward<_Args_>(__args)...);
					}
				}
			}
			else {
				if constexpr (__has_early_exit) {
					if (!__f(_Tag_{}, __aligned_size, std::forward<_Args_>(__args)...)) {
						if constexpr (requires { __f.result(); }) return __f.result();
						else return;
					}
				}
				else {
					__f(_Tag_{}, __aligned_size, std::forward<_Args_>(__args)...);
				}
			}
#endif // defined(raze_cpp_msvc)

			__f(vx::scalar_tag{}, std::forward<_Args_>(__args)...);
			if constexpr (requires { __f.result(); }) return __f.result();
		}

		template <class _Function_, class ... _Args_>
		constexpr raze_always_inline auto operator()(_Function_ __f, _Args_&& ... __args) const noexcept 
			requires(std::is_same_v<_Tag_, vx::scalar_tag>)
		{
			__f(vx::scalar_tag{}, std::forward<_Args_>(__args)...);
			if constexpr (requires { __f.result(); }) return __f.result();
		}
	};
};

template <class _Traits_, class _Simd_>
static inline constexpr auto __unroller = typename _Unroller<_Traits_>::template __impl<_Simd_>{};

#if !defined(traits_unroller_t)
#  define traits_unroller_t(_Traits_) raze::options::_Unroller<_Traits_>::template __impl
#endif // !defined(traits_unroller_t)

__RAZE_OPTIONS_NAMESPACE_END
