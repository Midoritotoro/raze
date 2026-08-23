#pragma once 

#include <src/raze/vx/hw/x86/merge/Select.h>
#include <src/raze/vx/hw/x86/memory/Load.h>
#include <src/raze/vx/hw/x86/memory/Store.h>

__RAZE_VX_NAMESPACE_BEGIN

template <arch::ISA	_ISA_, arithmetic_type _Type_, bool _Safe_ = false>
struct _Mask_load {
	static constexpr auto __avx512vl = has_avx512vl<_ISA_>;
	static constexpr auto __avx512bw = has_avx512bw<_ISA_>;

	template <raw_mask_type _Mask_, intrin_or_arithmetic_type _Tp_>
	raze_nodiscard static raze_always_inline _Tp_ __loadu(const void* raze_restrict __mem, _Mask_ __mask, _Tp_ __src) noexcept {
		if constexpr (sizeof(_Tp_) == 16 && __avx512vl) {
			if constexpr (__is_epi64_v<_Type_> || __is_epu64_v<_Type_>) return __as<_Tp_>(_mm_mask_loadu_epi64(__as<__m128i>(__src), __mask, __mem));
			else if constexpr (__is_epi32_v<_Type_> || __is_epu32_v<_Type_>) return __as<_Tp_>(_mm_mask_loadu_epi32(__as<__m128i>(__src), __mask, __mem));
			else if constexpr (__is_pd_v<_Type_>) return __as<_Tp_>(_mm_mask_loadu_pd(__as<__m128d>(__src), __mask, __mem));
			else if constexpr (__is_ps_v<_Type_>) return __as<_Tp_>(_mm_mask_loadu_ps(__as<__m128>(__src), __mask, __mem));
			else if constexpr (__avx512bw) {
				if constexpr (__is_epi16_v<_Type_> || __is_epu16_v<_Type_>) return __as<_Tp_>(_mm_mask_loadu_epi16(__as<__m128i>(__src), __mask, __mem));
				else if constexpr (__is_epi8_v<_Type_> || __is_epu8_v<_Type_>) return __as<_Tp_>(_mm_mask_loadu_epi8(__as<__m128i>(__src), __mask, __mem));
			}
		}
		else if constexpr (sizeof(_Tp_) == 32 && __avx512vl) {
			if constexpr (__is_epi64_v<_Type_> || __is_epu64_v<_Type_>) return __as<_Tp_>(_mm256_mask_loadu_epi64(__as<__m256i>(__src), __mask, __mem));
			else if constexpr (__is_epi32_v<_Type_> || __is_epu32_v<_Type_>) return __as<_Tp_>(_mm256_mask_loadu_epi32(__as<__m256i>(__src), __mask, __mem));
			else if constexpr (__is_pd_v<_Type_>) return __as<_Tp_>(_mm256_mask_loadu_pd(__as<__m256d>(__src), __mask, __mem));
			else if constexpr (__is_ps_v<_Type_>) return __as<_Tp_>(_mm256_mask_loadu_ps(__as<__m256>(__src), __mask, __mem));
			else if constexpr (__avx512bw) {
				if constexpr (__is_epi16_v<_Type_> || __is_epu16_v<_Type_>) return __as<_Tp_>(_mm256_mask_loadu_epi16(__as<__m256i>(__src), __mask, __mem));
				else if constexpr (__is_epi8_v<_Type_> || __is_epu8_v<_Type_>) return __as<_Tp_>(_mm256_mask_loadu_epi8(__as<__m256i>(__src), __mask, __mem));
			}
		}
		else if constexpr (sizeof(_Tp_) == 64) {
			if constexpr (__is_epi64_v<_Type_> || __is_epu64_v<_Type_>) return __as<_Tp_>(_mm512_mask_loadu_epi64(__as<__m512i>(__src), __mask, __mem));
			else if constexpr (__is_epi32_v<_Type_> || __is_epu32_v<_Type_>) return __as<_Tp_>(_mm512_mask_loadu_epi32(__as<__m512i>(__src), __mask, __mem));
			else if constexpr (__is_pd_v<_Type_>) return __as<_Tp_>(_mm512_mask_loadu_pd(__as<__m512d>(__src), __mask, __mem));
			else if constexpr (__is_ps_v<_Type_>) return __as<_Tp_>(_mm512_mask_loadu_ps(__as<__m512>(__src), __mask, __mem));
			else if constexpr (__avx512bw) {
				if constexpr (__is_epi16_v<_Type_> || __is_epu16_v<_Type_>) return __as<_Tp_>(_mm512_mask_loadu_epi16(__as<__m512i>(__src), __mask, __mem));
				else if constexpr (__is_epi8_v<_Type_> || __is_epu8_v<_Type_>) return __as<_Tp_>(_mm512_mask_loadu_epi8(__as<__m512i>(__src), __mask, __mem));
			}
		}
		
		if constexpr (arithmetic_type<_Tp_>) return __mask ? *static_cast<const _Tp_*>(__mem) : __src;
		else {
			if constexpr (_Safe_) {
				constexpr auto __size = sizeof(_Tp_) / sizeof(_Type_);
				alignas(sizeof(_Tp_)) _Type_ __array[__size];
				_Store<_ISA_>()(__array, __src);

				if constexpr (intrin_type<_Mask_>) {
					alignas(sizeof(_Tp_)) typename IntegerForSizeof<_Type_>::Signed __marray[__size];
					_Store<_ISA_>()(__marray, __mask, __aligned_policy{});

					for (auto __i = 0; __i < __size; ++__i)
						__array[__i] = __marray[__i] == 0 ? __array[__i] : static_cast<const _Type_*>(__mem)[__i];
				}
				else {
					for (auto __i = 0; __i < __size; ++__i)
						__array[__i] = math::__bit_test(__mask, __i) ? static_cast<const _Type_*>(__mem)[__i] : __array[__i];
				}

				return _Load<_ISA_, _Tp_>()(__array, __aligned_policy{});
			}
			else {
				return _Select<_ISA_, _Type_>()(_Load<_ISA_, _Tp_>()(__mem), __src, __mask);
			}
		}
	}

	template <raw_mask_type _Mask_, intrin_or_arithmetic_type _Tp_>
	raze_nodiscard static raze_always_inline _Tp_ __load(const void* raze_restrict __mem, _Mask_ __mask, _Tp_ __src) noexcept {
		if constexpr (sizeof(_Tp_) == 16) {
			if constexpr ((__is_epi64_v<_Type_> || __is_epu64_v<_Type_>) && __avx512vl) return __as<_Tp_>(_mm_mask_load_epi64(__as<__m128i>(__src), __mask, __mem));
			else if constexpr ((__is_epi32_v<_Type_> || __is_epu32_v<_Type_>) && __avx512vl) return __as<_Tp_>(_mm_mask_load_epi32(__as<__m128i>(__src), __mask, __mem));
			else if constexpr (__is_pd_v<_Type_> && __avx512vl) return __as<_Tp_>(_mm_mask_load_pd(__as<__m128d>(__src), __mask, __mem));
			else if constexpr (__is_ps_v<_Type_> && __avx512vl) return __as<_Tp_>(_mm_mask_load_ps(__as<__m128>(__src), __mask, __mem));
			else return __loadu(__mem, __mask, __src);
		}
		else if constexpr (sizeof(_Tp_) == 32) {
			if constexpr ((__is_epi64_v<_Type_> || __is_epu64_v<_Type_>) && __avx512vl) return __as<_Tp_>(_mm256_mask_load_epi64(__as<__m256i>(__src), __mask, __mem));
			else if constexpr ((__is_epi32_v<_Type_> || __is_epu32_v<_Type_>) && __avx512vl) return __as<_Tp_>(_mm256_mask_load_epi32(__as<__m256i>(__src), __mask, __mem));
			else if constexpr (__is_pd_v<_Type_> && __avx512vl) return __as<_Tp_>(_mm256_mask_load_pd(__as<__m256d>(__src), __mask, __mem));
			else if constexpr (__is_ps_v<_Type_> && __avx512vl) return __as<_Tp_>(_mm256_mask_load_ps(__as<__m256>(__src), __mask, __mem));
			else return __loadu(__mem, __mask, __src);
		}
		else if constexpr (sizeof(_Tp_) == 64) {
			if constexpr (__is_epi64_v<_Type_> || __is_epu64_v<_Type_>) return __as<_Tp_>(_mm512_mask_load_epi64(__as<__m512i>(__src), __mask, __mem));
			else if constexpr (__is_epi32_v<_Type_> || __is_epu32_v<_Type_>) return __as<_Tp_>(_mm512_mask_load_epi32(__as<__m512i>(__src), __mask, __mem));
			else if constexpr (__is_pd_v<_Type_>) return __as<_Tp_>(_mm512_mask_load_pd(__as<__m512d>(__src), __mask, __mem));
			else if constexpr (__is_ps_v<_Type_>) return __as<_Tp_>(_mm512_mask_load_ps(__as<__m512>(__src), __mask, __mem));
			else return __loadu(__mem, __mask, __src);
		}

		if constexpr (arithmetic_type<_Tp_>) return __mask ? *static_cast<const _Tp_*>(__mem) : __src;
		else {
			if constexpr (_Safe_) {
				constexpr auto __size = sizeof(_Tp_) / sizeof(_Type_);
				alignas(sizeof(_Tp_)) _Type_ __array[__size];
				_Store<_ISA_>()(__array, __src, __aligned_policy{});

				if constexpr (intrin_type<_Mask_>) {
					alignas(sizeof(_Tp_)) typename IntegerForSizeof<_Type_>::Signed __marray[__size];
					_Store<_ISA_>()(__marray, __mask, __aligned_policy{});

					for (auto __i = 0; __i < __size; ++__i)
						__array[__i] = __marray[__i] == 0 ? __array[__i] : static_cast<const _Type_*>(__mem)[__i];
				}
				else {
					for (auto __i = 0; __i < __size; ++__i)
						__array[__i] = math::__bit_test(__mask, __i) ? static_cast<const _Type_*>(__mem)[__i] : __array[__i];
				}

				return _Load<_ISA_, _Tp_>()(__array, __aligned_policy{});
			}
			else {
				return _Select<_ISA_, _Type_>()(_Load<_ISA_, _Tp_>()(__mem, __aligned_policy{}), __src, __mask);
			}
		}
	}

	template <raw_mask_type _Mask_, intrin_or_arithmetic_type _Tp_, class _AlignPolicy_ = __unaligned_policy>
	raze_nodiscard raze_static_operator raze_always_inline _Tp_ operator()(const void* raze_restrict __mem, 
		_Mask_ __mask, _Tp_ __src, _AlignPolicy_&& __policy = _AlignPolicy_{}) raze_const_operator noexcept
	{
		if constexpr (__is_aligned_v<_AlignPolicy_>) return __load(__mem, __mask, __src);
		else return __loadu(__mem, __mask, __src);

	}
};

__RAZE_VX_NAMESPACE_END
