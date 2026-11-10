#pragma once

#include <raze/compatibility/Compatibility.h>
#include <src/raze/algorithm/AdvanceBytes.h>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

#if defined(raze_cpp_msvc_only) 

raze_always_inline void* memcpy_scalar_impl(volatile u8* raze_restrict dst_ch,
	const volatile u8* raze_restrict src_ch, sizetype bytes) noexcept
{
	while (bytes >= 8) {
		*reinterpret_cast<volatile raze::i64*>(dst_ch) = *reinterpret_cast<const volatile raze::i64*>(src_ch);
		dst_ch += 8; src_ch += 8; bytes -= 8;
	}
	if (bytes >= 4) {
		*reinterpret_cast<volatile raze::i32*>(dst_ch) = *reinterpret_cast<const volatile raze::i32*>(src_ch);
		dst_ch += 4; src_ch += 4; bytes -= 4;
	}
	if (bytes >= 2) {
		*reinterpret_cast<volatile raze::i16*>(dst_ch) = *reinterpret_cast<const volatile raze::i16*>(src_ch);
		dst_ch += 2; src_ch += 2; bytes -= 2;
	}
	if (bytes) {
		*dst_ch++ = *src_ch++;
	}

	return const_cast<u8*>(dst_ch);
}

raze_unmangled raze_no_stack_protector raze_declare_const_function void* raze_stdcall raze_memcpy_scalar(void* raze_restrict dst,
	const void* raze_restrict src, sizetype bytes) noexcept
{
	return memcpy_scalar_impl(static_cast<u8*>(dst), static_cast<const u8*>(src), bytes);
}

raze_always_inline void memcpy_16(void* raze_restrict dst, const void* raze_restrict src) noexcept {
	_mm_storeu_si128(reinterpret_cast<__m128i*>(dst), _mm_loadu_si128(reinterpret_cast<const __m128i*>(src)));
}

raze_always_inline void memcpy_sse2_32(void* raze_restrict dst, const void* raze_restrict src) noexcept {
	_mm_storeu_si128(reinterpret_cast<__m128i*>(dst), _mm_loadu_si128(reinterpret_cast<const __m128i*>(src)));
	_mm_storeu_si128(reinterpret_cast<__m128i*>(dst) + 1, _mm_loadu_si128(reinterpret_cast<const __m128i*>(src) + 1));
}

raze_always_inline void memcpy_avx_32(void* raze_restrict dst, const void* raze_restrict src) noexcept {
	_mm256_storeu_si256(reinterpret_cast<__m256i*>(dst), _mm256_loadu_si256(reinterpret_cast<const __m256i*>(src)));
}

raze_always_inline void memcpy_avx512_64(void* raze_restrict dst, const void* raze_restrict src) noexcept {
	_mm512_storeu_si512(dst, _mm512_loadu_si512(src));
}

raze_always_inline void memcpy_small_avx512f(void* raze_restrict dst, const void* raze_restrict src, sizetype size) {
	u8* ch_dst = reinterpret_cast<u8*>(dst) + size;
	const u8* ch_src = reinterpret_cast<const u8*>(src) + size;

	switch (size) {
		case 0: break;
		case 1: ch_dst[-1] = ch_src[-1]; break;
		case 2: *((uint16_t*)(ch_dst - 2)) = *((uint16_t*)(ch_src - 2)); break;
		case 3: *((uint16_t*)(ch_dst - 3)) = *((uint16_t*)(ch_src - 3)); ch_dst[-1] = ch_src[-1]; break;
		case 4: *((uint32_t*)(ch_dst - 4)) = *((uint32_t*)(ch_src - 4)); break;
		case 5: *((uint32_t*)(ch_dst - 5)) = *((uint32_t*)(ch_src - 5)); ch_dst[-1] = ch_src[-1]; break;
		case 6: *((uint32_t*)(ch_dst - 6)) = *((uint32_t*)(ch_src - 6)); *((uint16_t*)(ch_dst - 2)) = *((uint16_t*)(ch_src - 2)); break;
		case 7: *((uint32_t*)(ch_dst - 7)) = *((uint32_t*)(ch_src - 7)); *((uint32_t*)(ch_dst - 4)) = *((uint32_t*)(ch_src - 4)); break;
		case 8: *((uint64_t*)(ch_dst - 8)) = *((uint64_t*)(ch_src - 8)); break;
		case 9: *((uint64_t*)(ch_dst - 9)) = *((uint64_t*)(ch_src - 9)); ch_dst[-1] = ch_src[-1]; break;
		case 10: *((uint64_t*)(ch_dst - 10)) = *((uint64_t*)(ch_src - 10)); *((uint16_t*)(ch_dst - 2)) = *((uint16_t*)(ch_src - 2)); break;
		case 11: *((uint64_t*)(ch_dst - 11)) = *((uint64_t*)(ch_src - 11)); *((uint32_t*)(ch_dst - 4)) = *((uint32_t*)(ch_src - 4)); break;
		case 12: *((uint64_t*)(ch_dst - 12)) = *((uint64_t*)(ch_src - 12)); *((uint32_t*)(ch_dst - 4)) = *((uint32_t*)(ch_src - 4)); break;
		case 13: *((uint64_t*)(ch_dst - 13)) = *((uint64_t*)(ch_src - 13)); *((uint64_t*)(ch_dst - 8)) = *((uint64_t*)(ch_src - 8)); break;
		case 14: *((uint64_t*)(ch_dst - 14)) = *((uint64_t*)(ch_src - 14)); *((uint64_t*)(ch_dst - 8)) = *((uint64_t*)(ch_src - 8)); break;
		case 15: *((uint64_t*)(ch_dst - 15)) = *((uint64_t*)(ch_src - 15)); *((uint64_t*)(ch_dst - 8)) = *((uint64_t*)(ch_src - 8)); break;
		case 16: memcpy_16(ch_dst - 16, ch_src - 16); break;
		case 17: memcpy_16(ch_dst - 17, ch_src - 17); memcpy_16(ch_dst - 16, ch_src - 16); break;
		case 18: memcpy_16(ch_dst - 18, ch_src - 18); memcpy_16(ch_dst - 16, ch_src - 16); break;
		case 19: memcpy_16(ch_dst - 19, ch_src - 19); memcpy_16(ch_dst - 16, ch_src - 16); break;
		case 20: memcpy_16(ch_dst - 20, ch_src - 20); memcpy_16(ch_dst - 16, ch_src - 16); break;
		case 21: memcpy_16(ch_dst - 21, ch_src - 21); memcpy_16(ch_dst - 16, ch_src - 16); break;
		case 22: memcpy_16(ch_dst - 22, ch_src - 22); memcpy_16(ch_dst - 16, ch_src - 16); break;
		case 23: memcpy_16(ch_dst - 23, ch_src - 23); memcpy_16(ch_dst - 16, ch_src - 16); break;
		case 24: memcpy_16(ch_dst - 24, ch_src - 24); memcpy_16(ch_dst - 16, ch_src - 16); break;
		case 25: memcpy_16(ch_dst - 25, ch_src - 25); memcpy_16(ch_dst - 16, ch_src - 16); break;
		case 26: memcpy_16(ch_dst - 26, ch_src - 26); memcpy_16(ch_dst - 16, ch_src - 16); break;
		case 27: memcpy_16(ch_dst - 27, ch_src - 27); memcpy_16(ch_dst - 16, ch_src - 16); break;
		case 28: memcpy_16(ch_dst - 28, ch_src - 28); memcpy_16(ch_dst - 16, ch_src - 16); break;
		case 29: memcpy_16(ch_dst - 29, ch_src - 29); memcpy_16(ch_dst - 16, ch_src - 16); break;
		case 30: memcpy_16(ch_dst - 30, ch_src - 30); memcpy_16(ch_dst - 16, ch_src - 16); break;
		case 31: memcpy_16(ch_dst - 31, ch_src - 31); memcpy_16(ch_dst - 16, ch_src - 16); break;
		case 32: memcpy_avx_32(ch_dst - 32, ch_src - 32); break;
		case 33: memcpy_avx_32(ch_dst - 33, ch_src - 33); memcpy_avx_32(ch_dst - 32, ch_src - 32); break;
		case 34: memcpy_avx_32(ch_dst - 34, ch_src - 34); memcpy_avx_32(ch_dst - 32, ch_src - 32); break;
		case 35: memcpy_avx_32(ch_dst - 35, ch_src - 35); memcpy_avx_32(ch_dst - 32, ch_src - 32); break;
		case 36: memcpy_avx_32(ch_dst - 36, ch_src - 36); memcpy_avx_32(ch_dst - 32, ch_src - 32); break;
		case 37: memcpy_avx_32(ch_dst - 37, ch_src - 37); memcpy_avx_32(ch_dst - 32, ch_src - 32); break;
		case 38: memcpy_avx_32(ch_dst - 38, ch_src - 38); memcpy_avx_32(ch_dst - 32, ch_src - 32); break;
		case 39: memcpy_avx_32(ch_dst - 39, ch_src - 39); memcpy_avx_32(ch_dst - 32, ch_src - 32); break;
		case 40: memcpy_avx_32(ch_dst - 40, ch_src - 40); memcpy_avx_32(ch_dst - 32, ch_src - 32); break;
		case 41: memcpy_avx_32(ch_dst - 41, ch_src - 41); memcpy_avx_32(ch_dst - 32, ch_src - 32); break;
		case 42: memcpy_avx_32(ch_dst - 42, ch_src - 42); memcpy_avx_32(ch_dst - 32, ch_src - 32); break;
		case 43: memcpy_avx_32(ch_dst - 43, ch_src - 43); memcpy_avx_32(ch_dst - 32, ch_src - 32); break;
		case 44: memcpy_avx_32(ch_dst - 44, ch_src - 44); memcpy_avx_32(ch_dst - 32, ch_src - 32); break;
		case 45: memcpy_avx_32(ch_dst - 45, ch_src - 45); memcpy_avx_32(ch_dst - 32, ch_src - 32); break;
		case 46: memcpy_avx_32(ch_dst - 46, ch_src - 46); memcpy_avx_32(ch_dst - 32, ch_src - 32); break;
		case 47: memcpy_avx_32(ch_dst - 47, ch_src - 47); memcpy_avx_32(ch_dst - 32, ch_src - 32); break;
		case 48: memcpy_avx_32(ch_dst - 48, ch_src - 48); memcpy_avx_32(ch_dst - 32, ch_src - 32); break;
		case 49: memcpy_avx_32(ch_dst - 49, ch_src - 49); memcpy_avx_32(ch_dst - 32, ch_src - 32); break;
		case 50: memcpy_avx_32(ch_dst - 50, ch_src - 50); memcpy_avx_32(ch_dst - 32, ch_src - 32); break;
		case 51: memcpy_avx_32(ch_dst - 51, ch_src - 51); memcpy_avx_32(ch_dst - 32, ch_src - 32); break;
		case 52: memcpy_avx_32(ch_dst - 52, ch_src - 52); memcpy_avx_32(ch_dst - 32, ch_src - 32); break;
		case 53: memcpy_avx_32(ch_dst - 53, ch_src - 53); memcpy_avx_32(ch_dst - 32, ch_src - 32); break;
		case 54: memcpy_avx_32(ch_dst - 54, ch_src - 54); memcpy_avx_32(ch_dst - 32, ch_src - 32); break;
		case 55: memcpy_avx_32(ch_dst - 55, ch_src - 55); memcpy_avx_32(ch_dst - 32, ch_src - 32); break;
		case 56: memcpy_avx_32(ch_dst - 56, ch_src - 56); memcpy_avx_32(ch_dst - 32, ch_src - 32); break;
		case 57: memcpy_avx_32(ch_dst - 57, ch_src - 57); memcpy_avx_32(ch_dst - 32, ch_src - 32); break;
		case 58: memcpy_avx_32(ch_dst - 58, ch_src - 58); memcpy_avx_32(ch_dst - 32, ch_src - 32); break;
		case 59: memcpy_avx_32(ch_dst - 59, ch_src - 59); memcpy_avx_32(ch_dst - 32, ch_src - 32); break;
		case 60: memcpy_avx_32(ch_dst - 60, ch_src - 60); memcpy_avx_32(ch_dst - 32, ch_src - 32); break;
		case 61: memcpy_avx_32(ch_dst - 61, ch_src - 61); memcpy_avx_32(ch_dst - 32, ch_src - 32); break;
		case 62: memcpy_avx_32(ch_dst - 62, ch_src - 62); memcpy_avx_32(ch_dst - 32, ch_src - 32); break;
		case 63: memcpy_avx_32(ch_dst - 63, ch_src - 63); memcpy_avx_32(ch_dst - 32, ch_src - 32); break;
		case 64: memcpy_avx512_64(ch_dst - 64, ch_src - 64); break;
	}
}

raze_always_inline void memcpy_small_avx(void* raze_restrict dst, const void* raze_restrict src, sizetype size) {
	u8* ch_dst = reinterpret_cast<u8*>(dst) + size;
	const u8* ch_src = reinterpret_cast<const u8*>(src) + size;

	switch (size) {
		case 0: break;
		case 1: ch_dst[-1] = ch_src[-1]; break;
		case 2: *((uint16_t*)(ch_dst - 2)) = *((uint16_t*)(ch_src - 2)); break;
		case 3: *((uint16_t*)(ch_dst - 3)) = *((uint16_t*)(ch_src - 3)); ch_dst[-1] = ch_src[-1]; break;
		case 4: *((uint32_t*)(ch_dst - 4)) = *((uint32_t*)(ch_src - 4)); break;
		case 5: *((uint32_t*)(ch_dst - 5)) = *((uint32_t*)(ch_src - 5)); ch_dst[-1] = ch_src[-1]; break;
		case 6: *((uint32_t*)(ch_dst - 6)) = *((uint32_t*)(ch_src - 6)); *((uint16_t*)(ch_dst - 2)) = *((uint16_t*)(ch_src - 2)); break;
		case 7: *((uint32_t*)(ch_dst - 7)) = *((uint32_t*)(ch_src - 7)); *((uint32_t*)(ch_dst - 4)) = *((uint32_t*)(ch_src - 4)); break;
		case 8: *((uint64_t*)(ch_dst - 8)) = *((uint64_t*)(ch_src - 8)); break;
		case 9: *((uint64_t*)(ch_dst - 9)) = *((uint64_t*)(ch_src - 9)); ch_dst[-1] = ch_src[-1]; break;
		case 10: *((uint64_t*)(ch_dst - 10)) = *((uint64_t*)(ch_src - 10)); *((uint16_t*)(ch_dst - 2)) = *((uint16_t*)(ch_src - 2)); break;
		case 11: *((uint64_t*)(ch_dst - 11)) = *((uint64_t*)(ch_src - 11)); *((uint32_t*)(ch_dst - 4)) = *((uint32_t*)(ch_src - 4)); break;
		case 12: *((uint64_t*)(ch_dst - 12)) = *((uint64_t*)(ch_src - 12)); *((uint32_t*)(ch_dst - 4)) = *((uint32_t*)(ch_src - 4)); break;
		case 13: *((uint64_t*)(ch_dst - 13)) = *((uint64_t*)(ch_src - 13)); *((uint64_t*)(ch_dst - 8)) = *((uint64_t*)(ch_src - 8)); break;
		case 14: *((uint64_t*)(ch_dst - 14)) = *((uint64_t*)(ch_src - 14)); *((uint64_t*)(ch_dst - 8)) = *((uint64_t*)(ch_src - 8)); break;
		case 15: *((uint64_t*)(ch_dst - 15)) = *((uint64_t*)(ch_src - 15)); *((uint64_t*)(ch_dst - 8)) = *((uint64_t*)(ch_src - 8)); break;
		case 16: memcpy_16(ch_dst - 16, ch_src - 16); break;
		case 17: memcpy_16(ch_dst - 17, ch_src - 17); memcpy_16(ch_dst - 16, ch_src - 16); break;
		case 18: memcpy_16(ch_dst - 18, ch_src - 18); memcpy_16(ch_dst - 16, ch_src - 16); break;
		case 19: memcpy_16(ch_dst - 19, ch_src - 19); memcpy_16(ch_dst - 16, ch_src - 16); break;
		case 20: memcpy_16(ch_dst - 20, ch_src - 20); memcpy_16(ch_dst - 16, ch_src - 16); break;
		case 21: memcpy_16(ch_dst - 21, ch_src - 21); memcpy_16(ch_dst - 16, ch_src - 16); break;
		case 22: memcpy_16(ch_dst - 22, ch_src - 22); memcpy_16(ch_dst - 16, ch_src - 16); break;
		case 23: memcpy_16(ch_dst - 23, ch_src - 23); memcpy_16(ch_dst - 16, ch_src - 16); break;
		case 24: memcpy_16(ch_dst - 24, ch_src - 24); memcpy_16(ch_dst - 16, ch_src - 16); break;
		case 25: memcpy_16(ch_dst - 25, ch_src - 25); memcpy_16(ch_dst - 16, ch_src - 16); break;
		case 26: memcpy_16(ch_dst - 26, ch_src - 26); memcpy_16(ch_dst - 16, ch_src - 16); break;
		case 27: memcpy_16(ch_dst - 27, ch_src - 27); memcpy_16(ch_dst - 16, ch_src - 16); break;
		case 28: memcpy_16(ch_dst - 28, ch_src - 28); memcpy_16(ch_dst - 16, ch_src - 16); break;
		case 29: memcpy_16(ch_dst - 29, ch_src - 29); memcpy_16(ch_dst - 16, ch_src - 16); break;
		case 30: memcpy_16(ch_dst - 30, ch_src - 30); memcpy_16(ch_dst - 16, ch_src - 16); break;
		case 31: memcpy_16(ch_dst - 31, ch_src - 31); memcpy_16(ch_dst - 16, ch_src - 16); break;
		case 32: memcpy_avx_32(ch_dst - 32, ch_src - 32); break;
		case 33: memcpy_avx_32(ch_dst - 33, ch_src - 33); memcpy_avx_32(ch_dst - 32, ch_src - 32); break;
		case 34: memcpy_avx_32(ch_dst - 34, ch_src - 34); memcpy_avx_32(ch_dst - 32, ch_src - 32); break;
		case 35: memcpy_avx_32(ch_dst - 35, ch_src - 35); memcpy_avx_32(ch_dst - 32, ch_src - 32); break;
		case 36: memcpy_avx_32(ch_dst - 36, ch_src - 36); memcpy_avx_32(ch_dst - 32, ch_src - 32); break;
		case 37: memcpy_avx_32(ch_dst - 37, ch_src - 37); memcpy_avx_32(ch_dst - 32, ch_src - 32); break;
		case 38: memcpy_avx_32(ch_dst - 38, ch_src - 38); memcpy_avx_32(ch_dst - 32, ch_src - 32); break;
		case 39: memcpy_avx_32(ch_dst - 39, ch_src - 39); memcpy_avx_32(ch_dst - 32, ch_src - 32); break;
		case 40: memcpy_avx_32(ch_dst - 40, ch_src - 40); memcpy_avx_32(ch_dst - 32, ch_src - 32); break;
		case 41: memcpy_avx_32(ch_dst - 41, ch_src - 41); memcpy_avx_32(ch_dst - 32, ch_src - 32); break;
		case 42: memcpy_avx_32(ch_dst - 42, ch_src - 42); memcpy_avx_32(ch_dst - 32, ch_src - 32); break;
		case 43: memcpy_avx_32(ch_dst - 43, ch_src - 43); memcpy_avx_32(ch_dst - 32, ch_src - 32); break;
		case 44: memcpy_avx_32(ch_dst - 44, ch_src - 44); memcpy_avx_32(ch_dst - 32, ch_src - 32); break;
		case 45: memcpy_avx_32(ch_dst - 45, ch_src - 45); memcpy_avx_32(ch_dst - 32, ch_src - 32); break;
		case 46: memcpy_avx_32(ch_dst - 46, ch_src - 46); memcpy_avx_32(ch_dst - 32, ch_src - 32); break;
		case 47: memcpy_avx_32(ch_dst - 47, ch_src - 47); memcpy_avx_32(ch_dst - 32, ch_src - 32); break;
		case 48: memcpy_avx_32(ch_dst - 48, ch_src - 48); memcpy_avx_32(ch_dst - 32, ch_src - 32); break;
		case 49: memcpy_avx_32(ch_dst - 49, ch_src - 49); memcpy_avx_32(ch_dst - 32, ch_src - 32); break;
		case 50: memcpy_avx_32(ch_dst - 50, ch_src - 50); memcpy_avx_32(ch_dst - 32, ch_src - 32); break;
		case 51: memcpy_avx_32(ch_dst - 51, ch_src - 51); memcpy_avx_32(ch_dst - 32, ch_src - 32); break;
		case 52: memcpy_avx_32(ch_dst - 52, ch_src - 52); memcpy_avx_32(ch_dst - 32, ch_src - 32); break;
		case 53: memcpy_avx_32(ch_dst - 53, ch_src - 53); memcpy_avx_32(ch_dst - 32, ch_src - 32); break;
		case 54: memcpy_avx_32(ch_dst - 54, ch_src - 54); memcpy_avx_32(ch_dst - 32, ch_src - 32); break;
		case 55: memcpy_avx_32(ch_dst - 55, ch_src - 55); memcpy_avx_32(ch_dst - 32, ch_src - 32); break;
		case 56: memcpy_avx_32(ch_dst - 56, ch_src - 56); memcpy_avx_32(ch_dst - 32, ch_src - 32); break;
		case 57: memcpy_avx_32(ch_dst - 57, ch_src - 57); memcpy_avx_32(ch_dst - 32, ch_src - 32); break;
		case 58: memcpy_avx_32(ch_dst - 58, ch_src - 58); memcpy_avx_32(ch_dst - 32, ch_src - 32); break;
		case 59: memcpy_avx_32(ch_dst - 59, ch_src - 59); memcpy_avx_32(ch_dst - 32, ch_src - 32); break;
		case 60: memcpy_avx_32(ch_dst - 60, ch_src - 60); memcpy_avx_32(ch_dst - 32, ch_src - 32); break;
		case 61: memcpy_avx_32(ch_dst - 61, ch_src - 61); memcpy_avx_32(ch_dst - 32, ch_src - 32); break;
		case 62: memcpy_avx_32(ch_dst - 62, ch_src - 62); memcpy_avx_32(ch_dst - 32, ch_src - 32); break;
		case 63: memcpy_avx_32(ch_dst - 63, ch_src - 63); memcpy_avx_32(ch_dst - 32, ch_src - 32); break;
		case 64: memcpy_avx_32(ch_dst - 64, ch_src - 64); memcpy_avx_32(ch_dst - 32, ch_src - 32);  break;
	}
}

raze_always_inline void memcpy_small_sse2(void* raze_restrict dst, const void* raze_restrict src, sizetype size) {
	u8* ch_dst = reinterpret_cast<u8*>(dst) + size;
	const u8* ch_src = reinterpret_cast<const u8*>(src) + size;

	switch (size) {
		case 0: break;
		case 1: ch_dst[-1] = ch_src[-1]; break;
		case 2: *((uint16_t*)(ch_dst - 2)) = *((uint16_t*)(ch_src - 2)); break;
		case 3: *((uint16_t*)(ch_dst - 3)) = *((uint16_t*)(ch_src - 3)); ch_dst[-1] = ch_src[-1]; break;
		case 4: *((uint32_t*)(ch_dst - 4)) = *((uint32_t*)(ch_src - 4)); break;
		case 5: *((uint32_t*)(ch_dst - 5)) = *((uint32_t*)(ch_src - 5)); ch_dst[-1] = ch_src[-1]; break;
		case 6: *((uint32_t*)(ch_dst - 6)) = *((uint32_t*)(ch_src - 6)); *((uint16_t*)(ch_dst - 2)) = *((uint16_t*)(ch_src - 2)); break;
		case 7: *((uint32_t*)(ch_dst - 7)) = *((uint32_t*)(ch_src - 7)); *((uint32_t*)(ch_dst - 4)) = *((uint32_t*)(ch_src - 4)); break;
		case 8: *((uint64_t*)(ch_dst - 8)) = *((uint64_t*)(ch_src - 8)); break;
		case 9: *((uint64_t*)(ch_dst - 9)) = *((uint64_t*)(ch_src - 9)); ch_dst[-1] = ch_src[-1]; break;
		case 10: *((uint64_t*)(ch_dst - 10)) = *((uint64_t*)(ch_src - 10)); *((uint16_t*)(ch_dst - 2)) = *((uint16_t*)(ch_src - 2)); break;
		case 11: *((uint64_t*)(ch_dst - 11)) = *((uint64_t*)(ch_src - 11)); *((uint32_t*)(ch_dst - 4)) = *((uint32_t*)(ch_src - 4)); break;
		case 12: *((uint64_t*)(ch_dst - 12)) = *((uint64_t*)(ch_src - 12)); *((uint32_t*)(ch_dst - 4)) = *((uint32_t*)(ch_src - 4)); break;
		case 13: *((uint64_t*)(ch_dst - 13)) = *((uint64_t*)(ch_src - 13)); *((uint64_t*)(ch_dst - 8)) = *((uint64_t*)(ch_src - 8)); break;
		case 14: *((uint64_t*)(ch_dst - 14)) = *((uint64_t*)(ch_src - 14)); *((uint64_t*)(ch_dst - 8)) = *((uint64_t*)(ch_src - 8)); break;
		case 15: *((uint64_t*)(ch_dst - 15)) = *((uint64_t*)(ch_src - 15)); *((uint64_t*)(ch_dst - 8)) = *((uint64_t*)(ch_src - 8)); break;
		case 16: memcpy_16(ch_dst - 16, ch_src - 16); break;
		case 17: memcpy_16(ch_dst - 17, ch_src - 17); memcpy_16(ch_dst - 16, ch_src - 16); break;
		case 18: memcpy_16(ch_dst - 18, ch_src - 18); memcpy_16(ch_dst - 16, ch_src - 16); break;
		case 19: memcpy_16(ch_dst - 19, ch_src - 19); memcpy_16(ch_dst - 16, ch_src - 16); break;
		case 20: memcpy_16(ch_dst - 20, ch_src - 20); memcpy_16(ch_dst - 16, ch_src - 16); break;
		case 21: memcpy_16(ch_dst - 21, ch_src - 21); memcpy_16(ch_dst - 16, ch_src - 16); break;
		case 22: memcpy_16(ch_dst - 22, ch_src - 22); memcpy_16(ch_dst - 16, ch_src - 16); break;
		case 23: memcpy_16(ch_dst - 23, ch_src - 23); memcpy_16(ch_dst - 16, ch_src - 16); break;
		case 24: memcpy_16(ch_dst - 24, ch_src - 24); memcpy_16(ch_dst - 16, ch_src - 16); break;
		case 25: memcpy_16(ch_dst - 25, ch_src - 25); memcpy_16(ch_dst - 16, ch_src - 16); break;
		case 26: memcpy_16(ch_dst - 26, ch_src - 26); memcpy_16(ch_dst - 16, ch_src - 16); break;
		case 27: memcpy_16(ch_dst - 27, ch_src - 27); memcpy_16(ch_dst - 16, ch_src - 16); break;
		case 28: memcpy_16(ch_dst - 28, ch_src - 28); memcpy_16(ch_dst - 16, ch_src - 16); break;
		case 29: memcpy_16(ch_dst - 29, ch_src - 29); memcpy_16(ch_dst - 16, ch_src - 16); break;
		case 30: memcpy_16(ch_dst - 30, ch_src - 30); memcpy_16(ch_dst - 16, ch_src - 16); break;
		case 31: memcpy_16(ch_dst - 31, ch_src - 31); memcpy_16(ch_dst - 16, ch_src - 16); break;
		case 32: memcpy_16(ch_dst - 32, ch_src - 32); memcpy_16(ch_dst - 16, ch_src - 16); break;
	}
}

raze_always_inline void memcpy_small(void* raze_restrict dst, const void* raze_restrict src, sizetype size) {
	u8* ch_dst = reinterpret_cast<u8*>(dst) + size;
	const u8* ch_src = reinterpret_cast<const u8*>(src) + size;

	switch (size) {
		case 0: break;
		case 1: ch_dst[-1] = ch_src[-1]; break;
		case 2: *((uint16_t*)(ch_dst - 2)) = *((uint16_t*)(ch_src - 2)); break;
		case 3: *((uint16_t*)(ch_dst - 3)) = *((uint16_t*)(ch_src - 3)); ch_dst[-1] = ch_src[-1]; break;
		case 4: *((uint32_t*)(ch_dst - 4)) = *((uint32_t*)(ch_src - 4)); break;
		case 5: *((uint32_t*)(ch_dst - 5)) = *((uint32_t*)(ch_src - 5)); ch_dst[-1] = ch_src[-1]; break;
		case 6: *((uint32_t*)(ch_dst - 6)) = *((uint32_t*)(ch_src - 6)); *((uint16_t*)(ch_dst - 2)) = *((uint16_t*)(ch_src - 2)); break;
		case 7: *((uint32_t*)(ch_dst - 7)) = *((uint32_t*)(ch_src - 7)); *((uint32_t*)(ch_dst - 4)) = *((uint32_t*)(ch_src - 4)); break;
		case 8: *((uint64_t*)(ch_dst - 8)) = *((uint64_t*)(ch_src - 8)); break;
		case 9: *((uint64_t*)(ch_dst - 9)) = *((uint64_t*)(ch_src - 9)); ch_dst[-1] = ch_src[-1]; break;
		case 10: *((uint64_t*)(ch_dst - 10)) = *((uint64_t*)(ch_src - 10)); *((uint16_t*)(ch_dst - 2)) = *((uint16_t*)(ch_src - 2)); break;
		case 11: *((uint64_t*)(ch_dst - 11)) = *((uint64_t*)(ch_src - 11)); *((uint32_t*)(ch_dst - 4)) = *((uint32_t*)(ch_src - 4)); break;
		case 12: *((uint64_t*)(ch_dst - 12)) = *((uint64_t*)(ch_src - 12)); *((uint32_t*)(ch_dst - 4)) = *((uint32_t*)(ch_src - 4)); break;
		case 13: *((uint64_t*)(ch_dst - 13)) = *((uint64_t*)(ch_src - 13)); *((uint64_t*)(ch_dst - 8)) = *((uint64_t*)(ch_src - 8)); break;
		case 14: *((uint64_t*)(ch_dst - 14)) = *((uint64_t*)(ch_src - 14)); *((uint64_t*)(ch_dst - 8)) = *((uint64_t*)(ch_src - 8)); break;
		case 15: *((uint64_t*)(ch_dst - 15)) = *((uint64_t*)(ch_src - 15)); *((uint64_t*)(ch_dst - 8)) = *((uint64_t*)(ch_src - 8)); break;
	}
}

raze_unmangled raze_never_inline raze_declare_const_function void* raze_stdcall __raze_memcpy(void* raze_restrict dst,
	const void* raze_restrict src, sizetype bytes) noexcept
{
	constexpr auto erms_threshold = 0x1000;

	u8* ch_dst = (u8*)dst;
	const u8* ch_src = (const u8*)src;

	if (bytes >= 16) {
		if (bytes >= 32) {
			if (bytes >= erms_threshold) {
				i32 all_features = arch::ProcessorFeatures::all();

				if (arch::ProcessorFeatures::has<arch::features::ERMS>(all_features)) {
					__movsb(ch_dst, ch_src, bytes);
					return bytes_pointer_offset(ch_dst, bytes);
				}
				else {
					if (arch::ProcessorFeatures::has<arch::features::AVX512F>(all_features)) {
						do {
							_mm512_storeu_si512(ch_dst, _mm512_loadu_si512(ch_src));
							_mm512_storeu_si512(ch_dst + 64, _mm512_loadu_si512(ch_src + 64));
							_mm512_storeu_si512(ch_dst + 128, _mm512_loadu_si512(ch_src + 128));
							_mm512_storeu_si512(ch_dst + 192, _mm512_loadu_si512(ch_src + 192));
							ch_dst += 256;
							ch_src += 256;
							bytes -= 256;
						} while (bytes >= 256);

						goto avx512_process_tail_from_0_to_256;
					}
					else if (arch::ProcessorFeatures::has<arch::features::AVX2>(all_features)) {
						do {
							_mm256_storeu_si256(reinterpret_cast<__m256i*>(ch_dst), _mm256_loadu_si256(reinterpret_cast<const __m256i*>(ch_src)));
							_mm256_storeu_si256(reinterpret_cast<__m256i*>(ch_dst) + 1, _mm256_loadu_si256(reinterpret_cast<const __m256i*>(ch_src) + 1));
							_mm256_storeu_si256(reinterpret_cast<__m256i*>(ch_dst) + 2, _mm256_loadu_si256(reinterpret_cast<const __m256i*>(ch_src) + 2));
							_mm256_storeu_si256(reinterpret_cast<__m256i*>(ch_dst) + 3, _mm256_loadu_si256(reinterpret_cast<const __m256i*>(ch_src) + 3));
							ch_dst += 128;
							ch_src += 128;
							bytes -= 128;
						} while (bytes >= 128);

						goto avx_process_tail_from_0_to_128;
					}
					else {
						do {
							_mm_storeu_si128(reinterpret_cast<__m128i*>(ch_dst), _mm_loadu_si128(reinterpret_cast<const __m128i*>(ch_src)));
							_mm_storeu_si128(reinterpret_cast<__m128i*>(ch_dst) + 1, _mm_loadu_si128(reinterpret_cast<const __m128i*>(ch_src) + 1));
							_mm_storeu_si128(reinterpret_cast<__m128i*>(ch_dst) + 2, _mm_loadu_si128(reinterpret_cast<const __m128i*>(ch_src) + 2));
							_mm_storeu_si128(reinterpret_cast<__m128i*>(ch_dst) + 3, _mm_loadu_si128(reinterpret_cast<const __m128i*>(ch_src) + 3));
							ch_dst += 64;
							ch_src += 64;
							bytes -= 64;
						} while (bytes >= 64);

						goto sse2_process_tail_from_0_to_64;
					}

					memcpy_scalar_impl(ch_dst, ch_src, bytes);
					return bytes_pointer_offset(ch_dst, bytes);
				}
			}
			else {
				i32 all_features = arch::ProcessorFeatures::all();

				if (arch::ProcessorFeatures::has<arch::features::AVX512F>(all_features)) {
					while (bytes >= 256) {
						_mm512_storeu_si512(ch_dst, _mm512_loadu_si512(ch_src));
						_mm512_storeu_si512(ch_dst + 64, _mm512_loadu_si512(ch_src + 64));
						_mm512_storeu_si512(ch_dst + 128, _mm512_loadu_si512(ch_src + 128));
						_mm512_storeu_si512(ch_dst + 192, _mm512_loadu_si512(ch_src + 192));
						ch_dst += 256;
						ch_src += 256;
						bytes -= 256;
					}

					goto avx512_process_tail_from_0_to_256;
				}
				else if (arch::ProcessorFeatures::has<arch::features::AVX>(all_features)) {
					while (bytes >= 128) {
						_mm256_storeu_si256(reinterpret_cast<__m256i*>(ch_dst), _mm256_loadu_si256(reinterpret_cast<const __m256i*>(ch_src)));
						_mm256_storeu_si256(reinterpret_cast<__m256i*>(ch_dst) + 1, _mm256_loadu_si256(reinterpret_cast<const __m256i*>(ch_src) + 1));
						_mm256_storeu_si256(reinterpret_cast<__m256i*>(ch_dst) + 2, _mm256_loadu_si256(reinterpret_cast<const __m256i*>(ch_src) + 2));
						_mm256_storeu_si256(reinterpret_cast<__m256i*>(ch_dst) + 3, _mm256_loadu_si256(reinterpret_cast<const __m256i*>(ch_src) + 3));
						ch_dst += 128;
						ch_src += 128;
						bytes -= 128;
					}

					goto avx_process_tail_from_0_to_128;
				}
				else {
					while (bytes >= 64) {
						_mm_storeu_si128(reinterpret_cast<__m128i*>(ch_dst), _mm_loadu_si128(reinterpret_cast<const __m128i*>(ch_src)));
						_mm_storeu_si128(reinterpret_cast<__m128i*>(ch_dst) + 1, _mm_loadu_si128(reinterpret_cast<const __m128i*>(ch_src) + 1));
						_mm_storeu_si128(reinterpret_cast<__m128i*>(ch_dst) + 2, _mm_loadu_si128(reinterpret_cast<const __m128i*>(ch_src) + 2));
						_mm_storeu_si128(reinterpret_cast<__m128i*>(ch_dst) + 3, _mm_loadu_si128(reinterpret_cast<const __m128i*>(ch_src) + 3));
						ch_dst += 64;
						ch_src += 64;
						bytes -= 64;
					}

					goto sse2_process_tail_from_0_to_64;
				}
			}

			if (arch::ProcessorFeatures::AVX()) memcpy_small_avx(dst, src, bytes);
			return bytes_pointer_offset(ch_dst, bytes);
		}

		memcpy_small_sse2(dst, src, bytes);
		return bytes_pointer_offset(ch_dst, bytes);
	}
	else {
		memcpy_small(dst, src, bytes);
	}

avx512_process_tail_from_0_to_256:
	if (bytes >= 128) {
		_mm512_storeu_si512(ch_dst, _mm512_loadu_si512(ch_src));
		_mm512_storeu_si512(ch_dst + 64, _mm512_loadu_si512(ch_src + 64));
		ch_dst += 128;
		ch_src += 128;
		bytes -= 128;
	}

	if (bytes >= 64) {
		_mm512_storeu_si512(ch_dst, _mm512_loadu_si512(ch_src));
		ch_dst += 64;
		ch_src += 64;
		bytes -= 64;
	}

	memcpy_small_avx512f(ch_dst, ch_src, bytes);
	return bytes_pointer_offset(ch_dst, bytes);

avx_process_tail_from_0_to_128:
	if (bytes >= 64) {
		_mm256_storeu_si256(reinterpret_cast<__m256i*>(ch_dst), _mm256_loadu_si256(reinterpret_cast<const __m256i*>(ch_src)));
		_mm256_storeu_si256(reinterpret_cast<__m256i*>(ch_dst) + 1, _mm256_loadu_si256(reinterpret_cast<const __m256i*>(ch_src) + 1));
		ch_dst += 64;
		ch_src += 64;
		bytes -= 64;
	}


	memcpy_small_avx(ch_dst, ch_src, bytes);
	return bytes_pointer_offset(ch_dst, bytes);

sse2_process_tail_from_0_to_64:
	if (bytes >= 32) {
		_mm_storeu_si128(reinterpret_cast<__m128i*>(ch_dst), _mm_loadu_si128(reinterpret_cast<const __m128i*>(ch_src)));
		_mm_storeu_si128(reinterpret_cast<__m128i*>(ch_dst) + 1, _mm_loadu_si128(reinterpret_cast<const __m128i*>(ch_src) + 1));
		ch_dst += 32;
		ch_src += 32;
		bytes -= 32;
	}

	memcpy_small_sse2(ch_dst, ch_src, bytes);
	return bytes_pointer_offset(ch_dst, bytes);

}

#else

raze_unmangled raze_never_inline void* raze_stdcall __raze_memcpy(void* raze_restrict dst,
	const void* raze_restrict src, sizetype bytes) noexcept
{
	memcpy(dst, src, bytes);
	return (u8*)dst + bytes;
}

#endif

__RAZE_ALGORITHM_NAMESPACE_END