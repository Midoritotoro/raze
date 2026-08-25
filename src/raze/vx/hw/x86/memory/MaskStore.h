#pragma once 

#include <src/raze/vx/hw/x86/merge/Select.h>
#include <src/raze/vx/hw/x86/memory/Load.h>
#include <src/raze/vx/hw/x86/memory/Store.h>
#include <src/raze/math/BitTest.h>

__RAZE_VX_NAMESPACE_BEGIN

template <arch::ISA	ISA, arithmetic_type T, bool Safe, intrin_or_arithmetic_type V, raw_mask_type M>
raze_always_inline void mask_storeu_(void* mem, M mask, V x) noexcept {
	if constexpr (sizeof(V) == 16) {
		if constexpr (epi64<T> || epu64<T>) {
			if constexpr (has_avx512vl<ISA>) return _mm_mask_storeu_epi64(mem, mask, as<__m128i>(x));
			else if constexpr (has_avx2<ISA>) return _mm_maskstore_epi64(reinterpret_cast<i64*>(mem), as<__m128i>(mask), as<__m128i>(x));
			else if constexpr (has_avx<ISA>) return _mm_maskstore_pd(reinterpret_cast<f64*>(mem), as<__m128i>(mask), as<__m128d>(x));
		}
		if constexpr (epi32<T> || epu32<T>) {
			if constexpr (has_avx512vl<ISA>) return _mm_mask_storeu_epi32(mem, mask, as<__m128i>(x));
			else if constexpr (has_avx2<ISA>) return _mm_maskstore_epi32(reinterpret_cast<i32*>(mem), as<__m128i>(mask), as<__m128i>(x));
			else if constexpr (has_avx<ISA>) return _mm_maskstore_ps(reinterpret_cast<f32*>(mem), as<__m128i>(mask), as<__m128>(x));
		}
		else if constexpr (ps<T>) {
			if constexpr (has_avx512vl<ISA>) return _mm_mask_storeu_ps(mem, mask, as<__m128>(x));
			else if constexpr (has_avx<ISA>) return _mm_maskstore_ps(reinterpret_cast<f32*>(mem), as<__m128i>(mask), as<__m128>(x));
		}
		else if constexpr (pd<T>) {
			if constexpr (has_avx512vl<ISA>) return _mm_mask_storeu_pd(mem, mask, as<__m128d>(x));
			else if constexpr (has_avx<ISA>) return _mm_maskstore_pd(reinterpret_cast<f64*>(mem), as<__m128i>(mask), as<__m128d>(x));
		}
		else if constexpr (has_avx512bw<ISA> && has_avx512vl<ISA>) {
			if constexpr (epi16<T> || epu16<T>) return _mm_mask_storeu_epi16(mem, mask, as<__m128i>(x));
			if constexpr (epi8<T> || epu8<T>) return _mm_mask_storeu_epi8(mem, mask, as<__m128i>(x));
		}
	}
	else if constexpr (sizeof(V) == 32) {
		if constexpr (epi64<T> || epu64<T>) {
			if constexpr (has_avx512vl<ISA>) return _mm256_mask_storeu_epi64(mem, mask, as<__m256i>(x));
			else if constexpr (has_avx2<ISA>) return _mm256_maskstore_epi64(reinterpret_cast<i64*>(mem), as<__m256i>(mask), as<__m256i>(x));
			else return _mm256_maskstore_pd(reinterpret_cast<f64*>(mem), as<__m256i>(mask), as<__m256d>(x));
		}
		if constexpr (epi32<T> || epu32<T>) {
			if constexpr (has_avx512vl<ISA>) return _mm256_mask_storeu_epi32(mem, mask, as<__m256i>(x));
			else if constexpr (has_avx2<ISA>) return _mm256_maskstore_epi32(reinterpret_cast<i32*>(mem), as<__m256i>(mask), as<__m256i>(x));
			else return _mm256_maskstore_ps(reinterpret_cast<f32*>(mem), as<__m256i>(mask), as<__m256>(x));
		}
		else if constexpr (ps<T>) {
			if constexpr (has_avx512vl<ISA>) return _mm256_mask_storeu_ps(mem, mask, as<__m256>(x));
			else return _mm256_maskstore_ps(reinterpret_cast<f32*>(mem), as<__m256i>(mask), as<__m256>(x));
		}
		else if constexpr (pd<T>) {
			if constexpr (has_avx512vl<ISA>) return _mm256_mask_storeu_pd(mem, mask, as<__m256d>(x));
			else return _mm256_maskstore_pd(reinterpret_cast<f64*>(mem), as<__m256i>(mask), as<__m256d>(x));
		}
		else if constexpr (has_avx512bw<ISA> && has_avx512vl<ISA>) {
			if constexpr (epi16<T> || epu16<T>) return _mm256_mask_storeu_epi16(mem, mask, as<__m256i>(x));
			if constexpr (epi8<T> || epu8<T>) return _mm256_mask_storeu_epi8(mem, mask, as<__m256i>(x));
		}
	}
	else if constexpr (sizeof(V) == 64) {
		if constexpr (epi64<T> || epu64<T>) return _mm512_mask_storeu_epi64(mem, mask, as<__m512i>(x));
		if constexpr (epi32<T> || epu32<T>) return _mm512_mask_storeu_epi32(mem, mask, as<__m512i>(x));
		else if constexpr (ps<T>) return _mm512_mask_storeu_ps(mem, mask, as<__m512>(x));
		else if constexpr (pd<T>) return _mm512_mask_storeu_pd(mem, mask, as<__m512d>(x));
		else if constexpr (has_avx512bw<ISA>) {
			if constexpr (epi16<T> || epu16<T>) return _mm512_mask_storeu_epi16(mem, mask, as<__m512i>(x));
			if constexpr (epi8<T> || epu8<T>) return _mm512_mask_storeu_epi8(mem, mask, as<__m512i>(x));
		}
	}

	if constexpr (arithmetic_type<V>) { if (mask) *static_cast<V*>(mem) = x; }
	else {
		if constexpr (Safe) {
			constexpr auto size = sizeof(V) / sizeof(T);
			alignas(sizeof(V)) T array[size];
			store_(array, x, aligned_policy{});

			if constexpr (intrin_type<M>) {
				alignas(sizeof(V)) typename IntegerForSizeof<T>::Signed mask_array[size];
				store_(mask_array, mask, aligned_policy{});

				for (auto i = 0; i < size; ++i)
					if (mask_array[i] != 0)
						static_cast<T*>(mem)[i] = array[i];
			}
			else {
				for (auto i = 0; i < size; ++i)
					if (math::bit_test(mask, i))
						static_cast<T*>(mem)[i] = array[i];
			}
		}
		else {
			store_(mem, select_<ISA, T>(x, load_<ISA, V>(mem), mask));
		}
	}
}

template <arch::ISA	ISA, arithmetic_type T, bool Safe, intrin_or_arithmetic_type V, raw_mask_type M>
raze_always_inline void mask_storea_(void* mem, M mask, V x) noexcept {
	if constexpr (sizeof(V) == 16) {
		if constexpr ((epi64<T> || epu64<T>) && has_avx512vl<ISA>) return _mm_mask_store_epi64(mem, mask, as<__m128i>(x));
		if constexpr ((epi32<T> || epu32<T>) && has_avx512vl<ISA>) return _mm_mask_store_epi32(mem, mask, as<__m128i>(x));
		else if constexpr (ps<T> && has_avx512vl<ISA>) return _mm_mask_store_ps(mem, mask, as<__m128>(x));
		else if constexpr (pd<T> && has_avx512vl<ISA>) return _mm_mask_store_pd(mem, mask, as<__m128d>(x));
		else return mask_storeu_<ISA, T, Safe>(mem, mask, x);
	}
	else if constexpr (sizeof(V) == 32) {
		if constexpr ((epi64<T> || epu64<T>) && has_avx512vl<ISA>) return _mm256_mask_store_epi64(mem, mask, as<__m256i>(x));
		if constexpr ((epi32<T> || epu32<T>) && has_avx512vl<ISA>) return _mm256_mask_store_epi32(mem, mask, as<__m256i>(x));
		else if constexpr (ps<T> && has_avx512vl<ISA>) return _mm256_mask_store_ps(mem, mask, as<__m256>(x));
		else if constexpr (pd<T> && has_avx512vl<ISA>) return _mm256_mask_store_pd(mem, mask, as<__m256d>(x));
		else return mask_storeu_<ISA, T, Safe>(mem, mask, x);
	}
	else if constexpr (sizeof(V) == 64) {
		if constexpr (epi64<T> || epu64<T>) return _mm512_mask_store_epi64(mem, mask, as<__m512i>(x));
		if constexpr (epi32<T> || epu32<T>) return _mm512_mask_store_epi32(mem, mask, as<__m512i>(x));
		else if constexpr (ps<T>) return _mm512_mask_store_ps(mem, mask, as<__m512>(x));
		else if constexpr (pd<T>) return _mm512_mask_store_pd(mem, mask, as<__m512d>(x));
		else return mask_storeu_<ISA, T, Safe>(mem, mask, x);
	}

	if constexpr (arithmetic_type<V>) { if (mask) *static_cast<V*>(mem) = x; }
	else { 
		if constexpr (Safe) {
			constexpr auto size = sizeof(V) / sizeof(T);
			alignas(sizeof(V)) T array[size];
			store_(array, x, aligned_policy{});

			if constexpr (intrin_type<M>) {
				alignas(sizeof(V)) typename IntegerForSizeof<T>::Signed mask_array[size];
				store_(mask_array, mask, aligned_policy{});

				for (auto i = 0; i < size; ++i)
					if (mask_array[i] != 0)
						static_cast<T*>(mem)[i] = array[i];
			}
			else {
				for (auto i = 0; i < size; ++i)
					if (math::bit_test(mask, i))
						static_cast<T*>(mem)[i] = array[i];
			}
		}
		else {
			store_(mem, select_<ISA, T>(x, load_<ISA, V>(mem), mask), aligned_policy{});
		}
	}
}

template <arch::ISA	ISA, arithmetic_type T, bool Safe, intrin_or_arithmetic_type V, 
	raw_mask_type M, class Policy = unaligned_policy>
raze_always_inline void store_(void* mem, M mask, V x, Policy = Policy{}) noexcept {
	if constexpr (is_aligned_v<Policy>) mask_storea_<ISA, T, Safe>(mem, mask, x);
	else mask_storeu_<ISA, T, Safe>(mem, mask, x);
}

__RAZE_VX_NAMESPACE_END
