#pragma once 

#include <src/raze/vx/hw/x86/merge/Select.h>
#include <src/raze/vx/hw/x86/memory/Load.h>


__RAZE_VX_NAMESPACE_BEGIN

template <arch::ISA	_ISA_, intrin_or_arithmetic_type _Tp_, arithmetic_type _Type_>
struct _Maskz_load {
	static constexpr auto __avx512vl = __has_avx512vl_support_v<_ISA_>;
	static constexpr auto __avx512bw = __has_avx512bw_support_v<_ISA_>;
	static constexpr auto __avx2 = __has_avx2_support_v<_ISA_>;
	static constexpr auto __avx = __has_avx_support_v<_ISA_>;

	template <raw_mask_type _Mask_>
	raze_nodiscard static raze_always_inline _Tp_ __loadu(const void* __mem, _Mask_ __mask) noexcept {
		if constexpr (sizeof(_Tp_) == 16) {
			if constexpr (__is_epi64_v<_Type_> || __is_epu64_v<_Type_>) {
				if constexpr (__avx512vl) return __as<_Tp_>(_mm_maskz_loadu_epi64(__mask, __mem));
				else if constexpr (__avx2) return __as<_Tp_>(_mm_maskload_epi64(static_cast<const i64*>(__mem), __as<__m128i>(__mask)));
				else if constexpr (__avx) return __as<_Tp_>(_mm_maskload_pd(static_cast<const f64*>(__mem), __as<__m128i>(__mask)));
			}
			else if constexpr (__is_epi32_v<_Type_> || __is_epu32_v<_Type_>) {
				if constexpr (__avx512vl) return __as<_Tp_>(_mm_maskz_loadu_epi32(__mask, __mem));
				else if constexpr (__avx2) return __as<_Tp_>(_mm_maskload_epi32(static_cast<const i32*>(__mem), __as<__m128i>(__mask)));
				else if constexpr (__avx) return __as<_Tp_>(_mm_maskload_ps(static_cast<const f32*>(__mem), __as<__m128i>(__mask)));
			}
			else if constexpr (__is_pd_v<_Type_>) {
				if constexpr (__avx512vl) return __as<_Tp_>(_mm_maskz_loadu_pd(__mask, __mem));
				else if constexpr (__avx) return __as<_Tp_>(_mm_maskload_pd(static_cast<const f64*>(__mem), __as<__m128i>(__mask)));
			}
			else if constexpr (__is_ps_v<_Type_>) {
				if constexpr (__avx512vl) return __as<_Tp_>(_mm_maskz_loadu_ps(__mask, __mem));
				else if constexpr (__avx) return __as<_Tp_>(_mm_maskload_ps(static_cast<const f32*>(__mem), __as<__m128i>(__mask)));
			}
			else if constexpr (__avx512bw) {
				if constexpr (__is_epi16_v<_Type_> || __is_epu16_v<_Type_>) return __as<_Tp_>(_mm_maskz_loadu_epi16(__mask, __mem));
				else if constexpr (__is_epi8_v<_Type_> || __is_epu8_v<_Type_>) return __as<_Tp_>(_mm_maskz_loadu_epi8(__mask, __mem));
			}
		}
		else if constexpr (sizeof(_Tp_) == 32) {
			if constexpr (__is_epi64_v<_Type_> || __is_epu64_v<_Type_>) {
				if constexpr (__avx512vl) return __as<_Tp_>(_mm256_maskz_loadu_epi64(__mask, __mem));
				else if constexpr (__avx2) return __as<_Tp_>(_mm256_maskload_epi64(static_cast<const i64*>(__mem), __as<__m256i>(__mask)));
				else return __as<_Tp_>(_mm256_maskload_pd(static_cast<const f64*>(__mem), __as<__m256i>(__mask)));
			}
			else if constexpr (__is_epi32_v<_Type_> || __is_epu32_v<_Type_>) {
				if constexpr (__avx512vl) return __as<_Tp_>(_mm256_maskz_loadu_epi32(__mask, __mem));
				else if constexpr (__avx2) return __as<_Tp_>(_mm256_maskload_epi32(static_cast<const i32*>(__mem), __as<__m256i>(__mask)));
				else return __as<_Tp_>(_mm256_maskload_ps(static_cast<const f32*>(__mem), __as<__m256i>(__mask)));
			}
			else if constexpr (__is_pd_v<_Type_>) {
				if constexpr (__avx512vl) return __as<_Tp_>(_mm256_maskz_loadu_pd(__mask, __mem));
				else return __as<_Tp_>(_mm256_maskload_pd(static_cast<const f64*>(__mem), __as<__m256i>(__mask)));
			}
			else if constexpr (__is_ps_v<_Type_>) {
				if constexpr (__avx512vl) return __as<_Tp_>(_mm256_maskz_loadu_ps(__mask, __mem));
				else return __as<_Tp_>(_mm256_maskload_ps(static_cast<const f32*>(__mem), __as<__m256i>(__mask)));
			}
			else if constexpr (__avx512bw) {
				if constexpr (__is_epi16_v<_Type_> || __is_epu16_v<_Type_>) return __as<_Tp_>(_mm256_maskz_loadu_epi16(__mask, __mem));
				else if constexpr (__is_epi8_v<_Type_> || __is_epu8_v<_Type_>) return __as<_Tp_>(_mm256_maskz_loadu_epi8(__mask, __mem));
			}
		}
		else if constexpr (sizeof(_Tp_) == 64) {
			if constexpr (__is_epi64_v<_Type_> || __is_epu64_v<_Type_>) return __as<_Tp_>(_mm512_maskz_loadu_epi64(__mask, __mem));
			else if constexpr (__is_epi32_v<_Type_> || __is_epu32_v<_Type_>) return __as<_Tp_>(_mm512_maskz_loadu_epi32(__mask, __mem));
			else if constexpr (__is_pd_v<_Type_>) return __as<_Tp_>(_mm512_maskz_loadu_pd(__mask, __mem));
			else if constexpr (__is_ps_v<_Type_>) return __as<_Tp_>(_mm512_maskz_loadu_ps(__mask, __mem));
			else if constexpr (__avx512bw) {
				if constexpr (__is_epi16_v<_Type_> || __is_epu16_v<_Type_>) return __as<_Tp_>(_mm512_maskz_loadu_epi16(__mask, __mem));
				else if constexpr (__is_epi8_v<_Type_> || __is_epu8_v<_Type_>) return __as<_Tp_>(_mm512_maskz_loadu_epi8(__mask, __mem));
			}
		}
		
		if (arithmetic_type<_Tp_>)
			return __mask ? *static_cast<const _Tp_*>(__mem) : 0;
		else
			return _Selectz<_ISA_, _Type_>()(_Load<_ISA_, _Tp_>()(__mem), __mask);
	}

	template <raw_mask_type _Mask_>
	raze_nodiscard static raze_always_inline _Tp_ __load(const void* __mem, _Mask_ __mask) noexcept {
		if constexpr (sizeof(_Tp_) == 16) {
			if constexpr (__is_epi64_v<_Type_> || __is_epu64_v<_Type_>) return __as<_Tp_>(_mm_maskz_load_epi64(__mask, __mem));
			else if constexpr (__is_epi32_v<_Type_> || __is_epu32_v<_Type_>) return __as<_Tp_>(_mm_maskz_load_epi32(__mask, __mem));
			else if constexpr (__is_pd_v<_Type_>) return __as<_Tp_>(_mm_maskz_load_pd(__mask, __mem));
			else if constexpr (__is_ps_v<_Type_>) return __as<_Tp_>(_mm_maskz_load_ps(__mask, __mem));
			else return __loadu(__mem, __mask);
		}
		else if constexpr (sizeof(_Tp_) == 32) {
			if constexpr (__is_epi64_v<_Type_> || __is_epu64_v<_Type_>) return __as<_Tp_>(_mm256_maskz_load_epi64(__mask, __mem));
			else if constexpr (__is_epi32_v<_Type_> || __is_epu32_v<_Type_>) return __as<_Tp_>(_mm256_maskz_load_epi32(__mask, __mem));
			else if constexpr (__is_pd_v<_Type_>) return __as<_Tp_>(_mm256_maskz_load_pd(__mask, __mem));
			else if constexpr (__is_ps_v<_Type_>) return __as<_Tp_>(_mm256_maskz_load_ps(__mask, __mem));
			else return __loadu(__mem, __mask);
		}
		else if constexpr (sizeof(_Tp_) == 64) {
			if constexpr (__is_epi64_v<_Type_> || __is_epu64_v<_Type_>) return __as<_Tp_>(_mm512_maskz_load_epi64(__mask, __mem));
			else if constexpr (__is_epi32_v<_Type_> || __is_epu32_v<_Type_>) return __as<_Tp_>(_mm512_maskz_load_epi32(__mask, __mem));
			else if constexpr (__is_pd_v<_Type_>) return __as<_Tp_>(_mm512_maskz_load_pd(__mask, __mem));
			else if constexpr (__is_ps_v<_Type_>) return __as<_Tp_>(_mm512_maskz_load_ps(__mask, __mem));
			else return __loadu(__mem, __mask);
		}

		if (arithmetic_type<_Tp_>)
			return __mask ? *static_cast<const _Tp_*>(__mem) : 0;
		else
			return _Selectz<_ISA_, _Type_>()(_Load<_ISA_, _Tp_>()(__mem), __mask);
	}

	template <raw_mask_type _Mask_, class _AlignPolicy_ = __unaligned_policy>
	raze_nodiscard raze_static_operator raze_always_inline _Tp_ operator()(const void* __mem, 
		_Mask_ __mask, _AlignPolicy_&& __policy = _AlignPolicy_{}) raze_const_operator noexcept
	{
		if constexpr (__is_aligned_v<_AlignPolicy_>) return __load(__mem, __mask);
		else return __loadu(__mem, __mask);

	}
};

__RAZE_VX_NAMESPACE_END
