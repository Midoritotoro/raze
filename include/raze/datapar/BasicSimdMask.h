#pragma once 

#include <src/raze/datapar/bitwise/MaskImplementation.h>
#include <src/raze/datapar/bitwise/MaskElementReference.h>


__RAZE_DATAPAR_NAMESPACE_BEGIN


template <
	arch::ISA	_ISA_,
	typename	_Type_,
	uint32		_SimdWidth_>
class simd_mask
{
	static_assert(type_traits::__is_generation_supported_v<_ISA_>);
	static_assert(type_traits::__is_vector_type_supported_v<_Type_>);

	using _Impl = _Mask_implementation<_ISA_, _Type_, _SimdWidth_>;
public:
	using mask_type = typename _Impl::mask_type;
	using element_type = _Type_;

	static constexpr auto __width = _SimdWidth_;
	static constexpr auto __isa = _ISA_;

	simd_mask() = default;
	simd_mask(const simd_mask& __mask) = default;
	simd_mask(simd_mask&& __mask) = default;


	simd_mask(bool __value) noexcept {
		_mask = _Impl::__broadcast(__value);
	}

	simd_mask(mask_type __mask) noexcept {
		_mask = __mask;
	}

	template <class _Simd_> requires (std::is_convertible_v<_Simd_, mask_type>)
	simd_mask(_Simd_ __mask) noexcept { 
		_mask = static_cast<mask_type>(__mask);
	}

	constexpr raze_always_inline bool operator[](int32 __index) const noexcept {
		return __mask_element_reference<_ISA_, _Type_, _SimdWidth_>(_mask, __index);
	}

	constexpr raze_always_inline __mask_element_reference<_ISA_, _Type_, _SimdWidth_> operator[](int32 __index) noexcept {
		return __mask_element_reference<_ISA_, _Type_, _SimdWidth_>(_mask, __index);
	}

	friend constexpr raze_always_inline simd_mask operator&(
		const simd_mask& __left,
		const simd_mask& __right) noexcept
	{
		return _Impl::__bit_and(__left, __right);
	}

	friend constexpr raze_always_inline simd_mask operator|(
		const simd_mask& __left,
		const simd_mask& __right) noexcept
	{
		return _Impl::__bit_or(__left, __right);
	}

	friend constexpr raze_always_inline simd_mask operator^(
		const simd_mask& __left,
		const simd_mask& __right) noexcept
	{
		return _Impl::__bit_xor(__left, __right);
	}

	constexpr raze_always_inline simd_mask& operator&=(const simd_mask& __other) noexcept {
		return *this & __other;
	}

	constexpr raze_always_inline simd_mask& operator|=(const simd_mask& __other) noexcept {
		return *this | __other;
	}

	constexpr raze_always_inline simd_mask& operator^=(const simd_mask& __other) noexcept {
		return *this ^ __other;
	}

	constexpr raze_always_inline simd_mask operator!() const noexcept {
		return _Impl::__bit_not(_mask);
	}

	friend constexpr raze_always_inline simd_mask operator!=(
		const simd_mask& __left,
		const simd_mask& __right) noexcept
	{
		return _Impl::__bit_xor(__left, __right);
	}

	friend constexpr raze_always_inline simd_mask operator==(
		const simd_mask& __left,
		const simd_mask& __right) noexcept
	{
		return !(__left != __right);
	}

	static constexpr raze_always_inline int32 bit_width() noexcept {
		return _Impl::__bit_width();
	}

	static constexpr raze_always_inline int32 elements() noexcept {
		return _Impl::__elements();
	}

	raze_nodiscard raze_always_inline constexpr bool __all_of() const noexcept {
		return _Impl::__all_of(_mask);
	}

	raze_nodiscard raze_always_inline constexpr bool __any_of() const noexcept {
		return _Impl::__any_of(_mask);
	}

	raze_nodiscard raze_always_inline constexpr bool __none_of() const noexcept {
		return _Impl::__none_of(_mask);
	}

	raze_nodiscard raze_always_inline constexpr bool __some_of() const noexcept {
		return _Impl::__some_of(_mask);
	}

	raze_always_inline constexpr operator mask_type() const noexcept {
		return _mask;
	}

	raze_nodiscard raze_always_inline constexpr int32 __count_set() const noexcept {
		return _Impl::__count_set(_mask);
	}

	raze_nodiscard raze_always_inline constexpr int32 __count_trailing_zero_bits() const noexcept {
		return _Impl::__count_trailing_zero_bits(_mask);
	}

	raze_nodiscard raze_always_inline constexpr int32 __count_leading_zero_bits() const noexcept {
		return _Impl::__count_leading_zero_bits(_mask);
	}
private:
	mask_type _mask = 0;
};

__RAZE_DATAPAR_NAMESPACE_END