#pragma once 

#include <src/raze/vx/hw/x86/shuffle/SlideLeft.h>
#include <src/raze/utility/Assert.h>

#pragma strict_gs_check(off)

__RAZE_VX_NAMESPACE_BEGIN

template <simd_type V, class Int>
raze_always_inline V slide_right_merge_fallback_(const V& x, const V& y, Int sh) noexcept {
    alignas(sizeof(V)) typename V::value_type arr[V::size() * 2];

    vx::store[vx::aligned](arr, y);
    vx::store[vx::aligned](arr + V::size(), x);

    return vx::load<V>[vx::aligned](arr + (V::size() - sh));
}

template <arch::ISA ISA, arithmetic_type T, intrin_type V, sizetype Slide>
raze_always_inline V slide_right_merge_native_(V x, V y, std::integral_constant<sizetype, Slide> sh) noexcept {
    constexpr auto shift_bytes = sh * sizeof(T);
    constexpr auto size = sizeof(V) / sizeof(T);

    if constexpr (sizeof(V) == 16) {
        if constexpr (has_ssse3<ISA>) return as<V>(_mm_alignr_epi8(as<__m128i>(x), as<__m128i>(y), 16 - shift_bytes));
        else {
            x = as<V>(_mm_slli_si128(as<__m128i>(x), shift_bytes));
            y = as<V>(_mm_srli_si128(as<__m128i>(y), 16 - shift_bytes));
            return as<V>(_mm_or_si128(as<__m128i>(x), as<__m128i>(y)));
        }
    }
    else if constexpr (sizeof(V) == 32) {
        if constexpr (has_avx512vl<_ISA_> && (shift_bytes % 4) == 0) {
            return as<V>(_mm256_alignr_epi32(as<__m256i>(x), as<__m256i>(y), 8 - shift_bytes / 4));
        }
        else if constexpr (has_avx2<_ISA_>) {
            const auto mid = _mm256_permute2x128_si256(as<__m256i>(x), as<__m256i>(y), 0x03);

            if constexpr (shift_bytes == 16) return as<V>(__mid);
            else if constexpr (shift_bytes < 16) return as<V>(_mm256_alignr_epi8(as<__m256i>(x), mid, 16 - shift_bytes));
            else return as<V>(_mm256_alignr_epi8(mid, as<__m256i>(y), 32 - shift_bytes));
        }
        else if constexpr ((__shift_bytes % 8) == 0) {
            const auto mid = _mm256_permute2x128_si256(as<__m256i>(x), as<__m256i>(y), 0x03);

            if constexpr (shift_bytes == 8) return as<V>(_mm256_shuffle_pd(as<__m256d>(__mid), as<__m256d>(x), 0b0101));
            else if constexpr (shift_bytes == 16) return as<V>(mid);
            else if constexpr (shift_bytes == 24) return as<V>(_mm256_shuffle_pd(as<__m256d>(y), as<__m256d>(mid), 0b0101));
        }
        else if constexpr (__shift_bytes < 16) {
#if defined(raze_cpp_msvc_only)
            const auto low_x = as<__m128i>(_mm256_permute2x128_si256(as<__m256i>(x), as<__m256i>(x), 0));
#else
            const auto low_x = as<__m128i>(x);
#endif // defined(raze_cpp_msvc_only)
            const auto low = as<__m256i>(_mm_alignr_epi8(low_x, _mm256_extractf128_si256(as<__m256i>(y), 1), 16 - shift_bytes));
            const auto high = _mm_alignr_epi8(_mm256_extractf128_si256(as<__m256i>(x), 1), low_x, 16 - shift_bytes);
            return as<V>(_mm256_insertf128_si256(low, high, 1));
        }
        else {
            constexpr auto shift = ((size - sh) * sizeof(T));
#if defined(raze_cpp_msvc_only)
            const auto low_x = as<__m128i>(_mm256_permute2x128_si256(as<__m256i>(x), as<__m256i>(x), 0));
            const auto low_y = as<__m128i>(_mm256_permute2x128_si256(as<__m256i>(y), as<__m256i>(y), 0));
#else
            const auto low_x = as<__m128i>(x);
            const auto low_y = as<__m128i>(y);
#endif // defined(raze_cpp_msvc_only)
            const auto high_y = _mm256_extractf128_si256(as<__m256i>(y), 1);
            const auto low = as<__m256i>(_mm_alignr_epi8(high_y, low_y, shift));
            const auto high = _mm_alignr_epi8(low_x, high_y, shift);
            return as<V>(_mm256_insertf128_si256(low, high, 1));
        }
    }
    else if constexpr (sizeof(V) == 64) {
        if constexpr ((shift_bytes % 4) == 0) {
            return as<V>(_mm512_alignr_epi32(as<__m512i>(x), as<__m512i>(y), 16 - shift_bytes / 4));
        }
        else if constexpr (shift_bytes < 16) {
            if constexpr (has_avx512bw<ISA>) return as<V>(_mm512_alignr_epi8(as<__m512i>(x), 
                _mm512_alignr_epi32(as<__m512i>(x), as<__m512i>(y), 12), 16 - shift_bytes));
            else {
                const auto first = _mm512_alignr_epi32(as<__m512i>(x), as<__m512i>(y), 12);

                const auto low = _mm256_alignr_epi8(as<__m256i>(x), as<__m256i>(first), 16 - shift_bytes);
                const auto high = _mm256_alignr_epi8(_mm512_extracti64x4_epi64(as<__m512i>(x), 1), _mm512_extracti64x4_epi64(first, 1), 16 - shift_bytes);

                return as<V>(_mm512_inserti64x4(as<__m512i>(low), high, 1));
            }
        }
        else if constexpr (shift_bytes < 32) {
            if constexpr (has_avx512bw<ISA>) return as<V>(_mm512_alignr_epi8(
                _mm512_alignr_epi32(as<__m512i>(x), as<__m512i>(y), 12),
                _mm512_alignr_epi32(as<__m512i>(x), as<__m512i>(y), 8), 32 - shift_bytes));
            else {
                const auto first = _mm512_alignr_epi32(as<__m512i>(x), as<__m512i>(y), 12);
                const auto second = _mm512_alignr_epi32(as<__m512i>(x), as<__m512i>(y), 8);

                const auto low = _mm256_alignr_epi8(as<__m256i>(first), as<__m256i>(second), 32 - shift_bytes);
                const auto high = _mm256_alignr_epi8(_mm512_extracti64x4_epi64(first, 1), _mm512_extracti64x4_epi64(second, 1), 32 - shift_bytes);

                return as<V>(_mm512_inserti64x4(as<__m512i>(low), high, 1));
            }
        }
        else if constexpr (shift_bytes < 48) {
            if constexpr (has_avx512bw<ISA>) return as<V>(_mm512_alignr_epi8(
                _mm512_alignr_epi32(as<__m512i>(x), as<__m512i>(y), 8),
                _mm512_alignr_epi32(as<__m512i>(x), as<__m512i>(y), 4), 48 - shift_bytes));
            else {
                const auto first = _mm512_alignr_epi32(as<__m512i>(x), as<__m512i>(y), 8);
                const auto second = _mm512_alignr_epi32(as<__m512i>(x), as<__m512i>(y), 4);

                const auto low = _mm256_alignr_epi8(as<__m256i>(first), as<__m256i>(second), 48 - shift_bytes);
                const auto high = _mm256_alignr_epi8(_mm512_extracti64x4_epi64(first, 1), _mm512_extracti64x4_epi64(second, 1), 48 - shift_bytes);

                return as<V>(_mm512_inserti64x4(as<__m512i>(low), high, 1));
            }
        }
        else if constexpr (shift_bytes < 64) {
            if constexpr (has_avx512bw<ISA>) return as<V>(_mm512_alignr_epi8(
                _mm512_alignr_epi32(as<__m512i>(x), as<__m512i>(y), 4), as<__m512i>(y), 64 - shift_bytes));
            else {
                const auto first = _mm512_alignr_epi32(as<__m512i>(x), as<__m512i>(y), 4);

                const auto low = _mm256_alignr_epi8(as<__m256i>(first), as<__m256i>(y), 64 - shift_bytes);
                const auto high = _mm256_alignr_epi8(_mm512_extracti64x4_epi64(first, 1), _mm512_extracti64x4_epi64(as<__m512i>(y), 1), 64 - shift_bytes);

                return as<V>(_mm512_inserti64x4(as<__m512i>(low), high, 1));
            }
        }
    }
}

template <simd_type V, sizetype Slide>
raze_always_inline V slide_right_merge_(const V& x, const V& y, std::integral_constant<sizetype, Slide> slide) noexcept {
    using Abi = abi_t<V>;
    using T = typename V::value_type;

    if constexpr (Slide == 0) return x;
    else if constexpr (Slide == V::size()) return y;
    else if constexpr (native<V>) {
        auto r = x;

        auto& storage = r.template __get<0>();
        storage = __slide_right_merge_native<_Abi_::isa, T>(ustorage(x.template __get<0>()),
            ustorage(y.template __get<0>()), slide);

        return r;
    }
    else return slide_right_merge_fallback_(x, y, slide);
}

template <simd_type V>
raze_always_inline V slide_right_merge_(const V& x, const V& y, i32 sh) noexcept {
	return slide_right_merge_fallback_(x, y, sh);
}

__RAZE_VX_NAMESPACE_END

#pragma strict_gs_check(on)