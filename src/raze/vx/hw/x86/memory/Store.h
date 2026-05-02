#pragma once 

#include <src/raze/vx/hw/Cast.h>
#include <src/raze/vx/hw/x86/memory/AlignmentPolicy.h>


__RAZE_VX_NAMESPACE_BEGIN

template <arch::ISA	_ISA_>
struct _Store {
	template <intrin_or_arithmetic_type	_Tp_>
	static raze_always_inline void __store(void* __mem, _Tp_ __v) noexcept {
		if constexpr (std::is_same_v<_Tp_, __m128i>) return _mm_store_si128(reinterpret_cast<__m128i*>(__mem), __v);
		else if constexpr (std::is_same_v<_Tp_, __m128d>) return _mm_store_pd(reinterpret_cast<f64*>(__mem), __v);
		else if constexpr (std::is_same_v<_Tp_, __m128>) return _mm_store_ps(reinterpret_cast<f32*>(__mem), __v);
		else if constexpr (std::is_same_v<_Tp_, __m256i>) return _mm256_store_si256(reinterpret_cast<__m256i*>(__mem), __v);
		else if constexpr (std::is_same_v<_Tp_, __m256d>) return _mm256_store_pd(reinterpret_cast<f64*>(__mem), __v);
		else if constexpr (std::is_same_v<_Tp_, __m256>) return _mm256_store_ps(reinterpret_cast<f32*>(__mem), __v);
		else if constexpr (std::is_same_v<_Tp_, __m512i>) return _mm512_store_si512(__mem, __v);
		else if constexpr (std::is_same_v<_Tp_, __m512d>) return _mm512_store_pd(__mem, __v);
		else if constexpr (std::is_same_v<_Tp_, __m512>) return _mm512_store_ps(__mem, __v);
		else *static_cast<_Tp_*>(__mem) = __v;
	}

	template <intrin_or_arithmetic_type	_Tp_>
	static raze_always_inline void __storeu(void* __mem, _Tp_ __v) noexcept {
		if constexpr (std::is_same_v<_Tp_, __m128i>) return _mm_storeu_si128(reinterpret_cast<__m128i*>(__mem), __v);
		else if constexpr (std::is_same_v<_Tp_, __m128d>) return _mm_storeu_pd(reinterpret_cast<f64*>(__mem), __v);
		else if constexpr (std::is_same_v<_Tp_, __m128>) return _mm_storeu_ps(reinterpret_cast<f32*>(__mem), __v);
		else if constexpr (std::is_same_v<_Tp_, __m256i>) return _mm256_storeu_si256(reinterpret_cast<__m256i*>(__mem), __v);
		else if constexpr (std::is_same_v<_Tp_, __m256d>) return _mm256_storeu_pd(reinterpret_cast<f64*>(__mem), __v);
		else if constexpr (std::is_same_v<_Tp_, __m256>) return _mm256_storeu_ps(reinterpret_cast<f32*>(__mem), __v);
		else if constexpr (std::is_same_v<_Tp_, __m512i>) return _mm512_storeu_si512(__mem, __v);
		else if constexpr (std::is_same_v<_Tp_, __m512d>) return _mm512_storeu_pd(__mem, __v);
		else if constexpr (std::is_same_v<_Tp_, __m512>) return _mm512_storeu_ps(__mem, __v);
		else *static_cast<_Tp_*>(__mem) = __v;
	}

	template <intrin_or_arithmetic_type	_Tp_, class	_AlignPolicy_ = __unaligned_policy>
	raze_static_operator raze_always_inline void operator()(void* __mem,
		_Tp_ __v, _AlignPolicy_&& = _AlignPolicy_{}) raze_const_operator noexcept
	{
		if constexpr (__is_aligned_v<_AlignPolicy_>) __store(__mem, __v);
		else __storeu(__mem, __v);
	}
};

__RAZE_VX_NAMESPACE_END
