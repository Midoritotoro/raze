#include <src/raze/algorithm/memory/Memcpy.h>
#include <raze/arch/ProcessorFeatures.h>
#include <src/raze/algorithm/AdvanceBytes.h>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

raze_always_inline void* __memcpy_scalar_impl(volatile u8* raze_restrict __dst_ch,
    const volatile u8* raze_restrict __src_ch, sizetype __bytes) noexcept
{
	#pragma loop(no_vector)
	while (__bytes >= 8) {
		*reinterpret_cast<volatile raze::i64*>(__dst_ch) = *reinterpret_cast<const volatile raze::i64*>(__src_ch);
		__dst_ch += 8; __src_ch += 8; __bytes -= 8;
	}
	if (__bytes >= 4) {
		*reinterpret_cast<volatile raze::i32*>(__dst_ch) = *reinterpret_cast<const volatile raze::i32*>(__src_ch);
		__dst_ch += 4; __src_ch += 4; __bytes -= 4;
	}
	if (__bytes >= 2) {
		*reinterpret_cast<volatile raze::i16*>(__dst_ch) = *reinterpret_cast<const volatile raze::i16*>(__src_ch);
		__dst_ch += 2; __src_ch += 2; __bytes -= 2;
	}
	if (__bytes) {
		*__dst_ch++ = *__src_ch++;
	}

	return const_cast<u8*>(__dst_ch);
}

raze_unmangled raze_no_stack_protector raze_declare_const_function void* raze_stdcall __raze_memcpy_scalar(void* raze_restrict __dst,
	const void* raze_restrict __src, sizetype __bytes) noexcept
{
    return __memcpy_scalar_impl(static_cast<u8*>(__dst), static_cast<const u8*>(__src), __bytes);
}

raze_always_inline void __memcpy_16(void* raze_restrict __dst, const void* raze_restrict __src) noexcept {
	_mm_storeu_si128(reinterpret_cast<__m128i*>(__dst), _mm_loadu_si128(reinterpret_cast<const __m128i*>(__src)));
}

raze_always_inline void __memcpy_sse2_32(void* raze_restrict __dst, const void* raze_restrict __src) noexcept {
	_mm_storeu_si128(reinterpret_cast<__m128i*>(__dst), _mm_loadu_si128(reinterpret_cast<const __m128i*>(__src)));
	_mm_storeu_si128(reinterpret_cast<__m128i*>(__dst) + 1, _mm_loadu_si128(reinterpret_cast<const __m128i*>(__src) + 1));
}

raze_always_inline void __memcpy_avx_32(void* raze_restrict __dst, const void* raze_restrict __src) noexcept {
	_mm256_storeu_si256(reinterpret_cast<__m256i*>(__dst), _mm256_loadu_si256(reinterpret_cast<const __m256i*>(__src)));
}

raze_always_inline void __memcpy_avx512_64(void* raze_restrict __dst, const void* raze_restrict __src) noexcept {
	_mm512_storeu_si512(__dst, _mm512_loadu_si512(__src));
}

raze_always_inline void __memcpy_small_avx512f(void* raze_restrict __dst, const void* raze_restrict __src, sizetype __size) {
	u8* __ch_dst = reinterpret_cast<u8*>(__dst) + __size;
	const u8* __ch_src = reinterpret_cast<const u8*>(__src) + __size;

	switch (__size) {
		case 0: break;
		case 1: __ch_dst[-1] = __ch_src[-1]; break;
		case 2: *((uint16_t*)(__ch_dst - 2)) = *((uint16_t*)(__ch_src - 2)); break;
		case 3: *((uint16_t*)(__ch_dst - 3)) = *((uint16_t*)(__ch_src - 3)); __ch_dst[-1] = __ch_src[-1]; break;
		case 4: *((uint32_t*)(__ch_dst - 4)) = *((uint32_t*)(__ch_src - 4)); break;
		case 5: *((uint32_t*)(__ch_dst - 5)) = *((uint32_t*)(__ch_src - 5)); __ch_dst[-1] = __ch_src[-1]; break;
		case 6: *((uint32_t*)(__ch_dst - 6)) = *((uint32_t*)(__ch_src - 6)); *((uint16_t*)(__ch_dst - 2)) = *((uint16_t*)(__ch_src - 2)); break;
		case 7: *((uint32_t*)(__ch_dst - 7)) = *((uint32_t*)(__ch_src - 7)); *((uint32_t*)(__ch_dst - 4)) = *((uint32_t*)(__ch_src - 4)); break;
		case 8: *((uint64_t*)(__ch_dst - 8)) = *((uint64_t*)(__ch_src - 8)); break;
		case 9: *((uint64_t*)(__ch_dst - 9)) = *((uint64_t*)(__ch_src - 9)); __ch_dst[-1] = __ch_src[-1]; break;
		case 10: *((uint64_t*)(__ch_dst - 10)) = *((uint64_t*)(__ch_src - 10)); *((uint16_t*)(__ch_dst - 2)) = *((uint16_t*)(__ch_src - 2)); break;
		case 11: *((uint64_t*)(__ch_dst - 11)) = *((uint64_t*)(__ch_src - 11)); *((uint32_t*)(__ch_dst - 4)) = *((uint32_t*)(__ch_src - 4)); break;
		case 12: *((uint64_t*)(__ch_dst - 12)) = *((uint64_t*)(__ch_src - 12)); *((uint32_t*)(__ch_dst - 4)) = *((uint32_t*)(__ch_src - 4)); break;
		case 13: *((uint64_t*)(__ch_dst - 13)) = *((uint64_t*)(__ch_src - 13)); *((uint64_t*)(__ch_dst - 8)) = *((uint64_t*)(__ch_src - 8)); break;
		case 14: *((uint64_t*)(__ch_dst - 14)) = *((uint64_t*)(__ch_src - 14)); *((uint64_t*)(__ch_dst - 8)) = *((uint64_t*)(__ch_src - 8)); break;
		case 15: *((uint64_t*)(__ch_dst - 15)) = *((uint64_t*)(__ch_src - 15)); *((uint64_t*)(__ch_dst - 8)) = *((uint64_t*)(__ch_src - 8)); break;
		case 16: __memcpy_16(__ch_dst - 16, __ch_src - 16); break;
		case 17: __memcpy_16(__ch_dst - 17, __ch_src - 17); __memcpy_16(__ch_dst - 16, __ch_src - 16); break;
		case 18: __memcpy_16(__ch_dst - 18, __ch_src - 18); __memcpy_16(__ch_dst - 16, __ch_src - 16); break;
		case 19: __memcpy_16(__ch_dst - 19, __ch_src - 19); __memcpy_16(__ch_dst - 16, __ch_src - 16); break;
		case 20: __memcpy_16(__ch_dst - 20, __ch_src - 20); __memcpy_16(__ch_dst - 16, __ch_src - 16); break;
		case 21: __memcpy_16(__ch_dst - 21, __ch_src - 21); __memcpy_16(__ch_dst - 16, __ch_src - 16); break;
		case 22: __memcpy_16(__ch_dst - 22, __ch_src - 22); __memcpy_16(__ch_dst - 16, __ch_src - 16); break;
		case 23: __memcpy_16(__ch_dst - 23, __ch_src - 23); __memcpy_16(__ch_dst - 16, __ch_src - 16); break;
		case 24: __memcpy_16(__ch_dst - 24, __ch_src - 24); __memcpy_16(__ch_dst - 16, __ch_src - 16); break;
		case 25: __memcpy_16(__ch_dst - 25, __ch_src - 25); __memcpy_16(__ch_dst - 16, __ch_src - 16); break;
		case 26: __memcpy_16(__ch_dst - 26, __ch_src - 26); __memcpy_16(__ch_dst - 16, __ch_src - 16); break;
		case 27: __memcpy_16(__ch_dst - 27, __ch_src - 27); __memcpy_16(__ch_dst - 16, __ch_src - 16); break;
		case 28: __memcpy_16(__ch_dst - 28, __ch_src - 28); __memcpy_16(__ch_dst - 16, __ch_src - 16); break;
		case 29: __memcpy_16(__ch_dst - 29, __ch_src - 29); __memcpy_16(__ch_dst - 16, __ch_src - 16); break;
		case 30: __memcpy_16(__ch_dst - 30, __ch_src - 30); __memcpy_16(__ch_dst - 16, __ch_src - 16); break;
		case 31: __memcpy_16(__ch_dst - 31, __ch_src - 31); __memcpy_16(__ch_dst - 16, __ch_src - 16); break;
		case 32: __memcpy_avx_32(__ch_dst - 32, __ch_src - 32); break;
		case 33: __memcpy_avx_32(__ch_dst - 33, __ch_src - 33); __memcpy_avx_32(__ch_dst - 32, __ch_src - 32); break;
		case 34: __memcpy_avx_32(__ch_dst - 34, __ch_src - 34); __memcpy_avx_32(__ch_dst - 32, __ch_src - 32); break;
		case 35: __memcpy_avx_32(__ch_dst - 35, __ch_src - 35); __memcpy_avx_32(__ch_dst - 32, __ch_src - 32); break;
		case 36: __memcpy_avx_32(__ch_dst - 36, __ch_src - 36); __memcpy_avx_32(__ch_dst - 32, __ch_src - 32); break;
		case 37: __memcpy_avx_32(__ch_dst - 37, __ch_src - 37); __memcpy_avx_32(__ch_dst - 32, __ch_src - 32); break;
		case 38: __memcpy_avx_32(__ch_dst - 38, __ch_src - 38); __memcpy_avx_32(__ch_dst - 32, __ch_src - 32); break;
		case 39: __memcpy_avx_32(__ch_dst - 39, __ch_src - 39); __memcpy_avx_32(__ch_dst - 32, __ch_src - 32); break;
		case 40: __memcpy_avx_32(__ch_dst - 40, __ch_src - 40); __memcpy_avx_32(__ch_dst - 32, __ch_src - 32); break;
		case 41: __memcpy_avx_32(__ch_dst - 41, __ch_src - 41); __memcpy_avx_32(__ch_dst - 32, __ch_src - 32); break;
		case 42: __memcpy_avx_32(__ch_dst - 42, __ch_src - 42); __memcpy_avx_32(__ch_dst - 32, __ch_src - 32); break;
		case 43: __memcpy_avx_32(__ch_dst - 43, __ch_src - 43); __memcpy_avx_32(__ch_dst - 32, __ch_src - 32); break;
		case 44: __memcpy_avx_32(__ch_dst - 44, __ch_src - 44); __memcpy_avx_32(__ch_dst - 32, __ch_src - 32); break;
		case 45: __memcpy_avx_32(__ch_dst - 45, __ch_src - 45); __memcpy_avx_32(__ch_dst - 32, __ch_src - 32); break;
		case 46: __memcpy_avx_32(__ch_dst - 46, __ch_src - 46); __memcpy_avx_32(__ch_dst - 32, __ch_src - 32); break;
		case 47: __memcpy_avx_32(__ch_dst - 47, __ch_src - 47); __memcpy_avx_32(__ch_dst - 32, __ch_src - 32); break;
		case 48: __memcpy_avx_32(__ch_dst - 48, __ch_src - 48); __memcpy_avx_32(__ch_dst - 32, __ch_src - 32); break;
		case 49: __memcpy_avx_32(__ch_dst - 49, __ch_src - 49); __memcpy_avx_32(__ch_dst - 32, __ch_src - 32); break;
		case 50: __memcpy_avx_32(__ch_dst - 50, __ch_src - 50); __memcpy_avx_32(__ch_dst - 32, __ch_src - 32); break;
		case 51: __memcpy_avx_32(__ch_dst - 51, __ch_src - 51); __memcpy_avx_32(__ch_dst - 32, __ch_src - 32); break;
		case 52: __memcpy_avx_32(__ch_dst - 52, __ch_src - 52); __memcpy_avx_32(__ch_dst - 32, __ch_src - 32); break;
		case 53: __memcpy_avx_32(__ch_dst - 53, __ch_src - 53); __memcpy_avx_32(__ch_dst - 32, __ch_src - 32); break;
		case 54: __memcpy_avx_32(__ch_dst - 54, __ch_src - 54); __memcpy_avx_32(__ch_dst - 32, __ch_src - 32); break;
		case 55: __memcpy_avx_32(__ch_dst - 55, __ch_src - 55); __memcpy_avx_32(__ch_dst - 32, __ch_src - 32); break;
		case 56: __memcpy_avx_32(__ch_dst - 56, __ch_src - 56); __memcpy_avx_32(__ch_dst - 32, __ch_src - 32); break;
		case 57: __memcpy_avx_32(__ch_dst - 57, __ch_src - 57); __memcpy_avx_32(__ch_dst - 32, __ch_src - 32); break;
		case 58: __memcpy_avx_32(__ch_dst - 58, __ch_src - 58); __memcpy_avx_32(__ch_dst - 32, __ch_src - 32); break;
		case 59: __memcpy_avx_32(__ch_dst - 59, __ch_src - 59); __memcpy_avx_32(__ch_dst - 32, __ch_src - 32); break;
		case 60: __memcpy_avx_32(__ch_dst - 60, __ch_src - 60); __memcpy_avx_32(__ch_dst - 32, __ch_src - 32); break;
		case 61: __memcpy_avx_32(__ch_dst - 61, __ch_src - 61); __memcpy_avx_32(__ch_dst - 32, __ch_src - 32); break;
		case 62: __memcpy_avx_32(__ch_dst - 62, __ch_src - 62); __memcpy_avx_32(__ch_dst - 32, __ch_src - 32); break;
		case 63: __memcpy_avx_32(__ch_dst - 63, __ch_src - 63); __memcpy_avx_32(__ch_dst - 32, __ch_src - 32); break;
		case 64: __memcpy_avx512_64(__ch_dst - 64, __ch_src - 64); break;
	}
}

raze_always_inline void __memcpy_small_avx(void* raze_restrict __dst, const void* raze_restrict __src, sizetype __size) {
    u8* __ch_dst = reinterpret_cast<u8*>(__dst) + __size;
    const u8* __ch_src = reinterpret_cast<const u8*>(__src) + __size;

    switch (__size) {
		case 0: break;
		case 1: __ch_dst[-1] = __ch_src[-1]; break;
		case 2: *((uint16_t*)(__ch_dst - 2)) = *((uint16_t*)(__ch_src - 2)); break;
		case 3: *((uint16_t*)(__ch_dst - 3)) = *((uint16_t*)(__ch_src - 3)); __ch_dst[-1] = __ch_src[-1]; break;
		case 4: *((uint32_t*)(__ch_dst - 4)) = *((uint32_t*)(__ch_src - 4)); break;
		case 5: *((uint32_t*)(__ch_dst - 5)) = *((uint32_t*)(__ch_src - 5)); __ch_dst[-1] = __ch_src[-1]; break;
		case 6: *((uint32_t*)(__ch_dst - 6)) = *((uint32_t*)(__ch_src - 6)); *((uint16_t*)(__ch_dst - 2)) = *((uint16_t*)(__ch_src - 2)); break;
		case 7: *((uint32_t*)(__ch_dst - 7)) = *((uint32_t*)(__ch_src - 7)); *((uint32_t*)(__ch_dst - 4)) = *((uint32_t*)(__ch_src - 4)); break;
		case 8: *((uint64_t*)(__ch_dst - 8)) = *((uint64_t*)(__ch_src - 8)); break;
		case 9: *((uint64_t*)(__ch_dst - 9)) = *((uint64_t*)(__ch_src - 9)); __ch_dst[-1] = __ch_src[-1]; break;
		case 10: *((uint64_t*)(__ch_dst - 10)) = *((uint64_t*)(__ch_src - 10)); *((uint16_t*)(__ch_dst - 2)) = *((uint16_t*)(__ch_src - 2)); break;
		case 11: *((uint64_t*)(__ch_dst - 11)) = *((uint64_t*)(__ch_src - 11)); *((uint32_t*)(__ch_dst - 4)) = *((uint32_t*)(__ch_src - 4)); break;
		case 12: *((uint64_t*)(__ch_dst - 12)) = *((uint64_t*)(__ch_src - 12)); *((uint32_t*)(__ch_dst - 4)) = *((uint32_t*)(__ch_src - 4)); break;
		case 13: *((uint64_t*)(__ch_dst - 13)) = *((uint64_t*)(__ch_src - 13)); *((uint64_t*)(__ch_dst - 8)) = *((uint64_t*)(__ch_src - 8)); break;
		case 14: *((uint64_t*)(__ch_dst - 14)) = *((uint64_t*)(__ch_src - 14)); *((uint64_t*)(__ch_dst - 8)) = *((uint64_t*)(__ch_src - 8)); break;
		case 15: *((uint64_t*)(__ch_dst - 15)) = *((uint64_t*)(__ch_src - 15)); *((uint64_t*)(__ch_dst - 8)) = *((uint64_t*)(__ch_src - 8)); break;
		case 16: __memcpy_16(__ch_dst - 16, __ch_src - 16); break;
		case 17: __memcpy_16(__ch_dst - 17, __ch_src - 17); __memcpy_16(__ch_dst - 16, __ch_src - 16); break;
		case 18: __memcpy_16(__ch_dst - 18, __ch_src - 18); __memcpy_16(__ch_dst - 16, __ch_src - 16); break;
		case 19: __memcpy_16(__ch_dst - 19, __ch_src - 19); __memcpy_16(__ch_dst - 16, __ch_src - 16); break;
		case 20: __memcpy_16(__ch_dst - 20, __ch_src - 20); __memcpy_16(__ch_dst - 16, __ch_src - 16); break;
		case 21: __memcpy_16(__ch_dst - 21, __ch_src - 21); __memcpy_16(__ch_dst - 16, __ch_src - 16); break;
		case 22: __memcpy_16(__ch_dst - 22, __ch_src - 22); __memcpy_16(__ch_dst - 16, __ch_src - 16); break;
		case 23: __memcpy_16(__ch_dst - 23, __ch_src - 23); __memcpy_16(__ch_dst - 16, __ch_src - 16); break;
		case 24: __memcpy_16(__ch_dst - 24, __ch_src - 24); __memcpy_16(__ch_dst - 16, __ch_src - 16); break;
		case 25: __memcpy_16(__ch_dst - 25, __ch_src - 25); __memcpy_16(__ch_dst - 16, __ch_src - 16); break;
		case 26: __memcpy_16(__ch_dst - 26, __ch_src - 26); __memcpy_16(__ch_dst - 16, __ch_src - 16); break;
		case 27: __memcpy_16(__ch_dst - 27, __ch_src - 27); __memcpy_16(__ch_dst - 16, __ch_src - 16); break;
		case 28: __memcpy_16(__ch_dst - 28, __ch_src - 28); __memcpy_16(__ch_dst - 16, __ch_src - 16); break;
		case 29: __memcpy_16(__ch_dst - 29, __ch_src - 29); __memcpy_16(__ch_dst - 16, __ch_src - 16); break;
		case 30: __memcpy_16(__ch_dst - 30, __ch_src - 30); __memcpy_16(__ch_dst - 16, __ch_src - 16); break;
		case 31: __memcpy_16(__ch_dst - 31, __ch_src - 31); __memcpy_16(__ch_dst - 16, __ch_src - 16); break;
		case 32: __memcpy_avx_32(__ch_dst - 32, __ch_src - 32); break;
		case 33: __memcpy_avx_32(__ch_dst - 33, __ch_src - 33); __memcpy_avx_32(__ch_dst - 32, __ch_src - 32); break;
		case 34: __memcpy_avx_32(__ch_dst - 34, __ch_src - 34); __memcpy_avx_32(__ch_dst - 32, __ch_src - 32); break;
		case 35: __memcpy_avx_32(__ch_dst - 35, __ch_src - 35); __memcpy_avx_32(__ch_dst - 32, __ch_src - 32); break;
		case 36: __memcpy_avx_32(__ch_dst - 36, __ch_src - 36); __memcpy_avx_32(__ch_dst - 32, __ch_src - 32); break;
		case 37: __memcpy_avx_32(__ch_dst - 37, __ch_src - 37); __memcpy_avx_32(__ch_dst - 32, __ch_src - 32); break;
		case 38: __memcpy_avx_32(__ch_dst - 38, __ch_src - 38); __memcpy_avx_32(__ch_dst - 32, __ch_src - 32); break;
		case 39: __memcpy_avx_32(__ch_dst - 39, __ch_src - 39); __memcpy_avx_32(__ch_dst - 32, __ch_src - 32); break;
		case 40: __memcpy_avx_32(__ch_dst - 40, __ch_src - 40); __memcpy_avx_32(__ch_dst - 32, __ch_src - 32); break;
		case 41: __memcpy_avx_32(__ch_dst - 41, __ch_src - 41); __memcpy_avx_32(__ch_dst - 32, __ch_src - 32); break;
		case 42: __memcpy_avx_32(__ch_dst - 42, __ch_src - 42); __memcpy_avx_32(__ch_dst - 32, __ch_src - 32); break;
		case 43: __memcpy_avx_32(__ch_dst - 43, __ch_src - 43); __memcpy_avx_32(__ch_dst - 32, __ch_src - 32); break;
		case 44: __memcpy_avx_32(__ch_dst - 44, __ch_src - 44); __memcpy_avx_32(__ch_dst - 32, __ch_src - 32); break;
		case 45: __memcpy_avx_32(__ch_dst - 45, __ch_src - 45); __memcpy_avx_32(__ch_dst - 32, __ch_src - 32); break;
		case 46: __memcpy_avx_32(__ch_dst - 46, __ch_src - 46); __memcpy_avx_32(__ch_dst - 32, __ch_src - 32); break;
		case 47: __memcpy_avx_32(__ch_dst - 47, __ch_src - 47); __memcpy_avx_32(__ch_dst - 32, __ch_src - 32); break;
		case 48: __memcpy_avx_32(__ch_dst - 48, __ch_src - 48); __memcpy_avx_32(__ch_dst - 32, __ch_src - 32); break;
		case 49: __memcpy_avx_32(__ch_dst - 49, __ch_src - 49); __memcpy_avx_32(__ch_dst - 32, __ch_src - 32); break;
		case 50: __memcpy_avx_32(__ch_dst - 50, __ch_src - 50); __memcpy_avx_32(__ch_dst - 32, __ch_src - 32); break;
		case 51: __memcpy_avx_32(__ch_dst - 51, __ch_src - 51); __memcpy_avx_32(__ch_dst - 32, __ch_src - 32); break;
		case 52: __memcpy_avx_32(__ch_dst - 52, __ch_src - 52); __memcpy_avx_32(__ch_dst - 32, __ch_src - 32); break;
		case 53: __memcpy_avx_32(__ch_dst - 53, __ch_src - 53); __memcpy_avx_32(__ch_dst - 32, __ch_src - 32); break;
		case 54: __memcpy_avx_32(__ch_dst - 54, __ch_src - 54); __memcpy_avx_32(__ch_dst - 32, __ch_src - 32); break;
		case 55: __memcpy_avx_32(__ch_dst - 55, __ch_src - 55); __memcpy_avx_32(__ch_dst - 32, __ch_src - 32); break;
		case 56: __memcpy_avx_32(__ch_dst - 56, __ch_src - 56); __memcpy_avx_32(__ch_dst - 32, __ch_src - 32); break;
		case 57: __memcpy_avx_32(__ch_dst - 57, __ch_src - 57); __memcpy_avx_32(__ch_dst - 32, __ch_src - 32); break;
		case 58: __memcpy_avx_32(__ch_dst - 58, __ch_src - 58); __memcpy_avx_32(__ch_dst - 32, __ch_src - 32); break;
		case 59: __memcpy_avx_32(__ch_dst - 59, __ch_src - 59); __memcpy_avx_32(__ch_dst - 32, __ch_src - 32); break;
		case 60: __memcpy_avx_32(__ch_dst - 60, __ch_src - 60); __memcpy_avx_32(__ch_dst - 32, __ch_src - 32); break;
		case 61: __memcpy_avx_32(__ch_dst - 61, __ch_src - 61); __memcpy_avx_32(__ch_dst - 32, __ch_src - 32); break;
		case 62: __memcpy_avx_32(__ch_dst - 62, __ch_src - 62); __memcpy_avx_32(__ch_dst - 32, __ch_src - 32); break;
		case 63: __memcpy_avx_32(__ch_dst - 63, __ch_src - 63); __memcpy_avx_32(__ch_dst - 32, __ch_src - 32); break;
		case 64: __memcpy_avx_32(__ch_dst - 64, __ch_src - 64); __memcpy_avx_32(__ch_dst - 32, __ch_src - 32);  break;
    }
}

raze_always_inline void __memcpy_small_sse2(void* raze_restrict __dst, const void* raze_restrict __src, sizetype __size) {
	u8* __ch_dst = reinterpret_cast<u8*>(__dst) + __size;
	const u8* __ch_src = reinterpret_cast<const u8*>(__src) + __size;

	switch (__size) {
		case 0: break;
		case 1: __ch_dst[-1] = __ch_src[-1]; break;
		case 2: *((uint16_t*)(__ch_dst - 2)) = *((uint16_t*)(__ch_src - 2)); break;
		case 3: *((uint16_t*)(__ch_dst - 3)) = *((uint16_t*)(__ch_src - 3)); __ch_dst[-1] = __ch_src[-1]; break;
		case 4: *((uint32_t*)(__ch_dst - 4)) = *((uint32_t*)(__ch_src - 4)); break;
		case 5: *((uint32_t*)(__ch_dst - 5)) = *((uint32_t*)(__ch_src - 5)); __ch_dst[-1] = __ch_src[-1]; break;
		case 6: *((uint32_t*)(__ch_dst - 6)) = *((uint32_t*)(__ch_src - 6)); *((uint16_t*)(__ch_dst - 2)) = *((uint16_t*)(__ch_src - 2)); break;
		case 7: *((uint32_t*)(__ch_dst - 7)) = *((uint32_t*)(__ch_src - 7)); *((uint32_t*)(__ch_dst - 4)) = *((uint32_t*)(__ch_src - 4)); break;
		case 8: *((uint64_t*)(__ch_dst - 8)) = *((uint64_t*)(__ch_src - 8)); break;
		case 9: *((uint64_t*)(__ch_dst - 9)) = *((uint64_t*)(__ch_src - 9)); __ch_dst[-1] = __ch_src[-1]; break;
		case 10: *((uint64_t*)(__ch_dst - 10)) = *((uint64_t*)(__ch_src - 10)); *((uint16_t*)(__ch_dst - 2)) = *((uint16_t*)(__ch_src - 2)); break;
		case 11: *((uint64_t*)(__ch_dst - 11)) = *((uint64_t*)(__ch_src - 11)); *((uint32_t*)(__ch_dst - 4)) = *((uint32_t*)(__ch_src - 4)); break;
		case 12: *((uint64_t*)(__ch_dst - 12)) = *((uint64_t*)(__ch_src - 12)); *((uint32_t*)(__ch_dst - 4)) = *((uint32_t*)(__ch_src - 4)); break;
		case 13: *((uint64_t*)(__ch_dst - 13)) = *((uint64_t*)(__ch_src - 13)); *((uint64_t*)(__ch_dst - 8)) = *((uint64_t*)(__ch_src - 8)); break;
		case 14: *((uint64_t*)(__ch_dst - 14)) = *((uint64_t*)(__ch_src - 14)); *((uint64_t*)(__ch_dst - 8)) = *((uint64_t*)(__ch_src - 8)); break;
		case 15: *((uint64_t*)(__ch_dst - 15)) = *((uint64_t*)(__ch_src - 15)); *((uint64_t*)(__ch_dst - 8)) = *((uint64_t*)(__ch_src - 8)); break;
		case 16: __memcpy_16(__ch_dst - 16, __ch_src - 16); break;
		case 17: __memcpy_16(__ch_dst - 17, __ch_src - 17); __memcpy_16(__ch_dst - 16, __ch_src - 16); break;
		case 18: __memcpy_16(__ch_dst - 18, __ch_src - 18); __memcpy_16(__ch_dst - 16, __ch_src - 16); break;
		case 19: __memcpy_16(__ch_dst - 19, __ch_src - 19); __memcpy_16(__ch_dst - 16, __ch_src - 16); break;
		case 20: __memcpy_16(__ch_dst - 20, __ch_src - 20); __memcpy_16(__ch_dst - 16, __ch_src - 16); break;
		case 21: __memcpy_16(__ch_dst - 21, __ch_src - 21); __memcpy_16(__ch_dst - 16, __ch_src - 16); break;
		case 22: __memcpy_16(__ch_dst - 22, __ch_src - 22); __memcpy_16(__ch_dst - 16, __ch_src - 16); break;
		case 23: __memcpy_16(__ch_dst - 23, __ch_src - 23); __memcpy_16(__ch_dst - 16, __ch_src - 16); break;
		case 24: __memcpy_16(__ch_dst - 24, __ch_src - 24); __memcpy_16(__ch_dst - 16, __ch_src - 16); break;
		case 25: __memcpy_16(__ch_dst - 25, __ch_src - 25); __memcpy_16(__ch_dst - 16, __ch_src - 16); break;
		case 26: __memcpy_16(__ch_dst - 26, __ch_src - 26); __memcpy_16(__ch_dst - 16, __ch_src - 16); break;
		case 27: __memcpy_16(__ch_dst - 27, __ch_src - 27); __memcpy_16(__ch_dst - 16, __ch_src - 16); break;
		case 28: __memcpy_16(__ch_dst - 28, __ch_src - 28); __memcpy_16(__ch_dst - 16, __ch_src - 16); break;
		case 29: __memcpy_16(__ch_dst - 29, __ch_src - 29); __memcpy_16(__ch_dst - 16, __ch_src - 16); break;
		case 30: __memcpy_16(__ch_dst - 30, __ch_src - 30); __memcpy_16(__ch_dst - 16, __ch_src - 16); break;
		case 31: __memcpy_16(__ch_dst - 31, __ch_src - 31); __memcpy_16(__ch_dst - 16, __ch_src - 16); break;
		case 32: __memcpy_16(__ch_dst - 32, __ch_src - 32); __memcpy_16(__ch_dst - 16, __ch_src - 16); break;
	}
}

raze_always_inline void __memcpy_small(void* raze_restrict __dst, const void* raze_restrict __src, sizetype __size) {
	u8* __ch_dst = reinterpret_cast<u8*>(__dst) + __size;
	const u8* __ch_src = reinterpret_cast<const u8*>(__src) + __size;

	switch (__size) {
		case 0: break;
		case 1: __ch_dst[-1] = __ch_src[-1]; break;
		case 2: *((uint16_t*)(__ch_dst - 2)) = *((uint16_t*)(__ch_src - 2)); break;
		case 3: *((uint16_t*)(__ch_dst - 3)) = *((uint16_t*)(__ch_src - 3)); __ch_dst[-1] = __ch_src[-1]; break;
		case 4: *((uint32_t*)(__ch_dst - 4)) = *((uint32_t*)(__ch_src - 4)); break;
		case 5: *((uint32_t*)(__ch_dst - 5)) = *((uint32_t*)(__ch_src - 5)); __ch_dst[-1] = __ch_src[-1]; break;
		case 6: *((uint32_t*)(__ch_dst - 6)) = *((uint32_t*)(__ch_src - 6)); *((uint16_t*)(__ch_dst - 2)) = *((uint16_t*)(__ch_src - 2)); break;
		case 7: *((uint32_t*)(__ch_dst - 7)) = *((uint32_t*)(__ch_src - 7)); *((uint32_t*)(__ch_dst - 4)) = *((uint32_t*)(__ch_src - 4)); break;
		case 8: *((uint64_t*)(__ch_dst - 8)) = *((uint64_t*)(__ch_src - 8)); break;
		case 9: *((uint64_t*)(__ch_dst - 9)) = *((uint64_t*)(__ch_src - 9)); __ch_dst[-1] = __ch_src[-1]; break;
		case 10: *((uint64_t*)(__ch_dst - 10)) = *((uint64_t*)(__ch_src - 10)); *((uint16_t*)(__ch_dst - 2)) = *((uint16_t*)(__ch_src - 2)); break;
		case 11: *((uint64_t*)(__ch_dst - 11)) = *((uint64_t*)(__ch_src - 11)); *((uint32_t*)(__ch_dst - 4)) = *((uint32_t*)(__ch_src - 4)); break;
		case 12: *((uint64_t*)(__ch_dst - 12)) = *((uint64_t*)(__ch_src - 12)); *((uint32_t*)(__ch_dst - 4)) = *((uint32_t*)(__ch_src - 4)); break;
		case 13: *((uint64_t*)(__ch_dst - 13)) = *((uint64_t*)(__ch_src - 13)); *((uint64_t*)(__ch_dst - 8)) = *((uint64_t*)(__ch_src - 8)); break;
		case 14: *((uint64_t*)(__ch_dst - 14)) = *((uint64_t*)(__ch_src - 14)); *((uint64_t*)(__ch_dst - 8)) = *((uint64_t*)(__ch_src - 8)); break;
		case 15: *((uint64_t*)(__ch_dst - 15)) = *((uint64_t*)(__ch_src - 15)); *((uint64_t*)(__ch_dst - 8)) = *((uint64_t*)(__ch_src - 8)); break;
	}
}


raze_unmangled raze_never_inline raze_declare_const_function void* raze_stdcall __raze_memcpy(void* raze_restrict __dst,
	const void* raze_restrict __src, sizetype __bytes) noexcept
{
#if 1
	std::memcpy(__dst, __src, __bytes);
	return __bytes_pointer_offset(__dst, __bytes);
#else
	constexpr auto __erms_threshold = 0x1000;

	u8* __ch_dst = (u8*)__dst;
	const u8* __ch_src = (const u8*)__src;
	
	if (__bytes >= 16) {
		if (__bytes >= 32) {
			if (__bytes >= __erms_threshold) {
				i32 __all_features = arch::ProcessorFeatures::all();

				if (arch::ProcessorFeatures::has<arch::__features::ERMS>(__all_features)) {
					__movsb(__ch_dst, __ch_src, __bytes);
					return __bytes_pointer_offset(__ch_dst, __bytes);
				}
				else {
					if (arch::ProcessorFeatures::has<arch::__features::AVX512F>(__all_features)) {
						#pragma loop(no_vector)
						do {
							_mm512_storeu_si512(__ch_dst, _mm512_loadu_si512(__ch_src));
							_mm512_storeu_si512(__ch_dst + 64, _mm512_loadu_si512(__ch_src + 64));
							_mm512_storeu_si512(__ch_dst + 128, _mm512_loadu_si512(__ch_src + 128));
							_mm512_storeu_si512(__ch_dst + 192, _mm512_loadu_si512(__ch_src + 192));
							__ch_dst += 256;
							__ch_src += 256;
							__bytes -= 256;
						} while (__bytes >= 256);

						goto __avx512_process_tail_from_0_to_256;
					}
					else if (arch::ProcessorFeatures::has<arch::__features::AVX>(__all_features)) {
						#pragma loop(no_vector)
						do {
							_mm256_storeu_si256(reinterpret_cast<__m256i*>(__ch_dst), _mm256_loadu_si256(reinterpret_cast<const __m256i*>(__ch_src)));
							_mm256_storeu_si256(reinterpret_cast<__m256i*>(__ch_dst) + 1, _mm256_loadu_si256(reinterpret_cast<const __m256i*>(__ch_src) + 1));
							_mm256_storeu_si256(reinterpret_cast<__m256i*>(__ch_dst) + 2, _mm256_loadu_si256(reinterpret_cast<const __m256i*>(__ch_src) + 2));
							_mm256_storeu_si256(reinterpret_cast<__m256i*>(__ch_dst) + 3, _mm256_loadu_si256(reinterpret_cast<const __m256i*>(__ch_src) + 3));
							__ch_dst += 128;
							__ch_src += 128;
							__bytes -= 128;
						} while (__bytes >= 128);

						goto __avx_process_tail_from_0_to_128;
					}
					else {
						#pragma loop(no_vector)
						do {
							_mm_storeu_si128(reinterpret_cast<__m128i*>(__ch_dst), _mm_loadu_si128(reinterpret_cast<const __m128i*>(__ch_src)));
							_mm_storeu_si128(reinterpret_cast<__m128i*>(__ch_dst) + 1, _mm_loadu_si128(reinterpret_cast<const __m128i*>(__ch_src) + 1));
							_mm_storeu_si128(reinterpret_cast<__m128i*>(__ch_dst) + 2, _mm_loadu_si128(reinterpret_cast<const __m128i*>(__ch_src) + 2));
							_mm_storeu_si128(reinterpret_cast<__m128i*>(__ch_dst) + 3, _mm_loadu_si128(reinterpret_cast<const __m128i*>(__ch_src) + 3));
							__ch_dst += 64;
							__ch_src += 64;
							__bytes -= 64;
						} while (__bytes >= 64);

						goto __sse2_process_tail_from_0_to_64;
					}

					__memcpy_scalar_impl(__ch_dst, __ch_src, __bytes);
					return __bytes_pointer_offset(__ch_dst, __bytes);
				}
			}
			else {
				i32 __all_features = arch::ProcessorFeatures::all();

				if (arch::ProcessorFeatures::has<arch::__features::AVX512F>(__all_features)) {
					#pragma loop(no_vector)
					while (__bytes >= 256) {
						_mm512_storeu_si512(__ch_dst, _mm512_loadu_si512(__ch_src));
						_mm512_storeu_si512(__ch_dst + 64, _mm512_loadu_si512(__ch_src + 64));
						_mm512_storeu_si512(__ch_dst + 128, _mm512_loadu_si512(__ch_src + 128));
						_mm512_storeu_si512(__ch_dst + 192, _mm512_loadu_si512(__ch_src + 192));
						__ch_dst += 256;
						__ch_src += 256;
						__bytes -= 256;
					}

					goto __avx512_process_tail_from_0_to_256;
				}
				else if (arch::ProcessorFeatures::has<arch::__features::AVX>(__all_features)) {
					#pragma loop(no_vector)
					while (__bytes >= 128) {
						_mm256_storeu_si256(reinterpret_cast<__m256i*>(__ch_dst), _mm256_loadu_si256(reinterpret_cast<const __m256i*>(__ch_src)));
						_mm256_storeu_si256(reinterpret_cast<__m256i*>(__ch_dst) + 1, _mm256_loadu_si256(reinterpret_cast<const __m256i*>(__ch_src) + 1));
						_mm256_storeu_si256(reinterpret_cast<__m256i*>(__ch_dst) + 2, _mm256_loadu_si256(reinterpret_cast<const __m256i*>(__ch_src) + 2));
						_mm256_storeu_si256(reinterpret_cast<__m256i*>(__ch_dst) + 3, _mm256_loadu_si256(reinterpret_cast<const __m256i*>(__ch_src) + 3));
						__ch_dst += 128;
						__ch_src += 128;
						__bytes -= 128;
					}

					goto __avx_process_tail_from_0_to_128;
				}
				else {
					#pragma loop(no_vector)
					while (__bytes >= 64) {
						_mm_storeu_si128(reinterpret_cast<__m128i*>(__ch_dst), _mm_loadu_si128(reinterpret_cast<const __m128i*>(__ch_src)));
						_mm_storeu_si128(reinterpret_cast<__m128i*>(__ch_dst) + 1, _mm_loadu_si128(reinterpret_cast<const __m128i*>(__ch_src) + 1));
						_mm_storeu_si128(reinterpret_cast<__m128i*>(__ch_dst) + 2, _mm_loadu_si128(reinterpret_cast<const __m128i*>(__ch_src) + 2));
						_mm_storeu_si128(reinterpret_cast<__m128i*>(__ch_dst) + 3, _mm_loadu_si128(reinterpret_cast<const __m128i*>(__ch_src) + 3));
						__ch_dst += 64;
						__ch_src += 64;
						__bytes -= 64;
					}

					goto __sse2_process_tail_from_0_to_64;
				}
			}

			if (arch::ProcessorFeatures::AVX()) __memcpy_small_avx(__dst, __src, __bytes);
			return __bytes_pointer_offset(__ch_dst, __bytes);
		}

		__memcpy_small_sse2(__dst, __src, __bytes);
		return __bytes_pointer_offset(__ch_dst, __bytes);
	}
	else {
		__memcpy_small(__dst, __src, __bytes);
		return __bytes_pointer_offset(__ch_dst, __bytes);
	}

__avx_process_tail_from_0_to_128:
	if (__bytes >= 64) {
		_mm256_storeu_si256(reinterpret_cast<__m256i*>(__ch_dst), _mm256_loadu_si256(reinterpret_cast<const __m256i*>(__ch_src)));
		_mm256_storeu_si256(reinterpret_cast<__m256i*>(__ch_dst) + 1, _mm256_loadu_si256(reinterpret_cast<const __m256i*>(__ch_src) + 1));
		__ch_dst += 64;
		__ch_src += 64;
		__bytes -= 64;
	}

	__memcpy_small_avx(__ch_dst, __ch_src, __bytes);
	return __bytes_pointer_offset(__ch_dst, __bytes);

__sse2_process_tail_from_0_to_64:
	if (__bytes >= 32) {
		_mm_storeu_si128(reinterpret_cast<__m128i*>(__ch_dst), _mm_loadu_si128(reinterpret_cast<const __m128i*>(__ch_src)));
		_mm_storeu_si128(reinterpret_cast<__m128i*>(__ch_dst) + 1, _mm_loadu_si128(reinterpret_cast<const __m128i*>(__ch_src) + 1));
		__ch_dst += 32;
		__ch_src += 32;
		__bytes -= 32;
	}

	__memcpy_small_sse2(__ch_dst, __ch_src, __bytes);
	return __bytes_pointer_offset(__ch_dst, __bytes);

__avx512_process_tail_from_0_to_256:
	if (__bytes >= 128) {
		_mm512_storeu_si512(__ch_dst, _mm512_loadu_si512(__ch_src));
		_mm512_storeu_si512(__ch_dst + 64, _mm512_loadu_si512(__ch_src + 64));
		__ch_dst += 128;
		__ch_src += 128;
		__bytes -= 128;
	}

	if (__bytes >= 64) {
		_mm512_storeu_si512(__ch_dst, _mm512_loadu_si512(__ch_src));
		__ch_dst += 64;
		__ch_src += 64;
		__bytes -= 64;
	}

	__memcpy_small_avx512f(__ch_dst, __ch_src, __bytes);
	return __bytes_pointer_offset(__ch_dst, __bytes);
#endif
}

__RAZE_ALGORITHM_NAMESPACE_END
