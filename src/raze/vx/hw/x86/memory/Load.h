#pragma once 

#include <src/raze/vx/hw/Cast.h>
#include <src/raze/vx/hw/x86/memory/AlignmentPolicy.h>

__RAZE_VX_NAMESPACE_BEGIN



template <
	arch::ISA					_ISA_,
	intrin_or_arithmetic_type	_Type_>
struct _Load {
	template <class _AlignmentPolicy_ = __unaligned_policy>
	raze_nodiscard raze_static_operator raze_always_inline _Type_ operator()(
		const void* __address,
		_AlignmentPolicy_ && = _AlignmentPolicy_{}) raze_const_operator noexcept
	{
		if constexpr (intrin_type<_Type_>) {
			if constexpr (std::remove_cvref_t<_AlignmentPolicy_>::__alignment) {
				if constexpr (std::is_same_v<_Type_, __m128i>) {
					return _mm_load_si128(reinterpret_cast<const __m128i*>(__address));
				}
				else if constexpr (std::is_same_v<_Type_, __m128d>) {
					return _mm_load_pd(reinterpret_cast<const double*>(__address));
				}
				else if constexpr (std::is_same_v<_Type_, __m128>) {
					return _mm_load_ps(reinterpret_cast<const float*>(__address));
				}
				else if constexpr (std::is_same_v<_Type_, __m256i>) {
					return _mm256_load_si256(reinterpret_cast<const __m256i*>(__address));
				}
				else if constexpr (std::is_same_v<_Type_, __m256d>) {
					return _mm256_load_pd(reinterpret_cast<const double*>(__address));
				}
				else if constexpr (std::is_same_v<_Type_, __m256>) {
					return _mm256_load_ps(reinterpret_cast<const float*>(__address));
				}
				else if constexpr (std::is_same_v<_Type_, __m512i>) {
					return _mm512_load_si512(__address);
				}
				else if constexpr (std::is_same_v<_Type_, __m512d>) {
					return _mm512_load_pd(__address);
				}
				else if constexpr (std::is_same_v<_Type_, __m512>) {
					return _mm512_load_ps(__address);
				}
			}
			else {
				if constexpr (sizeof(_Type_) == 16) {
					if constexpr (__has_sse3_support_v<_ISA_>) {
						return _mm_lddqu_si128(reinterpret_cast<const __m128i*>(__address));
					}
					else if constexpr (std::is_same_v<_Type_, __m128i>) {
						return _mm_loadu_si128(reinterpret_cast<const __m128i*>(__address));
					}
					else if constexpr (std::is_same_v<_Type_, __m128d>) {
						return _mm_loadu_pd(reinterpret_cast<const double*>(__address));
					}
					else if constexpr (std::is_same_v<_Type_, __m128>) {
						return _mm_loadu_ps(reinterpret_cast<const float*>(__address));
					}
				}
				else if constexpr (sizeof(_Type_) == 32) {
					if constexpr (__has_avx2_support_v<_ISA_>) {
						return _mm256_lddqu_si256(reinterpret_cast<const __m256i*>(__address));
					}
					else if constexpr (std::is_same_v<_Type_, __m256i>) {
						return _mm256_loadu_si256(reinterpret_cast<const __m256i*>(__address));
					}
					else if constexpr (std::is_same_v<_Type_, __m256d>) {
						return _mm256_loadu_pd(reinterpret_cast<const double*>(__address));
					}
					else if constexpr (std::is_same_v<_Type_, __m256>) {
						return _mm256_loadu_ps(reinterpret_cast<const float*>(__address));
					}
				}
				else if constexpr (sizeof(_Type_) == 64) {
					if constexpr (std::is_same_v<_Type_, __m512i>) {
						return _mm512_loadu_si512(reinterpret_cast<const __m512i*>(__address));
					}
					else if constexpr (std::is_same_v<_Type_, __m512d>) {
						return _mm512_loadu_pd(reinterpret_cast<const double*>(__address));
					}
					else if constexpr (std::is_same_v<_Type_, __m512>) {
						return _mm512_loadu_ps(reinterpret_cast<const float*>(__address));
					}
				}
			}
		}

		return *static_cast<const _Type_*>(__address);
	}
};

__RAZE_VX_NAMESPACE_END
