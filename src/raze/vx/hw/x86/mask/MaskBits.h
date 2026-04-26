#pragma once 

#include <src/raze/vx/hw/x86/mask/MaskImplementation.h>


__RAZE_VX_NAMESPACE_BEGIN

template <
	class _Type_,
	class _Abi_>
class _Mask_bits {
	static_assert(type_traits::__is_generation_supported_v<_Abi_::isa>);
	static_assert(type_traits::__is_vector_type_supported_v<_Type_>);
public:
	static constexpr auto __isa = _Abi_::isa;
	static constexpr auto __width = _Abi_::width;
	static constexpr auto __elements = simd<_Type_, _Abi_>::size();

	using element_type = _Type_;
	using abi_type = _Abi_;
	using mask_type = __mmask_for_elements_t<__elements>;
	using __operations = _Mask_operations<_Type_, _Abi_>;


	template <class _Simd_>
	_Mask_bits(const _Simd_& __simd) noexcept
		requires(__is_valid_simd_v<_Simd_>)
	{
		_bits = _To_mask<__isa, __width, _Type_>()(__data(__simd));
	}

	template <std::unsigned_integral _Mask_>
	_Mask_bits(const _Mask_& __integral) noexcept {
		_bits = static_cast<mask_type>(__integral);
	}

	raze_always_inline operator mask_type() const noexcept {
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

	raze_nodiscard raze_always_inline int32 __count_trailing_zero_bits() const noexcept {
		return __operations::__count_trailing_zero_bits(_bits);
	}

	raze_nodiscard raze_always_inline int32 __count_leading_zero_bits() const noexcept {
		return __operations::__count_leading_zero_bits(_bits);
	}

	raze_nodiscard raze_always_inline int32 __count_trailing_one_bits() const noexcept {
		return __operations::__count_trailing_one_bits(_bits);
	}

	raze_nodiscard raze_always_inline int32 __count_leading_one_bits() const noexcept {
		return __operations::__count_leading_one_bits(_bits);
	}

	raze_always_inline void __clear_left() noexcept {
		_bits = __operations::__clear_left(_bits);
	}

	raze_always_inline void __clear_right() noexcept {
		_bits = __operations::__clear_right(_bits);
	}

	raze_nodiscard raze_always_inline bool __is_contiguous(uint32 __n, uint32 __k) const noexcept {
		return __operations::__is_contiguous(_bits, __n, __k);
	}
private:
	mask_type _bits = 0;
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
    std::void_t<_Mask_bits<typename _SimdMaskBits_::element_type,
                typename _SimdMaskBits_::abi_type>>>
    : std::bool_constant<
        type_traits::is_virtual_base_of_v<
            _Mask_bits<typename _SimdMaskBits_::element_type,
                typename _SimdMaskBits_::abi_type>,
            _SimdMaskBits_> ||
        std::is_same_v<
            _Mask_bits<typename _SimdMaskBits_::element_type,
				typename _SimdMaskBits_::abi_type>,
            _SimdMaskBits_>> 
{};

template <class _SimdMaskBits_>
constexpr bool __is_simd_mask_bits_v = __is_simd_mask_bits<std::remove_cvref_t<_SimdMaskBits_>>::value;

__RAZE_VX_NAMESPACE_END
