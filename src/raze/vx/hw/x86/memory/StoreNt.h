#pragma once 

#include <src/raze/vx/hw/Cast.h>
#include <src/raze/vx/hw/x86/memory/AlignmentPolicy.h>


__RAZE_VX_NAMESPACE_BEGIN

template <arch::ISA	_ISA_>
struct _Store_nt {
	template <intrin_or_arithmetic_type	_Tp_>
	raze_static_operator raze_no_stack_protector raze_always_inline void operator()(void* raze_restrict __mem, _Tp_ __v) raze_const_operator noexcept {
		if constexpr (std::is_same_v<_Tp_, __m128i>) return _mm_stream_si128(reinterpret_cast<__m128i*>(__mem), __v);
		else if constexpr (std::is_same_v<_Tp_, __m128d>) return _mm_stream_pd(reinterpret_cast<f64*>(__mem), __v);
		else if constexpr (std::is_same_v<_Tp_, __m128>) return _mm_stream_ps(reinterpret_cast<f32*>(__mem), __v);
		else if constexpr (std::is_same_v<_Tp_, __m256i>) return _mm256_stream_si256(reinterpret_cast<__m256i*>(__mem), __v);
		else if constexpr (std::is_same_v<_Tp_, __m256d>) return _mm256_stream_pd(reinterpret_cast<f64*>(__mem), __v);
		else if constexpr (std::is_same_v<_Tp_, __m256>) return _mm256_stream_ps(reinterpret_cast<f32*>(__mem), __v);
		else if constexpr (std::is_same_v<_Tp_, __m512i>) return _mm512_stream_si512(__mem, __v);
		else if constexpr (std::is_same_v<_Tp_, __m512d>) return _mm512_stream_pd(__mem, __v);
		else if constexpr (std::is_same_v<_Tp_, __m512>) return _mm512_stream_ps(__mem, __v);
		else *static_cast<_Tp_*>(__mem) = __v;
	}
};

__RAZE_VX_NAMESPACE_END
