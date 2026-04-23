#pragma once 

#include <src/raze/vx/hw/x86/arithmetic/Sub.h>
#include <src/raze/vx/hw/x86/compare/Equal.h>
#include <src/raze/vx/hw/x86/mask/ToBitmask.h>


__RAZE_VX_NAMESPACE_BEGIN

template <arch::ISA	_ISA_>
struct _Test_all_ones {
	template <intrin_or_arithmetic_type _Tp_>
	raze_nodiscard raze_always_inline bool operator()(_Tp_ __x) const noexcept {
		/*if constexpr (sizeof(_Tp_) == 16) {
			if constexpr (__has_sse41_support_v<_ISA_>) {
				const auto __ones = _Equal<arch::ISA::SSE41, int64>()(__x, __x);
				return _mm_testc_si128(__as<__m128i>(__x), __as<__m128i>(__ones));
			}
			else {
				const auto __ones = _Equal<arch::ISA::SSE2, int32>()(__x, __x);
				const auto __compared = _Equal<arch::ISA::SSE2, int32>()(__x, __ones);

				return _To_bitmask<arch::ISA::SSE2, int32>()(__compared) == 0xF;
			}
		}
		else if constexpr (sizeof(_Tp_) == 32) {
			const auto __ones = _Equal<arch::ISA::AVX, 256, double>()(__x, __x);
			return _mm256_testc_si256(__as<__m256i>(__x), __as<__m256i>(__ones));
		}
		else {

		}*/
	}
};


__RAZE_VX_NAMESPACE_END
