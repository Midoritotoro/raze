#pragma once 

#include <src/raze/vx/hw/x86/bitwise/Ternarylogic.h>
#include <src/raze/vx/hw/x86/compare/Equal.h>


__RAZE_VX_NAMESPACE_BEGIN

template <
	arch::ISA	_ISA_,
	class		_Tp_>
concept native_ternarylogic = intrin_type<_Tp_> && ((__has_avx512f_support_v<_ISA_> && sizeof(_Tp_) == 64) || 
	(__has_avx512vl_support_v<_ISA_> && (sizeof(_Tp_) == 32 || sizeof(_Tp_) == 16)));

template <
	arch::ISA		_ISA_,
	arithmetic_type	_Type_>
struct _Not {
	template <intrin_or_arithmetic_type _Tp_>
	raze_nodiscard raze_always_inline _Tp_ operator()(_Tp_ __x) const noexcept {
		if constexpr (native_ternarylogic<_ISA_, _Tp_>)
			return _Ternarylogic<_ISA_, _Type_>()(__x, __x, __x, std::integral_constant<uint8, 0x55>{});
		else if constexpr (arithmetic_type<_Tp_>) {
			using _Unsigned = typename IntegerForSizeof<_Tp_>::Unsigned;
			return ~__x;
		}
		else
			return _Xor<_ISA_, _Type_>()(__x, _Equal<_ISA_, int32>()(__x, __x));
	}

	template <
		intrin_or_arithmetic_type	_Tp_,
		raw_mask_type				_Mask_>
	raze_nodiscard raze_always_inline _Tp_ operator()(_Tp_ __x, _Mask_ __mask) const noexcept {
		if constexpr (native_ternarylogic<_ISA_, _Tp_>)
			return _Ternarylogic<_ISA_, _Type_>()(__x, __x, __x, std::integral_constant<uint8, 0x55>{}, __mask);
		else if constexpr (arithmetic_type<_Tp_>) {
			using _Unsigned = typename IntegerForSizeof<_Tp_>::Unsigned;
			return __mask ? ~__x : 0;
		}
		else
			return _Xor<_ISA_, _Type_>()(__x, _Equal<_ISA_, int32>()(__x, __x), __mask);
	}

	template <
		intrin_or_arithmetic_type	_Tp_,
		raw_mask_type				_Mask_>
	raze_nodiscard raze_always_inline _Tp_ operator()(_Tp_ __x, _Mask_ __mask, _Tp_ __src) const noexcept {
		if constexpr (native_ternarylogic<_ISA_, _Tp_>)
			return _Ternarylogic<_ISA_, _Type_>()(__x, __x, __x, std::integral_constant<uint8, 0x55>{}, __mask, __src);
		else if constexpr (arithmetic_type<_Tp_>) {
			using _Unsigned = typename IntegerForSizeof<_Tp_>::Unsigned;
			return __mask ? ~__x : __src;
		}
		else
			return _Xor<_ISA_, _Type_>()(__x, _Equal<_ISA_, int32>()(__x, __x), __mask, __src);
	}
};

__RAZE_VX_NAMESPACE_END
