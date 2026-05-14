//#pragma once 
//
//#include <src/raze/vx/hw/x86/mask/operations/ToMask.h>
//#include <src/raze/vx/hw/x86/compact/CompressTables.h>
//#include <src/raze/vx/hw/x86/memory/Load.h>
//#include <src/raze/vx/hw/x86/memory/Store.h>
//#include <src/raze/vx/hw/x86/merge/Select.h>
//#include <src/raze/math/PopulationCount.h>
//#include <src/raze/algorithm/AdvanceBytes.h>
//#include <src/raze/vx/hw/x86/mask/operations/MaskNot.h>
//#include <utility>
//
//__RAZE_VX_NAMESPACE_BEGIN
//
//template <arch::ISA _ISA_, arithmetic_type _Type_> 
//struct _Compress {
//	template <intrin_type _Tp_, std::unsigned_integral _CompressMask_>
//	raze_nodiscard static raze_always_inline std::pair<i32, _Tp_> __fallback(_Tp_ __x, _CompressMask_ __compress_mask) noexcept {
//		constexpr auto __length = sizeof(_Tp_) / sizeof(_Type_);
//		alignas(sizeof(_Tp_)) _Type_ __source[__length], __result[__length];
//
//		_Store<_ISA_>()(__source, __x);
//
//		_Type_* __result_pointer = __result;
//		auto __start = __result_pointer;
//
//		for (auto __index = 0; __index < __length; ++__index)
//			if (!((__compress_mask >> __index) & 1))
//				*__result_pointer++ = __source[__index];
//
//		const auto __processed_size = (__result_pointer - __start);
//		std::memcpy(__result_pointer, __source + __processed_size, sizeof(_Tp_) - __processed_size * sizeof(_Type_));
//
//		return { __processed_size, _Load<_ISA_, _Tp_>()(__result) };
//	}
//
//	template <intrin_or_arithmetic_type _Tp_, raw_mask_type _CompressMask_>
//	raze_nodiscard raze_static_operator raze_always_inline std::pair<i32, _Tp_> operator()(_Tp_ __x, _CompressMask_ __compress_mask) raze_const_operator noexcept {
//		constexpr auto __ssse3 = __has_ssse3_support_v<_ISA_>;
//		constexpr auto __avx2 = __has_avx2_support_v<_ISA_>;
//		constexpr auto __avx512bw = __has_avx512bw_support_v<_ISA_>;
//		constexpr auto __avx512vl = __has_avx512vl_support_v<_ISA_>;
//		constexpr auto __avx512vbmi2 = __has_avx512vbmi2_support_v<_ISA_>;
//		constexpr auto __size = sizeof(_Tp_) / sizeof(_Type_);
//
//		const auto __int_mask = _To_mask<_ISA_, _Type_>()(__compress_mask);
//
//		if constexpr (sizeof(_Tp_) == 16) {
//			if constexpr (__avx512vl) {
//				const auto __not_mask = _Mask_not<_ISA_, _Type_>()(__not_mask);
//				const auto __set_bits = math::__native_popcnt_n_bits<__size>(__not_mask);
//
//				if constexpr (sizeof(_Type_) == 8) return { __set_bits, __as<_Tp_>(_mm_mask_compress_epi64(__as<__m128i>(__x), __not_mask, __as<__m128i>(__x))) };
//				else if constexpr (sizeof(_Type_) == 4) return { __set_bits, __as<_Tp_>(_mm_mask_compress_epi32( __as<__m128i>(__x), __not_mask, __as<__m128i>(__x))) };
//				else if constexpr (sizeof(_Type_) == 2 && __avx512vbmi2) return { __set_bits, __as<_Tp_>(_mm_mask_compress_epi16( __as<__m128i>(__x), __not_mask, __as<__m128i>(__x))) };
//				else if constexpr (sizeof(_Type_) == 1 && __avx512vbmi2) return { __set_bits, __as<_Tp_>(_mm_mask_compress_epi8( __as<__m128i>(__x), __not_mask, __as<__m128i>(__x))) };
//			}
//			if constexpr (__ssse3) {
//				if constexpr (sizeof(_Type_) == 1) {
//					const auto __mask_low = __int_mask & 0xFF;
//					const auto __mask_high = (__int_mask >> 8) & 0xFF;
//
//					alignas(sizeof(_Tp_)) _Type_ __temp_buf[__size];
//					_Type_* __dst_ptr = __temp_buf;
//
//					const auto __count_lo = __tables_sse<sizeof(_Type_)>.__size[__mask_low];
//					const auto __count_hi = __tables_sse<sizeof(_Type_)>.__size[__mask_high];
//
//					const auto __total_bytes = __count_lo + __count_hi;
//					const auto __tail_mask = (__mmask_for_elements_t<__size>(
//						1u << (sizeof(_Tp_) - __total_bytes)) - 1) << __total_bytes;
//
//					const auto __shuffle_mask_lo = _mm_loadl_epi64(reinterpret_cast<const __m128i*>(__tables_sse<sizeof(_Type_)>.__shuffle[__mask_low]));
//					const auto __shuffle_mask_hi = _mm_loadl_epi64(reinterpret_cast<const __m128i*>(__tables_sse<sizeof(_Type_)>.__shuffle[__mask_high]));
//
//					const auto __vec_upper = __as<__m128i>(_mm_movehl_ps(
//						__as<__m128>(_mm_slli_si128(__as<__m128i>(__x), 8)), __as<__m128>(__x)));
//
//					const auto __packed_lo = _mm_shuffle_epi8(__as<__m128i>(__x), __shuffle_mask_lo);
//					const auto __packed_hi = _mm_shuffle_epi8(__vec_upper, __as<__m128i>(__shuffle_mask_hi));
//
//					_mm_storel_epi64(reinterpret_cast<__m128i*>(__dst_ptr), __as<__m128i>(__packed_lo));
//					algorithm::__advance_bytes(__dst_ptr, __count_lo);
//
//					_mm_storel_epi64(reinterpret_cast<__m128i*>(__dst_ptr), __as<__m128i>(__packed_hi));
//
//					const auto __packed_vec = _mm_load_si128(reinterpret_cast<const __m128i*>(__temp_buf));
//					const auto __result_vec = _Select<_ISA_, _Type_>()(__as<__m128i>(__x), __packed_vec, __tail_mask);
//
//					return { __total_bytes / sizeof(_Type_), __as<_Tp_>(__result_vec) };
//				}
//				else {
//					const auto __shuffle_mask = _Load<_ISA_, __m128i>(__tables_sse<sizeof(_Type_)>.__shuffle[__int_mask]);
//					const auto __processed_bytes = __tables_sse<sizeof(_Type_)>.__size[__int_mask];
//					return { __processed_bytes / sizeof(_Type_), __as<_Tp_>(_mm_shuffle_epi8(__as<__m128i>(__x), __shuffle_mask)) };
//				}
//			}
//			else {
//				if constexpr (sizeof(_Type_) == 8) {
//					switch (__int_mask) {
//						case 0: return { 2, __x };
//						case 1: return { 1, __as<_Tp_>(_mm_shuffle_pd(__as<__m128d>(__x), __as<__m128d>(__x), 0x3)) };
//						case 2: return { 1, __x };
//						default: raze_assert_unreachable();
//					}
//				}
//				else if constexpr (sizeof(_Type_) == 4) {
//					switch (__int_mask) {
//						case 0x0: return { 4, __x };
//						case 0x1: return { 3, __as<_Tp_>(_mm_shuffle_ps(__as<__m128>(__x), __as<__m128>(__x), 0xF9)) };
//						case 0x2: return { 3, __as<_Tp_>(_mm_shuffle_ps(__as<__m128>(__x), __as<__m128>(__x), 0xF8)) };
//						case 0x3: return { 2, __as<_Tp_>(_mm_shuffle_ps(__as<__m128>(__x), __as<__m128>(__x), 0xEE)) };
//						case 0x4: return { 3, __as<_Tp_>(_mm_shuffle_ps(__as<__m128>(__x), __as<__m128>(__x), 0xF4)) };
//						case 0x5: return { 2, __as<_Tp_>(_mm_shuffle_ps(__as<__m128>(__x), __as<__m128>(__x), 0xED)) };
//						case 0x6: return { 2, __as<_Tp_>(_mm_shuffle_ps(__as<__m128>(__x), __as<__m128>(__x), 0xEC)) };
//						case 0x7: return { 1, __as<_Tp_>(_mm_shuffle_ps(__as<__m128>(__x), __as<__m128>(__x), 0xE7)) };
//						case 0x8: return { 3, __x };
//						case 0x9: return { 2, __as<_Tp_>(_mm_shuffle_ps(__as<__m128>(__x), __as<__m128>(__x), 0xE9)) };
//						case 0xA: return { 2, __as<_Tp_>(_mm_shuffle_ps(__as<__m128>(__x), __as<__m128>(__x), 0xE8)) };
//						case 0xB: return { 1, __as<_Tp_>(_mm_shuffle_ps(__as<__m128>(__x), __as<__m128>(__x), 0xE6)) };
//						case 0xC: return { 3, __as<_Tp_>(_mm_shuffle_ps(__as<__m128>(__x), __as<__m128>(__x), 0xE5)) };
//						case 0xD: return { 1, __as<_Tp_>(_mm_shuffle_ps(__as<__m128>(__x), __as<__m128>(__x), 0xE5)) };
//						case 0xE: return { 1, __x };
//						case 0xF: return { 0, __x };
//						default: raze_assert_unreachable();
//					}
//				}
//			}
//		}
//		else if constexpr (sizeof(_Tp_) == 32) {
//			if constexpr (__avx512vl) {
//				const auto __not_mask = _Mask_not<_ISA_, _Type_>()(__not_mask);
//				const auto __set_bits = math::__native_popcnt_n_bits<__size>(__not_mask);
//
//				if constexpr (sizeof(_Type_) == 8) return { __set_bits, __as<_Tp_>(_mm256_mask_compress_epi64(__as<__m256i>(__x), __not_mask, __as<__m256i>(__x))) };
//				else if constexpr (sizeof(_Type_) == 4) return { __set_bits, __as<_Tp_>(_mm256_mask_compress_epi32( __as<__m256i>(__x), __not_mask, __as<__m256i>(__x))) };
//				else if constexpr (sizeof(_Type_) == 2 && __avx512vbmi2) return { __set_bits, __as<_Tp_>(_mm256_mask_compress_epi16( __as<__m256i>(__x), __not_mask, __as<__m256i>(__x))) };
//				else if constexpr (sizeof(_Type_) == 1 && __avx512vbmi2) return { __set_bits, __as<_Tp_>(_mm256_mask_compress_epi8( __as<__m256i>(__x), __not_mask, __as<__m256i>(__x))) };
//			}
//			if constexpr (sizeof(_Type_) >= 4 && __avx2) {
//				const auto __shuffle = _mm_loadl_epi64(reinterpret_cast<const __m128i*>(__tables_avx<sizeof(_Type_)>.__shuffle[__int_mask]));
//				return { __tables_avx<sizeof(_Type_)>.__size[__int_mask] / sizeof(_Type_), 
//					__as<_Tp_>(_mm256_permutevar8x32_epi32(__as<__m256i>(__x),  _mm256_cvtepu8_epi32(__shuffle)))};
//			}
//			else if constexpr (sizeof(_Type_) == 2) {
//				alignas(sizeof(_Tp_)) _Type_ __temp_buf[__size];
//				_Type_* __write_ptr = __temp_buf;
//
//				const auto __vec_low = __as<__m128i>(__x);
//				const auto __vec_high = _mm256_extracti128_si256(__as<__m256i>(__x), 1);
//
//				const auto __mask_low = __int_mask & 0xFF;
//				const auto __mask_high = (__int_mask >> 8) & 0xFF;
//
//				const auto __count_bytes_low = __tables_sse<sizeof(_Type_)>.__size[__mask_low];
//				const auto __count_bytes_high = __tables_sse<sizeof(_Type_)>.__size[__mask_high];
//
//				const auto __total_bytes = __count_bytes_low + __count_bytes_high;
//				const auto __total_elements = __total_bytes / sizeof(_Type_);
//
//				const auto __tail_blend_mask = (__mmask_for_elements_t<__size>(
//					1u << (__size - __total_elements)) - 1) << __total_elements;
//
//				const auto __shuffle_mask_low = _mm_load_si128(reinterpret_cast<const __m128i*>(__tables_sse<sizeof(_Type_)>.__shuffle[__mask_low]));
//				const auto __shuffle_mask_high = _mm_load_si128(reinterpret_cast<const __m128i*>(__tables_sse<sizeof(_Type_)>.__shuffle[__mask_high]));
//
//				const auto __packed_low = _mm_shuffle_epi8(__vec_low, __shuffle_mask_low);
//				const auto __packed_high = _mm_shuffle_epi8(__vec_high, __shuffle_mask_high);
//
//				_mm_store_si128(reinterpret_cast<__m128i*>(__write_ptr), __as<__m128i>(__packed_low));
//				algorithm::__advance_bytes(__write_ptr, __count_bytes_low);
//
//				_mm_store_si128(reinterpret_cast<__m128i*>(__write_ptr), __as<__m128i>(__packed_high));
//
//				const auto __packed_vec = _mm256_load_si256(reinterpret_cast<const __m256i*>(__temp_buf));
//				const auto __result_vec = _Select<_ISA_, _Type_>()(__as<__m256i>(__x), __packed_vec, __tail_blend_mask);
//
//				return { __total_bytes / sizeof(_Type_), __as<_Tp_>(__result_vec) };
//			}
//			else if constexpr (sizeof(_Type_) == 1) {
//				alignas(sizeof(_Tp_)) _Type_ __temp_buf[__size];
//				_Type_* __dst_ptr = __temp_buf;
//
//				const auto __vec_low = __as<__m128i>(__x);
//				const auto __vec_high = _mm256_extracti128_si256(__as<__m256i>(__x), 1);
//
//				const auto __vec_low_hi = __as<__m128i>(_mm_movehl_ps(
//					__as<__m128>(_mm_slli_si128(__vec_low, 8)), __as<__m128>(__vec_low)));
//
//				const auto __vec_high_hi = __as<__m128i>(_mm_movehl_ps(
//					__as<__m128>(_mm_slli_si128(__vec_high, 8)), __as<__m128>(__vec_high)));
//
//				const auto __mask_1 = __int_mask & 0xFF;
//				const auto __mask_2 = (__int_mask >> 8) & 0xFF;
//				const auto __mask_3 = (__int_mask >> 16) & 0xFF;
//				const auto __mask_4 = (__int_mask >> 24) & 0xFF;
//
//				const auto __bytes_1 = __tables_sse<sizeof(_Type_)>.__size[__mask_1];
//				const auto __bytes_2 = __tables_sse<sizeof(_Type_)>.__size[__mask_2];
//				const auto __bytes_3 = __tables_sse<sizeof(_Type_)>.__size[__mask_3];
//				const auto __bytes_4 = __tables_sse<sizeof(_Type_)>.__size[__mask_4];
//
//				const auto __bytes_low = __bytes_1 + __bytes_2;
//				const auto __bytes_high = __bytes_3 + __bytes_4;
//
//				const auto __total_bytes = __bytes_low + __bytes_high;
//				const auto __tail_mask = (__mmask_for_elements_t<__size>(
//					1u << (sizeof(_Tp_) - __total_bytes)) - 1) << __total_bytes;
//
//				const auto __shuffle_1 = _mm_loadl_epi64(reinterpret_cast<const __m128i*>(__tables_sse<sizeof(_Type_)>.__shuffle[__mask_1]));
//				const auto __shuffle_2 = _mm_loadl_epi64(reinterpret_cast<const __m128i*>(__tables_sse<sizeof(_Type_)>.__shuffle[__mask_2]));
//				const auto __shuffle_3 = _mm_loadl_epi64(reinterpret_cast<const __m128i*>(__tables_sse<sizeof(_Type_)>.__shuffle[__mask_3]));
//				const auto __shuffle_4 = _mm_loadl_epi64(reinterpret_cast<const __m128i*>(__tables_sse<sizeof(_Type_)>.__shuffle[__mask_4]));
//
//				const auto __packed_1 = _mm_shuffle_epi8(__vec_low, __shuffle_1);
//				const auto __packed_2 = _mm_shuffle_epi8(__vec_low_hi, __shuffle_2);
//				const auto __packed_3 = _mm_shuffle_epi8(__vec_high, __shuffle_3);
//				const auto __packed_4 = _mm_shuffle_epi8(__vec_high_hi, __shuffle_4);
//
//				_mm_storel_epi64(reinterpret_cast<__m128i*>(__dst_ptr), __as<__m128i>(__packed_1));
//				__dst_ptr += __bytes_1;
//
//				_mm_storel_epi64(reinterpret_cast<__m128i*>(__dst_ptr), __as<__m128i>(__packed_2));
//				__dst_ptr += __bytes_2;
//
//				_mm_storel_epi64(reinterpret_cast<__m128i*>(__dst_ptr), __as<__m128i>(__packed_3));
//				__dst_ptr += __bytes_3;
//
//				_mm_storel_epi64(reinterpret_cast<__m128i*>(__dst_ptr), __as<__m128i>(__packed_4));
//
//				const auto __packed_vec = _mm256_load_si256(reinterpret_cast<const __m256i*>(__temp_buf));
//				const auto __result_vec = _Select<_ISA_, _Type_>()(__as<__m256i>(__x), __packed_vec, __tail_mask);
//
//				return { __total_bytes / sizeof(_Type_), __as<_Tp_>(__result_vec) };
//			}
//		}
//		else if constexpr (sizeof(_Tp_) == 64) {
//			const auto __not_mask = _Mask_not<_ISA_, _Type_>()(__not_mask);
//			const auto __set_bits = math::__native_popcnt_n_bits<__size>(__not_mask);
//
//			if constexpr (sizeof(_Type_) == 8) return { __set_bits, __as<_Tp_>(_mm512_mask_compress_epi64(__as<__m512i>(__x), __not_mask, __as<__m512i>(__x))) };
//			else if constexpr (sizeof(_Type_) == 4) return { __set_bits, __as<_Tp_>(_mm512_mask_compress_epi32(__as<__m512i>(__x), __not_mask, __as<__m512i>(__x))) };
//			else if constexpr (sizeof(_Type_) == 2) {
//				if constexpr (__avx512vbmi2) return { __set_bits, __as<_Tp_>(_mm512_mask_compress_epi16(__as<__m512i>(__x), __not_mask, __as<__m512i>(__x))) };
//				else {
//					alignas(sizeof(_Tp_)) _Type_ __temp_buf[__size];
//					_Type_* __dst_ptr = __temp_buf;
//
//					const auto __vec1 = __as<__m128i>(__x);
//					const auto __vec2 = _mm256_extracti128_si256(__as<__m256i>(__x), 1);
//					const auto __vec3 = __as<__m128i>(_mm512_extractf32x4_ps(__as<__m512>(__x), 2));
//					const auto __vec4 = __as<__m128i>(_mm512_extractf32x4_ps(__as<__m512>(__x), 3));
//
//					const auto __mask1 = __int_mask & 0xFF;
//					const auto __mask2 = (__int_mask >> 8) & 0xFF;
//					const auto __mask3 = (__int_mask >> 16) & 0xFF;
//					const auto __mask4 = (__int_mask >> 24) & 0xFF;
//
//					const auto __bytes1 = __tables_sse<sizeof(_Type_)>.__size[__mask1];
//					const auto __bytes2 = __tables_sse<sizeof(_Type_)>.__size[__mask2];
//					const auto __bytes3 = __tables_sse<sizeof(_Type_)>.__size[__mask3];
//					const auto __bytes4 = __tables_sse<sizeof(_Type_)>.__size[__mask4];
//
//					const auto __total_bytes = __bytes1 + __bytes2 + __bytes3 + __bytes4;
//					const auto __total_elems = __total_bytes / sizeof(_Type_);
//
//					const auto __tail_mask = (__mmask_for_elements_t<0x40 / sizeof(_Type_)>(
//						1u << (__size - __total_elems)) - 1) << __total_elems;
//
//					const auto __shuffle1 = _mm_load_si128(reinterpret_cast<const __m128i*>(__tables_sse<sizeof(_Type_)>.__shuffle[__mask1]));
//					const auto __shuffle2 = _mm_load_si128(reinterpret_cast<const __m128i*>(__tables_sse<sizeof(_Type_)>.__shuffle[__mask2]));
//					const auto __shuffle3 = _mm_load_si128(reinterpret_cast<const __m128i*>(__tables_sse<sizeof(_Type_)>.__shuffle[__mask3]));
//					const auto __shuffle4 = _mm_load_si128(reinterpret_cast<const __m128i*>(__tables_sse<sizeof(_Type_)>.__shuffle[__mask4]));
//
//					const auto __packed1 = _mm_shuffle_epi8(__vec1, __shuffle1);
//					const auto __packed2 = _mm_shuffle_epi8(__vec2, __shuffle2);
//					const auto __packed3 = _mm_shuffle_epi8(__vec3, __shuffle3);
//					const auto __packed4 = _mm_shuffle_epi8(__vec4, __shuffle4);
//
//					_mm_store_si128(reinterpret_cast<__m128i*>(__dst_ptr), __as<__m128i>(__packed1));
//					algorithm::__advance_bytes(__dst_ptr, __bytes1);
//
//					_mm_store_si128(reinterpret_cast<__m128i*>(__dst_ptr), __as<__m128i>(__packed2));
//					algorithm::__advance_bytes(__dst_ptr, __bytes2);
//
//					_mm_store_si128(reinterpret_cast<__m128i*>(__dst_ptr), __as<__m128i>(__packed3));
//					algorithm::__advance_bytes(__dst_ptr, __bytes3);
//
//					_mm_store_si128(reinterpret_cast<__m128i*>(__dst_ptr), __as<__m128i>(__packed4));
//
//					const auto __packed_vec = _mm512_load_si512(__temp_buf);
//					const auto __result_vec = _Select<_ISA_, _Type_>()(
//						__as<__m512i>(__x), __packed_vec, __tail_mask);
//
//					return { __total_bytes / sizeof(_Type_), __as<_Tp_>(__result_vec)};
//				}
//			}
//			else if constexpr (sizeof(_Type_) == 1) {
//				if constexpr (__avx512vbmi2) return { __set_bits, __as<_Tp_>(_mm512_mask_compress_epi8(__as<__m512i>(__x), __not_mask, __as<__m512i>(__x))) };
//				else {
//					alignas(sizeof(_Tp_)) _Type_ __stack_buffer[__size];
//					_Type_* __write_ptr = __stack_buffer;
//
//					const auto __ymm_lower = __as<__m256i>(__x);
//					const auto __ymm_higher = _mm512_extractf64x4_pd(__as<__m512d>(__x), 1);
//
//					const auto __xmm1 = __as<__m128i>(__ymm_lower);
//					const auto __xmm2 = __as<__m128i>(_mm256_extractf128_pd(__as<__m256d>(__ymm_lower), 1));
//
//					const auto __xmm3 = __as<__m128i>(__ymm_higher);
//					const auto __xmm4 = __as<__m128i>(_mm256_extractf128_pd(__as<__m256d>(__ymm_higher), 1));
//
//					const auto __xmm1_upper = __as<__m128i>(_mm_movehl_ps(
//						__as<__m128>(_mm_slli_si128(__xmm1, 8)), __as<__m128>(__xmm1)));
//
//					const auto __xmm2_upper = __as<__m128i>(_mm_movehl_ps(
//						__as<__m128>(_mm_slli_si128(__xmm2, 8)), __as<__m128>(__xmm2)));
//
//					const auto __xmm3_upper = __as<__m128i>(_mm_movehl_ps(
//						__as<__m128>(_mm_slli_si128(__xmm3, 8)), __as<__m128>(__xmm3)));
//
//					const auto __xmm4_upper = __as<__m128i>(_mm_movehl_ps(
//						__as<__m128>(_mm_slli_si128(__xmm4, 8)), __as<__m128>(__xmm4)));
//
//					const auto __mask1 = __int_mask & 0xFF;
//					const auto __mask2 = (__int_mask >> 8) & 0xFF;
//					const auto __mask3 = (__int_mask >> 16) & 0xFF;
//					const auto __mask4 = (__int_mask >> 24) & 0xFF;
//					const auto __mask5 = (__int_mask >> 32) & 0xFF;
//					const auto __mask6 = (__int_mask >> 40) & 0xFF;
//					const auto __mask7 = (__int_mask >> 48) & 0xFF;
//					const auto __mask8 = (__int_mask >> 56) & 0xFF;
//
//					const auto __bytes1 = __tables_sse<sizeof(_Type_)>.__size[__mask1];
//					const auto __bytes2 = __tables_sse<sizeof(_Type_)>.__size[__mask2];
//					const auto __bytes3 = __tables_sse<sizeof(_Type_)>.__size[__mask3];
//					const auto __bytes4 = __tables_sse<sizeof(_Type_)>.__size[__mask4];
//					const auto __bytes5 = __tables_sse<sizeof(_Type_)>.__size[__mask5];
//					const auto __bytes6 = __tables_sse<sizeof(_Type_)>.__size[__mask6];
//					const auto __bytes7 = __tables_sse<sizeof(_Type_)>.__size[__mask7];
//					const auto __bytes8 = __tables_sse<sizeof(_Type_)>.__size[__mask8];
//
//					const auto __bytes_lane1 = __bytes1 + __bytes2;
//					const auto __bytes_lane2 = __bytes3 + __bytes4;
//					const auto __bytes_lane3 = __bytes5 + __bytes6;
//					const auto __bytes_lane4 = __bytes7 + __bytes8;
//
//					const auto __bytes_total = __bytes_lane1 + __bytes_lane2 + __bytes_lane3 + __bytes_lane4;
//					const auto __tail_mask = (__mmask_for_elements_t<__size>(u64(1) << (__size - __bytes_total)) - 1) << __bytes_total;
//
//					const auto __shuffle1 = _mm_loadl_epi64(reinterpret_cast<const __m128i*>(__tables_sse<sizeof(_Type_)>.__shuffle[__mask1]));
//					const auto __shuffle2 = _mm_loadl_epi64(reinterpret_cast<const __m128i*>(__tables_sse<sizeof(_Type_)>.__shuffle[__mask2]));
//					const auto __shuffle3 = _mm_loadl_epi64(reinterpret_cast<const __m128i*>(__tables_sse<sizeof(_Type_)>.__shuffle[__mask3]));
//					const auto __shuffle4 = _mm_loadl_epi64(reinterpret_cast<const __m128i*>(__tables_sse<sizeof(_Type_)>.__shuffle[__mask4]));
//					const auto __shuffle5 = _mm_loadl_epi64(reinterpret_cast<const __m128i*>(__tables_sse<sizeof(_Type_)>.__shuffle[__mask5]));
//					const auto __shuffle6 = _mm_loadl_epi64(reinterpret_cast<const __m128i*>(__tables_sse<sizeof(_Type_)>.__shuffle[__mask6]));
//					const auto __shuffle7 = _mm_loadl_epi64(reinterpret_cast<const __m128i*>(__tables_sse<sizeof(_Type_)>.__shuffle[__mask7]));
//					const auto __shuffle8 = _mm_loadl_epi64(reinterpret_cast<const __m128i*>(__tables_sse<sizeof(_Type_)>.__shuffle[__mask8]));
//
//					const auto __packed1 = _mm_shuffle_epi8(__xmm1, __shuffle1);
//					const auto __packed2 = _mm_shuffle_epi8(__xmm1_upper, __shuffle2);
//					const auto __packed3 = _mm_shuffle_epi8(__xmm2, __shuffle3);
//					const auto __packed4 = _mm_shuffle_epi8(__xmm2_upper, __shuffle4);
//					const auto __packed5 = _mm_shuffle_epi8(__xmm3, __shuffle5);
//					const auto __packed6 = _mm_shuffle_epi8(__xmm3_upper, __shuffle6);
//					const auto __packed7 = _mm_shuffle_epi8(__xmm4, __shuffle7);
//					const auto __packed8 = _mm_shuffle_epi8(__xmm4_upper, __shuffle8);
//
//					_mm_storel_epi64(reinterpret_cast<__m128i*>(__write_ptr), __as<__m128i>(__packed1));
//					algorithm::__advance_bytes(__write_ptr, __bytes1);
//
//					_mm_storel_epi64(reinterpret_cast<__m128i*>(__write_ptr), __as<__m128i>(__packed2));
//					algorithm::__advance_bytes(__write_ptr, __bytes2);
//
//					_mm_storel_epi64(reinterpret_cast<__m128i*>(__write_ptr), __as<__m128i>(__packed3));
//					algorithm::__advance_bytes(__write_ptr, __bytes3);
//
//					_mm_storel_epi64(reinterpret_cast<__m128i*>(__write_ptr), __as<__m128i>(__packed4));
//					algorithm::__advance_bytes(__write_ptr, __bytes4);
//
//					_mm_storel_epi64(reinterpret_cast<__m128i*>(__write_ptr), __as<__m128i>(__packed5));
//					algorithm::__advance_bytes(__write_ptr, __bytes5);
//
//					_mm_storel_epi64(reinterpret_cast<__m128i*>(__write_ptr), __as<__m128i>(__packed6));
//					algorithm::__advance_bytes(__write_ptr, __bytes6);
//
//					_mm_storel_epi64(reinterpret_cast<__m128i*>(__write_ptr), __as<__m128i>(__packed7));
//					algorithm::__advance_bytes(__write_ptr, __bytes7);
//
//					_mm_storel_epi64(reinterpret_cast<__m128i*>(__write_ptr), __as<__m128i>(__packed8));
//
//					const auto __packed_vec = _mm512_load_si512(__stack_buffer);
//					const auto __result_vec = _Select<_ISA_, _Type_>()(__as<__m512i>(__x), __packed_vec, __tail_mask);
//
//					return { __bytes_total / sizeof(_Type_), __as<_Tp_>(__result_vec)};
//				}
//			}
//		}
//		
//		if constexpr (arithmetic_type<_Tp_>) return __compress_mask ? std::pair{ 1, __x } : std::pair{ 0, 0 };
//		else return __fallback(__x, __compress_mask);
//	}
//};
//
//__RAZE_VX_NAMESPACE_END
