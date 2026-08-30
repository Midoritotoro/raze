#pragma once 

#include <src/raze/vx/hw/x86/mask/operations/ToMask.h>
#include <src/raze/vx/hw/x86/compact/CompressTables.h>
#include <src/raze/vx/hw/x86/memory/Load.h>
#include <src/raze/vx/hw/x86/memory/Store.h>
#include <src/raze/vx/hw/x86/merge/Select.h>
#include <src/raze/math/PopulationCount.h>
#include <src/raze/algorithm/AdvanceBytes.h>
#include <src/raze/vx/hw/x86/mask/operations/MaskNot.h>
#include <utility>

__RAZE_VX_NAMESPACE_BEGIN

template <arch::ISA ISA, arithmetic_type T, intrin_type V, std::unsigned_integral CompressMask>
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
	const auto int_mask = to_mask_<ISA, T>(compress_mask);

	if constexpr (sizeof(V) == 16) {
		if constexpr (has_avx512vl<ISA>) {
			if constexpr (sizeof(T) == 8) {
				const auto not_mask = mask_not_<ISA, T>(int_mask);
				_mm_mask_compressstoreu_epi64(ptr, not_mask, as<__m128i>(x));
				return algorithm::bytes_pointer_offset(ptr, math::native_popcnt_n_bits<size>(not_mask) * sizeof(T));
			}
			else if constexpr (sizeof(T) == 4) {
				const auto not_mask = mask_not_<ISA, T>(int_mask);
				_mm_mask_compressstoreu_epi32(ptr, not_mask, as<__m128i>(x));
				return algorithm::bytes_pointer_offset(ptr, math::native_popcnt_n_bits<size>(not_mask) * sizeof(T));
			}
			else if constexpr (sizeof(T) == 2 && has_avx512vbmi2<ISA>) {
				const auto not_mask = mask_not_<ISA, T>(int_mask);
				_mm_mask_compressstoreu_epi16(ptr, not_mask, as<__m128i>(x));
				return algorithm::bytes_pointer_offset(ptr, math::native_popcnt_n_bits<size>(not_mask) * sizeof(T));
			}
			else if constexpr (sizeof(T) == 1 && has_avx512vbmi2<ISA>) {
				const auto not_mask = mask_not_<ISA, T>(int_mask);
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

				const auto packed_lo = _mm_shuffle_epi8(as<__m128i>(x), shuffle_mask_lo);
				const auto packed_hi = _mm_shuffle_epi8(_mm_srli_si128(as<__m128i>(x), 8), as<__m128i>(shuffle_mask_hi));

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
					}
				};

				auto [processed_bytes, packed] = calculate(int_mask, x);
				_mm_storeu_si128(reinterpret_cast<__m128i*>(ptr), as<__m128i>(packed));
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
					}
				};

				auto [processed_bytes, packed] = calculate(int_mask, x);
				_mm_storeu_si128(reinterpret_cast<__m128i*>(ptr), as<__m128i>(packed));
				return algorithm::bytes_pointer_offset(ptr, processed_bytes);
			}
		}
	}
	else if constexpr (sizeof(V) == 32) {
		if constexpr (has_avx512vl<ISA>) {
			if constexpr (sizeof(T) == 8) {
				const auto not_mask = mask_not_<ISA, T>(int_mask);
				_mm256_mask_compressstoreu_epi64(ptr, not_mask, as<__m256i>(x));
				return algorithm::bytes_pointer_offset(ptr, math::native_popcnt_n_bits<size>(not_mask) * sizeof(T));
			}
			else if constexpr (sizeof(T) == 4) {
				const auto not_mask = mask_not_<ISA, T>(int_mask);
				_mm256_mask_compressstoreu_epi32(ptr, not_mask, as<__m256i>(x));
				return algorithm::bytes_pointer_offset(ptr, math::native_popcnt_n_bits<size>(not_mask) * sizeof(T));
			}
			else if constexpr (sizeof(T) == 2 && has_avx512vbmi2<ISA>) {
				const auto not_mask = mask_not_<ISA, T>(int_mask);
				_mm256_mask_compressstoreu_epi16(ptr, not_mask, as<__m256i>(x));
				return algorithm::bytes_pointer_offset(ptr, math::native_popcnt_n_bits<size>(not_mask) * sizeof(T));
			}
			else if constexpr (sizeof(T) == 1 && has_avx512vbmi2<ISA>) {
				const auto not_mask = mask_not_<ISA, T>(int_mask);
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

			const auto vec_low = as<__m128i>(__x);
			const auto vec_high = _mm256_extracti128_si256(as<__m256i>(x), 1);

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
		else if constexpr (sizeof(T) == 1) {
			T* dst_ptr = reinterpret_cast<T*>(ptr);

			const auto vec_low = as<__m128i>(x);
			const auto vec_high = _mm256_extracti128_si256(as<__m256i>(x), 1);

			const auto vec_low_hi = as<__m128i>(_mm_movehl_ps(
				as<__m128>(_mm_slli_si128(vec_low, 8)), as<__m128>(vec_low)));

			const auto vec_high_hi = as<__m128i>(_mm_movehl_ps(
				as<__m128>(_mm_slli_si128(vec_high, 8)), as<__m128>(vec_high)));

			const auto mask_1 = int_mask & 0xFF;
			const auto mask_2 = (int_mask >> 8) & 0xFF;
			const auto mask_3 = (int_mask >> 16) & 0xFF;
			const auto mask_4 = (int_mask >> 24) & 0xFF;

			const auto bytes_1 = tables_sse<sizeof(T)>.size[mask_1];
			const auto bytes_2 = tables_sse<sizeof(T)>.size[mask_2];
			const auto bytes_3 = tables_sse<sizeof(T)>.size[mask_3];
			const auto bytes_4 = tables_sse<sizeof(T)>.size[mask_4];

			const auto bytes_low = bytes_1 + bytes_2;
			const auto bytes_high = bytes_3 + bytes_4;

			const auto total_bytes = bytes_low + bytes_high;

			const auto shuffle_1 = _mm_loadl_epi64(reinterpret_cast<const __m128i*>(tables_sse<sizeof(T)>.shuffle[mask_1]));
			const auto shuffle_2 = _mm_loadl_epi64(reinterpret_cast<const __m128i*>(tables_sse<sizeof(T)>.shuffle[mask_2]));
			const auto shuffle_3 = _mm_loadl_epi64(reinterpret_cast<const __m128i*>(tables_sse<sizeof(T)>.shuffle[mask_3]));
			const auto shuffle_4 = _mm_loadl_epi64(reinterpret_cast<const __m128i*>(tables_sse<sizeof(T)>.shuffle[mask_4]));

			const auto packed_1 = _mm_shuffle_epi8(vec_low, shuffle_1);
			const auto packed_2 = _mm_shuffle_epi8(vec_low_hi, shuffle_2);
			const auto packed_3 = _mm_shuffle_epi8(vec_high, shuffle_3);
			const auto packed_4 = _mm_shuffle_epi8(vec_high_hi, shuffle_4);

			_mm_storel_epi64(reinterpret_cast<__m128i*>(dst_ptr), as<__m128i>(packed_1));
			dst_ptr += bytes_1;

			_mm_storel_epi64(reinterpret_cast<__m128i*>(dst_ptr), as<__m128i>(packed_2));
			dst_ptr += bytes_2;

			_mm_storel_epi64(reinterpret_cast<__m128i*>(dst_ptr), as<__m128i>(packed_3));
			dst_ptr += bytes_3;

			_mm_storel_epi64(reinterpret_cast<__m128i*>(dst_ptr), as<__m128i>(packed_4));

			return algorithm::bytes_pointer_offset(ptr, total_bytes);
		}
	}
	else if constexpr (sizeof(V) == 64) {
		if constexpr (sizeof(T) == 8) {
			const auto not_mask = mask_not_<ISA, T>(int_mask);
			_mm512_mask_compressstoreu_epi64(ptr, not_mask, as<__m512i>(x));
			return algorithm::bytes_pointer_offset(ptr, math::native_popcnt_n_bits<size>(not_mask) * sizeof(T));
		}
		else if constexpr (sizeof(T) == 4) {
			const auto not_mask = mask_not_<ISA, T>(int_mask);
			_mm512_mask_compressstoreu_epi32(ptr, not_mask, as<__m512i>(x));
			return algorithm::bytes_pointer_offset(ptr, math::native_popcnt_n_bits<size>(not_mask) * sizeof(T));
		}
		else if constexpr (sizeof(T) == 2) {
			if constexpr (has_avx512vbmi2<ISA>) {
				const auto not_mask = mask_not_<ISA, T>(int_mask);
				_mm512_mask_compressstoreu_epi16(ptr, not_mask, as<__m512i>(x));
				return algorithm::bytes_pointer_offset(ptr, math::native_popcnt_n_bits<size>(not_mask) * sizeof(T));
			}
			else {
				T* dst_ptr = reinterpret_cast<T*>(ptr);

				const auto vec1 = as<__m128i>(x);
				const auto vec2 = _mm256_extracti128_si256(as<__m256i>(x), 1);
				const auto vec3 = as<__m128i>(_mm512_extractf32x4_ps(as<__m512>(x), 2));
				const auto vec4 = as<__m128i>(_mm512_extractf32x4_ps(as<__m512>(x), 3));

				const auto mask1 = int_mask & 0xFF;
				const auto mask2 = (int_mask >> 8) & 0xFF;
				const auto mask3 = (int_mask >> 16) & 0xFF;
				const auto mask4 = (int_mask >> 24) & 0xFF;

				const auto bytes1 = tables_sse<sizeof(T)>.size[mask1];
				const auto bytes2 = tables_sse<sizeof(T)>.size[mask2];
				const auto bytes3 = tables_sse<sizeof(T)>.size[mask3];
				const auto bytes4 = tables_sse<sizeof(T)>.size[mask4];

				const auto total_bytes = bytes1 + bytes2 + bytes3 + bytes4;
				const auto total_elems = total_bytes / sizeof(T);

				const auto shuffle1 = _mm_load_si128(reinterpret_cast<const __m128i*>(tables_sse<sizeof(T)>.shuffle[mask1]));
				const auto shuffle2 = _mm_load_si128(reinterpret_cast<const __m128i*>(tables_sse<sizeof(T)>.shuffle[mask2]));
				const auto shuffle3 = _mm_load_si128(reinterpret_cast<const __m128i*>(tables_sse<sizeof(T)>.shuffle[mask3]));
				const auto shuffle4 = _mm_load_si128(reinterpret_cast<const __m128i*>(tables_sse<sizeof(T)>.shuffle[mask4]));

				const auto packed1 = _mm_shuffle_epi8(vec1, shuffle1);
				const auto packed2 = _mm_shuffle_epi8(vec2, shuffle2);
				const auto packed3 = _mm_shuffle_epi8(vec3, shuffle3);
				const auto packed4 = _mm_shuffle_epi8(vec4, shuffle4);

				_mm_storeu_si128(reinterpret_cast<__m128i*>(dst_ptr), as<__m128i>(packed1));
				algorithm::advance_bytes(dst_ptr, bytes1);

				_mm_storeu_si128(reinterpret_cast<__m128i*>(dst_ptr), as<__m128i>(packed2));
				algorithm::advance_bytes(dst_ptr, bytes2);

				_mm_storeu_si128(reinterpret_cast<__m128i*>(dst_ptr), as<__m128i>(__packed3));
				algorithm::advance_bytes(dst_ptr, bytes3);

				_mm_storeu_si128(reinterpret_cast<__m128i*>(dst_ptr), as<__m128i>(packed4));
				return algorithm::bytes_pointer_offset(ptr, total_bytes);
			}
		}
		else if constexpr (sizeof(T) == 1) {
			if constexpr (has_avx512vbmi2<ISA>) {
				const auto __not_mask = _Mask_not<ISA, T>()(__int_mask);
				_mm512_mask_compressstoreu_epi8(__ptr, __not_mask, as<__m512i>(__x));
				return algorithm::__bytes_pointer_offset(__ptr, math::__native_popcnt_n_bits<size>(__not_mask) * sizeof(T));
			}
			else {
				T* write_ptr = reinterpret_cast<T*>(ptr);

				const auto ymm_lower = as<__m256i>(x);
				const auto ymm_higher = _mm512_extractf64x4_pd(as<__m512d>(x), 1);

				const auto xmm1 = as<__m128i>(ymm_lower);
				const auto xmm2 = as<__m128i>(_mm256_extractf128_pd(as<__m256d>(ymm_lower), 1));

				const auto xmm3 = as<__m128i>(ymm_higher);
				const auto xmm4 = as<__m128i>(_mm256_extractf128_pd(as<__m256d>(ymm_higher), 1));

				const auto xmm1_upper = as<__m128i>(_mm_movehl_ps(
					as<__m128>(_mm_slli_si128(xmm1, 8)), as<__m128>(xmm1)));

				const auto xmm2_upper = as<__m128i>(_mm_movehl_ps(
					as<__m128>(_mm_slli_si128(xmm2, 8)), as<__m128>(xmm2)));

				const auto __xmm3_upper = as<__m128i>(_mm_movehl_ps(
					as<__m128>(_mm_slli_si128(xmm3, 8)), as<__m128>(xmm3)));

				const auto __xmm4_upper = as<__m128i>(_mm_movehl_ps(
					as<__m128>(_mm_slli_si128(xmm4, 8)), as<__m128>(xmm4)));

				const auto mask1 = int_mask & 0xFF;
				const auto mask2 = (int_mask >> 8) & 0xFF;
				const auto mask3 = (int_mask >> 16) & 0xFF;
				const auto mask4 = (int_mask >> 24) & 0xFF;
				const auto mask5 = (int_mask >> 32) & 0xFF;
				const auto mask6 = (int_mask >> 40) & 0xFF;
				const auto mask7 = (int_mask >> 48) & 0xFF;
				const auto mask8 = (int_mask >> 56) & 0xFF;

				const auto bytes1 = tables_sse<sizeof(T)>.size[mask1];
				const auto bytes2 = tables_sse<sizeof(T)>.size[mask2];
				const auto bytes3 = tables_sse<sizeof(T)>.size[mask3];
				const auto bytes4 = tables_sse<sizeof(T)>.size[mask4];
				const auto bytes5 = tables_sse<sizeof(T)>.size[mask5];
				const auto bytes6 = tables_sse<sizeof(T)>.size[mask6];
				const auto bytes7 = tables_sse<sizeof(T)>.size[mask7];
				const auto bytes8 = tables_sse<sizeof(T)>.size[mask8];

				const auto bytes_lane1 = bytes1 + bytes2;
				const auto bytes_lane2 = bytes3 + bytes4;
				const auto bytes_lane3 = bytes5 + bytes6;
				const auto bytes_lane4 = bytes7 + bytes8;

				const auto bytes_total = bytes_lane1 + bytes_lane2 + bytes_lane3 + bytes_lane4;

				const auto shuffle1 = _mm_loadl_epi64(reinterpret_cast<const __m128i*>(tables_sse<sizeof(T)>.shuffle[mask1]));
				const auto shuffle2 = _mm_loadl_epi64(reinterpret_cast<const __m128i*>(tables_sse<sizeof(T)>.shuffle[mask2]));
				const auto shuffle3 = _mm_loadl_epi64(reinterpret_cast<const __m128i*>(tables_sse<sizeof(T)>.shuffle[mask3]));
				const auto shuffle4 = _mm_loadl_epi64(reinterpret_cast<const __m128i*>(tables_sse<sizeof(T)>.shuffle[mask4]));
				const auto shuffle5 = _mm_loadl_epi64(reinterpret_cast<const __m128i*>(tables_sse<sizeof(T)>.shuffle[mask5]));
				const auto shuffle6 = _mm_loadl_epi64(reinterpret_cast<const __m128i*>(tables_sse<sizeof(T)>.shuffle[mask6]));
				const auto shuffle7 = _mm_loadl_epi64(reinterpret_cast<const __m128i*>(tables_sse<sizeof(T)>.shuffle[mask7]));
				const auto shuffle8 = _mm_loadl_epi64(reinterpret_cast<const __m128i*>(tables_sse<sizeof(T)>.shuffle[mask8]));

				const auto packed1 = _mm_shuffle_epi8(xmm1, shuffle1);
				const auto packed2 = _mm_shuffle_epi8(xmm1_upper, shuffle2);
				const auto packed3 = _mm_shuffle_epi8(xmm2, shuffle3);
				const auto packed4 = _mm_shuffle_epi8(xmm2_upper, shuffle4);
				const auto packed5 = _mm_shuffle_epi8(xmm3, shuffle5);
				const auto packed6 = _mm_shuffle_epi8(xmm3_upper, shuffle6);
				const auto packed7 = _mm_shuffle_epi8(xmm4, shuffle7);
				const auto packed8 = _mm_shuffle_epi8(xmm4_upper, shuffle8);

				_mm_storel_epi64(reinterpret_cast<__m128i*>(write_ptr), as<__m128i>(packed1));
				algorithm::advance_bytes(write_ptr, bytes1);

				_mm_storel_epi64(reinterpret_cast<__m128i*>(write_ptr), as<__m128i>(packed2));
				algorithm::advance_bytes(write_ptr, bytes2);

				_mm_storel_epi64(reinterpret_cast<__m128i*>(write_ptr), as<__m128i>(packed3));
				algorithm::advance_bytes(write_ptr, bytes3);

				_mm_storel_epi64(reinterpret_cast<__m128i*>(write_ptr), as<__m128i>(packed4));
				algorithm::advance_bytes(write_ptr, bytes4);

				_mm_storel_epi64(reinterpret_cast<__m128i*>(write_ptr), as<__m128i>(packed5));
				algorithm::advance_bytes(write_ptr, bytes5);

				_mm_storel_epi64(reinterpret_cast<__m128i*>(write_ptr), as<__m128i>(packed6));
				algorithm::advance_bytes(write_ptr, bytes6);

				_mm_storel_epi64(reinterpret_cast<__m128i*>(write_ptr), as<__m128i>(packed7));
				algorithm::advance_bytes(write_ptr, bytes7);

				_mm_storel_epi64(reinterpret_cast<__m128i*>(write_ptr), as<__m128i>(packed8));
				return algorithm::bytes_pointer_offset(ptr, bytes_total);
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
	else return compress_store_fallback_(ptr, x, int_mask);
}

__RAZE_VX_NAMESPACE_END
