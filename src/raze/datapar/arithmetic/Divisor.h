#pragma once 

#include <raze/arch/CpuFeature.h>
#include <raze/math/BitMath.h>
#include <src/raze/math/BitTestAndReset.h>


__RAZE_DATAPAR_NAMESPACE_BEGIN

template <
	arch::ISA	_ISA_,
	class		_Type_>
class _Divisor;

template <>
class _Divisor<arch::ISA::SSE2, int32> {
public:
	_Divisor() noexcept = default;

	_Divisor(int32 __divisor) noexcept {
		__from_divisor(__divisor);
	}

	_Divisor(
		int32 __multiplier,
		int32 __shift,
		int32 __sign) noexcept
	{
		__init(__multiplier, __shift, __sign);
	}

	raze_nodiscard raze_always_inline __m128i sign() const noexcept {
		return _sign;
	}

	raze_nodiscard raze_always_inline __m128i shift() const noexcept {
		return _shift;
	}

	raze_nodiscard raze_always_inline __m128i multiplier() const noexcept {
		return _multiplier;
	}
private:
	raze_always_inline void __from_divisor(int32 __divisor) noexcept {
		auto __shift = int32(0), __multiplier = int32(0);
		const auto __absolute_divisor = std::abs(__divisor);

		if (static_cast<uint32>(__divisor) == 0x80000000u) {
			__multiplier = 0x80000001;
			__shift = 30;
		}
		else if (__absolute_divisor > 1) {
			__shift = 31 - math::count_leading_zero_bits(static_cast<uint32>(__absolute_divisor - 1));
			__multiplier = int32((int64(1) << (32 + __shift)) / __absolute_divisor - ((int64(1) << 32) - 1));
		}
		else {
			__multiplier = 1;
			__shift = 0;

			if (__divisor == 0)
				__multiplier /= __divisor;
		}

		__init(__multiplier, __shift, (__divisor < 0) ? -1 : 0);
	}

	raze_always_inline void __init(
		int32 __multiplier,
		int32 __shift,
		int32 __sign) noexcept 
	{
		_multiplier = _mm_set1_epi32(__multiplier);
		_sign		= _mm_set1_epi32(__sign);
		_shift		= _mm_cvtsi32_si128(__shift);
	}

	__m128i _sign;
	__m128i _shift;
	__m128i _multiplier;
};

template <>
class _Divisor<arch::ISA::SSE2, uint32> {
public:
	_Divisor() noexcept = default;

	_Divisor(uint32 __divisor) noexcept {
		__from_divisor(__divisor);
	}

	_Divisor(
		uint32 __multiplier,
		int32 __shift1,
		int32 __shift2) noexcept
	{
		__init(__multiplier, __shift1, __shift2);
	}

	raze_nodiscard raze_always_inline __m128i shift1() const noexcept {
		return _shift1;
	}

	raze_nodiscard raze_always_inline __m128i shift2() const noexcept {
		return _shift2;
	}

	raze_nodiscard raze_always_inline __m128i multiplier() const noexcept {
		return _multiplier;
	}
private:
	raze_always_inline void __from_divisor(uint32 __divisor) noexcept {
		auto __shift1		= uint32(0);
		auto __shift2		= uint32(0);
		auto __multiplier	= uint32(0);

		switch (__divisor) {
			case 0:
				__multiplier = __shift1 = __shift2 = 1 / __divisor;
				break;
			case 1:
				__multiplier = 1;
				__shift1 = __shift2 = 0;
				break;
			case 2:
				__multiplier = 1;
				__shift1 = 1;
				__shift2 = 0;
				break;
			default:
				{
					const auto __leading_zeros = 32 - math::count_leading_zero_bits(__divisor - 1u);

					__multiplier = 1 + uint32((uint64(uint32(
						__leading_zeros < 32 ? 1 << __leading_zeros : 0) - __divisor) << 32) / __divisor);

					__shift1 = 1; 
					__shift2 = __leading_zeros - 1;
				}
		}
		
		__init(__multiplier, __shift1, __shift2);
	}

	raze_always_inline void __init(
		uint32 __multiplier,
		int32 __shift1,
		int32 __shift2) noexcept
	{
		_multiplier = _mm_set1_epi32(__multiplier);
		_shift1		= _mm_set_epi32(0, 0, 0, __shift1);
		_shift2		= _mm_set_epi32(0, 0, 0, __shift2);
	}

	__m128i _shift2;
	__m128i _shift1;
	__m128i _multiplier;
};

template <>
class _Divisor<arch::ISA::SSE2, int16> {
public:
	_Divisor() noexcept = default;

	_Divisor(int16 __divisor) noexcept {
		__from_divisor(__divisor);
	}

	_Divisor(
		int16 __multiplier,
		int16 __shift,
		int16 __sign) noexcept
	{
		__init(__multiplier, __shift, __sign);
	}

	raze_nodiscard raze_always_inline __m128i sign() const noexcept {
		return _sign;
	}

	raze_nodiscard raze_always_inline __m128i shift() const noexcept {
		return _shift;
	}

	raze_nodiscard raze_always_inline __m128i multiplier() const noexcept {
		return _multiplier;
	}
private:
	raze_always_inline void __from_divisor(int16 __divisor) noexcept {
		auto __shift = int16(0), __multiplier = int16(0);
		const auto __absolute_divisor = std::abs(__divisor);

		if (static_cast<int16>(__divisor) == 0x8000u) {
			__multiplier = 0x8001;
			__shift = 14;
		}
		else if (__absolute_divisor > 1) {
			__shift = 15 - math::count_leading_zero_bits(uint32(__absolute_divisor - 1));
			__multiplier = int32((int32(1) << (16 + __shift)) / __absolute_divisor - ((int64(1) << 16) - 1));
		}
		else {
			__multiplier = 1;
			__shift = 0;

			if (__divisor == 0)
				__multiplier /= __divisor;
		}

		__init(__multiplier, __shift, (__divisor < 0) ? -1 : 0);
	}

	raze_always_inline void __init(
		int32 __multiplier,
		int32 __shift,
		int32 __sign) noexcept
	{
		_multiplier = _mm_set1_epi32(__multiplier);
		_sign = _mm_set1_epi32(__sign);
		_shift = _mm_cvtsi32_si128(__shift);
	}

	__m128i _sign;
	__m128i _shift;
	__m128i _multiplier;
};

template <>
class _Divisor<arch::ISA::SSE2, uint16> {
public:
	_Divisor() noexcept = default;

	_Divisor(uint16 __divisor) noexcept {
		__from_divisor(__divisor);
	}

	_Divisor(
		uint16 __multiplier,
		int16  __shift1,
		int16  __shift2) noexcept
	{
		__init(__multiplier, __shift1, __shift2);
	}

	raze_nodiscard raze_always_inline __m128i shift1() const noexcept {
		return _shift1;
	}

	raze_nodiscard raze_always_inline __m128i shift2() const noexcept {
		return _shift2;
	}

	raze_nodiscard raze_always_inline __m128i multiplier() const noexcept {
		return _multiplier;
	}
private:
	raze_always_inline void __from_divisor(uint16 __divisor) noexcept {
		auto __shift1		= uint16(0);
		auto __shift2		= uint16(0);
		auto __multiplier	= uint16(0);

		switch (__divisor) {
			case 0:
				__multiplier = __shift1 = __shift2 = 1u / __divisor;
				break;
			case 1:
				__multiplier = 1;
				__shift1 = __shift2 = 0;
				break;
			case 2:
				__multiplier = 1;
				__shift1 = 1;
				__shift2 = 0;
				break;
			default:
				{
					const auto __leading_zeros = 16 - math::count_leading_zero_bits(static_cast<uint16>(__divisor - 1));

					__multiplier = 1u + uint16((uint32((1u << __leading_zeros) - __divisor) << 16) / __divisor);

					__shift1 = 1; 
					__shift2 = __leading_zeros - 1;
				}
		}
		
		__init(__multiplier, __shift1, __shift2);
	}

	raze_always_inline void __init(
		uint32 __multiplier,
		int32 __shift1,
		int32 __shift2) noexcept
	{
		_multiplier = _mm_set1_epi32(__multiplier);
		_shift1		= _mm_set_epi32(0, 0, 0, __shift1);
		_shift2		= _mm_set_epi32(0, 0, 0, __shift2);
	}

	__m128i _shift2;
	__m128i _shift1;
	__m128i _multiplier;
};


__RAZE_DATAPAR_NAMESPACE_END
