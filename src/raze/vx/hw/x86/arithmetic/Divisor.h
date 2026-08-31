#pragma once 

#include <raze/arch/CpuFeature.h>
#include <raze/math/BitMath.h>
#include <src/raze/math/BitTestAndReset.h>

raze_disable_warning_msvc(4310)

__RAZE_VX_NAMESPACE_BEGIN

template <class	T>
class vector_divisor;

template <>
class vector_divisor<i32> {
public:
	vector_divisor() noexcept = default;

	vector_divisor(i32 divisor) noexcept {
		from_divisor(divisor);
	}

	vector_divisor(i32 multiplier, i32 shift, i32 sign) noexcept {
		init(multiplier, shift, sign);
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
	raze_always_inline void from_divisor(i32 divisor) noexcept {
		auto shift = i32(0), multiplier = i32(0);
		const auto absolute_divisor = std::abs(divisor);

		if (static_cast<u32>(divisor) == 0x80000000u) {
			multiplier = 0x80000001;
			shift = 30;
		}
		else if (absolute_divisor > 1) {
			ulong32 leading_zeros;
			_BitScanReverse(&leading_zeros, static_cast<u32>(absolute_divisor - 1));

			shift = leading_zeros;
			multiplier = i32((i64(1) << (32 + shift)) / absolute_divisor - ((i64(1) << 32) - 1));
		}
		else {
			multiplier = 1;
			shift = 0;

			if (divisor == 0) multiplier /= divisor;
		}

		init(multiplier, shift, (divisor < 0) ? -1 : 0);
	}

	raze_always_inline void init(i32 multiplier, i32 shift, i32 sign) noexcept {
		_multiplier = _mm_set1_epi32(multiplier);
		_sign		= _mm_set1_epi32(sign);
		_shift		= _mm_cvtsi32_si128(shift);
	}

	__m128i _sign;
	__m128i _shift;
	__m128i _multiplier;
};

template <>
class vector_divisor<u32> {
public:
	vector_divisor() noexcept = default;

	vector_divisor(u32 divisor) noexcept {
		from_divisor(divisor);
	}

	vector_divisor(u32 multiplier, i32 shift1, i32 shift2) noexcept {
		init(multiplier, shift1, shift2);
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
	raze_always_inline void from_divisor(u32 divisor) noexcept {
		auto shift1		= u32(0);
		auto shift2		= u32(0);
		auto multiplier	= u32(0);

		switch (divisor) {
			case 0:
				multiplier = shift1 = shift2 = 1 / divisor;
				break;
			case 1:
				multiplier = 1;
				shift1 = shift2 = 0;
				break;
			case 2:
				multiplier = 1;
				shift1 = 1;
				shift2 = 0;
				break;
			default:
				{
					ulong32 leading_zeros;
					_BitScanReverse(&leading_zeros, static_cast<u32>(divisor - 1u));
					++leading_zeros;

					multiplier = 1 + u32((u64(u32(
						leading_zeros < 32 ? 1 << leading_zeros : 0) - divisor) << 32) / divisor);

					shift1 = 1; 
					shift2 = leading_zeros - 1;
				}
		}
		
		init(multiplier, shift1, shift2);
	}

	raze_always_inline void init(u32 multiplier, i32 shift1, i32 shift2) noexcept {
		_multiplier = _mm_set1_epi32(static_cast<i32>(multiplier));
		_shift1		= _mm_set_epi32(0, 0, 0, shift1);
		_shift2		= _mm_set_epi32(0, 0, 0, shift2);
	}

	__m128i _shift2;
	__m128i _shift1;
	__m128i _multiplier;
};

template <>
class vector_divisor<i16> {
public:
	vector_divisor() noexcept = default;

	vector_divisor(i16 divisor) noexcept {
		from_divisor(divisor);
	}

	vector_divisor(i16 multiplier, i16 shift, i16 sign) noexcept {
		init(multiplier, shift, sign);
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
	raze_always_inline void from_divisor(i16 divisor) noexcept {
		auto shift = i32(0), multiplier = i32(0);
		const auto absolute_divisor = std::abs(divisor);

		if (static_cast<u16>(divisor) == 0x8000u) {
			multiplier = 0x8001;
			shift = 14;
		}
		else if (absolute_divisor > 1) {
			ulong32 leading_zeros;
			_BitScanReverse(&leading_zeros, static_cast<u16>(absolute_divisor - 1));

			shift = leading_zeros;
			multiplier = i32((i32(1) << (16 + shift)) / absolute_divisor - ((i64(1) << 16) - 1));
		}
		else {
			multiplier = 1;
			shift = 0;

			if (divisor == 0) multiplier /= divisor;
		}

		init(multiplier, shift, (divisor < 0) ? -1 : 0);
	}

	raze_always_inline void init(i16 multiplier, i16 shift, i16 sign) noexcept {
		_multiplier = _mm_set1_epi16(multiplier);
		_sign		= _mm_set1_epi32(sign);
		_shift		= _mm_cvtsi32_si128(shift);
	}

	__m128i _sign;
	__m128i _shift;
	__m128i _multiplier;
};

template <>
class vector_divisor<u16> {
public:
	vector_divisor() noexcept = default;

	vector_divisor(u16 divisor) noexcept {
		from_divisor(divisor);
	}

	vector_divisor(u16 multiplier, i32 shift1, i32 shift2) noexcept {
		init(multiplier, shift1, shift2);
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
	raze_always_inline void from_divisor(u16 divisor) noexcept {
		auto shift1		= u16(0);
		auto shift2		= u16(0);
		auto multiplier	= u16(0);

		switch (divisor) {
			case 0:
				multiplier = shift1 = shift2 = 1u / divisor;
				break;
			case 1:
				multiplier = 1;
				break;
			case 2:
				multiplier = 1;
				shift1 = 1;
				break;
			default:
				{
					ulong32 leading_zeros;
					_BitScanReverse(&leading_zeros, static_cast<u16>(divisor - 1u));
					++leading_zeros;

					const auto power = u16(1 << leading_zeros);

					multiplier = 1u + u16((u32(power - divisor) << 16) / divisor);
					shift1 = 1; 
					shift2 = leading_zeros - 1;
				}
		}
		
		init(multiplier, shift1, shift2);
	}

	raze_always_inline void init(u32 multiplier, i32 shift1, i32 shift2) noexcept {
		_multiplier = _mm_set1_epi16(static_cast<i16>(multiplier));
		_shift1		= _mm_setr_epi32(shift1, 0, 0, 0);
		_shift2		= _mm_setr_epi32(shift2, 0, 0, 0);
	}

	__m128i _shift1;
	__m128i _shift2;
	__m128i _multiplier;
};


__RAZE_VX_NAMESPACE_END
