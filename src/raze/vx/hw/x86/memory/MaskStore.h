#pragma once 

#include <src/raze/vx/hw/x86/merge/Select.h>
#include <src/raze/vx/hw/x86/memory/Load.h>
#include <src/raze/vx/hw/x86/memory/Store.h>


__RAZE_VX_NAMESPACE_BEGIN

template <arch::ISA	_ISA_, class _Type_>
struct _Mask_store {
	static constexpr auto __avx512vl = __has_avx512vl_support_v<_ISA_>;
	static constexpr auto __avx512bw = __has_avx512bw_support_v<_ISA_>;
	static constexpr auto __avx2 = __has_avx2_support_v<_ISA_>;
	static constexpr auto __avx = __has_avx_support_v<_ISA_>;

	template <intrin_or_arithmetic_type _Tp_, raw_mask_type _Mask_>
	raze_static_operator raze_always_inline void __store(void* __mem, _Mask_ __mask, _Tp_ __x) raze_const_operator noexcept {
		if constexpr (sizeof(_Tp_) == 16) {
			if constexpr (__is_epi64_v<_Type_> || __is_epu64_v<_Type_>) return _mm_mask_store_epi64(__mem, __mask, __as<__m128i>(__x));
			if constexpr (__is_epi32_v<_Type_> || __is_epu32_v<_Type_>) return _mm_mask_store_epi32(__mem, __mask, __as<__m128i>(__x));
			else if constexpr (__is_ps_v<_Type_>) return _mm_mask_store_ps(__mem, __mask, __as<__m128>(__x));
			else if constexpr (__is_pd_v<_Type_>) return _mm_mask_store_pd(__mem, __mask, __as<__m128d>(__x));
			else return __storeu(__mem, __mask, __x);
		}
		else if constexpr (sizeof(_Tp_) == 32) {
			if constexpr (__is_epi64_v<_Type_> || __is_epu64_v<_Type_>) return _mm256_mask_store_epi64(__mem, __mask, __as<__m256i>(__x));
			if constexpr (__is_epi32_v<_Type_> || __is_epu32_v<_Type_>) return _mm256_mask_store_epi32(__mem, __mask, __as<__m256i>(__x));
			else if constexpr (__is_ps_v<_Type_>) return _mm256_mask_store_ps(__mem, __mask, __as<__m256>(__x));
			else if constexpr (__is_pd_v<_Type_>) return _mm256_mask_store_pd(__mem, __mask, __as<__m256d>(__x));
			else return __storeu(__mem, __mask, __x);
		}
		else if constexpr (sizeof(_Tp_) == 64) {
			if constexpr (__is_epi64_v<_Type_> || __is_epu64_v<_Type_>) return _mm512_mask_store_epi64(__mem, __mask, __as<__m512i>(__x));
			if constexpr (__is_epi32_v<_Type_> || __is_epu32_v<_Type_>) return _mm512_mask_store_epi32(__mem, __mask, __as<__m512i>(__x));
			else if constexpr (__is_ps_v<_Type_>) return _mm512_mask_store_ps(__mem, __mask, __as<__m512>(__x));
			else if constexpr (__is_pd_v<_Type_>) return _mm512_mask_store_pd(__mem, __mask, __as<__m512d>(__x));
			else return __storeu(__mem, __mask, __x);
		}

		if constexpr (arithmetic_type<_Tp_>) { if (__mask) *static_cast<_Tp_*>(__mem) = __x; }
		else { _Store<_ISA_>()(_Select<_ISA_, _Type_>()(__x, _Load<_ISA_, _Tp_>()(__mem), __mask)); }
	}

	template <intrin_or_arithmetic_type _Tp_, raw_mask_type _Mask_>
	raze_static_operator raze_always_inline void __storeu(void* __mem, _Mask_ __mask, _Tp_ __x) raze_const_operator noexcept {
		if constexpr (sizeof(_Tp_) == 16) {
			if constexpr (__is_epi64_v<_Type_> || __is_epu64_v<_Type_>) {
				if constexpr (__avx512vl) return _mm_mask_storeu_epi64(__mem, __mask, __as<__m128i>(__x));
				else if constexpr (__avx2) return _mm_maskstore_epi64(reinterpret_cast<i64*>(__mem), __as<__m128i>(__mask), __as<__m128i>(__x));
				else if constexpr (__avx) return _mm_maskstore_pd(reinterpret_cast<f64*>(__mem), __as<__m128i>(__mask), __as<__m128d>(__x));
			}
			if constexpr (__is_epi32_v<_Type_> || __is_epu32_v<_Type_>) {
				if constexpr (__avx512vl) return _mm_mask_storeu_epi32(__mem, __mask, __as<__m128i>(__x));
				else if constexpr (__avx2) return _mm_maskstore_epi32(reinterpret_cast<i32*>(__mem), __as<__m128i>(__mask), __as<__m128i>(__x));
				else if constexpr (__avx) return _mm_maskstore_ps(reinterpret_cast<f32*>(__mem), __as<__m128i>(__mask), __as<__m128>(__x));
			}
			else if constexpr (__is_ps_v<_Type_>) {
				if constexpr (__avx512vl) return _mm_mask_storeu_ps(__mem, __mask, __as<__m128>(__x));
				else if constexpr (__avx) return _mm_maskstore_ps(reinterpret_cast<f32*>(__mem), __as<__m128i>(__mask), __as<__m128>(__x));
			}
			else if constexpr (__is_pd_v<_Type_>) {
				if constexpr (__avx512vl) return _mm_mask_storeu_pd(__mem, __mask, __as<__m128d>(__x));
				else if constexpr (__avx) return _mm_maskstore_pd(reinterpret_cast<f64*>(__mem), __as<__m128i>(__mask), __as<__m128d>(__x));
			}
			else if constexpr (__avx512bw && __avx512vl) {
				if constexpr (__is_epi16_v<_Type_> || __is_epu16_v<_Type_>) return _mm_mask_storeu_epi16(__mem, __mask, __as<__m128i>(__x));
				if constexpr (__is_epi8_v<_Type_> || __is_epu8_v<_Type_>) return _mm_mask_storeu_epi8(__mem, __mask, __as<__m128i>(__x));
			}
		}
		else if constexpr (sizeof(_Tp_) == 32) {
			if constexpr (__is_epi64_v<_Type_> || __is_epu64_v<_Type_>) {
				if constexpr (__avx512vl) return _mm256_mask_storeu_epi64(__mem, __mask, __as<__m256i>(__x));
				else if constexpr (__avx2) return _mm256_maskstore_epi64(reinterpret_cast<i64*>(__mem), __as<__m256i>(__mask), __as<__m256i>(__x));
				else return _mm256_maskstore_pd(reinterpret_cast<f64*>(__mem), __as<__m256i>(__mask), __as<__m256d>(__x));
			}
			if constexpr (__is_epi32_v<_Type_> || __is_epu32_v<_Type_>) {
				if constexpr (__avx512vl) return _mm256_mask_storeu_epi32(__mem, __mask, __as<__m256i>(__x));
				else if constexpr (__avx2) return _mm256_maskstore_epi32(reinterpret_cast<i32*>(__mem), __as<__m256i>(__mask), __as<__m256i>(__x));
				else return _mm256_maskstore_ps(reinterpret_cast<f32*>(__mem), __as<__m256i>(__mask), __as<__m256>(__x));
			}
			else if constexpr (__is_ps_v<_Type_>) {
				if constexpr (__avx512vl) return _mm256_mask_storeu_ps(__mem, __mask, __as<__m256>(__x));
				else return _mm256_maskstore_ps(reinterpret_cast<f32*>(__mem), __as<__m256i>(__mask), __as<__m256>(__x));
			}
			else if constexpr (__is_pd_v<_Type_>) {
				if constexpr (__avx512vl) return _mm256_mask_storeu_pd(__mem, __mask, __as<__m256d>(__x));
				else return _mm256_maskstore_pd(reinterpret_cast<f64*>(__mem), __as<__m256i>(__mask), __as<__m256d>(__x));
			}
			else if constexpr (__avx512bw && __avx512vl) {
				if constexpr (__is_epi16_v<_Type_> || __is_epu16_v<_Type_>) return _mm256_mask_storeu_epi16(__mem, __mask, __as<__m256i>(__x));
				if constexpr (__is_epi8_v<_Type_> || __is_epu8_v<_Type_>) return _mm256_mask_storeu_epi8(__mem, __mask, __as<__m256i>(__x));
			}
		}
		else if constexpr (sizeof(_Tp_) == 64) {
			if constexpr (__is_epi64_v<_Type_> || __is_epu64_v<_Type_>) return _mm512_mask_storeu_epi64(__mem, __mask, __as<__m512i>(__x));
			if constexpr (__is_epi32_v<_Type_> || __is_epu32_v<_Type_>) return _mm512_mask_storeu_epi32(__mem, __mask, __as<__m512i>(__x));
			else if constexpr (__is_ps_v<_Type_>) return _mm512_mask_storeu_ps(__mem, __mask, __as<__m512>(__x));
			else if constexpr (__is_pd_v<_Type_>) return _mm512_mask_storeu_pd(__mem, __mask, __as<__m512d>(__x));
			else if constexpr (__avx512bw) {
				if constexpr (__is_epi16_v<_Type_> || __is_epu16_v<_Type_>) return _mm512_mask_storeu_epi16(__mem, __mask, __as<__m512i>(__x));
				if constexpr (__is_epi8_v<_Type_> || __is_epu8_v<_Type_>) return _mm512_mask_storeu_epi8(__mem, __mask, __as<__m512i>(__x));
			}
		}

		if constexpr (arithmetic_type<_Tp_>) { if (__mask) *static_cast<_Tp_*>(__mem) = __x; }
		else { _Store<_ISA_>()(_Select<_ISA_, _Type_>()(__x, _Load<_ISA_, _Tp_>()(__mem), __mask)); }
	}

	template <intrin_or_arithmetic_type _Tp_, raw_mask_type _Mask_, class _AligntPolicy_ = __unaligned_policy>
	raze_static_operator raze_always_inline void operator()(void* __mem, 
		_Mask_ __mask, _Tp_ __x, _AligntPolicy_&& __policy = _AligntPolicy_{}) raze_const_operator noexcept
	{
		if constexpr (__is_aligned_v<_ISA_>) __store(__mem, __mask, __x);
		else __storeu(__mem, __mask, __x);
	}
};

template <arch::ISA  _ISA_, arithmetic_type _Type_>
inline constexpr bool __is_native_mask_store_supported_v = (__has_avx_support_v<_ISA_> && sizeof(_Type_) >= 4) || __has_avx512bw_support_v<_ISA_>;

__RAZE_VX_NAMESPACE_END
