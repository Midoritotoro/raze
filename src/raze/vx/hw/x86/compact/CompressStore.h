#pragma once 

#include <src/raze/vx/hw/x86/mask/operations/ToMask.h>
#include <src/raze/vx/hw/x86/compact/CompressTables.h>
#include <src/raze/vx/hw/x86/memory/Load.h>
#include <src/raze/vx/hw/x86/memory/MaskStore.h>
#include <src/raze/vx/hw/x86/merge/Select.h>
#include <src/raze/math/PopulationCount.h>
#include <src/raze/math/IntegralTypesConversions.h>
#include <src/raze/algorithm/AdvanceBytes.h>
#include <src/raze/vx/hw/x86/mask/operations/MaskNot.h>
#include <utility>

__RAZE_VX_NAMESPACE_BEGIN

template <arithmetic_type T, intrin_type V, std::unsigned_integral CompressMask>
raze_always_inline void* compress_store_fallback_(void* ptr, V x, CompressMask compress_mask) noexcept {
	constexpr auto size = sizeof(V) / sizeof(T);
	alignas(sizeof(V)) T source[size];

	store_(source, x, aligned_policy{});

	T* result_pointer = reinterpret_cast<T*>(ptr);
	auto start = result_pointer;

	for (auto index = 0; index < size; ++index)
		if (!((compress_mask >> index) & 1))
			*result_pointer++ = source[index];

	return algorithm::bytes_pointer_offset(ptr, (result_pointer - start) * sizeof(T));
}

template <arch::ISA ISA, arithmetic_type T, intrin_or_arithmetic_type V, raw_mask_type CompressMask>
raze_always_inline void* compress_store_(void* ptr, V x, CompressMask compress_mask) noexcept {
	constexpr auto size = sizeof(V) / sizeof(T);
	auto int_mask = to_mask_<ISA, T>(compress_mask);

	if constexpr (sizeof(V) == 16) {
		if constexpr (has_avx512vl<ISA>) {
			if constexpr (sizeof(T) == 8) {
				const auto not_mask = mask_not_<ISA, size, T>(int_mask);
				_mm_mask_compressstoreu_epi64(ptr, not_mask, as<__m128i>(x));
				return algorithm::bytes_pointer_offset(ptr, math::native_popcnt_n_bits<size>(not_mask) * sizeof(T));
			}
			else if constexpr (sizeof(T) == 4) {
				const auto not_mask = mask_not_<ISA, size, T>(int_mask);
				_mm_mask_compressstoreu_epi32(ptr, not_mask, as<__m128i>(x));
				return algorithm::bytes_pointer_offset(ptr, math::native_popcnt_n_bits<size>(not_mask) * sizeof(T));
			}
			else if constexpr (sizeof(T) == 2 && has_avx512vbmi2<ISA>) {
				const auto not_mask = mask_not_<ISA, size, T>(int_mask);
				_mm_mask_compressstoreu_epi16(ptr, not_mask, as<__m128i>(x));
				return algorithm::bytes_pointer_offset(ptr, math::native_popcnt_n_bits<size>(not_mask) * sizeof(T));
			}
			else if constexpr (sizeof(T) == 1 && has_avx512vbmi2<ISA>) {
				const auto not_mask = mask_not_<ISA, size, T>(int_mask);
				_mm_mask_compressstoreu_epi8(ptr, not_mask, as<__m128i>(x));
				return algorithm::bytes_pointer_offset(ptr, math::native_popcnt_n_bits<size>(not_mask) * sizeof(T));
			}
		}
		if constexpr (has_ssse3<ISA> && sizeof(T) < 4) {
			if constexpr (sizeof(T) == 1) {
				const auto mask_low = int_mask & 0xFF;
				const auto mask_high = (int_mask >> 8) & 0xFF;

				T* dst_ptr = reinterpret_cast<T*>(ptr);

				const auto count_lo = tables_sse<sizeof(T)>.size[mask_low];
				const auto count_hi = tables_sse<sizeof(T)>.size[mask_high];

				const auto shuffle_mask_lo = _mm_loadl_epi64(reinterpret_cast<const __m128i*>(tables_sse<sizeof(T)>.shuffle[mask_low]));
				const auto shuffle_mask_hi = _mm_loadl_epi64(reinterpret_cast<const __m128i*>(tables_sse<sizeof(T)>.shuffle[mask_high]));

				const auto x_in_low = _mm_srli_si128(as<__m128i>(x), 8);
				const auto packed_lo = _mm_shuffle_epi8(as<__m128i>(x), shuffle_mask_lo);
				const auto packed_hi = _mm_shuffle_epi8(x_in_low, as<__m128i>(shuffle_mask_hi));

				_mm_storel_epi64(reinterpret_cast<__m128i*>(dst_ptr), as<__m128i>(packed_lo));
				algorithm::advance_bytes(dst_ptr, count_lo);
				_mm_storel_epi64(reinterpret_cast<__m128i*>(dst_ptr), as<__m128i>(packed_hi));

				return algorithm::bytes_pointer_offset(ptr, count_lo + count_hi);
			}
			else {
				const auto shuffle_mask = load_<ISA, __m128i>(tables_sse<sizeof(T)>.shuffle[int_mask], aligned_policy{});
				const auto processed_bytes = tables_sse<sizeof(T)>.size[int_mask];
				_mm_storeu_si128(reinterpret_cast<__m128i*>(ptr), _mm_shuffle_epi8(as<__m128i>(x), shuffle_mask));
				return algorithm::bytes_pointer_offset(ptr, processed_bytes);
			}
		}
		else {
			if constexpr (sizeof(T) == 8) {
				constexpr auto calculate = [] <class Vec> (auto mask, Vec v) raze_always_inline_lambda -> std::pair<size_t, Vec> {
					switch (mask) {
						case 0: return { 16, v };
						case 1: return { 8, as<Vec>(_mm_shuffle_pd(as<__m128d>(v), as<__m128d>(v), 0x3)) };
						case 2: return { 8, v };
						case 3: return { 0, v };
						// default: { raze_assert_unreachable(); return { 0, v }; }
					}
				};

				const auto& [processed_bytes, packed] = calculate(int_mask, x);
				store_<ISA, T, false>(ptr, first_n_<ISA, size, V, T>(processed_bytes / sizeof(T)), packed);
				return algorithm::bytes_pointer_offset(ptr, processed_bytes);
			}
			else if constexpr (sizeof(T) == 4) {
				constexpr auto calculate = [] <class Vec> (auto mask, Vec v) raze_always_inline_lambda -> std::pair<size_t, Vec> {
					switch (mask) {
						case 0x0: return { 16, v };
						case 0x1: return { 12, as<Vec>(_mm_shuffle_ps(as<__m128>(v), as<__m128>(v), 0xF9)) };
						case 0x2: return { 12, as<Vec>(_mm_shuffle_ps(as<__m128>(v), as<__m128>(v), 0xF8)) };
						case 0x3: return { 8, as<Vec>(_mm_shuffle_ps(as<__m128>(v), as<__m128>(v), 0xEE)) };
						case 0x4: return { 12, as<Vec>(_mm_shuffle_ps(as<__m128>(v), as<__m128>(v), 0xF4)) };
						case 0x5: return { 8, as<Vec>(_mm_shuffle_ps(as<__m128>(v), as<__m128>(v), 0xED)) };
						case 0x6: return { 8, as<Vec>(_mm_shuffle_ps(as<__m128>(v), as<__m128>(v), 0xEC)) };
						case 0x7: return { 4, as<Vec>(_mm_shuffle_ps(as<__m128>(v), as<__m128>(v), 0xE7)) };
						case 0x8: return { 12, v };
						case 0x9: return { 8, as<Vec>(_mm_shuffle_ps(as<__m128>(v), as<__m128>(v), 0xE9)) };
						case 0xA: return { 8, as<Vec>(_mm_shuffle_ps(as<__m128>(v), as<__m128>(v), 0xE8)) };
						case 0xB: return { 4, as<Vec>(_mm_shuffle_ps(as<__m128>(v), as<__m128>(v), 0xE6)) };
						case 0xC: return { 8, v };
						case 0xD: return { 4, as<Vec>(_mm_shuffle_ps(as<__m128>(v), as<__m128>(v), 0x55)) };
						case 0xE: return { 4, v };
						case 0xF: return { 0, v };
						// default: { raze_assert_unreachable(); return { 0, v }; }
					}
				};

				const auto& [processed_bytes, packed] = calculate(int_mask, x);
				store_<ISA, T, false>(ptr, first_n_<ISA, size, V, T>(processed_bytes / sizeof(T)), packed);
				return algorithm::bytes_pointer_offset(ptr, processed_bytes);
			}
		}
	}
	else if constexpr (sizeof(V) == 32) {
		if constexpr (has_avx512vl<ISA>) {
			if constexpr (sizeof(T) == 8) {
				const auto not_mask = mask_not_<ISA, size, T>(int_mask);
				_mm256_mask_compressstoreu_epi64(ptr, not_mask, as<__m256i>(x));
				return algorithm::bytes_pointer_offset(ptr, math::native_popcnt_n_bits<size>(not_mask) * sizeof(T));
			}
			else if constexpr (sizeof(T) == 4) {
				const auto not_mask = mask_not_<ISA, size, T>(int_mask);
				_mm256_mask_compressstoreu_epi32(ptr, not_mask, as<__m256i>(x));
				return algorithm::bytes_pointer_offset(ptr, math::native_popcnt_n_bits<size>(not_mask) * sizeof(T));
			}
			else if constexpr (sizeof(T) == 2 && has_avx512vbmi2<ISA>) {
				const auto not_mask = mask_not_<ISA, size, T>(int_mask);
				_mm256_mask_compressstoreu_epi16(ptr, not_mask, as<__m256i>(x));
				return algorithm::bytes_pointer_offset(ptr, math::native_popcnt_n_bits<size>(not_mask) * sizeof(T));
			}
			else if constexpr (sizeof(T) == 1 && has_avx512vbmi2<ISA>) {
				const auto not_mask = mask_not_<ISA, size, T>(int_mask);
				_mm256_mask_compressstoreu_epi8(ptr, not_mask, as<__m256i>(x));
				return algorithm::bytes_pointer_offset(ptr, math::native_popcnt_n_bits<size>(not_mask) * sizeof(T));
			}
		}
		if constexpr (sizeof(T) >= 4 && has_avx2<ISA>) {
			const auto shuffle = _mm_loadl_epi64(reinterpret_cast<const __m128i*>(tables_avx<sizeof(T)>.shuffle[int_mask]));
			const auto processed_bytes = tables_avx<sizeof(T)>.size[int_mask];
			_mm256_storeu_si256(reinterpret_cast<__m256i*>(ptr), _mm256_permutevar8x32_epi32(as<__m256i>(x), _mm256_cvtepu8_epi32(shuffle)));
			return algorithm::bytes_pointer_offset(ptr, processed_bytes);
		}
		else if constexpr (sizeof(T) == 2) {
			T* write_ptr = reinterpret_cast<T*>(ptr);

			const auto vec_low = as<__m128i>(x);
			const auto vec_high = _mm256_extractf128_si256(as<__m256i>(x), 1);

			const auto mask_low = int_mask & 0xFF;
			const auto mask_high = (int_mask >> 8) & 0xFF;

			const auto count_bytes_low = tables_sse<sizeof(T)>.size[mask_low];
			const auto count_bytes_high = tables_sse<sizeof(T)>.size[mask_high];

			const auto total_bytes = count_bytes_low + count_bytes_high;

			const auto shuffle_mask_low = _mm_load_si128(reinterpret_cast<const __m128i*>(tables_sse<sizeof(T)>.shuffle[mask_low]));
			const auto shuffle_mask_high = _mm_load_si128(reinterpret_cast<const __m128i*>(tables_sse<sizeof(T)>.shuffle[mask_high]));

			const auto packed_low = _mm_shuffle_epi8(vec_low, shuffle_mask_low);
			const auto packed_high = _mm_shuffle_epi8(vec_high, shuffle_mask_high);

			_mm_storeu_si128(reinterpret_cast<__m128i*>(write_ptr), as<__m128i>(packed_low));
			algorithm::advance_bytes(write_ptr, count_bytes_low);

			_mm_storeu_si128(reinterpret_cast<__m128i*>(write_ptr), as<__m128i>(packed_high));
			return algorithm::bytes_pointer_offset(ptr, total_bytes);
		}
	}
	else if constexpr (sizeof(V) == 64) {
		if constexpr (sizeof(T) == 8) {
			const auto not_mask = mask_not_<ISA, size, T>(int_mask);
			_mm512_mask_compressstoreu_epi64(ptr, not_mask, as<__m512i>(x));
			return algorithm::bytes_pointer_offset(ptr, math::native_popcnt_n_bits<size>(not_mask) * sizeof(T));
		}
		else if constexpr (sizeof(T) == 4) {
			const auto not_mask = mask_not_<ISA, size, T>(int_mask);
			_mm512_mask_compressstoreu_epi32(ptr, not_mask, as<__m512i>(x));
			return algorithm::bytes_pointer_offset(ptr, math::native_popcnt_n_bits<size>(not_mask) * sizeof(T));
		}
		else if constexpr (sizeof(T) == 2) {
			if constexpr (has_avx512vbmi2<ISA>) {
				const auto not_mask = mask_not_<ISA, size, T>(int_mask);
				_mm512_mask_compressstoreu_epi16(ptr, not_mask, as<__m512i>(x));
				return algorithm::bytes_pointer_offset(ptr, math::native_popcnt_n_bits<size>(not_mask) * sizeof(T));
			}
		}
		else if constexpr (sizeof(T) == 1) {
			if constexpr (has_avx512vbmi2<ISA>) {
				const auto not_mask = mask_not_<ISA, size, T>(int_mask);
				_mm512_mask_compressstoreu_epi8(ptr, not_mask, as<__m512i>(x));
				return algorithm::bytes_pointer_offset(ptr, math::native_popcnt_n_bits<size>(not_mask) * sizeof(T));
			}
		}
	}

	if constexpr (arithmetic_type<V>) {
		if (!compress_mask) {
			reinterpret_cast<V*>(ptr)[0] = x;
			return algorithm::bytes_pointer_offset(ptr, sizeof(V));
		}
		else {
			return ptr;
		}
	}
	else return compress_store_fallback_<T>(ptr, x, int_mask);
}

__RAZE_VX_NAMESPACE_END
