#pragma once 

#include <src/raze/vx/hw/x86/cast/As.h>


__RAZE_VX_NAMESPACE_BEGIN 

template <
	arch::ISA	_ISA_,
	uint32		_Width_,
	class		_IntrinType_>
struct _Zero {
	raze_nodiscard raze_always_inline _IntrinType_ operator()() const noexcept {
		if constexpr (std::is_same_v<_IntrinType_, __m128i>) {
			return _mm_setzero_si128();
		}
		else if constexpr (std::is_same_v<_IntrinType_, __m128d>) {
			return _mm_setzero_pd();
		}
		else if constexpr (std::is_same_v<_IntrinType_, __m128>) {
			return _mm_setzero_ps();
		}
		else if constexpr (std::is_same_v<_IntrinType_, __m256i>) {
			return _mm256_setzero_si256();
		}
		else if constexpr (std::is_same_v<_IntrinType_, __m256d>) {
			return _mm256_setzero_pd();
		}
		else if constexpr (std::is_same_v<_IntrinType_, __m256>) {
			return _mm256_setzero_ps();
		}
		else if constexpr (std::is_same_v<_IntrinType_, __m512i>) {
			return _mm512_setzero_si512();
		}
		else if constexpr (std::is_same_v<_IntrinType_, __m512d>) {
			return _mm512_setzero_pd();
		}
		else if constexpr (std::is_same_v<_IntrinType_, __m512>) {
			return _mm512_setzero_ps();
		}
	}
};

__RAZE_VX_NAMESPACE_END
