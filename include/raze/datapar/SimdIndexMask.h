#pragma once 


#include <src/raze/datapar/MaskTypeSelector.h>
#include <src/raze/datapar/bitwise/IndexMaskDivisor.h>
#include <src/raze/datapar/bitwise/ToK.h>
#include <src/raze/datapar/bitwise/ToGPR.h>
#include <raze/math/BitMath.h>


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

	constexpr raze_always_inline int32 __count_trailing_zero_bits() const noexcept {
		if constexpr (__used_bits < 8)
			return math::__ctz_n_bits<__used_bits>(__to_gpr<__isa>(_mask)) / __divisor;

		else if constexpr (__has_avx2_support_v<__isa>)
			return math::__tzcnt_ctz(__to_gpr<__isa>(_mask)) / __divisor;

		else
			return math::__bsf_ctz(__to_gpr<__isa>(_mask)) / __divisor;
	}

	constexpr raze_always_inline int32 __count_leading_zero_bits() const noexcept {
		constexpr auto __unused_bits = raze_sizeof_in_bits(mask_type) - __bit_width();

		if constexpr (__has_avx2_support_v<__isa>)
			return (math::__lzcnt_clz(__to_gpr<__isa>(_mask)) - __unused_bits) / __divisor;
		else
			return (math::__bsr_clz(__to_gpr<__isa>(_mask)) - __unused_bits) / __divisor;
	}

	constexpr raze_always_inline int32 __count_set() const noexcept {
		return math::__popcnt_n_bits<__bit_width()>(__to_gpr<__isa>(_mask)) / __divisor;
	}

	static constexpr raze_always_inline int32 __bit_width() noexcept {
		return __used_bits;
	}

	static constexpr raze_always_inline int32 __elements() noexcept {
		return __used_bits / __divisor;
	}
private:
	mask_type _mask = 0;
};

__RAZE_DATAPAR_NAMESPACE_END
