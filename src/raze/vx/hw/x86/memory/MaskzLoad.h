#pragma once 

#include <src/raze/vx/hw/x86/merge/Select.h>
#include <src/raze/vx/hw/x86/memory/Load.h>


__RAZE_VX_NAMESPACE_BEGIN

template <arch::ISA	ISA, intrin_or_arithmetic_type V, arithmetic_type T, bool Safe, raw_mask_type M>
raze_always_inline V maskz_loadu_(const void* mem, M mask) noexcept {
	if constexpr (sizeof(V) == 16) {
		if constexpr (epi64<T> || epu64<T>) {
			if constexpr (has_avx512vl<ISA>) return as<V>(_mm_maskz_loadu_epi64(mask, mem));
			else if constexpr (has_avx2<ISA>) return as<V>(_mm_maskload_epi64(static_cast<const i64*>(mem), as<__m128i>(mask)));
			else if constexpr (has_avx<ISA>) return as<V>(_mm_maskload_pd(static_cast<const f64*>(mem), as<__m128i>(mask)));
		}
		else if constexpr (epi32<T> || epu32<T>) {
			if constexpr (has_avx512vl<ISA>) return as<V>(_mm_maskz_loadu_epi32(mask, mem));
			else if constexpr (has_avx2<ISA>) return as<V>(_mm_maskload_epi32(static_cast<const i32*>(mem), as<__m128i>(mask)));
			else if constexpr (has_avx<ISA>) return as<V>(_mm_maskload_ps(static_cast<const f32*>(mem), as<__m128i>(mask)));
		}
		else if constexpr (pd<T>) {
			if constexpr (has_avx512vl<ISA>) return as<V>(_mm_maskz_loadu_pd(mask, mem));
			else if constexpr (has_avx<ISA>) return as<V>(_mm_maskload_pd(static_cast<const f64*>(mem), as<__m128i>(mask)));
		}
		else if constexpr (ps<T>) {
			if constexpr (has_avx512vl<ISA>) return as<V>(_mm_maskz_loadu_ps(mask, mem));
			else if constexpr (has_avx<ISA>) return as<V>(_mm_maskload_ps(static_cast<const f32*>(mem), as<__m128i>(mask)));
		}
		else if constexpr (has_avx512bw<ISA>) {
			if constexpr (epi16<T> || epu16<T>) return as<V>(_mm_maskz_loadu_epi16(mask, mem));
			else if constexpr (epi8<T> || epu8<T>) return as<V>(_mm_maskz_loadu_epi8(mask, mem));
		}
	}
	else if constexpr (sizeof(V) == 32) {
		if constexpr (epi64<T> || epu64<T>) {
			if constexpr (has_avx512vl<ISA>) return as<V>(_mm256_maskz_loadu_epi64(mask, mem));
			else if constexpr (has_avx2<ISA>) return as<V>(_mm256_maskload_epi64(static_cast<const i64*>(mem), as<__m256i>(mask)));
			else return as<V>(_mm256_maskload_pd(static_cast<const f64*>(mem), as<__m256i>(mask)));
		}
		else if constexpr (epi32<T> || epu32<T>) {
			if constexpr (has_avx512vl<ISA>) return as<V>(_mm256_maskz_loadu_epi32(mask, mem));
			else if constexpr (has_avx2<ISA>) return as<V>(_mm256_maskload_epi32(static_cast<const i32*>(mem), as<__m256i>(mask)));
			else return as<V>(_mm256_maskload_ps(static_cast<const f32*>(mem), as<__m256i>(mask)));
		}
		else if constexpr (pd<T>) {
			if constexpr (has_avx512vl<ISA>) return as<V>(_mm256_maskz_loadu_pd(mask, mem));
			else return as<V>(_mm256_maskload_pd(static_cast<const f64*>(mem), as<__m256i>(mask)));
		}
		else if constexpr (ps<T>) {
			if constexpr (has_avx512vl<ISA>) return as<V>(_mm256_maskz_loadu_ps(mask, mem));
			else return as<V>(_mm256_maskload_ps(static_cast<const f32*>(mem), as<__m256i>(mask)));
		}
		else if constexpr (has_avx512bw<ISA>) {
			if constexpr (epi16<T> || epu16<T>) return as<V>(_mm256_maskz_loadu_epi16(mask, mem));
			else if constexpr (epi8<T> || epu8<T>) return as<V>(_mm256_maskz_loadu_epi8(mask, mem));
		}
	}
	else if constexpr (sizeof(V) == 64) {
		if constexpr (epi64<T> || epu64<T>) return as<V>(_mm512_maskz_loadu_epi64(mask, mem));
		else if constexpr (epi32<T> || epu32<T>) return as<V>(_mm512_maskz_loadu_epi32(mask, mem));
		else if constexpr (pd<T>) return as<V>(_mm512_maskz_loadu_pd(mask, mem));
		else if constexpr (ps<T>) return as<V>(_mm512_maskz_loadu_ps(mask, mem));
		else if constexpr (has_avx512bw<ISA>) {
			if constexpr (epi16<T> || epu16<T>) return as<V>(_mm512_maskz_loadu_epi16(mask, mem));
			else if constexpr (epi8<T> || epu8<T>) return as<V>(_mm512_maskz_loadu_epi8(mask, mem));
		}
	}
	
	if constexpr (arithmetic_type<V>) return mask ? *static_cast<const V*>(mem) : 0;
	else {
		if constexpr (Safe) {
			constexpr auto size = sizeof(V) / sizeof(T);
			alignas(sizeof(V)) T array[size];
			store_(array, zero_<ISA, V>(), aligned_policy{});

			if constexpr (intrin_type<M>) {
				alignas(sizeof(V)) typename IntegerForSizeof<T>::Signed marray[size];
				store_(marray, mask, aligned_policy{});

				for (auto i = 0; i < size; ++i)
					array[i] = marray[i] == 0 ? 0 : static_cast<const T*>(mem)[i];
			}
			else {
				for (auto i = 0; i < size; ++i)
					array[i] = math::bit_test(mask, i) ? static_cast<const T*>(mem)[i] : 0;
			}

			return load_<ISA, V>(array, aligned_policy{});
		}
		else {
			return select_<ISA, T>(load_<ISA, V>(mem), mask);
		}
	}
}

template <arch::ISA	ISA, intrin_or_arithmetic_type V, arithmetic_type T, bool Safe, raw_mask_type M>
raze_always_inline V maskz_loada_(const void* mem, M mask) noexcept {
	if constexpr (sizeof(V) == 16) {
		if constexpr ((epi64<T> || epu64<T>) && has_avx512vl<ISA>) return as<V>(_mm_maskz_load_epi64(mask, mem));
		else if constexpr ((epi32<T> || epu32<T>) && has_avx512vl<ISA>) return as<V>(_mm_maskz_load_epi32(mask, mem));
		else if constexpr (pd<T> && has_avx512vl<ISA>) return as<V>(_mm_maskz_load_pd(mask, mem));
		else if constexpr (ps<T> && has_avx512vl<ISA>) return as<V>(_mm_maskz_load_ps(mask, mem));
		else return maskz_loadu_<ISA, V, T, Safe>(mem, mask);
	}
	else if constexpr (sizeof(V) == 32) {
		if constexpr ((epi64<T> || epu64<T>) && has_avx512vl<ISA>) return as<V>(_mm256_maskz_load_epi64(mask, mem));
		else if constexpr ((epi32<T> || epu32<T>) && has_avx512vl<ISA>) return as<V>(_mm256_maskz_load_epi32(mask, mem));
		else if constexpr (pd<T> && has_avx512vl<ISA>) return as<V>(_mm256_maskz_load_pd(mask, mem));
		else if constexpr (ps<T> && has_avx512vl<ISA>) return as<V>(_mm256_maskz_load_ps(mask, mem));
		else return maskz_loadu_<ISA, V, T, Safe>(mem, mask);
	}
	else if constexpr (sizeof(V) == 64) {
		if constexpr (epi64<T> || epu64<T>) return as<V>(_mm512_maskz_load_epi64(mask, mem));
		else if constexpr (epi32<T> || epu32<T>) return as<V>(_mm512_maskz_load_epi32(mask, mem));
		else if constexpr (pd<T>) return as<V>(_mm512_maskz_load_pd(mask, mem));
		else if constexpr (ps<T>) return as<V>(_mm512_maskz_load_ps(mask, mem));
		else return maskz_loadu_<ISA, V, T, Safe>(mem, mask);
	}

	if constexpr (arithmetic_type<V>) return mask ? *static_cast<const V*>(mem) : 0;
	else {
		if constexpr (Safe) {
			constexpr auto size = sizeof(V) / sizeof(T);
			alignas(sizeof(V)) T array[size];
			store_(array, zero_<ISA, V>(), aligned_policy{});

			if constexpr (intrin_type<M>) {
				alignas(sizeof(V)) typename IntegerForSizeof<T>::Signed mask_array[size];
				store_(mask_array, mask, aligned_policy{});

				for (auto i = 0; i < size; ++i)
					array[i] = mask_array[i] == 0 ? 0 : static_cast<const T*>(mem)[i];
			}
			else {
				for (auto i = 0; i < size; ++i)
					array[i] = math::bit_test(mask, i) ? static_cast<const T*>(mem)[i] : 0;
			}

			return load_<ISA, V>(array, aligned_policy{});
		}
		else {
			return select_<ISA, T>(load_<ISA, V>(mem, aligned_policy{}), mask);
		}
	}
}

template <arch::ISA	ISA, intrin_or_arithmetic_type V, arithmetic_type T, 
	bool Safe, raw_mask_type M, class Policy = unaligned_policy>
raze_always_inline V load_(const void* mem, M mask, Policy = Policy{}) noexcept {
	if constexpr (is_aligned_v<Policy>) return maskz_loada_<ISA, V, T, Safe>(mem, mask);
	else return maskz_loadu_<ISA, V, T, Safe>(mem, mask);
}

__RAZE_VX_NAMESPACE_END
