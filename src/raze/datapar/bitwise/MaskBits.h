#pragma once 

#include <src/raze/datapar/bitwise/MaskImplementation.h>


__RAZE_DATAPAR_NAMESPACE_BEGIN

template <
	arch::ISA	_ISA_,
	class		_Type_,
	uint32		_SimdWidth_>
class _Mask_bits {
	static_assert(type_traits::__is_generation_supported_v<_ISA_>);
	static_assert(type_traits::__is_vector_type_supported_v<_Type_>);
public:
	static constexpr auto __isa = _ISA_;
	static constexpr auto __width = _SimdWidth_;
	static constexpr auto __elements = simd<_ISA_, _Type_, _SimdWidth_>::size();

	using element_type = _Type_;
	using __mask_type = __mmask_for_elements_t<__elements>;
	using __operations = _Mask_operations<_ISA_, _Type_, _SimdWidth_>;


	template <class _Simd_>
	_Mask_bits(const _Simd_& __simd) noexcept
		requires(__is_valid_simd_v<_Simd_>)
	{
		_bits = _To_mask<_ISA_, _SimdWidth_, _Type_>()(__data(__simd));
	}

	template <std::unsigned_integral _Mask_>
	_Mask_bits(const _Mask_& __integral) noexcept {
		_bits = static_cast<__mask_type>(__integral);
	}

	raze_always_inline operator __mask_type() const noexcept {
		return _bits;
	}

	raze_nodiscard raze_always_inline static constexpr int32 elements() noexcept {
		return __elements;
	}

	raze_nodiscard raze_always_inline static constexpr int32 bit_width() noexcept {
		return __elements;
	}

	raze_nodiscard raze_always_inline bool __all_of() const noexcept {
		return __operations::__all_of(_bits);
	}

	raze_nodiscard raze_always_inline bool __any_of() const noexcept {
		return __operations::__any_of(_bits);
	}

	raze_nodiscard raze_always_inline bool __none_of() const noexcept {
		return __operations::__none_of(_bits);
	}

	raze_nodiscard raze_always_inline bool __some_of() const noexcept {
		return __operations::__some_of(_bits);
	}

	raze_nodiscard raze_always_inline int32 __count_set() const noexcept {
		return __operations::__count_set(_bits);
	}

	raze_nodiscard raze_always_inline int32 
		__count_trailing_zero_bits() const noexcept
	{
		return __operations::__count_trailing_zero_bits(_bits);
	}

	raze_nodiscard raze_always_inline int32
		__count_leading_zero_bits() const noexcept
	{
		return __operations::__count_leading_zero_bits(_bits);
	}

	raze_nodiscard raze_always_inline int32 
		__count_trailing_one_bits() const noexcept 
	{
		return __operations::__count_trailing_one_bits(_bits);
	}

	raze_nodiscard raze_always_inline int32 
		__count_leading_one_bits() const noexcept 
	{
		return __operations::__count_leading_one_bits(_bits);
	}

	raze_always_inline void __clear_left() noexcept {
		_bits = __operations::__clear_left(_bits);
	}

	raze_always_inline void __clear_right() noexcept {
		_bits = __operations::__clear_right(_bits);
	}
private:
	__mask_type _bits = 0;
};

template <
	class _SimdMaskBits_,
	class = void>
struct __is_simd_mask_bits:
	std::false_type
{};

template <class _SimdMaskBits_>
struct __is_simd_mask_bits<
	_SimdMaskBits_,
    std::void_t<_Mask_bits<_SimdMaskBits_::__isa,
                typename _SimdMaskBits_::element_type,
                _SimdMaskBits_::__width>>>
    : std::bool_constant<
        type_traits::is_virtual_base_of_v<
            _Mask_bits<_SimdMaskBits_::__isa,
                typename _SimdMaskBits_::element_type,
                _SimdMaskBits_::__width>,
            _SimdMaskBits_> ||
        std::is_same_v<
            _Mask_bits<_SimdMaskBits_::__isa,
				typename _SimdMaskBits_::element_type,
				_SimdMaskBits_::__width>,
            _SimdMaskBits_>> 
{};

template <class _SimdMaskBits_>
constexpr bool __is_simd_mask_bits_v = __is_simd_mask_bits<std::remove_cvref_t<_SimdMaskBits_>>::value;

__RAZE_DATAPAR_NAMESPACE_END
