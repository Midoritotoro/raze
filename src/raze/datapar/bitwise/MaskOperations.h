#pragma once 

#include <src/raze/datapar/bitwise/ToK.h>
#include <src/raze/datapar/bitwise/MaskIntrin.h>
#include <src/raze/datapar/SimdIntegralTypesCheck.h>
#include <src/raze/datapar/SimdMaskTypeCheck.h>
#include <src/raze/math/BitTestAndReset.h>
#include <raze/math/BitMath.h>


__RAZE_DATAPAR_NAMESPACE_BEGIN

template <
	class _Type_,
	class _Abi_>
class _Mask_operations {
public:
	static constexpr auto __width = _Abi_::width;
	static constexpr auto __isa = _Abi_::isa;
	static constexpr auto __elements_count = ((__width / 8) / sizeof(_Type_));

	using element_type = _Type_;
	using mask_type = __mmask_for_elements_t<__elements_count>;

	raze_nodiscard raze_always_inline static constexpr int32 __bit_width() noexcept {
		return __elements_count;
	}

	raze_nodiscard raze_always_inline static constexpr int32 __elements() noexcept {
		return __elements_count;
	}

	raze_nodiscard raze_always_inline static bool __all_of(mask_type __mask) noexcept {
		raze_maybe_unused_attribute constexpr auto __max_for_bits = ((sizeof(mask_type) * 8) == __elements())
			? math::__maximum_integral_limit<mask_type>()
			: mask_type(((mask_type(1) << __elements()) - 1));

#if (defined(raze_cpp_clang) || defined(raze_cpp_gnu)) && !defined(raze_cpp_msvc)
		if constexpr (__elements() < 8 && __has_avx512dq_support_v<__isa>)
			return __ktestcb(__mask, _cvtu32_mask8(__max_for_bits));

		else if constexpr (sizeof(mask_type) == 1 && __has_avx512dq_support_v<__isa>)
			return __kortestcb(__mask, __mask);

		else if constexpr (sizeof(mask_type) == 2 && __has_avx512f_support_v<__isa>)
			return __kortestcw(__mask, __mask);

		else if constexpr (sizeof(mask_type) == 4 && __has_avx512bw_support_v<__isa>)
			return __kortestcd(__mask, __mask);

		else if constexpr (sizeof(mask_type) == 8 && __has_avx512bw_support_v<__isa>)
			return __kortestcq(__mask, __mask);

		else
#endif // (defined(raze_cpp_clang) || defined(raze_cpp_gnu)) && !defined(raze_cpp_msvc)
			return (__mask == __max_for_bits);
	}

	raze_nodiscard raze_always_inline static bool __none_of(mask_type __mask) noexcept {
#if (defined(raze_cpp_clang) || defined(raze_cpp_gnu)) && !defined(raze_cpp_msvc)
		if constexpr (sizeof(mask_type) == 2 && __has_avx512f_support_v<__isa>)
			return __kortestzw(__mask, __mask);

		else if constexpr (sizeof(mask_type) == 1 && __has_avx512dq_support_v<__isa>)
			return __kortestzb(__mask, __mask);

		else if constexpr (sizeof(mask_type) == 4 && __has_avx512bw_support_v<__isa>)
			return __kortestzd(__mask, __mask);

		else if constexpr (sizeof(mask_type) == 8 && __has_avx512bw_support_v<__isa>)
			return __kortestzq(__mask, __mask);

		else
#endif // (defined(raze_cpp_clang) || defined(raze_cpp_gnu)) && !defined(raze_cpp_msvc)
			return (__mask == 0);
	}


	raze_nodiscard raze_always_inline static bool __any_of(mask_type __mask) noexcept {
		return !__none_of(__mask);
	}

	raze_nodiscard raze_always_inline static bool __some_of(mask_type __mask) noexcept {
		return !__all_of(__mask);
	}

	raze_nodiscard raze_always_inline static mask_type __bit_and(
		mask_type __left,
		mask_type __right) noexcept
	{
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

	raze_nodiscard raze_always_inline static mask_type __bit_or(
		mask_type __left,
		mask_type __right) noexcept
	{
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

	raze_nodiscard raze_always_inline static mask_type __bit_xor(
		mask_type __left,
		mask_type __right) noexcept
	{
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

	raze_nodiscard raze_always_inline static mask_type __bit_not(mask_type __mask) noexcept {
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

	template <uint32 _Shift_>
	raze_nodiscard raze_always_inline static mask_type __bit_rshift(
		mask_type								__mask,
		std::integral_constant<uint32, _Shift_>	__shift) noexcept
	{
		if constexpr (__shift >= __elements())
			return 0;

		if constexpr (sizeof(mask_type) == 1 && __has_avx512dq_support_v<__isa>)
			return _kshiftri_mask8(__mask, __shift);

		else if constexpr (sizeof(mask_type) == 2 && __has_avx512f_support_v<__isa>)
			return _kshiftri_mask16(__mask, __shift);

		else if constexpr (sizeof(mask_type) == 4 && __has_avx512bw_support_v<__isa>)
			return _kshiftri_mask32(__mask, __shift);

		else if constexpr (sizeof(mask_type) == 8 && __has_avx512bw_support_v<__isa>)
			return _kshiftri_mask64(__mask, __shift);

		else
			return __mask >> __shift;
	}

	raze_nodiscard raze_always_inline static mask_type __bit_rshift(
		mask_type	__mask,
		uint32		__shift) noexcept
	{
		return (__shift >= __elements()) ? 0 : __mask >> __shift;
	}

	template <uint32 _Shift_>
	raze_nodiscard raze_always_inline static mask_type __bit_lshift(
		mask_type								__mask,
		std::integral_constant<uint32, _Shift_>	__shift) noexcept
	{
		raze_maybe_unused_attribute constexpr auto __all_mask = ((sizeof(mask_type) * 8) == __elements())
			? math::__maximum_integral_limit<mask_type>()
			: mask_type(((mask_type(1) << __elements()) - 1));

		if constexpr (sizeof(mask_type) == 1 && __has_avx512dq_support_v<__isa>) {
			if constexpr (__elements() < 8)
				return __bit_and(_kshiftli_mask8(__mask, __shift), __to_k<__isa>(__all_mask));
			else
				return _kshiftli_mask8(__mask, __shift);
		}
		else if constexpr (sizeof(mask_type) == 2 && __has_avx512f_support_v<__isa>) {
			return _kshiftli_mask16(__mask, __shift);
		}
		else if constexpr (sizeof(mask_type) == 4 && __has_avx512bw_support_v<__isa>) {
			return _kshiftli_mask32(__mask, __shift);
		}
		else if constexpr (sizeof(mask_type) == 8 && __has_avx512bw_support_v<__isa>) {
			return _kshiftli_mask64(__mask, __shift);
		}
		else {
			if constexpr (__elements() < 8)
				return (__mask << __shift) & __all_mask;
			else
				return __mask << __shift;
		}
	}

	raze_nodiscard raze_always_inline static mask_type __bit_lshift(
		mask_type	__mask,
		uint32		__shift) noexcept
	{
		raze_maybe_unused_attribute constexpr auto __all_mask = ((sizeof(mask_type) * 8) == __elements())
			? math::__maximum_integral_limit<mask_type>()
			: mask_type(((mask_type(1) << __elements()) - 1));

		if constexpr (__elements() < 8)
			return (__mask << __shift) & __all_mask;
		else
			return (__mask << __shift);
	}

	template <uint32 _Shift_>
	raze_nodiscard raze_always_inline static mask_type __bit_lslide(
		mask_type								__mask,
		std::integral_constant<uint32, _Shift_>	__shift) noexcept
	{
		if constexpr (__shift >= __elements())
			return 0;
		else
			return __bit_rshift(__mask, __shift);
	}

	raze_nodiscard raze_always_inline static mask_type __bit_lslide(
		mask_type	__mask,
		uint32		__shift) noexcept
	{
		return (__shift >= __elements()) ? 0 : __bit_rshift(__mask, __shift);
	}

	template <uint32 _Shift_>
	raze_nodiscard raze_always_inline static mask_type __bit_rslide(
		mask_type								__mask,
		std::integral_constant<uint32, _Shift_>	__shift) noexcept
	{
		if constexpr (__shift >= __elements())
			return 0;
		else
			return __bit_lshift(__mask, __shift);
	}

	raze_nodiscard raze_always_inline static mask_type __bit_rslide(
		mask_type	__mask,
		uint32		__shift) noexcept
	{
		return (__shift >= __elements()) ? 0 : __bit_lshift(__mask, __shift);
	}


	raze_nodiscard raze_always_inline static int32 __count_set(mask_type __mask) noexcept {
		return math::__popcnt_n_bits<__bit_width()>(__to_gpr<__isa>(__mask));
	}
	
	raze_nodiscard raze_always_inline static int32
		__count_trailing_zero_bits(mask_type __mask) noexcept
	{
		if constexpr (__bit_width() <= 8)
			return math::__ctz_n_bits<__bit_width()>(__to_gpr<__isa>(__mask));

		else if constexpr (__has_avx2_support_v<__isa>)
			return math::__tzcnt_ctz(__to_gpr<__isa>(__mask));

		else
			return math::__bsf_ctz(__to_gpr<__isa>(__mask));
	}

	raze_nodiscard raze_always_inline static int32
		__count_leading_zero_bits(mask_type __mask) noexcept
	{
		if constexpr (__bit_width() <= 8)
			return math::__clz_n_bits<__bit_width()>(__mask);

		else if constexpr (__has_avx2_support_v<__isa>)
			return math::__lzcnt_clz(static_cast<mask_type>(__to_gpr<__isa>(__mask)));

		else
			return math::__bsr_clz(static_cast<mask_type>(__to_gpr<__isa>(__mask)));
	}

	raze_nodiscard raze_always_inline static int32
		__count_trailing_one_bits(mask_type __mask) noexcept
	{
		if constexpr (__bit_width() >= 8) {
			return __count_trailing_zero_bits(~__mask);
		}
		else {
			auto __count = 0;

			for (auto __i = 0; __i < __elements(); ++__i) {
				if (!((__mask >> __i) & 1))
					break;

				++__count;
			}

			return __count;
		}
	}

	raze_nodiscard raze_always_inline static int32
		__count_leading_one_bits(mask_type __mask) noexcept
	{
		if constexpr (__bit_width() >= 8) {
			return __count_leading_zero_bits(~__mask);
		}
		else {
			auto __count = 0;

			for (auto __i = __elements() - 1; __i >= 0; --__i) {
				if (!((__mask >> __i) & 1))
					break;

				++__count;
			}

			return __count;
		}
	}

	raze_nodiscard raze_always_inline static bool __is_contiguous(
		mask_type	__mask,
		uint32		__n,
		uint32		__k) noexcept
	{
		constexpr auto __first_n_table = __generate_first_n_table<__elements()>();

		auto __size = uint32(__k - __n + 1);
		auto __x = mask_type(0);

		if (__size >= sizeof(mask_type) * 8)
			__x = math::__maximum_integral_limit<mask_type>();
		else
			__x = __bit_lshift(__first_n_table[__size], __n);

		return __bit_and(__mask, __x) == __x;
	}

	raze_nodiscard raze_always_inline static mask_type
		__clear_left(mask_type __mask) noexcept
	{
		const auto __last_set = __elements() - __count_leading_zero_bits(__mask) - 1;
		math::__bit_test_and_reset(__mask, __last_set);
		return __mask;
	}

	raze_nodiscard raze_always_inline static mask_type
		__clear_right(mask_type __mask) noexcept
	{
		return (__mask & (__mask - 1));
	}
};

__RAZE_DATAPAR_NAMESPACE_END
