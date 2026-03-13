#pragma once 

#include <src/raze/datapar/bitwise/Testz.h>
#include <src/raze/datapar/bitwise/TestAllOnes.h>
#include <src/raze/datapar/MaskTypeSelector.h>
#include <raze/datapar/SimdBitmask.h>
#include <raze/math/BitMath.h>
#include <src/raze/datapar/bitwise/ToMask.h>
#include <src/raze/datapar/bitwise/ToVector.h>
#include <src/raze/datapar/memory/MaskzLoad.h>
#include <src/raze/datapar/bitwise/FirstN.h>
#include <src/raze/datapar/SimdIntegralTypesCheck.h>
#include <src/raze/datapar/bitwise/MaskIntrin.h>
#include <raze/algorithm/minmax/Max.h>
#include <src/raze/math/BitTestAndReset.h>
#include <src/raze/math/BitTest.h>


__RAZE_DATAPAR_NAMESPACE_BEGIN

template <class _Simd_>
constexpr inline auto __is_native_compare_returns_number_v = 
	(__has_avx512f_support_v<_Simd_::__isa> && sizeof(typename _Simd_::value_type) >= 4) ||
	(__has_avx512bw_support_v<_Simd_::__isa>);

template <class _DataparType_>
__simd_nodiscard_inline auto __data(const _DataparType_& __datapar) noexcept
	requires(__is_valid_simd_v<_DataparType_>)
{
	return static_cast<typename _DataparType_::vector_type>(__datapar);
}

template <
	arch::ISA	_ISA_,
	typename	_Type_,
	uint32		_SimdWidth_>
class _Mask_implementation {
public:
	static constexpr auto __number_mask = __is_native_compare_returns_number_v<simd<_ISA_, _Type_, _SimdWidth_>>;
	static constexpr auto __width = _SimdWidth_;
	static constexpr auto __isa = _ISA_;

	using element_type = _Type_;
	using mask_type = std::conditional_t<__number_mask,
		__mmask_for_elements_t<simd<_ISA_, _Type_, _SimdWidth_>::size()>,
		simd<_ISA_, typename IntegerForSizeof<_Type_>::Unsigned, _SimdWidth_>>;

	raze_nodiscard raze_always_inline static constexpr int32 __bit_width() noexcept {
		constexpr auto __count = __number_mask ? simd<_ISA_, _Type_, _SimdWidth_>::size() : _SimdWidth_;
		return __count;
	}

	raze_nodiscard raze_always_inline static constexpr int32 __elements() noexcept {
		return simd<_ISA_, _Type_, _SimdWidth_>::size();
	}
	
	raze_nodiscard raze_always_inline static bool __all_of(mask_type __mask) noexcept {
		if constexpr (std::is_integral_v<mask_type>) {
			raze_maybe_unused_attribute constexpr auto __max_for_bits = ((sizeof(mask_type) * 8) == __elements())
				? math::__maximum_integral_limit<mask_type>()
				: mask_type(((mask_type(1) << __elements()) - 1));
			
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
				return (__mask == __max_for_bits);
		}
		else {
			return _Test_all_ones<__isa, __bit_width()>()(__data(__mask));
		}
	}

	raze_nodiscard raze_always_inline static bool __none_of(mask_type __mask) noexcept {
		if constexpr (std::is_integral_v<mask_type>) {
			if constexpr (sizeof(mask_type) == 2 && __has_avx512f_support_v<__isa>)
				return __kortestzw(__mask, __mask);

			else if constexpr (sizeof(mask_type) == 1 && __has_avx512dq_support_v<__isa>)
				return __kortestzb(__mask, __mask);

			else if constexpr (sizeof(mask_type) == 4 && __has_avx512bw_support_v<__isa>)
				return __kortestzd(__mask, __mask);

			else if constexpr (sizeof(mask_type) == 8 && __has_avx512bw_support_v<__isa>)
				return __kortestzq(__mask, __mask);

			else
				return (__mask == 0);
		}
		else {
			return _Testz<__isa, __bit_width()>()(__data(__mask));
		}
	}

	raze_nodiscard raze_always_inline static bool __some_of(mask_type __mask) noexcept {
		return !__all_of(__mask);
	}

	raze_nodiscard raze_always_inline static bool __any_of(mask_type __mask) noexcept {
		return !__none_of(__mask);
	}

	raze_nodiscard raze_always_inline static mask_type __bit_and(
		mask_type __left, 
		mask_type __right) noexcept
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

	raze_nodiscard raze_always_inline static mask_type __bit_or(
		mask_type __left,
		mask_type __right) noexcept
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

	raze_nodiscard raze_always_inline static mask_type __bit_not(mask_type __mask) noexcept {
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

	raze_nodiscard raze_always_inline static mask_type __bit_xor(
		mask_type __left,
		mask_type __right) noexcept
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

	raze_nodiscard raze_always_inline static mask_type __broadcast(bool __value) noexcept {
		if constexpr (std::is_integral_v<mask_type>)
			return _First_n<__isa, __width, _Type_>()(__elements() * int32(__value));
		else
			return __value ? mask_type::broadcast(-1) : mask_type::zero();
	}

	raze_nodiscard raze_always_inline static int32 __count_set(mask_type __mask) noexcept {
		if constexpr (std::is_integral_v<mask_type>)
			return math::__popcnt_n_bits<__bit_width()>(__to_gpr<__isa>(__mask));
		else
			return math::__popcnt_n_bits<(__bit_width() / 8) / sizeof(_Type_)>(
				_To_mask<__isa, __width, _Type_>()(__data(__mask)));
	}

	raze_nodiscard raze_always_inline static int32 
		__count_trailing_zero_bits(mask_type __mask) noexcept
	{
		if constexpr (std::is_integral_v<mask_type>) {
			if constexpr (__bit_width() <= 8)
				return math::__ctz_n_bits<__bit_width()>(__to_gpr<__isa>(__mask));

			else if constexpr (__has_avx2_support_v<__isa>)
				return math::__tzcnt_ctz(__to_gpr<__isa>(__mask));

			else
				return math::__bsf_ctz(__to_gpr<__isa>(__mask));
		}
		else {
			using _IndexMaskType = _Simd_bitmask<_ISA_, _Type_, __width>;
			const auto __index_mask = _To_bitmask<__isa, __width, _Type_>()(__data(__mask));
			return _IndexMaskType(__index_mask).__count_trailing_zero_bits();
		}
	}

	raze_nodiscard raze_always_inline static int32
		__count_leading_zero_bits(mask_type __mask) noexcept
	{
		return __count_leading_zero_bits_any(__mask);
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

	template <sizetype _Bits_>
	raze_nodiscard raze_always_inline static mask_type
		__from_bitset(const std::bitset<_Bits_>& __bitset) noexcept
	{
		static_assert(_Bits_ <= 64, "Not supported");
		const auto __bits = __bitset.to_ullong();

		if constexpr (std::is_integral_v<mask_type>)
			return __bits;
		else
			return _To_vector<__isa, __width, typename mask_type::vector_type, _Type_>()(__bits);
	}

	template <
		class _UnwrappedForwardIterator_,
		class _AlignmentPolicy_>
	raze_nodiscard raze_always_inline static mask_type __copy_from_unchecked(
		_UnwrappedForwardIterator_	__first,
		_AlignmentPolicy_&&			__alignment_policy) noexcept
	{
		using _ValueType = type_traits::iterator_value_type<_UnwrappedForwardIterator_>;
		return __copy_from_scalar(__first, __alignment_policy);
	}

	template <
		class _UnwrappedOutputIterator_,
		class _AlignmentPolicy_>
	raze_always_inline static void __copy_to_unchecked(
		_UnwrappedOutputIterator_	__first,
		mask_type					__mask,
		_AlignmentPolicy_&&			__alignment_policy) noexcept
	{
		__copy_to_scalar(__first, __mask, __alignment_policy);
	}

	raze_nodiscard raze_always_inline static mask_type
		__clear_right(mask_type __mask) noexcept
	{
		if constexpr (std::is_integral_v<mask_type>) {
			return (__mask & (__mask - 1));
		}
		else {
			const auto __converted_mask = _To_mask<__isa, __width, _Type_>()(__data(__mask));
			return _To_vector<__isa, __width, typename mask_type::vector_type,
				_Type_>()((__converted_mask & (__converted_mask - 1)));
		}
	}

	raze_nodiscard raze_always_inline static mask_type
		__clear_left(mask_type __mask) noexcept
	{
		auto __converted_mask = _To_mask<__isa, __width, _Type_>()(__data(__mask));
		const auto __last_set = __elements() - __count_leading_zero_bits_any(__converted_mask) - 1;

		if constexpr (std::is_integral_v<mask_type>) {
			math::__bit_test_and_reset(__converted_mask);
			return __converted_mask;
		}
		else {
			math::__bit_test_and_reset(__converted_mask, __last_set);
			return _To_vector<__isa, __width, typename mask_type::vector_type, _Type_>()(__converted_mask);
		}
	}
private:
	template <
		class _UnwrappedForwardIterator_,
		class _AlignmentPolicy_>
	raze_nodiscard raze_always_inline static mask_type __copy_from_scalar(
		_UnwrappedForwardIterator_	__first,
		_AlignmentPolicy_&&			__alignment_policy) noexcept
	{
		auto __mask = mask_type();

		if constexpr (std::is_integral_v<mask_type>) {
			for (auto __i = 0; __i < __elements(); ++__i)
				__mask |= static_cast<mask_type>(*__first++) << __i;
		}
		else {
			for (auto __i = 0; __i < __elements(); ++__i)
				__mask[__i] = ((*__first++) == 1) ? -1 : 0;
		}

		return __mask;
	}

	template <
		class _UnwrappedOutputIterator_,
		class _AlignmentPolicy_>
	raze_always_inline static void __copy_to_scalar(
		_UnwrappedOutputIterator_	__first,
		mask_type					__mask,
		_AlignmentPolicy_&&			__alignment_policy) noexcept
	{
		if constexpr (std::is_integral_v<mask_type>) {
			for (auto __i = 0; __i < __elements(); ++__i)
				*__first++ = static_cast<bool>(math::__bit_test(__mask, __i));
		}
		else {
			for (auto __i = 0; __i < __elements(); ++__i)
				*__first++ = static_cast<bool>(__mask[__i]);
		}
	}

	template <class _AnyMaskType_>
	raze_nodiscard raze_always_inline static int32
		__count_leading_zero_bits_any(_AnyMaskType_ __mask) noexcept
	{
		if constexpr (std::is_integral_v<_AnyMaskType_>) {
			if constexpr (__bit_width() <= 8)
				return math::__clz_n_bits<__bit_width()>(__mask);

			else if constexpr (__has_avx2_support_v<__isa>)
				return math::__lzcnt_clz(static_cast<_AnyMaskType_>(__to_gpr<__isa>(__mask)));

			else
				return math::__bsr_clz(static_cast<_AnyMaskType_>(__to_gpr<__isa>(__mask)));
		}
		else {
			using _IndexMaskType = _Simd_bitmask<_ISA_, _Type_, __width>;
			const auto __index_mask = _To_bitmask<__isa, __width, _Type_>()(__data(__mask));
			return _IndexMaskType(__index_mask).__count_leading_zero_bits();
		}
	}
};

__RAZE_DATAPAR_NAMESPACE_END
