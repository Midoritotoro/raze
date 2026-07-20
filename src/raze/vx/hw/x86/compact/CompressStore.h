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

template <arch::ISA _ISA_, arithmetic_type _Type_>
struct _Compress_store {
	template <intrin_type _Tp_, std::unsigned_integral _CompressMask_, class _AlignmentPolicy_ = __unaligned_policy>
	raze_nodiscard static raze_always_inline void* __fallback(void* __ptr, 
		_Tp_ __x, _CompressMask_ __compress_mask, _AlignmentPolicy_ __policy = {}) noexcept 
	{
		constexpr auto __length = sizeof(_Tp_) / sizeof(_Type_);
		alignas(sizeof(_Tp_)) _Type_ __source[__length];

		_Store<_ISA_>()(__source, __x, __aligned_policy{});

		_Type_* __result_pointer = reinterpret_cast<_Type_*>(__ptr);
		auto __start = __result_pointer;

		for (auto __index = 0; __index < __length; ++__index)
			if (!((__compress_mask >> __index) & 1))
				*__result_pointer++ = __source[__index];

		return algorithm::__bytes_pointer_offset(__ptr, (__result_pointer - __start) * sizeof(_Type_));
	}

	template <intrin_or_arithmetic_type _Tp_, raw_mask_type _CompressMask_, class _AlignmentPolicy_ = __unaligned_policy>
	raze_nodiscard raze_static_operator raze_always_inline void* operator()(void* __ptr, _Tp_ __x, 
		_CompressMask_ __compress_mask, _AlignmentPolicy_ __policy = {}) raze_const_operator noexcept
	{
		constexpr auto __ssse3 = __has_ssse3_support_v<_ISA_>;
		constexpr auto __avx2 = __has_avx2_support_v<_ISA_>;
		constexpr auto __avx512bw = __has_avx512bw_support_v<_ISA_>;
		constexpr auto __avx512vl = __has_avx512vl_support_v<_ISA_>;
		constexpr auto __avx512vbmi2 = __has_avx512vbmi2_support_v<_ISA_>;
		constexpr auto __size = sizeof(_Tp_) / sizeof(_Type_);

		const auto __int_mask = _To_mask<_ISA_, _Type_>()(__compress_mask);

		if constexpr (sizeof(_Tp_) == 16) {
			if constexpr (__avx512vl) {
				if constexpr (sizeof(_Type_) == 8) {
					const auto __not_mask = _Mask_not<_ISA_, _Type_>()(__int_mask);
					_mm_mask_compressstoreu_epi64(__ptr, __not_mask, __as<__m128i>(__x));
					return algorithm::__bytes_pointer_offset(__ptr, math::__native_popcnt_n_bits<__size>(__not_mask) * sizeof(_Type_));
				}
				else if constexpr (sizeof(_Type_) == 4) {
					const auto __not_mask = _Mask_not<_ISA_, _Type_>()(__int_mask);
					_mm_mask_compressstoreu_epi32(__ptr, __not_mask, __as<__m128i>(__x));
					return algorithm::__bytes_pointer_offset(__ptr, math::__native_popcnt_n_bits<__size>(__not_mask) * sizeof(_Type_));
				}
				else if constexpr (sizeof(_Type_) == 2 && __avx512vbmi2) {
					const auto __not_mask = _Mask_not<_ISA_, _Type_>()(__int_mask);
					_mm_mask_compressstoreu_epi16(__ptr, __not_mask, __as<__m128i>(__x));
					return algorithm::__bytes_pointer_offset(__ptr, math::__native_popcnt_n_bits<__size>(__not_mask) * sizeof(_Type_));
				}
				else if constexpr (sizeof(_Type_) == 1 && __avx512vbmi2) {
					const auto __not_mask = _Mask_not<_ISA_, _Type_>()(__int_mask);
					_mm_mask_compressstoreu_epi8(__ptr, __not_mask, __as<__m128i>(__x));
					return algorithm::__bytes_pointer_offset(__ptr, math::__native_popcnt_n_bits<__size>(__not_mask) * sizeof(_Type_));
				}
			}
			if constexpr (__ssse3 && sizeof(_Type_) < 4) {
				if constexpr (sizeof(_Type_) == 1) {
					const auto __mask_low = __int_mask & 0xFF;
					const auto __mask_high = (__int_mask >> 8) & 0xFF;

					_Type_* __dst_ptr = reinterpret_cast<_Type_*>(__ptr);

					const auto __count_lo = __tables_sse<sizeof(_Type_)>.__size[__mask_low];
					const auto __count_hi = __tables_sse<sizeof(_Type_)>.__size[__mask_high];

					const auto __shuffle_mask_lo = _mm_loadl_epi64(reinterpret_cast<const __m128i*>(__tables_sse<sizeof(_Type_)>.__shuffle[__mask_low]));
					const auto __shuffle_mask_hi = _mm_loadl_epi64(reinterpret_cast<const __m128i*>(__tables_sse<sizeof(_Type_)>.__shuffle[__mask_high]));

					const auto __packed_lo = _mm_shuffle_epi8(__as<__m128i>(__x), __shuffle_mask_lo);
					const auto __packed_hi = _mm_shuffle_epi8(_mm_srli_si128(__as<__m128i>(__x), 8), __as<__m128i>(__shuffle_mask_hi));

					_mm_storel_epi64(reinterpret_cast<__m128i*>(__dst_ptr), __as<__m128i>(__packed_lo));
					algorithm::__advance_bytes(__dst_ptr, __count_lo);
					_mm_storel_epi64(reinterpret_cast<__m128i*>(__dst_ptr), __as<__m128i>(__packed_hi));

					return algorithm::__bytes_pointer_offset(__ptr, __count_lo + __count_hi);
				}
				else {
					const auto __shuffle_mask = _Load<_ISA_, __m128i>()(__tables_sse<sizeof(_Type_)>.__shuffle[__int_mask], __aligned_policy{});
					const auto __processed_bytes = __tables_sse<sizeof(_Type_)>.__size[__int_mask];
					_mm_storeu_si128(reinterpret_cast<__m128i*>(__ptr), _mm_shuffle_epi8(__as<__m128i>(__x), __shuffle_mask));
					return algorithm::__bytes_pointer_offset(__ptr, __processed_bytes);
				}
			}
			else {
				if constexpr (sizeof(_Type_) == 8) {
					constexpr auto __calculate = [] <class _Vec_> (auto __mask, _Vec_ __v) raze_always_inline_lambda -> std::pair<size_t, _Vec_> {
						switch (__mask) {
							case 0: return { 16, __v };
							case 1: return { 8, __as<_Vec_>(_mm_shuffle_pd(__as<__m128d>(__v), __as<__m128d>(__v), 0x3)) };
							case 2: return { 8, __v };
							case 3: return { 0, __v };
						}
					};

					auto [__processed_bytes, __packed] = __calculate(__int_mask, __x);
					_mm_storeu_si128(reinterpret_cast<__m128i*>(__ptr), __as<__m128i>(__packed));
					return algorithm::__bytes_pointer_offset(__ptr, __processed_bytes);
				}
				else if constexpr (sizeof(_Type_) == 4) {
					constexpr auto __calculate = [] <class _Vec_> (auto __mask, _Vec_ __v) raze_always_inline_lambda -> std::pair<size_t, _Vec_> {
						switch (__mask) {
							case 0x0: return { 16, __v };
							case 0x1: return { 12, __as<_Vec_>(_mm_shuffle_ps(__as<__m128>(__v), __as<__m128>(__v), 0xF9)) };
							case 0x2: return { 12, __as<_Vec_>(_mm_shuffle_ps(__as<__m128>(__v), __as<__m128>(__v), 0xF8)) };
							case 0x3: return { 8, __as<_Vec_>(_mm_shuffle_ps(__as<__m128>(__v), __as<__m128>(__v), 0xEE)) };
							case 0x4: return { 12, __as<_Vec_>(_mm_shuffle_ps(__as<__m128>(__v), __as<__m128>(__v), 0xF4)) };
							case 0x5: return { 8, __as<_Vec_>(_mm_shuffle_ps(__as<__m128>(__v), __as<__m128>(__v), 0xED)) };
							case 0x6: return { 8, __as<_Vec_>(_mm_shuffle_ps(__as<__m128>(__v), __as<__m128>(__v), 0xEC)) };
							case 0x7: return { 4, __as<_Vec_>(_mm_shuffle_ps(__as<__m128>(__v), __as<__m128>(__v), 0xE7)) };
							case 0x8: return { 12, __v };
							case 0x9: return { 8, __as<_Vec_>(_mm_shuffle_ps(__as<__m128>(__v), __as<__m128>(__v), 0xE9)) };
							case 0xA: return { 8, __as<_Vec_>(_mm_shuffle_ps(__as<__m128>(__v), __as<__m128>(__v), 0xE8)) };
							case 0xB: return { 4, __as<_Vec_>(_mm_shuffle_ps(__as<__m128>(__v), __as<__m128>(__v), 0xE6)) };
							case 0xC: return { 8, __v };
							case 0xD: return { 4, __as<_Vec_>(_mm_shuffle_ps(__as<__m128>(__v), __as<__m128>(__v), 0x55)) };
							case 0xE: return { 4, __v };
							case 0xF: return { 0, __v };
						}
					};

					auto [__processed_bytes, __packed] = __calculate(__int_mask, __x);
					_mm_storeu_si128(reinterpret_cast<__m128i*>(__ptr), __as<__m128i>(__packed));
					return algorithm::__bytes_pointer_offset(__ptr, __processed_bytes);
				}
			}
		}
		else if constexpr (sizeof(_Tp_) == 32) {
			if constexpr (__avx512vl) {
				if constexpr (sizeof(_Type_) == 8) {
					const auto __not_mask = _Mask_not<_ISA_, _Type_>()(__int_mask);
					_mm256_mask_compressstoreu_epi64(__ptr, __not_mask, __as<__m256i>(__x));
					return algorithm::__bytes_pointer_offset(__ptr, math::__native_popcnt_n_bits<__size>(__not_mask) * sizeof(_Type_));
				}
				else if constexpr (sizeof(_Type_) == 4) {
					const auto __not_mask = _Mask_not<_ISA_, _Type_>()(__int_mask);
					_mm256_mask_compressstoreu_epi32(__ptr, __not_mask, __as<__m256i>(__x));
					return algorithm::__bytes_pointer_offset(__ptr, math::__native_popcnt_n_bits<__size>(__not_mask) * sizeof(_Type_));
				}
				else if constexpr (sizeof(_Type_) == 2 && __avx512vbmi2) {
					const auto __not_mask = _Mask_not<_ISA_, _Type_>()(__int_mask);
					_mm256_mask_compressstoreu_epi16(__ptr, __not_mask, __as<__m256i>(__x));
					return algorithm::__bytes_pointer_offset(__ptr, math::__native_popcnt_n_bits<__size>(__not_mask) * sizeof(_Type_));
				}
				else if constexpr (sizeof(_Type_) == 1 && __avx512vbmi2) {
					const auto __not_mask = _Mask_not<_ISA_, _Type_>()(__int_mask);
					_mm256_mask_compressstoreu_epi8(__ptr, __not_mask, __as<__m256i>(__x));
					return algorithm::__bytes_pointer_offset(__ptr, math::__native_popcnt_n_bits<__size>(__not_mask) * sizeof(_Type_));
				}
			}
			if constexpr (sizeof(_Type_) >= 4 && __avx2) {
				const auto __shuffle = _mm_loadl_epi64(reinterpret_cast<const __m128i*>(__tables_avx<sizeof(_Type_)>.__shuffle[__int_mask]));
				const auto __processed_bytes = __tables_avx<sizeof(_Type_)>.__size[__int_mask];
				_mm256_storeu_si256(reinterpret_cast<__m256i*>(__ptr), _mm256_permutevar8x32_epi32(__as<__m256i>(__x), _mm256_cvtepu8_epi32(__shuffle)));
				return algorithm::__bytes_pointer_offset(__ptr, __processed_bytes);
			}
			else if constexpr (sizeof(_Type_) == 2) {
				_Type_* __write_ptr = reinterpret_cast<_Type_*>(__ptr);

				const auto __vec_low = __as<__m128i>(__x);
				const auto __vec_high = _mm256_extracti128_si256(__as<__m256i>(__x), 1);

				const auto __mask_low = __int_mask & 0xFF;
				const auto __mask_high = (__int_mask >> 8) & 0xFF;

				const auto __count_bytes_low = __tables_sse<sizeof(_Type_)>.__size[__mask_low];
				const auto __count_bytes_high = __tables_sse<sizeof(_Type_)>.__size[__mask_high];

				const auto __total_bytes = __count_bytes_low + __count_bytes_high;

				const auto __shuffle_mask_low = _mm_load_si128(reinterpret_cast<const __m128i*>(__tables_sse<sizeof(_Type_)>.__shuffle[__mask_low]));
				const auto __shuffle_mask_high = _mm_load_si128(reinterpret_cast<const __m128i*>(__tables_sse<sizeof(_Type_)>.__shuffle[__mask_high]));

				const auto __packed_low = _mm_shuffle_epi8(__vec_low, __shuffle_mask_low);
				const auto __packed_high = _mm_shuffle_epi8(__vec_high, __shuffle_mask_high);

				_mm_storeu_si128(reinterpret_cast<__m128i*>(__write_ptr), __as<__m128i>(__packed_low));
				algorithm::__advance_bytes(__write_ptr, __count_bytes_low);

				_mm_storeu_si128(reinterpret_cast<__m128i*>(__write_ptr), __as<__m128i>(__packed_high));
				return algorithm::__bytes_pointer_offset(__ptr, __total_bytes);
			}
			else if constexpr (sizeof(_Type_) == 1) {
				_Type_* __dst_ptr = reinterpret_cast<_Type_*>(__ptr);

				const auto __vec_low = __as<__m128i>(__x);
				const auto __vec_high = _mm256_extracti128_si256(__as<__m256i>(__x), 1);

				const auto __vec_low_hi = __as<__m128i>(_mm_movehl_ps(
					__as<__m128>(_mm_slli_si128(__vec_low, 8)), __as<__m128>(__vec_low)));

				const auto __vec_high_hi = __as<__m128i>(_mm_movehl_ps(
					__as<__m128>(_mm_slli_si128(__vec_high, 8)), __as<__m128>(__vec_high)));

				const auto __mask_1 = __int_mask & 0xFF;
				const auto __mask_2 = (__int_mask >> 8) & 0xFF;
				const auto __mask_3 = (__int_mask >> 16) & 0xFF;
				const auto __mask_4 = (__int_mask >> 24) & 0xFF;

				const auto __bytes_1 = __tables_sse<sizeof(_Type_)>.__size[__mask_1];
				const auto __bytes_2 = __tables_sse<sizeof(_Type_)>.__size[__mask_2];
				const auto __bytes_3 = __tables_sse<sizeof(_Type_)>.__size[__mask_3];
				const auto __bytes_4 = __tables_sse<sizeof(_Type_)>.__size[__mask_4];

				const auto __bytes_low = __bytes_1 + __bytes_2;
				const auto __bytes_high = __bytes_3 + __bytes_4;

				const auto __total_bytes = __bytes_low + __bytes_high;

				const auto __shuffle_1 = _mm_loadl_epi64(reinterpret_cast<const __m128i*>(__tables_sse<sizeof(_Type_)>.__shuffle[__mask_1]));
				const auto __shuffle_2 = _mm_loadl_epi64(reinterpret_cast<const __m128i*>(__tables_sse<sizeof(_Type_)>.__shuffle[__mask_2]));
				const auto __shuffle_3 = _mm_loadl_epi64(reinterpret_cast<const __m128i*>(__tables_sse<sizeof(_Type_)>.__shuffle[__mask_3]));
				const auto __shuffle_4 = _mm_loadl_epi64(reinterpret_cast<const __m128i*>(__tables_sse<sizeof(_Type_)>.__shuffle[__mask_4]));

				const auto __packed_1 = _mm_shuffle_epi8(__vec_low, __shuffle_1);
				const auto __packed_2 = _mm_shuffle_epi8(__vec_low_hi, __shuffle_2);
				const auto __packed_3 = _mm_shuffle_epi8(__vec_high, __shuffle_3);
				const auto __packed_4 = _mm_shuffle_epi8(__vec_high_hi, __shuffle_4);

				_mm_storel_epi64(reinterpret_cast<__m128i*>(__dst_ptr), __as<__m128i>(__packed_1));
				__dst_ptr += __bytes_1;

				_mm_storel_epi64(reinterpret_cast<__m128i*>(__dst_ptr), __as<__m128i>(__packed_2));
				__dst_ptr += __bytes_2;

				_mm_storel_epi64(reinterpret_cast<__m128i*>(__dst_ptr), __as<__m128i>(__packed_3));
				__dst_ptr += __bytes_3;

				_mm_storel_epi64(reinterpret_cast<__m128i*>(__dst_ptr), __as<__m128i>(__packed_4));

				return algorithm::__bytes_pointer_offset(__ptr, __total_bytes);
			}
		}
		else if constexpr (sizeof(_Tp_) == 64) {
			if constexpr (sizeof(_Type_) == 8) {
				const auto __not_mask = _Mask_not<_ISA_, _Type_>()(__int_mask);
				_mm512_mask_compressstoreu_epi64(__ptr, __not_mask, __as<__m512i>(__x));
				return algorithm::__bytes_pointer_offset(__ptr, math::__native_popcnt_n_bits<__size>(__not_mask) * sizeof(_Type_));
			}
			else if constexpr (sizeof(_Type_) == 4) {
				const auto __not_mask = _Mask_not<_ISA_, _Type_>()(__int_mask);
				_mm512_mask_compressstoreu_epi32(__ptr, __not_mask, __as<__m512i>(__x));
				return algorithm::__bytes_pointer_offset(__ptr, math::__native_popcnt_n_bits<__size>(__not_mask) * sizeof(_Type_));
			}
			else if constexpr (sizeof(_Type_) == 2) {
				if constexpr (__avx512vbmi2) {
					const auto __not_mask = _Mask_not<_ISA_, _Type_>()(__int_mask);
					_mm512_mask_compressstoreu_epi16(__ptr, __not_mask, __as<__m512i>(__x));
					return algorithm::__bytes_pointer_offset(__ptr, math::__native_popcnt_n_bits<__size>(__not_mask) * sizeof(_Type_));
				}
				else {
					_Type_* __dst_ptr = reinterpret_cast<_Type_*>(__ptr);

					const auto __vec1 = __as<__m128i>(__x);
					const auto __vec2 = _mm256_extracti128_si256(__as<__m256i>(__x), 1);
					const auto __vec3 = __as<__m128i>(_mm512_extractf32x4_ps(__as<__m512>(__x), 2));
					const auto __vec4 = __as<__m128i>(_mm512_extractf32x4_ps(__as<__m512>(__x), 3));

					const auto __mask1 = __int_mask & 0xFF;
					const auto __mask2 = (__int_mask >> 8) & 0xFF;
					const auto __mask3 = (__int_mask >> 16) & 0xFF;
					const auto __mask4 = (__int_mask >> 24) & 0xFF;

					const auto __bytes1 = __tables_sse<sizeof(_Type_)>.__size[__mask1];
					const auto __bytes2 = __tables_sse<sizeof(_Type_)>.__size[__mask2];
					const auto __bytes3 = __tables_sse<sizeof(_Type_)>.__size[__mask3];
					const auto __bytes4 = __tables_sse<sizeof(_Type_)>.__size[__mask4];

					const auto __total_bytes = __bytes1 + __bytes2 + __bytes3 + __bytes4;
					const auto __total_elems = __total_bytes / sizeof(_Type_);

					const auto __shuffle1 = _mm_load_si128(reinterpret_cast<const __m128i*>(__tables_sse<sizeof(_Type_)>.__shuffle[__mask1]));
					const auto __shuffle2 = _mm_load_si128(reinterpret_cast<const __m128i*>(__tables_sse<sizeof(_Type_)>.__shuffle[__mask2]));
					const auto __shuffle3 = _mm_load_si128(reinterpret_cast<const __m128i*>(__tables_sse<sizeof(_Type_)>.__shuffle[__mask3]));
					const auto __shuffle4 = _mm_load_si128(reinterpret_cast<const __m128i*>(__tables_sse<sizeof(_Type_)>.__shuffle[__mask4]));

					const auto __packed1 = _mm_shuffle_epi8(__vec1, __shuffle1);
					const auto __packed2 = _mm_shuffle_epi8(__vec2, __shuffle2);
					const auto __packed3 = _mm_shuffle_epi8(__vec3, __shuffle3);
					const auto __packed4 = _mm_shuffle_epi8(__vec4, __shuffle4);

					_mm_storeu_si128(reinterpret_cast<__m128i*>(__dst_ptr), __as<__m128i>(__packed1));
					algorithm::__advance_bytes(__dst_ptr, __bytes1);

					_mm_storeu_si128(reinterpret_cast<__m128i*>(__dst_ptr), __as<__m128i>(__packed2));
					algorithm::__advance_bytes(__dst_ptr, __bytes2);

					_mm_storeu_si128(reinterpret_cast<__m128i*>(__dst_ptr), __as<__m128i>(__packed3));
					algorithm::__advance_bytes(__dst_ptr, __bytes3);

					_mm_storeu_si128(reinterpret_cast<__m128i*>(__dst_ptr), __as<__m128i>(__packed4));
					return algorithm::__bytes_pointer_offset(__ptr, __total_bytes);
				}
			}
			else if constexpr (sizeof(_Type_) == 1) {
				if constexpr (__avx512vbmi2) {
					const auto __not_mask = _Mask_not<_ISA_, _Type_>()(__int_mask);
					_mm512_mask_compressstoreu_epi8(__ptr, __not_mask, __as<__m512i>(__x));
					return algorithm::__bytes_pointer_offset(__ptr, math::__native_popcnt_n_bits<__size>(__not_mask) * sizeof(_Type_));
				}
				else {
					_Type_* __write_ptr = reinterpret_cast<_Type_*>(__ptr);

					const auto __ymm_lower = __as<__m256i>(__x);
					const auto __ymm_higher = _mm512_extractf64x4_pd(__as<__m512d>(__x), 1);

					const auto __xmm1 = __as<__m128i>(__ymm_lower);
					const auto __xmm2 = __as<__m128i>(_mm256_extractf128_pd(__as<__m256d>(__ymm_lower), 1));

					const auto __xmm3 = __as<__m128i>(__ymm_higher);
					const auto __xmm4 = __as<__m128i>(_mm256_extractf128_pd(__as<__m256d>(__ymm_higher), 1));

					const auto __xmm1_upper = __as<__m128i>(_mm_movehl_ps(
						__as<__m128>(_mm_slli_si128(__xmm1, 8)), __as<__m128>(__xmm1)));

					const auto __xmm2_upper = __as<__m128i>(_mm_movehl_ps(
						__as<__m128>(_mm_slli_si128(__xmm2, 8)), __as<__m128>(__xmm2)));

					const auto __xmm3_upper = __as<__m128i>(_mm_movehl_ps(
						__as<__m128>(_mm_slli_si128(__xmm3, 8)), __as<__m128>(__xmm3)));

					const auto __xmm4_upper = __as<__m128i>(_mm_movehl_ps(
						__as<__m128>(_mm_slli_si128(__xmm4, 8)), __as<__m128>(__xmm4)));

					const auto __mask1 = __int_mask & 0xFF;
					const auto __mask2 = (__int_mask >> 8) & 0xFF;
					const auto __mask3 = (__int_mask >> 16) & 0xFF;
					const auto __mask4 = (__int_mask >> 24) & 0xFF;
					const auto __mask5 = (__int_mask >> 32) & 0xFF;
					const auto __mask6 = (__int_mask >> 40) & 0xFF;
					const auto __mask7 = (__int_mask >> 48) & 0xFF;
					const auto __mask8 = (__int_mask >> 56) & 0xFF;

					const auto __bytes1 = __tables_sse<sizeof(_Type_)>.__size[__mask1];
					const auto __bytes2 = __tables_sse<sizeof(_Type_)>.__size[__mask2];
					const auto __bytes3 = __tables_sse<sizeof(_Type_)>.__size[__mask3];
					const auto __bytes4 = __tables_sse<sizeof(_Type_)>.__size[__mask4];
					const auto __bytes5 = __tables_sse<sizeof(_Type_)>.__size[__mask5];
					const auto __bytes6 = __tables_sse<sizeof(_Type_)>.__size[__mask6];
					const auto __bytes7 = __tables_sse<sizeof(_Type_)>.__size[__mask7];
					const auto __bytes8 = __tables_sse<sizeof(_Type_)>.__size[__mask8];

					const auto __bytes_lane1 = __bytes1 + __bytes2;
					const auto __bytes_lane2 = __bytes3 + __bytes4;
					const auto __bytes_lane3 = __bytes5 + __bytes6;
					const auto __bytes_lane4 = __bytes7 + __bytes8;

					const auto __bytes_total = __bytes_lane1 + __bytes_lane2 + __bytes_lane3 + __bytes_lane4;

					const auto __shuffle1 = _mm_loadl_epi64(reinterpret_cast<const __m128i*>(__tables_sse<sizeof(_Type_)>.__shuffle[__mask1]));
					const auto __shuffle2 = _mm_loadl_epi64(reinterpret_cast<const __m128i*>(__tables_sse<sizeof(_Type_)>.__shuffle[__mask2]));
					const auto __shuffle3 = _mm_loadl_epi64(reinterpret_cast<const __m128i*>(__tables_sse<sizeof(_Type_)>.__shuffle[__mask3]));
					const auto __shuffle4 = _mm_loadl_epi64(reinterpret_cast<const __m128i*>(__tables_sse<sizeof(_Type_)>.__shuffle[__mask4]));
					const auto __shuffle5 = _mm_loadl_epi64(reinterpret_cast<const __m128i*>(__tables_sse<sizeof(_Type_)>.__shuffle[__mask5]));
					const auto __shuffle6 = _mm_loadl_epi64(reinterpret_cast<const __m128i*>(__tables_sse<sizeof(_Type_)>.__shuffle[__mask6]));
					const auto __shuffle7 = _mm_loadl_epi64(reinterpret_cast<const __m128i*>(__tables_sse<sizeof(_Type_)>.__shuffle[__mask7]));
					const auto __shuffle8 = _mm_loadl_epi64(reinterpret_cast<const __m128i*>(__tables_sse<sizeof(_Type_)>.__shuffle[__mask8]));

					const auto __packed1 = _mm_shuffle_epi8(__xmm1, __shuffle1);
					const auto __packed2 = _mm_shuffle_epi8(__xmm1_upper, __shuffle2);
					const auto __packed3 = _mm_shuffle_epi8(__xmm2, __shuffle3);
					const auto __packed4 = _mm_shuffle_epi8(__xmm2_upper, __shuffle4);
					const auto __packed5 = _mm_shuffle_epi8(__xmm3, __shuffle5);
					const auto __packed6 = _mm_shuffle_epi8(__xmm3_upper, __shuffle6);
					const auto __packed7 = _mm_shuffle_epi8(__xmm4, __shuffle7);
					const auto __packed8 = _mm_shuffle_epi8(__xmm4_upper, __shuffle8);

					_mm_storel_epi64(reinterpret_cast<__m128i*>(__write_ptr), __as<__m128i>(__packed1));
					algorithm::__advance_bytes(__write_ptr, __bytes1);

					_mm_storel_epi64(reinterpret_cast<__m128i*>(__write_ptr), __as<__m128i>(__packed2));
					algorithm::__advance_bytes(__write_ptr, __bytes2);

					_mm_storel_epi64(reinterpret_cast<__m128i*>(__write_ptr), __as<__m128i>(__packed3));
					algorithm::__advance_bytes(__write_ptr, __bytes3);

					_mm_storel_epi64(reinterpret_cast<__m128i*>(__write_ptr), __as<__m128i>(__packed4));
					algorithm::__advance_bytes(__write_ptr, __bytes4);

					_mm_storel_epi64(reinterpret_cast<__m128i*>(__write_ptr), __as<__m128i>(__packed5));
					algorithm::__advance_bytes(__write_ptr, __bytes5);

					_mm_storel_epi64(reinterpret_cast<__m128i*>(__write_ptr), __as<__m128i>(__packed6));
					algorithm::__advance_bytes(__write_ptr, __bytes6);

					_mm_storel_epi64(reinterpret_cast<__m128i*>(__write_ptr), __as<__m128i>(__packed7));
					algorithm::__advance_bytes(__write_ptr, __bytes7);

					_mm_storel_epi64(reinterpret_cast<__m128i*>(__write_ptr), __as<__m128i>(__packed8));
					return algorithm::__bytes_pointer_offset(__ptr, __bytes_total);
				}
			}
		}

		if constexpr (arithmetic_type<_Tp_>) {
			if (!__compress_mask) {
				reinterpret_cast<_Tp_*>(__ptr)[0] = __x;
				return algorithm::__bytes_pointer_offset(__ptr, sizeof(_Tp_));
			}
			else {
				return __ptr;
			}
		}
		else return __fallback(__ptr, __x, __int_mask, __policy);
	}
};

__RAZE_VX_NAMESPACE_END
