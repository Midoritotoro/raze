#pragma once 


#include <src/raze/vx/MaskTypeSelector.h>
#include <src/raze/vx/bitwise/BitmaskBitsPerElement.h>
#include <src/raze/vx/bitwise/ToK.h>
#include <src/raze/vx/bitwise/ToGPR.h>
#include <raze/math/BitMath.h>


__RAZE_VX_NAMESPACE_BEGIN

template <
	class _Type_,
	class _Abi_>
class _Simd_bitmask {
public:
	static constexpr auto __isa = _Abi_::isa;
	static constexpr auto __width = _Abi_::width;

	static constexpr auto __is_k_register = __has_avx512f_support_v<__isa>;

	using element_type = _Type_;
	using abi_type = _Abi_;

	static constexpr uint8 __divisor = __bitmask_bits_per_element_v<__isa, __width, element_type>;
	static constexpr uint8 __used_bits = (__width / 8) / sizeof(element_type) * __divisor;

	using mask_type = __mmask_for_elements_t<__used_bits>;

	_Simd_bitmask() noexcept
	{}

	_Simd_bitmask(mask_type __mask) noexcept {
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

__RAZE_VX_NAMESPACE_END
