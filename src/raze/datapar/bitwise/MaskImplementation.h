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
#include <raze/algorithm/minmax/Max.h>
#include <src/raze/math/BitTestAndReset.h>
#include <src/raze/math/BitTest.h>
#include <src/raze/datapar/bitwise/MaskOperations.h>


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
	class _Type_,
	class _Abi_>
class _Mask_implementation {
public:
	static constexpr auto __number_mask = __is_native_compare_returns_number_v<simd<_Type_, _Abi_>>;
	static constexpr auto __width = _Abi_::width;
	static constexpr auto __isa = _Abi_::isa;

	using element_type = _Type_;
	using mask_type = std::conditional_t<__number_mask,
		__mmask_for_elements_t<simd<_Type_, _Abi_>::size()>,
		simd<typename IntegerForSizeof<_Type_>::Unsigned, _Abi_>>;

	using __operations = _Mask_operations<_Type_, _Abi_>;

	raze_nodiscard raze_always_inline static constexpr int32 __bit_width() noexcept {
		constexpr auto __count = __number_mask ? simd<_Type_, _Abi_>::size() : __width;
		return __count;
	}

	raze_nodiscard raze_always_inline static constexpr int32 __elements() noexcept {
		return simd<_Type_, _Abi_>::size();
	}
	
	raze_nodiscard raze_always_inline static bool __all_of(mask_type __mask) noexcept {
		if constexpr (std::is_integral_v<mask_type>)
			return __operations::__all_of(__mask);
		else
			return _Test_all_ones<__isa, __bit_width()>()(__data(__mask));
	}

	raze_nodiscard raze_always_inline static bool __none_of(mask_type __mask) noexcept {
		if constexpr (std::is_integral_v<mask_type>)
			return __operations::__none_of(__mask);
		else
			return _Testz<__isa, __bit_width()>()(__data(__mask));
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
		if constexpr (std::is_integral_v<mask_type>)
			return __operations::__bit_and(__left, __right);
		else
			return __left & __right;
	}

	raze_nodiscard raze_always_inline static mask_type __bit_or(
		mask_type __left,
		mask_type __right) noexcept
	{
		if constexpr (std::is_integral_v<mask_type>)
			return __operations::__bit_or(__left, __right);
		else
			return __left | __right;
	}

	raze_nodiscard raze_always_inline static mask_type __bit_not(mask_type __mask) noexcept {
		if constexpr (std::is_integral_v<mask_type>)
			return __operations::__bit_not(__mask);
		else
			return ~__mask;
	}

	raze_nodiscard raze_always_inline static mask_type __bit_xor(
		mask_type __left,
		mask_type __right) noexcept
	{
		if constexpr (std::is_integral_v<mask_type>)
			return __operations::__bit_xor(__left, __right);
		else
			return __left ^ __right;
	}

	raze_nodiscard raze_always_inline static mask_type __broadcast(bool __value) noexcept {
		if constexpr (std::is_integral_v<mask_type>)
			return _First_n<__isa, __width, _Type_>()(__elements() * int32(__value));
		else
			return __value ? mask_type::broadcast(-1) : mask_type::zero();
	}

	raze_nodiscard raze_always_inline static int32 __count_set(mask_type __mask) noexcept {
		if constexpr (std::is_integral_v<mask_type>)
			return __operations::__count_set(__mask);
		else
			return math::__popcnt_n_bits<(__bit_width() / 8) / sizeof(_Type_)>(
				_To_mask<__isa, __width, _Type_>()(__data(__mask)));
	}

	raze_nodiscard raze_always_inline static int32 
		__count_trailing_zero_bits(mask_type __mask) noexcept
	{
		if constexpr (std::is_integral_v<mask_type>) {
			return __operations::__count_trailing_zero_bits(__mask);
		}
		else {
			using _IndexMaskType = _Simd_bitmask<_Type_, _Abi_>;
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
		if constexpr (std::is_integral_v<mask_type>)
			return __operations::__count_trailing_one_bits(__mask);
		else
			return __count_trailing_zero_bits(~__mask);
	}

	raze_nodiscard raze_always_inline static int32 
		__count_leading_one_bits(mask_type __mask) noexcept
	{
		if constexpr (std::is_integral_v<mask_type>)
			return __operations::__count_leading_one_bits(__mask);
		else
			return __count_leading_zero_bits(~__mask);
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
			return __operations::__clear_right(__mask);
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
		if constexpr (std::is_integral_v<mask_type>) {
			return __operations::__clear_left(__mask);
		}
		else {
			auto __converted_mask = _To_mask<__isa, __width, _Type_>()(__data(__mask));
			return _To_vector<__isa, __width, typename mask_type::vector_type, _Type_>()(
				__operations::__clear_left(__converted_mask));
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
			return __operations::__count_leading_zero_bits(__mask);
		}
		else {
			using _IndexMaskType = _Simd_bitmask<_Type_, _Abi_>;
			const auto __index_mask = _To_bitmask<__isa, __width, _Type_>()(__data(__mask));
			return _IndexMaskType(__index_mask).__count_leading_zero_bits();
		}
	}
};

__RAZE_DATAPAR_NAMESPACE_END
