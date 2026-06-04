#pragma once 

#include <src/raze/vx/hw/x86/shuffle/Reverse.h>
#include <src/raze/vx/hw/x86/shuffle/RotateLeft.h>
#include <src/raze/vx/hw/x86/shuffle/RotateRight.h>
#include <src/raze/vx/hw/x86/shuffle/SlideLeft.h>
#include <src/raze/vx/hw/x86/shuffle/SlideRight.h>
#include <src/raze/vx/hw/x86/shuffle/BroadcastElement.h>

__RAZE_VX_NAMESPACE_BEGIN

template <class _Pattern_>
raze_always_inline typename _Pattern_::vector_type __shuffle(const typename _Pattern_::vector_type& __x, _Pattern_ __p) noexcept {
	using _Simd_ = typename _Pattern_::vector_type;
	using _Abi_ = typename _Simd_::abi_type;
	using _Value_ = typename _Simd_::value_type;
	
	if constexpr (__is_identity(__p)) {
		return __x;
	}
	else if constexpr (__is_reverse(__p)) {
		if constexpr (sizeof(_Simd_::is_native())) {
			return __generic_shuffle_native_size(__x, __p);
		}
		else {
			_Simd_ __result;

			[&] <sizetype ... _Indices_> (std::integer_sequence<sizetype, _Indices_...>) raze_always_inline_lambda {
				([&](auto __i) raze_always_inline_lambda {
					auto& __chunk1 = __x.template __get<__i>();
					auto& __chunk2 = __x.template __get<_Pattern_::size() - __i - 1>();
					
					std::remove_cvref_t<decltype(__chunk1)> __chunk1_temp = __chunk1;

					__chunk1 = __shuffle_native<_Abi_::isa, _Value_>(__storage_unwrap(__chunk2), make_reversed_pattern<typename decltype(__chunk2)::as_simd>{});
					__chunk2 = __shuffle_native<_Abi_::isa, _Value_>(__storage_unwrap(__chunk1_temp), make_reversed_pattern<typename decltype(__chunk1_temp)::as_simd>{});
				}(std::integral_constant<sizetype, _Indices_>{}), ...);
			}(std::make_integer_sequence<sizetype, _Pattern_::size()>{});

			return __result;
		}
	}
	else if constexpr (__is_broadcast(__p)) {
		return __broadcast_element<_Abi_::isa, _Value_>(__x, __p.at<0>());
	}
	else if constexpr (__is_slide_left(__p)) {

	}
	else if constexpr (__is_slide_right(__p)) {
		 
	}
	else if constexpr (__is_rotate_left(__p)) {

	}
	else if constexpr (__is_rotate_right(__p)) {

	}

	return _Simd_{};
}

__RAZE_VX_NAMESPACE_END

