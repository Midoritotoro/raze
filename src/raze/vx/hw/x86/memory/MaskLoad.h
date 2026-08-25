#pragma once 

#include <src/raze/vx/hw/x86/merge/Select.h>
#include <src/raze/vx/hw/x86/memory/Load.h>
#include <src/raze/vx/hw/x86/memory/Store.h>

__RAZE_VX_NAMESPACE_BEGIN

template <arch::ISA	ISA, arithmetic_type T, bool Safe, raw_mask_type M, intrin_or_arithmetic_type V>
raze_always_inline V mask_loadu_(const void* mem, M mask, V src) noexcept {
	if constexpr (sizeof(V) == 16 && has_avx512vl<ISA>) {
		if constexpr (epi64<T> || epu64<T>) return as<V>(_mm_mask_loadu_epi64(as<__m128i>(src), mask, mem));
		else if constexpr (epi32<T> || epu32<T>) return as<V>(_mm_mask_loadu_epi32(as<__m128i>(src), mask, mem));
		else if constexpr (pd<T>) return as<V>(_mm_mask_loadu_pd(as<__m128d>(src), mask, mem));
		else if constexpr (ps<T>) return as<V>(_mm_mask_loadu_ps(as<__m128>(src), mask, mem));
		else if constexpr (has_avx512bw<ISA>) {
			if constexpr (epi16<T> || epu16<T>) return as<V>(_mm_mask_loadu_epi16(as<__m128i>(src), mask, mem));
			else if constexpr (epi8<T> || epu8<T>) return as<V>(_mm_mask_loadu_epi8(as<__m128i>(src), mask, mem));
		}
	}
	else if constexpr (sizeof(V) == 32 && has_avx512vl<ISA>) {
		if constexpr (epi64<T> || epu64<T>) return as<V>(_mm256_mask_loadu_epi64(as<__m256i>(src), mask, mem));
		else if constexpr (epi32<T> || epu32<T>) return as<V>(_mm256_mask_loadu_epi32(as<__m256i>(src), mask, mem));
		else if constexpr (pd<T>) return as<V>(_mm256_mask_loadu_pd(as<__m256d>(src), mask, mem));
		else if constexpr (ps<T>) return as<V>(_mm256_mask_loadu_ps(as<__m256>(src), mask, mem));
		else if constexpr (has_avx512bw<ISA>) {
			if constexpr (epi16<T> || epu16<T>) return as<V>(_mm256_mask_loadu_epi16(as<__m256i>(src), mask, mem));
			else if constexpr (epi8<T> || epu8<T>) return as<V>(_mm256_mask_loadu_epi8(as<__m256i>(src), mask, mem));
		}
	}
	else if constexpr (sizeof(V) == 64) {
		if constexpr (epi64<T> || epu64<T>) return as<V>(_mm512_mask_loadu_epi64(as<__m512i>(src), mask, mem));
		else if constexpr (epi32<T> || epu32<T>) return as<V>(_mm512_mask_loadu_epi32(as<__m512i>(src), mask, mem));
		else if constexpr (pd<T>) return as<V>(_mm512_mask_loadu_pd(as<__m512d>(src), mask, mem));
		else if constexpr (ps<T>) return as<V>(_mm512_mask_loadu_ps(as<__m512>(src), mask, mem));
		else if constexpr (has_avx512bw<ISA>) {
			if constexpr (epi16<T> || epu16<T>) return as<V>(_mm512_mask_loadu_epi16(as<__m512i>(src), mask, mem));
			else if constexpr (epi8<T> || epu8<T>) return as<V>(_mm512_mask_loadu_epi8(as<__m512i>(src), mask, mem));
		}
	}
	
	if constexpr (arithmetic_type<V>) return mask ? *static_cast<const V*>(mem) : src;
	else {
		if constexpr (Safe) {
			constexpr auto size = sizeof(V) / sizeof(T);
			alignas(sizeof(V)) T array[size];
			store_(array, src);

			if constexpr (intrin_type<M>) {
				alignas(sizeof(V)) typename IntegerForSizeof<T>::Signed mask_array[size];
				store_(mask_array, mask, aligned_policy{});

				for (auto i = 0; i < size; ++i)
					array[i] = mask_array[i] == 0 ? array[i] : static_cast<const T*>(mem)[i];
			}
			else {
				for (auto i = 0; i < size; ++i)
					array[i] = math::bit_test(mask, i) ? static_cast<const T*>(mem)[i] : array[i];
			}

			return load_<ISA, V>(array, aligned_policy{});
		}
		else {
			return select_<ISA, T>(load_<ISA, V>(mem), src, mask);
		}
	}
}

template <arch::ISA	ISA, arithmetic_type T, bool Safe, raw_mask_type M, intrin_or_arithmetic_type V>
raze_nodiscard static raze_always_inline V mask_loada_(const void* mem, M mask, V src) noexcept {
	if constexpr (sizeof(V) == 16) {
		if constexpr ((epi64<T> || epu64<T>) && has_avx512vl<ISA>) return as<V>(_mm_mask_load_epi64(as<__m128i>(src), mask, mem));
		else if constexpr ((epi32<T> || epu32<T>) && has_avx512vl<ISA>) return as<V>(_mm_mask_load_epi32(as<__m128i>(src), mask, mem));
		else if constexpr (pd<T> && has_avx512vl<ISA>) return as<V>(_mm_mask_load_pd(as<__m128d>(src), mask, mem));
		else if constexpr (ps<T> && has_avx512vl<ISA>) return as<V>(_mm_mask_load_ps(as<__m128>(src), mask, mem));
		else return mask_loadu_<ISA, T, Safe>(mem, mask, src);
	}
	else if constexpr (sizeof(V) == 32) {
		if constexpr ((epi64<T> || epu64<T>) && has_avx512vl<ISA>) return as<V>(_mm256_mask_load_epi64(as<__m256i>(src), mask, mem));
		else if constexpr ((epi32<T> || epu32<T>) && has_avx512vl<ISA>) return as<V>(_mm256_mask_load_epi32(as<__m256i>(src), mask, mem));
		else if constexpr (pd<T> && has_avx512vl<ISA>) return as<V>(_mm256_mask_load_pd(as<__m256d>(src), mask, mem));
		else if constexpr (ps<T> && has_avx512vl<ISA>) return as<V>(_mm256_mask_load_ps(as<__m256>(src), mask, mem));
		else return mask_loadu_<ISA, T, Safe>(mem, mask, src);
	}
	else if constexpr (sizeof(V) == 64) {
		if constexpr (epi64<T> || epu64<T>) return as<V>(_mm512_mask_load_epi64(as<__m512i>(src), mask, mem));
		else if constexpr (epi32<T> || epu32<T>) return as<V>(_mm512_mask_load_epi32(as<__m512i>(src), mask, mem));
		else if constexpr (pd<T>) return as<V>(_mm512_mask_load_pd(as<__m512d>(src), mask, mem));
		else if constexpr (ps<T>) return as<V>(_mm512_mask_load_ps(as<__m512>(src), mask, mem));
		else return mask_loadu_<ISA, T, Safe>(mem, mask, src);
	}

	if constexpr (arithmetic_type<V>) return mask ? *static_cast<const V*>(mem) : src;
	else {
		if constexpr (Safe) {
			constexpr auto size = sizeof(V) / sizeof(T);
			alignas(sizeof(V)) T array[size];
			store_(array, src, aligned_policy{});

			if constexpr (intrin_type<M>) {
				alignas(sizeof(V)) typename IntegerForSizeof<T>::Signed mask_array[size];
				store_(marray, mask, aligned_policy{});

				for (auto i = 0; i < size; ++i)
					array[i] = mask_array[i] == 0 ? array[i] : static_cast<const T*>(mem)[i];
			}
			else {
				for (auto i = 0; i < size; ++i)
					array[i] = math::bit_test(mask, i) ? static_cast<const T*>(mem)[i] : array[i];
			}

			return load_<ISA, V>(array, aligned_policy{});
		}
		else {
			return select_<ISA, T>(load_<ISA, V>(mem, aligned_policy{}), src, mask);
		}
	}
}

template <arch::ISA	ISA, arithmetic_type T, bool Safe, raw_mask_type M, 
	intrin_or_arithmetic_type V, class Policy = unaligned_policy>
raze_always_inline V load_(const void* mem, M mask, V src, Policy = Policy{}) noexcept {
	if constexpr (is_aligned_v<Policy>) return mask_loada_<ISA, T, Safe>(mem, mask, src);
	else return mask_loadu_<ISA, T, Safe>(mem, mask, src);
}

__RAZE_VX_NAMESPACE_END
