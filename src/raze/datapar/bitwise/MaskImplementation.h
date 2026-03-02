#pragma once 

#include <src/raze/type_traits/TypeTraits.h>
#include <src/raze/datapar/bitwise/Testz.h>
#include <src/raze/datapar/bitwise/TestAllOnes.h>
#include <src/raze/datapar/MaskTypeSelector.h>
#include <src/raze/datapar/bitwise/ToMask.h>
#include <raze/math/BitMath.h>


__RAZE_DATAPAR_NAMESPACE_BEGIN

template <class _Simd_>
constexpr inline auto __is_native_compare_returns_number_v = std::is_integral_v<
	type_traits::invoke_result_type<_Simd_equal<_Simd_::__isa, _Simd_::__width, typename _Simd_::value_type>,
	typename _Simd_::vector_type, typename _Simd_::vector_type>>;

template <
	arch::ISA	_ISA_,
	typename	_Type_,
	uint32		_SimdWidth_>
class _Mask_implementation {
public:

	static constexpr auto __number_mask = __is_native_compare_returns_number_v<simd<_ISA_, _Type_, _SimdWidth_>>;
public:
	static constexpr auto __isa = _ISA_;
	static constexpr auto __width = _SimdWidth_;
	static constexpr auto __is_k_register = __has_avx512f_support_v<__isa>;
	static constexpr auto __used_bits = __number_mask ? (_SimdWidth_ / 8) / sizeof(_Type_) : _SimdWidth_;

	using element_type = _Type_;
	using mask_type = std::conditional_t<__number_mask,
		__mmask_for_size_t<((__used_bits <= 8) ? 1 : (__used_bits / 8))>,
		simd<_ISA_, typename IntegerForSizeof<_Type_>::Unsigned, __used_bits>>;

	raze_nodiscard raze_always_inline static constexpr int __bit_width() noexcept {
		return __used_bits;
	}

	raze_nodiscard raze_always_inline static constexpr bool __all_of(mask_type __mask) noexcept {
		if constexpr (std::is_integral_v<mask_type>) {
			constexpr auto __max_for_bits = ((sizeof(mask_type) * 8) == __bit_width())
				? math::__maximum_integral_limit<mask_type>()
				: mask_type(((mask_type(1) << __bit_width()) - 1));

			if constexpr (sizeof(mask_type) == 1 && __has_avx512dq_support_v<__isa>)
				return _ktestc_mask8_u8(__mask, __max_for_bits);

			else if constexpr (sizeof(mask_type) == 2 && __has_avx512f_support_v<__isa>)
				return _ktestc_mask16_u8(__mask, __max_for_bits);

			else if constexpr (sizeof(mask_type) == 4 && __has_avx512bw_support_v<__isa>)
				return _ktestc_mask32_u8(__mask, __max_for_bits);

			else if constexpr (sizeof(mask_type) == 8 && __has_avx512bw_support_v<__isa>)
				return _ktestc_mask64_u8(__mask, __max_for_bits);

			else
				return (__mask == __max_for_bits);
		}
		else {
			return _Simd_test_all_ones<__isa, __bit_width()>()(__simd_unwrap(__mask));
		}
	}

	raze_nodiscard raze_always_inline static constexpr bool __none_of(mask_type __mask) noexcept {
		if constexpr (std::is_integral_v<mask_type>) {
			if constexpr (sizeof(mask_type) == 2 && __has_avx512f_support_v<__isa>)
				return _kortestz_mask16_u8(__mask, __mask);

			else if constexpr (sizeof(mask_type) == 1 && __has_avx512dq_support_v<__isa>)
				return _kortestz_mask8_u8(__mask, __mask);

			else if constexpr (sizeof(mask_type) == 4 && __has_avx512bw_support_v<__isa>)
				return _kortestz_mask32_u8(__mask, __mask);

			else if constexpr (sizeof(mask_type) == 8 && __has_avx512bw_support_v<__isa>)
				return _kortestz_mask64_u8(__mask, __mask);

			else
				return (__mask == 0);
		}
		else {
			return _Simd_testz<__isa, __bit_width()>()(__simd_unwrap(__mask));
		}
	}

	raze_nodiscard raze_always_inline static constexpr bool __some_of(mask_type __mask) noexcept {
		return !__all_of(__mask);
	}

	raze_nodiscard raze_always_inline static constexpr bool __any_of(mask_type __mask) noexcept {
		return !__none_of(__mask);
	}

	raze_nodiscard raze_always_inline static constexpr 
		mask_type __bit_and(mask_type __left, mask_type __right) noexcept
	{
		if constexpr (std::is_integral_v<mask_type>) {
			if constexpr (sizeof(mask_type) == 1 && __has_avx512dq_support_v<__isa>)
				return _kand_mask8(__left, __right);

			else if constexpr (sizeof(mask_type) == 2 && __has_avx512f_support_v<__isa>)
				return _kand_mask16(__left, __right);

			else if constexpr (sizeof(mask_type) == 4 && __has_avx512bw_support_v<__isa>)
				return _kand_mask32(__left, __right);

			else if constexpr (sizeof(mask_type) == 8 && __has_avx512bw_support_v<__isa>)
				return _kand_mask64(__left, __right);

			else
				return __left & __right;
		}
		else {
			return __left & __right;
		}
	}

	raze_nodiscard raze_always_inline static constexpr
		mask_type __bit_or(mask_type __left, mask_type __right) noexcept
	{
		if constexpr (std::is_integral_v<mask_type>) {
			if constexpr (sizeof(mask_type) == 1 && __has_avx512dq_support_v<__isa>)
				return _kor_mask8(__left, __right);

			else if constexpr (sizeof(mask_type) == 2 && __has_avx512f_support_v<__isa>)
				return _kor_mask16(__left, __right);

			else if constexpr (sizeof(mask_type) == 4 && __has_avx512bw_support_v<__isa>)
				return _kor_mask32(__left, __right);

			else if constexpr (sizeof(mask_type) == 8 && __has_avx512bw_support_v<__isa>)
				return _kor_mask64(__left, __right);

			else
				return __left | __right;
		}
		else {
			return __left | __right;
		}
	}

	raze_nodiscard raze_always_inline static constexpr
		mask_type __bit_not(mask_type __mask) noexcept
	{
		if constexpr (std::is_integral_v<mask_type>) {
			if constexpr (sizeof(mask_type) == 1 && __has_avx512dq_support_v<__isa>)
				return _knot_mask8(__mask);

			else if constexpr (sizeof(mask_type) == 2 && __has_avx512f_support_v<__isa>)
				return _knot_mask16(__mask);

			else if constexpr (sizeof(mask_type) == 4 && __has_avx512bw_support_v<__isa>)
				return _knot_mask32(__mask);

			else if constexpr (sizeof(mask_type) == 8 && __has_avx512bw_support_v<__isa>)
				return _knot_mask64(__mask);

			else
				return ~__mask;
		}
		else {
			return ~__mask;
		}
	}

	raze_nodiscard raze_always_inline static constexpr
		mask_type __bit_xor(mask_type __left, mask_type __right) noexcept
	{
		if constexpr (std::is_integral_v<mask_type>) {
			if constexpr (sizeof(mask_type) == 1 && __has_avx512dq_support_v<__isa>)
				return _kxor_mask8(__left, __right);

			else if constexpr (sizeof(mask_type) == 2 && __has_avx512f_support_v<__isa>)
				return _kxor_mask16(__left, __right);

			else if constexpr (sizeof(mask_type) == 4 && __has_avx512bw_support_v<__isa>)
				return _kxor_mask32(__left, __right);

			else if constexpr (sizeof(mask_type) == 8 && __has_avx512bw_support_v<__isa>)
				return _kxor_mask64(__left, __right);

			else
				return __left ^ __right;
		}
		else {
			return __left ^ __right;
		}
	}

	raze_nodiscard raze_always_inline static constexpr mask_type __broadcast(bool __value) noexcept {
		if constexpr (std::is_integral_v<mask_type>) {
			return __value ? mask_type(-1) : mask_type(0);
		}
		else {
			return __value ? mask_type::broadcast(-1) : mask_type::zero();
		}
	}

	raze_nodiscard raze_always_inline static constexpr 
		auto __to_kmask(mask_type __mask) noexcept
	{
		if constexpr (std::is_integral_v<mask_type>) {
			if constexpr (sizeof(mask_type) == 1 && __has_avx512dq_support_v<__isa>)
				return _cvtmask8_u32(__mask);

			else if constexpr (sizeof(mask_type) == 2 && __has_avx512f_support_v<__isa>)
				return _cvtmask16_u32(__mask);

			else if constexpr (sizeof(mask_type) == 4 && __has_avx512bw_support_v<__isa>)
				return _cvtmask32_u32(__mask);

			else if constexpr (sizeof(mask_type) == 8 && __has_avx512bw_support_v<__isa>)
				return _cvtmask64_u64(__mask);

			else
				return __mask;
		}
	}

	raze_nodiscard raze_always_inline static constexpr
		auto __to_int(mask_type __mask) noexcept
	{
		if constexpr (std::is_integral_v<mask_type>) {
			if constexpr (sizeof(mask_type) == 1 && __has_avx512dq_support_v<__isa>)
				return _cvtu32_mask8(__mask);

			else if constexpr (sizeof(mask_type) == 2 && __has_avx512f_support_v<__isa>)
				return _cvtu32_mask16(__mask);

			else if constexpr (sizeof(mask_type) == 4 && __has_avx512bw_support_v<__isa>)
				return _cvtu32_mask32(__mask);

			else if constexpr (sizeof(mask_type) == 8 && __has_avx512bw_support_v<__isa>)
				return _cvtu64_mask64(__mask);

			else
				return __mask;
		}
	}

	raze_nodiscard raze_always_inline static constexpr 
		int __count_set(mask_type __mask) noexcept
	{
		if constexpr (std::is_integral_v<mask_type>) {
			return math::__popcnt_n_bits<__bit_width()>(__to_int(__mask));
		}
		else {
			return math::__popcnt_n_bits<(__bit_width() / 8) / sizeof(_Type_)>(
				_Simd_to_mask<__isa, __bit_width(), _Type_>()(__simd_unwrap(__mask)));
		}
	}
};

__RAZE_DATAPAR_NAMESPACE_END
