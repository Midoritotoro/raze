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

#if !defined(__RAZE_SLOW_BMI2)
#  if defined(raze_cpp_msvc_only)
#    define __RAZE_SLOW_BMI2 1 // For dynamic dispatch
#  else
#    if defined(__znver1__) || defined(__znver2__)
#      define __RAZE_SLOW_BMI2 1
#    else
#      define __RAZE_SLOW_BMI2 0
#    endif
#  endif
#endif

__RAZE_VX_NAMESPACE_BEGIN

constexpr auto __slow_bmi2 = __RAZE_SLOW_BMI2;

// https://stackoverflow.com/questions/36932240/UQAqJ3KzD1l6VDGbaijIcsSeOss0FSBZggu_gmpy2kcvzuIE-based-on-a-mask/61431303#61431303

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

template <arch::ISA ISA, arithmetic_type T, bool Unsafe, intrin_type V, std::unsigned_integral CompressMask>
raze_always_inline void* compress_avx2_lut_(void* ptr, V x, CompressMask mask) noexcept {
	constexpr auto size = sizeof(V) / sizeof(T);

	const auto shuffle = _mm_loadl_epi64(reinterpret_cast<const __m128i*>(tables_avx<sizeof(T)>.shuffle[mask]));
	const auto packed = _mm256_permutevar8x32_epi32(as<__m256i>(x), _mm256_cvtepu8_epi32(shuffle));
	const auto bytes = tables_avx<sizeof(T)>.size[mask];

	if constexpr (Unsafe) store_(ptr, packed);
	else store_<ISA, T, false>(ptr, first_n_<ISA, size, V, T>(bytes / sizeof(T)), packed);

	return algorithm::bytes_pointer_offset(ptr, bytes);
}

template <arch::ISA ISA, arithmetic_type T, bool Unsafe, intrin_type V, std::unsigned_integral CompressMask>
raze_always_inline void* compress_avx2_bmi2_(void* ptr, V x, CompressMask mask) noexcept {
	constexpr auto size = sizeof(V) / sizeof(T);

	if constexpr (sizeof(T) >= 4 && has_bmi2<ISA>) {
		static constexpr auto identity_vpermps = 0x0706050403020100ULL; // The identity shuffle for vpermps, packed to one index per byte
		static constexpr auto expand = [] (auto m) raze_always_inline_lambda {
			if constexpr (sizeof(T) == 8) return _pdep_u64(m, 0x1000100010001ULL) * 0xFFFF;
			else if constexpr (sizeof(T) == 4) return _pdep_u64(m, 0x0101010101010101) * 0xFF;
		};

		const auto not_mask = mask_not_<ISA, size, T>(mask);
		const auto bytes_vector = _mm_cvtsi64_si128(_pext_u64(identity_vpermps, expand(not_mask)));
		const auto packed = _mm256_permutevar8x32_epi32(as<__m256i>(x), _mm256_cvtepu8_epi32(bytes_vector));
		const auto bytes = math::native_popcnt_n_bits<size>(not_mask) * sizeof(T);

		if constexpr (Unsafe) store_(ptr, packed);
		else store_<ISA, T, false>(ptr, first_n_<ISA, size, V, T>(bytes / sizeof(T)), packed);

		return algorithm::bytes_pointer_offset(ptr, bytes);
	}
}

template <arch::ISA ISA, arithmetic_type T, bool Unsafe, intrin_or_arithmetic_type V, raw_mask_type CompressMask>
raze_always_inline void* compress_store_(void* ptr, V x, CompressMask compress_mask) noexcept {
	constexpr auto size = sizeof(V) / sizeof(T);
	auto int_mask = to_mask_<ISA, T>(compress_mask);		

	if constexpr (sizeof(V) == 16) {
		if constexpr (has_avx512vl<ISA>) {
			if constexpr (sizeof(T) == 8) {
				const auto not_mask = mask_not_<ISA, size, T>(int_mask);
				if constexpr (pd<T>) _mm_mask_compressstoreu_pd(ptr, not_mask, x);
				else _mm_mask_compressstoreu_epi64(ptr, not_mask, x);
				return algorithm::bytes_pointer_offset(ptr, math::native_popcnt_n_bits<size>(not_mask) * sizeof(T));
			}
			else if constexpr (sizeof(T) == 4) {
				const auto not_mask = mask_not_<ISA, size, T>(int_mask);
				if constexpr (ps<T>) _mm_mask_compressstoreu_ps(ptr, not_mask, x);
				else _mm_mask_compressstoreu_epi32(ptr, not_mask, x);
				return algorithm::bytes_pointer_offset(ptr, math::native_popcnt_n_bits<size>(not_mask) * sizeof(T));
			}
			else if constexpr (sizeof(T) == 2 && has_avx512vbmi2<ISA>) {
				const auto not_mask = mask_not_<ISA, size, T>(int_mask);
				_mm_mask_compressstoreu_epi16(ptr, not_mask, as<__m128i>(x));
				return algorithm::bytes_pointer_offset(ptr, math::native_popcnt_n_bits<size>(not_mask) * sizeof(T));
			}
			else if constexpr (sizeof(T) == 1 && has_avx512vbmi2<ISA>) {
				const auto not_mask = mask_not_<ISA, size, T>(int_mask);
				_mm_mask_compressstoreu_epi8(ptr, not_mask, x);
				return algorithm::bytes_pointer_offset(ptr, math::native_popcnt_n_bits<size>(not_mask) * sizeof(T));
			}
		}
		if constexpr (has_ssse3<ISA> && sizeof(T) < 4 && !has_avx512vl<ISA>) {
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
				const auto packed_hi = _mm_shuffle_epi8(x_in_low, shuffle_mask_hi);

				if constexpr (Unsafe) {
					_mm_storel_epi64(reinterpret_cast<__m128i*>(dst_ptr), packed_lo);
					algorithm::advance_bytes(dst_ptr, count_lo);
					_mm_storel_epi64(reinterpret_cast<__m128i*>(dst_ptr), packed_hi);
				}
				else {
					const auto loaded = load_<ISA, V>(ptr);
					_mm_storel_epi64(reinterpret_cast<__m128i*>(dst_ptr), packed_lo);
					algorithm::advance_bytes(dst_ptr, count_lo);
					_mm_storel_epi64(reinterpret_cast<__m128i*>(dst_ptr), packed_hi);
					store_<ISA, T, false>(ptr, mask_not_<ISA, size, T>(first_n_<ISA, size, V, T>(count_lo + count_hi)), loaded);
				}

				return algorithm::bytes_pointer_offset(ptr, count_lo + count_hi);
			}
			else {
				const auto shuffle_mask = load_<ISA, __m128i>(tables_sse<sizeof(T)>.shuffle[int_mask], aligned_policy{});
				const auto packed = _mm_shuffle_epi8(as<__m128i>(x), shuffle_mask);
				const auto bytes = tables_sse<sizeof(T)>.size[int_mask];

				if constexpr (Unsafe) store_(ptr, packed);
				else store_<ISA, T, false>(ptr, first_n_<ISA, size, V, T>(bytes / sizeof(T)), packed);

				return algorithm::bytes_pointer_offset(ptr, bytes);
			}
		}
		else if constexpr (!has_avx512vl<ISA>) {
			if constexpr (sizeof(T) == 8) {
				constexpr auto calculate = [] <class Vec> (auto mask, Vec v) raze_always_inline_lambda -> std::pair<size_t, Vec> {
					switch (mask) {
						case 0: return { 16, v };
						case 1: return { 8, as<Vec>(_mm_shuffle_pd(as<__m128d>(v), as<__m128d>(v), 0x3)) };
						case 2: return { 8, v };
						case 3: return { 0, v };
						default: { raze_assert_unreachable(); return { 0, v }; }
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
						default: { raze_assert_unreachable(); return { 0, v }; }
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
				if constexpr (pd<T>) _mm256_mask_compressstoreu_pd(ptr, not_mask, x);
				else _mm256_mask_compressstoreu_epi64(ptr, not_mask, x);
				return algorithm::bytes_pointer_offset(ptr, math::native_popcnt_n_bits<size>(not_mask) * sizeof(T));
			}
			else if constexpr (sizeof(T) == 4) {
				const auto not_mask = mask_not_<ISA, size, T>(int_mask);
				if constexpr (ps<T>) _mm256_mask_compressstoreu_ps(ptr, not_mask, x);
				else _mm256_mask_compressstoreu_epi32(ptr, not_mask, x);
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
		if constexpr (sizeof(T) >= 4 && has_avx2<ISA> && !has_avx512vl<ISA>) {
			if constexpr (!__slow_bmi2 && has_bmi2<ISA>) return compress_avx2_bmi2_<ISA, T, Unsafe>(ptr, x, int_mask);
			else return compress_avx2_lut_<ISA, T, Unsafe>(ptr, x, int_mask);
		}
	}
	else if constexpr (sizeof(V) == 64) {
		if constexpr (sizeof(T) == 8) {
			const auto not_mask = mask_not_<ISA, size, T>(int_mask);
			if constexpr (pd<T>) _mm512_mask_compressstoreu_pd(ptr, not_mask, x);
			else _mm512_mask_compressstoreu_epi64(ptr, not_mask, x);
			return algorithm::bytes_pointer_offset(ptr, math::native_popcnt_n_bits<size>(not_mask) * sizeof(T));
		}
		else if constexpr (sizeof(T) == 4) {
			const auto not_mask = mask_not_<ISA, size, T>(int_mask);
			if constexpr (ps<T>) _mm512_mask_compressstoreu_ps(ptr, not_mask, x);
			else _mm512_mask_compressstoreu_epi32(ptr, not_mask, x);
			return algorithm::bytes_pointer_offset(ptr, math::native_popcnt_n_bits<size>(not_mask) * sizeof(T));
		}
		else if constexpr (sizeof(T) == 2 && has_avx512vbmi2<ISA>) {
			const auto not_mask = mask_not_<ISA, size, T>(int_mask);
			_mm512_mask_compressstoreu_epi16(ptr, not_mask, as<__m512i>(x));
			return algorithm::bytes_pointer_offset(ptr, math::native_popcnt_n_bits<size>(not_mask) * sizeof(T));
		}
		else if constexpr (sizeof(T) == 1 && has_avx512vbmi2<ISA>) {
			const auto not_mask = mask_not_<ISA, size, T>(int_mask);
			_mm512_mask_compressstoreu_epi8(ptr, not_mask, as<__m512i>(x));
			return algorithm::bytes_pointer_offset(ptr, math::native_popcnt_n_bits<size>(not_mask) * sizeof(T));
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
