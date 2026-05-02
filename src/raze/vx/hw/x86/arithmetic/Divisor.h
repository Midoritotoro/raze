#pragma once 

#include <raze/arch/CpuFeature.h>
#include <raze/math/BitMath.h>
#include <src/raze/math/BitTestAndReset.h>

raze_disable_warning_msvc(4310)

__RAZE_VX_NAMESPACE_BEGIN

template <class	_Type_>
class _Divisor;

template <>
class _Divisor<i32> {
public:
	_Divisor() noexcept = default;

	_Divisor(i32 __divisor) noexcept {
		__from_divisor(__divisor);
	}

	_Divisor(
		i32 __multiplier,
		i32 __shift,
		i32 __sign) noexcept
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
	raze_always_inline void __from_divisor(i32 __divisor) noexcept {
		auto __shift = i32(0), __multiplier = i32(0);
		const auto __absolute_divisor = std::abs(__divisor);

		if (static_cast<u32>(__divisor) == 0x80000000u) {
			__multiplier = 0x80000001;
			__shift = 30;
		}
		else if (__absolute_divisor > 1) {
			ulong32 __leading_zeros;
			_BitScanReverse(&__leading_zeros, static_cast<u32>(__absolute_divisor - 1));

			__shift = __leading_zeros;
			__multiplier = i32((i64(1) << (32 + __shift)) / __absolute_divisor - ((i64(1) << 32) - 1));
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
		i32 __multiplier,
		i32 __shift,
		i32 __sign) noexcept 
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
class _Divisor<u32> {
public:
	_Divisor() noexcept = default;

	_Divisor(u32 __divisor) noexcept {
		__from_divisor(__divisor);
	}

	_Divisor(
		u32 __multiplier,
		i32 __shift1,
		i32 __shift2) noexcept
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
	raze_always_inline void __from_divisor(u32 __divisor) noexcept {
		auto __shift1		= u32(0);
		auto __shift2		= u32(0);
		auto __multiplier	= u32(0);

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
					ulong32 __leading_zeros;
					_BitScanReverse(&__leading_zeros, static_cast<u32>(__divisor - 1u));
					++__leading_zeros;

					__multiplier = 1 + u32((u64(u32(
						__leading_zeros < 32 ? 1 << __leading_zeros : 0) - __divisor) << 32) / __divisor);

					__shift1 = 1; 
					__shift2 = __leading_zeros - 1;
				}
		}
		
		__init(__multiplier, __shift1, __shift2);
	}

	raze_always_inline void __init(
		u32 __multiplier,
		i32 __shift1,
		i32 __shift2) noexcept
	{
		_multiplier = _mm_set1_epi32(static_cast<i32>(__multiplier));
		_shift1		= _mm_set_epi32(0, 0, 0, __shift1);
		_shift2		= _mm_set_epi32(0, 0, 0, __shift2);
	}

	__m128i _shift2;
	__m128i _shift1;
	__m128i _multiplier;
};

template <>
class _Divisor<i16> {
public:
	_Divisor() noexcept = default;

	_Divisor(i16 __divisor) noexcept {
		__from_divisor(__divisor);
	}

	_Divisor(
		i16 __multiplier,
		i16 __shift,
		i16 __sign) noexcept
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
	raze_always_inline void __from_divisor(i16 __divisor) noexcept {
		auto __shift = i16(0), __multiplier = i16(0);
		const auto __absolute_divisor = std::abs(__divisor);

		if (static_cast<i16>(__divisor) == 0x8000u) {
			__multiplier = i16(0x8001);
			__shift = 14;
		}
		else if (__absolute_divisor > 1) {
			ulong32 __leading_zeros;
			_BitScanReverse(&__leading_zeros, static_cast<u16>(__absolute_divisor - 1));

			__shift = __leading_zeros;
			__multiplier = i32((i32(1) << (16 + __shift)) / __absolute_divisor - ((i64(1) << 16) - 1));
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
		i16 __multiplier,
		i16 __shift,
		i16 __sign) noexcept
	{
		_multiplier = _mm_set1_epi16(__multiplier);
		_sign		= _mm_set1_epi32(__sign);
		_shift		= _mm_cvtsi32_si128(__shift);
	}

	__m128i _sign;
	__m128i _shift;
	__m128i _multiplier;
};

template <>
class _Divisor<u16> {
public:
	_Divisor() noexcept = default;

	_Divisor(u16 __divisor) noexcept {
		__from_divisor(__divisor);
	}

	_Divisor(
		u16 __multiplier,
		i32  __shift1,
		i32  __shift2) noexcept
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
	raze_always_inline void __from_divisor(u16 __divisor) noexcept {
		auto __shift1		= u16(0);
		auto __shift2		= u16(0);
		auto __multiplier	= u16(0);

		switch (__divisor) {
			case 0:
				__multiplier = __shift1 = __shift2 = 1u / __divisor;
				break;
			case 1:
				__multiplier = 1;
				break;
			case 2:
				__multiplier = 1;
				__shift1 = 1;
				break;
			default:
				{
					ulong32 __leading_zeros;
					_BitScanReverse(&__leading_zeros, static_cast<u16>(__divisor - 1u));
					++__leading_zeros;

					const auto __power = u16(1 << __leading_zeros);

					__multiplier = 1u + u16((u32(__power - __divisor) << 16) / __divisor);
					__shift1 = 1; 
					__shift2 = __leading_zeros - 1;
				}
		}
		
		__init(__multiplier, __shift1, __shift2);
	}

	raze_always_inline void __init(
		u32 __multiplier,
		i32 __shift1,
		i32 __shift2) noexcept
	{
		_multiplier = _mm_set1_epi16(static_cast<i16>(__multiplier));
		_shift1		= _mm_setr_epi32(__shift1, 0, 0, 0);
		_shift2		= _mm_setr_epi32(__shift2, 0, 0, 0);
	}

	__m128i _shift1;
	__m128i _shift2;
	__m128i _multiplier;
};


__RAZE_VX_NAMESPACE_END
