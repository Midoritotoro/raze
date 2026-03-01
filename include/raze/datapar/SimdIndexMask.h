#pragma once 


#include <src/raze/datapar/MaskTypeSelector.h>

#include <src/raze/datapar/bitwise/IndexMaskDivisor.h>


__RAZE_DATAPAR_NAMESPACE_BEGIN

template <
	arch::ISA	_ISA_,
	typename	_Type_,
	uint32		_SimdWidth_>
class simd_index_mask {
public:
	static constexpr auto __isa = _ISA_;
	static constexpr auto __width = _SimdWidth_;

	static constexpr auto __is_k_register = __has_avx512f_support_v<__isa>;

	using element_type = _Type_;

	static constexpr uint8 __divisor = __simd_index_mask_divisor_v<__isa, __width, element_type>;
	static constexpr uint8 __used_bits = (__width / 8) / sizeof(element_type) * __divisor;

	using mask_type = __mmask_for_size_t<((__used_bits <= 8) ? 1 : (__used_bits / 8))>;

	simd_index_mask() noexcept
	{}

	simd_index_mask(mask_type __mask) noexcept {
		_mask = __mask;
	}

	static constexpr raze_always_inline uint8 bit_width() noexcept {
		return __used_bits;
	}

	static constexpr raze_always_inline uint8 elements() noexcept {
		return __used_bits / __divisor;
	}

	static constexpr raze_always_inline uint8 divisor() noexcept {
		return __divisor;
	}
private:
	mask_type _mask = 0;
};

__RAZE_DATAPAR_NAMESPACE_END
