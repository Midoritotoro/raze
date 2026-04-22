#pragma once 

#include <src/raze/vx/hw/Cast.h>
#include <src/raze/vx/hw/x86/memory/AlignmentPolicy.h>


__RAZE_VX_NAMESPACE_BEGIN

template <arch::ISA	_ISA_>
struct _Store {
	template <intrin_or_arithmetic_type	_Type_>
	static raze_always_inline void __store_aligned(
		void*	__memory,
		_Type_	__value) noexcept
	{
		if constexpr (std::is_same_v<_Type_, __m128i>) return _mm_store_si128(reinterpret_cast<__m128i*>(__memory), __value);
		else if constexpr (std::is_same_v<_Type_, __m128d>) return _mm_store_pd(reinterpret_cast<double*>(__memory), __value);
		else if constexpr (std::is_same_v<_Type_, __m128>) return _mm_store_ps(reinterpret_cast<float*>(__memory), __value);
		else if constexpr (std::is_same_v<_Type_, __m256i>) return _mm256_store_si256(reinterpret_cast<__m256i*>(__memory), __value);
		else if constexpr (std::is_same_v<_Type_, __m256d>) return _mm256_store_pd(reinterpret_cast<double*>(__memory), __value);
		else if constexpr (std::is_same_v<_Type_, __m256>) return _mm256_store_ps(reinterpret_cast<float*>(__memory), __value);
		else if constexpr (std::is_same_v<_Type_, __m512i>) return _mm512_store_si512(__memory, __value);
		else if constexpr (std::is_same_v<_Type_, __m512d>) return _mm512_store_pd(__memory, __value);
		else if constexpr (std::is_same_v<_Type_, __m512>) return _mm512_store_ps(__memory, __value);
		else *static_cast<_Type_*>(__memory) = __value;
	}

	template <intrin_or_arithmetic_type	_Type_>
	static raze_always_inline void __store_unaligned(
		void*	__memory,
		_Type_	__value) noexcept
	{
		if constexpr (std::is_same_v<_Type_, __m128i>) return _mm_storeu_si128(reinterpret_cast<__m128i*>(__memory), __value);
		else if constexpr (std::is_same_v<_Type_, __m128d>) return _mm_storeu_pd(reinterpret_cast<double*>(__memory), __value);
		else if constexpr (std::is_same_v<_Type_, __m128>) return _mm_storeu_ps(reinterpret_cast<float*>(__memory), __value);
		else if constexpr (std::is_same_v<_Type_, __m256i>) return _mm256_storeu_si256(reinterpret_cast<__m256i*>(__memory), __value);
		else if constexpr (std::is_same_v<_Type_, __m256d>) return _mm256_storeu_pd(reinterpret_cast<double*>(__memory), __value);
		else if constexpr (std::is_same_v<_Type_, __m256>) return _mm256_storeu_ps(reinterpret_cast<float*>(__memory), __value);
		else if constexpr (std::is_same_v<_Type_, __m512i>) return _mm512_storeu_si512(__memory, __value);
		else if constexpr (std::is_same_v<_Type_, __m512d>) return _mm512_storeu_pd(__memory, __value);
		else if constexpr (std::is_same_v<_Type_, __m512>) return _mm512_storeu_ps(__memory, __value);
		else *static_cast<_Type_*>(__memory) = __value;
	}

	template <
		intrin_or_arithmetic_type	_Type_,
		class						_AlignmentPolicy_ = __unaligned_policy>
	raze_static_operator raze_always_inline void operator()(
		void*	__address,
		_Type_	__value,
		_AlignmentPolicy_&& = _AlignmentPolicy_{}) raze_const_operator noexcept
	{
		if constexpr (std::remove_cvref_t<_AlignmentPolicy_>::__alignment)
			__store_aligned(__address, __value);
		else
			__store_unaligned(__address, __value);
	}
};

__RAZE_VX_NAMESPACE_END
