#pragma once

#include <src/raze/vx/storage/Node.h>
#include <src/raze/vx/storage/Size.h>
#include <src/raze/vx/hw/x86/memory/Load.h>
#include <src/raze/vx/hw/x86/memory/Store.h>
#include <src/raze/vx/hw/x86/arithmetic/VerticalMax.h>
#include <src/raze/vx/hw/x86/arithmetic/VerticalMin.h>
#include <src/raze/math/SaturatingCast.h>
#include <utility>


__RAZE_VX_NAMESPACE_BEGIN

template <arch::ISA ISA, class From, class To, class V>
consteval auto deduce_convert_return_type_() {
    if constexpr (sizeof(V) < 16) {
        return To{};
    }
    else if constexpr (sizeof(From) == sizeof(To)) {
        return traits::deduce_simd_vector_type<To, sizeof(V) * 8>{};
    }
    else {
        constexpr sizetype total_bytes = sizeof(V) / sizeof(From) * sizeof(To);
        if constexpr (total_bytes <= 16) {
            return traits::deduce_simd_vector_type<To, 128>{};
        }
        else if constexpr (total_bytes <= 32) {
            if constexpr (has_avx<ISA>) {
                return traits::deduce_simd_vector_type<To, 256>{};
            }
            else {
                using V128 = traits::deduce_simd_vector_type<To, 128>;
                return simd_tuple_repeat<V128, 2>{};
            }
        }
        else if constexpr (total_bytes <= 64) {
            if constexpr (has_avx512f<ISA>) {
                return traits::deduce_simd_vector_type<To, 512>{};
            }
            else if constexpr (has_avx<ISA>) {
                using V256 = traits::deduce_simd_vector_type<To, 256>;
                return simd_tuple_repeat<V256, 2>{};
            }
            else {
                using V128 = traits::deduce_simd_vector_type<To, 128>;
                return simd_tuple_repeat<V128, 4>{};
            }
        }
        else {
            constexpr sizetype chunk_size = has_avx512f<ISA> ? 64 : (has_avx<ISA> ? 32 : 16);
            constexpr sizetype num_chunks = total_bytes / chunk_size;
            using V_ = traits::deduce_simd_vector_type<To, chunk_size * 8>;
            return simd_tuple_repeat<V_, num_chunks>{};
        }
    }
}

template <arch::ISA ISA, class From, class To, class V>
using deduce_convert_return_type = decltype(deduce_convert_return_type_<ISA, From, To, V>());

template <arch::ISA ISA, class Part>
raze_always_inline auto pack_or_tuple(Part p0, Part p1) noexcept {
    if constexpr (has_avx<ISA> && sizeof(Part) == 16) {
        if constexpr (std::same_as<Part, __m128>)  return _mm256_insertf128_ps(as<__m256>(p0), p1, 1);
        else if constexpr (std::same_as<Part, __m128d>) return _mm256_insertf128_pd(as<__m256d>(p0), p1, 1);
        else return _mm256_insertf128_si256(as<__m256i>(p0), p1, 1);
    }
    else if constexpr (has_avx512f<ISA> && sizeof(Part) == 32) {
        if constexpr (std::same_as<Part, __m256>)  return _mm512_insertf32x8(as<__m512>(p0), p1, 1);
        else if constexpr (std::same_as<Part, __m256d>) return _mm512_insertf64x4(as<__m512d>(p0), p1, 1);
        else return _mm512_inserti64x4(as<__m512i>(p0), p1, 1);
    }
    else {
        return simd_tuple_make(p0, p1);
    }
}

template <arch::ISA ISA, class Part>
raze_always_inline auto pack_or_tuple(Part p0, Part p1, Part p2, Part p3) noexcept {
    if constexpr (has_avx512f<ISA> && sizeof(Part) == 16) {
        if constexpr (std::same_as<Part, __m128>) {
            auto lo = _mm256_insertf128_ps(as<__m256>(p0), p1, 1);
            auto hi = _mm256_insertf128_ps(as<__m256>(p2), p3, 1);
            return _mm512_insertf32x8(as<__m512>(lo), hi, 1);
        }
        else if constexpr (std::same_as<Part, __m128d>) {
            auto lo = _mm256_insertf128_pd(as<__m256d>(p0), p1, 1);
            auto hi = _mm256_insertf128_pd(as<__m256d>(p2), p3, 1);
            return _mm512_insertf64x4(as<__m512d>(lo), hi, 1);
        }
        else {
            auto lo = _mm256_insertf128_si256(as<__m256i>(p0), p1, 1);
            auto hi = _mm256_insertf128_si256(as<__m256i>(p2), p3, 1);
            return _mm512_inserti64x4(as<__m512i>(lo), hi, 1);
        }
    }
    else if constexpr (has_avx<ISA> && sizeof(Part) == 16) {
        if constexpr (std::same_as<Part, __m128>) return simd_tuple_make(_mm256_insertf128_ps(as<__m256>(p0), p1, 1), _mm256_insertf128_ps(as<__m256>(p2), p3, 1));
        else if constexpr (std::same_as<Part, __m128d>) return simd_tuple_make(_mm256_insertf128_pd(as<__m256d>(p0), p1, 1), _mm256_insertf128_pd(as<__m256d>(p2), p3, 1));
        else return simd_tuple_make(_mm256_insertf128_si256(as<__m256i>(p0), p1, 1), _mm256_insertf128_si256(as<__m256i>(p2), p3, 1));
    }
    else return simd_tuple_make(p0, p1, p2, p3);
}

template <bool Saturated, class To, class From>
raze_always_inline To scalar_narrow_(From v) noexcept {
    if constexpr (Saturated) return math::saturating_cast<To>(v);
    else return static_cast<To>(v);
}

template <arch::ISA ISA, class Target, class To, sizetype N>
raze_always_inline Target load_target_from_buffer_(const To* buf) noexcept {
    if constexpr (is_simd_tuple<std::remove_cvref_t<Target>>::value) {
        return [] <sizetype... Is> (const To * b, std::index_sequence<Is...>) -> Target raze_always_inline_lambda {
            using FirstChunk = simd_tuple_element_t<0, Target>;
            return simd_tuple_make(load_<ISA, simd_tuple_element_t<Is, Target>>(b + Is * (sizeof(FirstChunk) / sizeof(To)))...);
        }(buf, std::make_index_sequence<simd_tuple_size<std::remove_cvref_t<Target>>::value>{});
    }
    else return load_<ISA, Target>(buf);
}

template <arch::ISA ISA, bool Saturated, class From, class To, class V>
raze_always_inline deduce_convert_return_type<ISA, From, To, V> scalar_fallback_convert_(V x) noexcept {
    using Ret = deduce_convert_return_type<ISA, From, To, V>;
    constexpr sizetype count = sizeof(V) / sizeof(From);
    constexpr sizetype out_count = (sizeof(Ret) > count * sizeof(To)) ? (sizeof(Ret) / sizeof(To)) : count;

    alignas(64) From arr[count];
    alignas(64) To out[out_count];

    store_(arr, x);

    for (sizetype i = 0; i < count; ++i) out[i] = scalar_narrow_<Saturated, To, From>(arr[i]);
    return load_target_from_buffer_<ISA, Ret, To, count>(out);
}

template <arch::ISA ISA, class V, class Recurse>
raze_always_inline auto split_and_recurse_(V x, Recurse&& recurse) noexcept {
    if constexpr (sizeof(V) == 32) {
        if constexpr (std::same_as<V, __m256d>)  return simd_tuple_cat(as_simd_tuple(recurse(as<__m128d>(x))), as_simd_tuple(recurse(_mm256_extractf128_pd(x, 1))));
        else if constexpr (std::same_as<V, __m256>) return simd_tuple_cat(as_simd_tuple(recurse(as<__m128>(x))), as_simd_tuple(recurse(_mm256_extractf128_ps(x, 1))));
        else return simd_tuple_cat(as_simd_tuple(recurse(as<__m128i>(x))), as_simd_tuple(recurse(_mm256_extractf128_si256(x, 1))));
    }
    else if constexpr (sizeof(V) == 64) {
        if constexpr (std::same_as<V, __m512d>) return simd_tuple_cat(as_simd_tuple(recurse(as<__m256d>(x))), as_simd_tuple(recurse(_mm512_extractf64x4_pd(x, 1))));
        else if constexpr (std::same_as<V, __m512>) return simd_tuple_cat(as_simd_tuple(recurse(as<__m256>(x))), as_simd_tuple(recurse(_mm512_extractf32x8_ps(x, 1))));
        else return simd_tuple_cat(as_simd_tuple(recurse(as<__m256i>(x))), as_simd_tuple(recurse(_mm512_extracti64x4_epi64(x, 1))));
    }
}

template <arch::ISA ISA, class From, class To, class V>
consteval bool should_split_() {
    using R = deduce_convert_return_type<ISA, From, To, V>;
    if constexpr (sizeof(R) > 128 && sizeof(V) >= 32) return (sizeof(V) == 32 && has_avx<ISA>) || (sizeof(V) == 64 && has_avx512f<ISA>);
    else if constexpr (sizeof(R) == 128 && sizeof(V) == 32 && has_avx<ISA> && !has_avx2<ISA> && !has_avx512f<ISA>) return true;
    else return false;
}

template <arch::ISA ISA, arithmetic_type From, arithmetic_type To, intrin_or_arithmetic_type V>
raze_always_inline deduce_convert_return_type<ISA, From, To, V>
convert_static_cast_semantics_(V x) noexcept;

template <arch::ISA ISA, arithmetic_type From, arithmetic_type To, intrin_or_arithmetic_type V>
raze_always_inline deduce_convert_return_type<ISA, From, To, V>
convert_saturated_(V x) noexcept;

template <arch::ISA ISA, bool Saturated, arithmetic_type From, arithmetic_type To, intrin_or_arithmetic_type V>
raze_always_inline deduce_convert_return_type<ISA, From, To, V> convert_(V x) noexcept {
    if constexpr (Saturated) return convert_saturated_<ISA, From, To>(x);
    else return convert_static_cast_semantics_<ISA, From, To>(x);
}

template <arch::ISA ISA, arithmetic_type From, arithmetic_type To, intrin_or_arithmetic_type V>
raze_always_inline deduce_convert_return_type<ISA, From, To, V>
convert_static_cast_semantics_(V x) noexcept {
    using R = deduce_convert_return_type<ISA, From, To, V>;

    if constexpr (std::same_as<To, From>) return x;
    else if constexpr (sizeof(V) < 16) return scalar_fallback_convert_<ISA, false, From, To>(x);
    else if constexpr (should_split_<ISA, From, To, V>()) {
        return split_and_recurse_<ISA>(x, [] (auto chunk) raze_always_inline_lambda {
            return convert_static_cast_semantics_<ISA, From, To>(chunk);
        });
    }
    else {
        constexpr bool i_to_i = std::is_integral_v<From> && std::is_integral_v<To>;
        constexpr bool f_to_i = std::is_floating_point_v<From> && std::is_integral_v<To>;
        constexpr bool i_to_f = std::is_integral_v<From> && std::is_floating_point_v<To>;
        constexpr bool f_to_f = std::is_floating_point_v<From> && std::is_floating_point_v<To>;
        constexpr bool frm_sgn = std::is_signed_v<From>;
        constexpr bool to_sgn = std::is_signed_v<To>;

        constexpr bool has_ymm_v = has_avx<ISA>;
        constexpr bool has_avx2_v = has_avx2<ISA>;
        constexpr bool has_zmm = has_avx512f<ISA>;
        constexpr bool has_bw = has_avx512bw<ISA>;
        constexpr bool has_dq = has_avx512dq<ISA>;
        constexpr bool has_vl = has_avx512vl<ISA>;
        constexpr bool has_bwvl = has_bw && has_vl;
        constexpr bool has_dqvl = has_dq && has_vl;
        constexpr bool has_s41 = has_sse41<ISA>;

        constexpr bool x_to_x = sizeof(V) <= 16 && sizeof(R) <= 16;
        constexpr bool x_to_y = sizeof(V) <= 16 && sizeof(R) == 32;
        constexpr bool x_to_z = sizeof(V) <= 16 && sizeof(R) == 64;
        constexpr bool y_to_x = sizeof(V) == 32 && sizeof(R) <= 16;
        constexpr bool y_to_y = sizeof(V) == 32 && sizeof(R) == 32;
        constexpr bool y_to_z = sizeof(V) == 32 && sizeof(R) == 64;
        constexpr bool z_to_x = sizeof(V) == 64 && sizeof(R) <= 16;
        constexpr bool z_to_y = sizeof(V) == 64 && sizeof(R) == 32;
        constexpr bool z_to_z = sizeof(V) == 64 && sizeof(R) == 64;

        constexpr bool i8_to_i16 = i_to_i && sizeof(From) == 1 && sizeof(To) == 2;
        constexpr bool i8_to_i32 = i_to_i && sizeof(From) == 1 && sizeof(To) == 4;
        constexpr bool i8_to_i64 = i_to_i && sizeof(From) == 1 && sizeof(To) == 8;
        constexpr bool i16_to_i8 = i_to_i && sizeof(From) == 2 && sizeof(To) == 1;
        constexpr bool i16_to_i32 = i_to_i && sizeof(From) == 2 && sizeof(To) == 4;
        constexpr bool i16_to_i64 = i_to_i && sizeof(From) == 2 && sizeof(To) == 8;
        constexpr bool i32_to_i8 = i_to_i && sizeof(From) == 4 && sizeof(To) == 1;
        constexpr bool i32_to_i16 = i_to_i && sizeof(From) == 4 && sizeof(To) == 2;
        constexpr bool i32_to_i64 = i_to_i && sizeof(From) == 4 && sizeof(To) == 8;
        constexpr bool i64_to_i8 = i_to_i && sizeof(From) == 8 && sizeof(To) == 1;
        constexpr bool i64_to_i16 = i_to_i && sizeof(From) == 8 && sizeof(To) == 2;
        constexpr bool i64_to_i32 = i_to_i && sizeof(From) == 8 && sizeof(To) == 4;

        if constexpr (i_to_i) {
            if constexpr (i8_to_i16) {
                if constexpr (x_to_y && has_avx2_v) {
                    if constexpr (frm_sgn) return _mm256_cvtepi8_epi16(x);
                    else return _mm256_cvtepu8_epi16(x);
                }
                else if constexpr (y_to_z && has_bw) {
                    if constexpr (frm_sgn) return _mm512_cvtepi8_epi16(x);
                    else return _mm512_cvtepu8_epi16(x);
                }
                else if constexpr (y_to_z && has_avx2_v) {
                    if constexpr (frm_sgn) return pack_or_tuple<ISA>(_mm256_cvtepi8_epi16(as<__m128i>(x)), _mm256_cvtepi8_epi16(_mm256_extracti128_si256(x, 1)));
                    else return pack_or_tuple<ISA>(_mm256_cvtepu8_epi16(as<__m128i>(x)), _mm256_cvtepu8_epi16(_mm256_extracti128_si256(x, 1)));
                }
                else if constexpr (sizeof(R) == 128 && has_bw) {
                    auto x0 = as<__m128i>(x);
                    auto x1 = _mm512_extracti32x4_epi32(x, 1);
                    auto x2 = _mm512_extracti32x4_epi32(x, 2);
                    auto x3 = _mm512_extracti32x4_epi32(x, 3);

                    if constexpr (frm_sgn) 
                        return simd_tuple_make(_mm512_cvtepi8_epi16(_mm256_inserti128_si256(as<__m256i>(x0), x1, 1)),
                            _mm512_cvtepi8_epi16(_mm256_inserti128_si256(as<__m256i>(x2), x3, 1)));
                    else 
                        return simd_tuple_make(_mm512_cvtepu8_epi16(_mm256_inserti128_si256(as<__m256i>(x0), x1, 1)),
                            _mm512_cvtepu8_epi16(_mm256_inserti128_si256(as<__m256i>(x2), x3, 1)));
                }
                else return scalar_fallback_convert_<ISA, false, From, To>(x);
            }
            else if constexpr (i8_to_i32) {
                if constexpr (x_to_z) {
                    if constexpr (has_zmm) {
                        if constexpr (frm_sgn) return _mm512_cvtepi8_epi32(x);
                        else return _mm512_cvtepu8_epi32(x);
                    }
                    else if constexpr (has_avx2_v) {
                        if constexpr (frm_sgn) return simd_tuple_make(_mm256_cvtepi8_epi32(x), _mm256_cvtepi8_epi32(_mm_srli_si128(x, 8)));
                        else return simd_tuple_make(_mm256_cvtepu8_epi32(x), _mm256_cvtepu8_epi32(_mm_srli_si128(x, 8)));
                    }
                    else if constexpr (has_avx<ISA> && has_s41) {
                        __m128i e0, e1, e2, e3;

                        if constexpr (frm_sgn) {
                            e0 = _mm_cvtepi8_epi32(x);
                            e1 = _mm_cvtepi8_epi32(_mm_srli_si128(x, 4));
                            e2 = _mm_cvtepi8_epi32(_mm_srli_si128(x, 8));
                            e3 = _mm_cvtepi8_epi32(_mm_srli_si128(x, 12));
                        }
                        else {
                            e0 = _mm_cvtepu8_epi32(x);
                            e1 = _mm_cvtepu8_epi32(_mm_srli_si128(x, 4));
                            e2 = _mm_cvtepu8_epi32(_mm_srli_si128(x, 8));
                            e3 = _mm_cvtepu8_epi32(_mm_srli_si128(x, 12));
                        }

                        return simd_tuple_make(_mm256_insertf128_si256(as<__m256i>(e0), e1, 1), _mm256_insertf128_si256(as<__m256i>(e2), e3, 1));
                    }
                    else if constexpr (has_s41) {
                        if constexpr (frm_sgn) 
                            return simd_tuple_make(_mm_cvtepi8_epi32(x), _mm_cvtepi8_epi32(_mm_srli_si128(x, 4)),
                                _mm_cvtepi8_epi32(_mm_srli_si128(x, 8)), _mm_cvtepi8_epi32(_mm_srli_si128(x, 12)));
                        else 
                            return simd_tuple_make(_mm_cvtepu8_epi32(x), _mm_cvtepu8_epi32(_mm_srli_si128(x, 4)),
                                _mm_cvtepu8_epi32(_mm_srli_si128(x, 8)), _mm_cvtepu8_epi32(_mm_srli_si128(x, 12)));
                    }
                    else {
                        __m128i lo16, hi16;

                        if constexpr (frm_sgn) {
                            lo16 = _mm_srai_epi16(_mm_unpacklo_epi8(x, x), 8);
                            hi16 = _mm_srai_epi16(_mm_unpackhi_epi8(x, x), 8);
                        }
                        else {
                            lo16 = _mm_unpacklo_epi8(x, _mm_setzero_si128());
                            hi16 = _mm_unpackhi_epi8(x, _mm_setzero_si128());
                        }

                        if constexpr (frm_sgn) 
                            return simd_tuple_make(_mm_srai_epi32(_mm_unpacklo_epi16(lo16, lo16), 16),
                                _mm_srai_epi32(_mm_unpackhi_epi16(lo16, lo16), 16),
                                _mm_srai_epi32(_mm_unpacklo_epi16(hi16, hi16), 16),
                                _mm_srai_epi32(_mm_unpackhi_epi16(hi16, hi16), 16));
                        else
                            return simd_tuple_make(_mm_unpacklo_epi16(lo16, _mm_setzero_si128()), _mm_unpackhi_epi16(lo16, _mm_setzero_si128()),
                                _mm_unpacklo_epi16(hi16, _mm_setzero_si128()), _mm_unpackhi_epi16(hi16, _mm_setzero_si128()));
                    }
                }
                else if constexpr (sizeof(R) == 128 && has_zmm) {
                    if constexpr (frm_sgn) 
                        return simd_tuple_make(_mm512_cvtepi8_epi32(as<__m128i>(x)), _mm512_cvtepi8_epi32(_mm256_extracti128_si256(x, 1)));
                    else 
                        return simd_tuple_make(_mm512_cvtepu8_epi32(as<__m128i>(x)),  _mm512_cvtepu8_epi32(_mm256_extracti128_si256(x, 1)));
                }
                else if constexpr (sizeof(R) == 128 && has_avx2_v) {
                    if constexpr (frm_sgn) 
                        return simd_tuple_make(_mm256_cvtepi8_epi32(as<__m128i>(x)), _mm256_cvtepi8_epi32(_mm_srli_si128(as<__m128i>(x), 8)),
                            _mm256_cvtepi8_epi32(_mm256_extracti128_si256(x, 1)), _mm256_cvtepi8_epi32(_mm_srli_si128(_mm256_extracti128_si256(x, 1), 8)));
                    else 
                        return simd_tuple_make(_mm256_cvtepu8_epi32(as<__m128i>(x)), _mm256_cvtepu8_epi32(_mm_srli_si128(as<__m128i>(x), 8)),
                            _mm256_cvtepu8_epi32(_mm256_extracti128_si256(x, 1)), _mm256_cvtepu8_epi32(_mm_srli_si128(_mm256_extracti128_si256(x, 1), 8)));
                }
                else return scalar_fallback_convert_<ISA, false, From, To>(x);
            }
            else if constexpr (i8_to_i64) {
                if constexpr (sizeof(R) == 128 && has_zmm) {
                    __m512i e32;

                    if constexpr (frm_sgn) e32 = _mm512_cvtepi8_epi32(x);
                    else e32 = _mm512_cvtepu8_epi32(x);

                    auto lo32 = as<__m256i>(e32);
                    auto hi32 = _mm512_extracti64x4_epi64(e32, 1);

                    if constexpr (frm_sgn) return simd_tuple_make(_mm512_cvtepi32_epi64(lo32), _mm512_cvtepi32_epi64(hi32));
                    else return simd_tuple_make(_mm512_cvtepu32_epi64(lo32), _mm512_cvtepu32_epi64(hi32));
                }
                else if constexpr (sizeof(R) == 128 && has_avx2_v) {
                    __m256i lo32, hi32;

                    if constexpr (frm_sgn) {
                        lo32 = _mm256_cvtepi8_epi32(x);
                        hi32 = _mm256_cvtepi8_epi32(_mm_srli_si128(x, 8));
                    }
                    else {
                        lo32 = _mm256_cvtepu8_epi32(x);
                        hi32 = _mm256_cvtepu8_epi32(_mm_srli_si128(x, 8));
                    }

                    if constexpr (frm_sgn) 
                        return simd_tuple_make(_mm256_cvtepi32_epi64(as<__m128i>(lo32)), _mm256_cvtepi32_epi64(_mm256_extracti128_si256(lo32, 1)), 
                            _mm256_cvtepi32_epi64(as<__m128i>(hi32)), _mm256_cvtepi32_epi64(_mm256_extracti128_si256(hi32, 1)));
                    else 
                        return simd_tuple_make(_mm256_cvtepu32_epi64(as<__m128i>(lo32)), _mm256_cvtepu32_epi64(_mm256_extracti128_si256(lo32, 1)),
                            _mm256_cvtepu32_epi64(as<__m128i>(hi32)), _mm256_cvtepu32_epi64(_mm256_extracti128_si256(hi32, 1)));
                }
                else return scalar_fallback_convert_<ISA, false, From, To>(x);
            }
            else if constexpr (i16_to_i8) {
                if constexpr (x_to_x) {
                    if constexpr (has_bwvl) return _mm_cvtepi16_epi8(x);
                    else if constexpr (has_ssse3<ISA>) return _mm_shuffle_epi8(x, _mm_setr_epi8(0, 2, 4, 6, 8, 10, 12, 14, -1, -1, -1, -1, -1, -1, -1, -1));
                    else return _mm_packs_epi16(_mm_srai_epi16(_mm_slli_epi16(x, 8), 8), _mm_setzero_si128());
                }
                else if constexpr (y_to_x) {
                    if constexpr (has_bwvl) return _mm256_cvtepi16_epi8(x);
                    else if constexpr (has_avx2_v) return as<__m128i>(_mm256_permute4x64_epi64(_mm256_shuffle_epi8(x, 
                        _mm256_setr_epi8(0, 2, 4, 6, 8, 10, 12, 14, -1, -1, -1, -1, -1, -1, -1, -1,
                        0, 2, 4, 6, 8, 10, 12, 14, -1, -1, -1, -1, -1, -1, -1, -1)), 0xF8));
                    else return scalar_fallback_convert_<ISA, false, From, To>(x);
                }
                else if constexpr (z_to_y && has_bw) return _mm512_cvtepi16_epi8(x);
                else return scalar_fallback_convert_<ISA, false, From, To>(x);
            }
            else if constexpr (i16_to_i32) {
                if constexpr (x_to_y && has_avx2_v) {
                    if constexpr (frm_sgn) return _mm256_cvtepi16_epi32(x);
                    else return _mm256_cvtepu16_epi32(x);
                }
                else if constexpr (y_to_z) {
                    if constexpr (has_zmm) {
                        if constexpr (frm_sgn) return _mm512_cvtepi16_epi32(x);
                        else return _mm512_cvtepu16_epi32(x);
                    }
                    else if constexpr (has_avx2_v) {
                        if constexpr (frm_sgn) return simd_tuple_make(_mm256_cvtepi16_epi32(as<__m128i>(x)), _mm256_cvtepi16_epi32(_mm256_extracti128_si256(x, 1)));
                        else return simd_tuple_make(_mm256_cvtepu16_epi32(as<__m128i>(x)), _mm256_cvtepu16_epi32(_mm256_extracti128_si256(x, 1)));
                    }
                    else return scalar_fallback_convert_<ISA, false, From, To>(x);
                }
                else if constexpr (x_to_y && has_avx<ISA> && has_s41) {
                    __m128i e0, e1;

                    if constexpr (frm_sgn) {
                        e0 = _mm_cvtepi16_epi32(x);
                        e1 = _mm_cvtepi16_epi32(_mm_srli_si128(x, 8));
                    }
                    else {
                        e0 = _mm_cvtepu16_epi32(x);
                        e1 = _mm_cvtepu16_epi32(_mm_srli_si128(x, 8));
                    }

                    return _mm256_insertf128_si256(as<__m256i>(e0), e1, 1);
                }
                else if constexpr (x_to_y && has_s41) {
                    if constexpr (frm_sgn) return simd_tuple_make(_mm_cvtepi16_epi32(x), _mm_cvtepi16_epi32(_mm_srli_si128(x, 8)));
                    else return simd_tuple_make(_mm_cvtepu16_epi32(x), _mm_cvtepu16_epi32(_mm_srli_si128(x, 8)));
                }
                else if constexpr (x_to_y) {
                    if constexpr (frm_sgn) return simd_tuple_make(_mm_srai_epi32(_mm_unpacklo_epi16(x, x), 16), _mm_srai_epi32(_mm_unpackhi_epi16(x, x), 16));
                    else return simd_tuple_make(_mm_unpacklo_epi16(x, _mm_setzero_si128()), _mm_unpackhi_epi16(x, _mm_setzero_si128()));
                }
                else if constexpr (sizeof(R) == 128 && has_zmm) {
                    if constexpr (frm_sgn) return simd_tuple_make(_mm512_cvtepi16_epi32(as<__m256i>(x)), _mm512_cvtepi16_epi32(_mm512_extracti64x4_epi64(x, 1)));
                    else return simd_tuple_make(_mm512_cvtepu16_epi32(as<__m256i>(x)), _mm512_cvtepu16_epi32(_mm512_extracti64x4_epi64(x, 1)));
                }
                else return scalar_fallback_convert_<ISA, false, From, To>(x);
            }
            else if constexpr (i16_to_i64) {
                if constexpr (x_to_z) {
                    if constexpr (has_zmm) {
                        if constexpr (frm_sgn) return _mm512_cvtepi16_epi64(x);
                        else return _mm512_cvtepu16_epi64(x);
                    }
                    else if constexpr (has_avx2_v) {
                        __m256i w32;

                        if constexpr (frm_sgn) w32 = _mm256_cvtepi16_epi32(x);
                        else w32 = _mm256_cvtepu16_epi32(x);
                        
                        if constexpr (frm_sgn) return simd_tuple_make(_mm256_cvtepi32_epi64(as<__m128i>(w32)), _mm256_cvtepi32_epi64(_mm256_extracti128_si256(w32, 1)));
                        else return simd_tuple_make(_mm256_cvtepu32_epi64(as<__m128i>(w32)), _mm256_cvtepu32_epi64(_mm256_extracti128_si256(w32, 1)));
                    }
                    else if constexpr (has_avx<ISA> && has_s41) {
                        __m128i w0, w1;
                        __m128i q0, q1, q2, q3;

                        if constexpr (frm_sgn) {
                            w0 = _mm_cvtepi16_epi32(x);
                            w1 = _mm_cvtepi16_epi32(_mm_srli_si128(x, 8));
                        }
                        else {
                            w0 = _mm_cvtepu16_epi32(x);
                            w1 = _mm_cvtepu16_epi32(_mm_srli_si128(x, 8));
                        }

                        if constexpr (frm_sgn) {
                            q0 = _mm_cvtepi32_epi64(w0);
                            q1 = _mm_cvtepi32_epi64(_mm_srli_si128(w0, 8));
                            q2 = _mm_cvtepi32_epi64(w1);
                            q3 = _mm_cvtepi32_epi64(_mm_srli_si128(w1, 8));
                        }
                        else {
                            q0 = _mm_cvtepu32_epi64(w0);
                            q1 = _mm_cvtepu32_epi64(_mm_srli_si128(w0, 8));
                            q2 = _mm_cvtepu32_epi64(w1);
                            q3 = _mm_cvtepu32_epi64(_mm_srli_si128(w1, 8));
                        }

                        return simd_tuple_make(_mm256_insertf128_si256(as<__m256i>(q0), q1, 1),
                            _mm256_insertf128_si256(as<__m256i>(q2), q3, 1));
                    }
                    else if constexpr (has_s41) {
                        __m128i w0, w1;

                        if constexpr (frm_sgn) {
                            w0 = _mm_cvtepi16_epi32(x);
                            w1 = _mm_cvtepi16_epi32(_mm_srli_si128(x, 8));
                        }
                        else {
                            w0 = _mm_cvtepu16_epi32(x);
                            w1 = _mm_cvtepu16_epi32(_mm_srli_si128(x, 8));
                        }

                        if constexpr (frm_sgn) 
                            return simd_tuple_make(_mm_cvtepi32_epi64(w0), _mm_cvtepi32_epi64(_mm_srli_si128(w0, 8)),
                                _mm_cvtepi32_epi64(w1), _mm_cvtepi32_epi64(_mm_srli_si128(w1, 8)));
                        else 
                            return simd_tuple_make(_mm_cvtepu32_epi64(w0), _mm_cvtepu32_epi64(_mm_srli_si128(w0, 8)),
                                _mm_cvtepu32_epi64(w1), _mm_cvtepu32_epi64(_mm_srli_si128(w1, 8)));
                    }
                    else {
                        if constexpr (frm_sgn) {
                            __m128i w0 = _mm_srai_epi32(_mm_unpacklo_epi16(x, x), 16);
                            __m128i w1 = _mm_srai_epi32(_mm_unpackhi_epi16(x, x), 16);
                            return simd_tuple_make(_mm_unpacklo_epi32(w0, _mm_srai_epi32(w0, 31)), _mm_unpackhi_epi32(w0, _mm_srai_epi32(w0, 31)),
                                _mm_unpacklo_epi32(w1, _mm_srai_epi32(w1, 31)), _mm_unpackhi_epi32(w1, _mm_srai_epi32(w1, 31)));
                        }
                        else {
                            __m128i w0 = _mm_unpacklo_epi16(x, _mm_setzero_si128());
                            __m128i w1 = _mm_unpackhi_epi16(x, _mm_setzero_si128());
                            return simd_tuple_make(_mm_unpacklo_epi32(w0, _mm_setzero_si128()), _mm_unpackhi_epi32(w0, _mm_setzero_si128()),
                                _mm_unpacklo_epi32(w1, _mm_setzero_si128()), _mm_unpackhi_epi32(w1, _mm_setzero_si128()));
                        }
                    }
                }
                else if constexpr (sizeof(R) == 128 && has_zmm) {
                    if constexpr (frm_sgn) return simd_tuple_make(_mm512_cvtepi16_epi64(as<__m128i>(x)), _mm512_cvtepi16_epi64(_mm256_extracti128_si256(x, 1)));
                    else return simd_tuple_make(_mm512_cvtepu16_epi64(as<__m128i>(x)), _mm512_cvtepu16_epi64(_mm256_extracti128_si256(x, 1)));
                }
                else if constexpr (sizeof(R) == 128 && has_avx2_v) {
                    __m256i w_lo, w_hi;
                    if constexpr (frm_sgn) {
                        w_lo = _mm256_cvtepi16_epi32(as<__m128i>(x));
                        w_hi = _mm256_cvtepi16_epi32(_mm256_extracti128_si256(x, 1));
                    }
                    else {
                        w_lo = _mm256_cvtepu16_epi32(as<__m128i>(x));
                        w_hi = _mm256_cvtepu16_epi32(_mm256_extracti128_si256(x, 1));
                    }

                    if constexpr (frm_sgn) 
                        return simd_tuple_make(_mm256_cvtepi32_epi64(as<__m128i>(w_lo)), _mm256_cvtepi32_epi64(_mm256_extracti128_si256(w_lo, 1)),
                            _mm256_cvtepi32_epi64(as<__m128i>(w_hi)), _mm256_cvtepi32_epi64(_mm256_extracti128_si256(w_hi, 1)));
                    else 
                        return simd_tuple_make(_mm256_cvtepu32_epi64(as<__m128i>(w_lo)), _mm256_cvtepu32_epi64(_mm256_extracti128_si256(w_lo, 1)),
                            _mm256_cvtepu32_epi64(as<__m128i>(w_hi)), _mm256_cvtepu32_epi64(_mm256_extracti128_si256(w_hi, 1)));
                }
                else if constexpr (sizeof(R) == 128 && has_avx<ISA> && has_s41) {
                    __m128i w0, w1, w2, w3;
                    __m128i q0, q1, q2, q3, q4, q5, q6, q7;

                    if constexpr (frm_sgn) {
                        w0 = _mm_cvtepi16_epi32(x);
                        w1 = _mm_cvtepi16_epi32(_mm_srli_si128(x, 4));
                        w2 = _mm_cvtepi16_epi32(_mm_srli_si128(x, 8));
                        w3 = _mm_cvtepi16_epi32(_mm_srli_si128(x, 12));
                    }
                    else {
                        w0 = _mm_cvtepu16_epi32(x);
                        w1 = _mm_cvtepu16_epi32(_mm_srli_si128(x, 4));
                        w2 = _mm_cvtepu16_epi32(_mm_srli_si128(x, 8));
                        w3 = _mm_cvtepu16_epi32(_mm_srli_si128(x, 12));
                    }

                    if constexpr (frm_sgn) {
                        q0 = _mm_cvtepi32_epi64(w0);
                        q1 = _mm_cvtepi32_epi64(_mm_srli_si128(w0, 8));
                        q2 = _mm_cvtepi32_epi64(w1);
                        q3 = _mm_cvtepi32_epi64(_mm_srli_si128(w1, 8));
                        q4 = _mm_cvtepi32_epi64(w2);
                        q5 = _mm_cvtepi32_epi64(_mm_srli_si128(w2, 8));
                        q6 = _mm_cvtepi32_epi64(w3);
                        q7 = _mm_cvtepi32_epi64(_mm_srli_si128(w3, 8));
                    }
                    else {
                        q0 = _mm_cvtepu32_epi64(w0);
                        q1 = _mm_cvtepu32_epi64(_mm_srli_si128(w0, 8));
                        q2 = _mm_cvtepu32_epi64(w1);
                        q3 = _mm_cvtepu32_epi64(_mm_srli_si128(w1, 8));
                        q4 = _mm_cvtepu32_epi64(w2);
                        q5 = _mm_cvtepu32_epi64(_mm_srli_si128(w2, 8));
                        q6 = _mm_cvtepu32_epi64(w3);
                        q7 = _mm_cvtepu32_epi64(_mm_srli_si128(w3, 8));
                    }

                    return simd_tuple_make(_mm256_insertf128_si256(as<__m256i>(q0), q1, 1), _mm256_insertf128_si256(as<__m256i>(q2), q3, 1),
                        _mm256_insertf128_si256(as<__m256i>(q4), q5, 1), _mm256_insertf128_si256(as<__m256i>(q6), q7, 1));
                }
                else if constexpr (sizeof(R) == 128 && has_s41) {
                    __m128i w0, w1, w2, w3;

                    if constexpr (frm_sgn) {
                        w0 = _mm_cvtepi16_epi32(x);
                        w1 = _mm_cvtepi16_epi32(_mm_srli_si128(x, 4));
                        w2 = _mm_cvtepi16_epi32(_mm_srli_si128(x, 8));
                        w3 = _mm_cvtepi16_epi32(_mm_srli_si128(x, 12));
                    }
                    else {
                        w0 = _mm_cvtepu16_epi32(x);
                        w1 = _mm_cvtepu16_epi32(_mm_srli_si128(x, 4));
                        w2 = _mm_cvtepu16_epi32(_mm_srli_si128(x, 8));
                        w3 = _mm_cvtepu16_epi32(_mm_srli_si128(x, 12));
                    }

                    if constexpr (frm_sgn) 
                        return simd_tuple_make(_mm_cvtepi32_epi64(w0), _mm_cvtepi32_epi64(_mm_srli_si128(w0, 8)),
                            _mm_cvtepi32_epi64(w1), _mm_cvtepi32_epi64(_mm_srli_si128(w1, 8)),
                            _mm_cvtepi32_epi64(w2), _mm_cvtepi32_epi64(_mm_srli_si128(w2, 8)),
                            _mm_cvtepi32_epi64(w3), _mm_cvtepi32_epi64(_mm_srli_si128(w3, 8)));
                    else 
                        return simd_tuple_make(_mm_cvtepu32_epi64(w0), _mm_cvtepu32_epi64(_mm_srli_si128(w0, 8)),
                            _mm_cvtepu32_epi64(w1), _mm_cvtepu32_epi64(_mm_srli_si128(w1, 8)),
                            _mm_cvtepu32_epi64(w2), _mm_cvtepu32_epi64(_mm_srli_si128(w2, 8)),
                            _mm_cvtepu32_epi64(w3), _mm_cvtepu32_epi64(_mm_srli_si128(w3, 8)));
                }
                else if constexpr (sizeof(R) == 128) {
                    if constexpr (frm_sgn) {
                        __m128i w0 = _mm_srai_epi32(_mm_unpacklo_epi16(x, x), 16);
                        __m128i w1 = _mm_srai_epi32(_mm_unpackhi_epi16(x, x), 16);

                        return simd_tuple_make(_mm_unpacklo_epi32(w0, _mm_srai_epi32(w0, 31)), _mm_unpackhi_epi32(w0, _mm_srai_epi32(w0, 31)),
                            _mm_unpacklo_epi32(w1, _mm_srai_epi32(w1, 31)), _mm_unpackhi_epi32(w1, _mm_srai_epi32(w1, 31)));
                    }
                    else {
                        __m128i w0 = _mm_unpacklo_epi16(x, _mm_setzero_si128());
                        __m128i w1 = _mm_unpackhi_epi16(x, _mm_setzero_si128());
                        return simd_tuple_make(_mm_unpacklo_epi32(w0, _mm_setzero_si128()), _mm_unpackhi_epi32(w0, _mm_setzero_si128()),
                            _mm_unpacklo_epi32(w1, _mm_setzero_si128()), _mm_unpackhi_epi32(w1, _mm_setzero_si128()));
                    }
                }
                else if constexpr (sizeof(R) == 256 && has_zmm) {
                    auto x0 = as<__m128i>(x);
                    auto x1 = _mm512_extracti32x4_epi32(x, 1);
                    auto x2 = _mm512_extracti32x4_epi32(x, 2);
                    auto x3 = _mm512_extracti32x4_epi32(x, 3);

                    if constexpr (frm_sgn) 
                        return simd_tuple_make(_mm512_cvtepi16_epi64(x0), _mm512_cvtepi16_epi64(x1),
                            _mm512_cvtepi16_epi64(x2), _mm512_cvtepi16_epi64(x3));
                    else 
                        return simd_tuple_make(_mm512_cvtepu16_epi64(x0), _mm512_cvtepu16_epi64(x1),
                            _mm512_cvtepu16_epi64(x2), _mm512_cvtepu16_epi64(x3));
                }
                else return scalar_fallback_convert_<ISA, false, From, To>(x);
            }
            else if constexpr (i32_to_i8) {
                if constexpr (x_to_x) {
                    if constexpr (has_bwvl) return _mm_cvtepi32_epi8(x);
                    else if constexpr (has_ssse3<ISA>) return _mm_shuffle_epi8(x, _mm_setr_epi8(0, 4, 8, 12, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1));
                    else return _mm_packs_epi16(_mm_packs_epi32(_mm_srai_epi32(_mm_slli_epi32(x, 24), 24), _mm_setzero_si128()), _mm_setzero_si128());
                }
                else if constexpr (y_to_x && has_bwvl) {
                    return _mm256_cvtepi32_epi8(x);
                }
                else if constexpr (y_to_x && has_avx2_v) {
                    auto r_lo = _mm_srai_epi32(_mm_slli_epi32(as<__m128i>(x), 24), 24);
                    auto r_hi = _mm_srai_epi32(_mm_slli_epi32(_mm256_extracti128_si256(x, 1), 24), 24);
                    return _mm_packs_epi16(_mm_packs_epi32(r_lo, r_hi), _mm_setzero_si128());
                }
                else if constexpr (z_to_x && has_bw) {
                    return _mm512_cvtepi32_epi8(x);
                }
                else return scalar_fallback_convert_<ISA, false, From, To>(x);
            }
            else if constexpr (i32_to_i16) {
                if constexpr (x_to_x) {
                    if constexpr (has_bwvl) return _mm_cvtepi32_epi16(x);
                    else if constexpr (has_ssse3<ISA>) return _mm_shuffle_epi8(x, _mm_setr_epi8(0, 1, 4, 5, 8, 9, 12, 13, -1, -1, -1, -1, -1, -1, -1, -1));
                    else return _mm_packs_epi32(_mm_srai_epi32(_mm_slli_epi32(x, 16), 16), _mm_setzero_si128());
                }
                else if constexpr (y_to_x) {
                    if constexpr (has_bwvl) return _mm256_cvtepi32_epi16(x);
                    else if constexpr (has_avx2_v) return as<__m128i>(_mm256_permute4x64_epi64(_mm256_shuffle_epi8(x,
                        _mm256_setr_epi8(0, 1, 4, 5, 8, 9, 12, 13, -1, -1, -1, -1, -1, -1, -1, -1,
                        0, 1, 4, 5, 8, 9, 12, 13, -1, -1, -1, -1, -1, -1, -1, -1)), 0xF8));
                    else return scalar_fallback_convert_<ISA, false, From, To>(x);
                }
                else if constexpr (z_to_y && has_bw) {
                    return _mm512_cvtepi32_epi16(x);
                }
                else return scalar_fallback_convert_<ISA, false, From, To>(x);
            }
            else if constexpr (i32_to_i64) {
                if constexpr (x_to_y && has_avx2_v) {
                    if constexpr (frm_sgn) return _mm256_cvtepi32_epi64(x);
                    else return _mm256_cvtepu32_epi64(x);
                }
                else if constexpr (y_to_z) {
                    if constexpr (has_zmm) {
                        if constexpr (frm_sgn) return _mm512_cvtepi32_epi64(x);
                        else return _mm512_cvtepu32_epi64(x);
                    }
                    else if constexpr (has_avx2_v) {
                        if constexpr (frm_sgn) return simd_tuple_make(_mm256_cvtepi32_epi64(as<__m128i>(x)), _mm256_cvtepi32_epi64(_mm256_extracti128_si256(x, 1)));
                        else return simd_tuple_make(_mm256_cvtepu32_epi64(as<__m128i>(x)), _mm256_cvtepu32_epi64(_mm256_extracti128_si256(x, 1)));
                    }
                    else return scalar_fallback_convert_<ISA, false, From, To>(x);
                }
                else if constexpr (x_to_y && has_avx<ISA> && has_s41) {
                    __m128i e0, e1;

                    if constexpr (frm_sgn) {
                        e0 = _mm_cvtepi32_epi64(x);
                        e1 = _mm_cvtepi32_epi64(_mm_srli_si128(x, 8));
                    }
                    else {
                        e0 = _mm_cvtepu32_epi64(x);
                        e1 = _mm_cvtepu32_epi64(_mm_srli_si128(x, 8));
                    }

                    return _mm256_insertf128_si256(as<__m256i>(e0), e1, 1);
                }
                else if constexpr (x_to_y && has_s41) {
                    if constexpr (frm_sgn) return simd_tuple_make(_mm_cvtepi32_epi64(x), _mm_cvtepi32_epi64(_mm_srli_si128(x, 8)));
                    else return simd_tuple_make(_mm_cvtepu32_epi64(x), _mm_cvtepu32_epi64(_mm_srli_si128(x, 8)));
                }
                else if constexpr (x_to_y) {
                    if constexpr (frm_sgn) return simd_tuple_make(_mm_unpacklo_epi32(x, _mm_srai_epi32(x, 31)), _mm_unpackhi_epi32(x, _mm_srai_epi32(x, 31)));
                    else return simd_tuple_make(_mm_unpacklo_epi32(x, _mm_setzero_si128()), _mm_unpackhi_epi32(x, _mm_setzero_si128()));
                }
                else if constexpr (sizeof(R) == 128 && has_zmm) {
                    if constexpr (frm_sgn) return simd_tuple_make(_mm512_cvtepi32_epi64(as<__m256i>(x)), _mm512_cvtepi32_epi64(_mm512_extracti64x4_epi64(x, 1)));
                    else return simd_tuple_make(_mm512_cvtepu32_epi64(as<__m256i>(x)), _mm512_cvtepu32_epi64(_mm512_extracti64x4_epi64(x, 1)));
                }
                else return scalar_fallback_convert_<ISA, false, From, To>(x);
            }
            else if constexpr (i64_to_i32) {
                if constexpr (x_to_x) {
                    if constexpr (has_bwvl) return _mm_cvtepi64_epi32(x);
                    else if constexpr (has_ssse3<ISA>) return _mm_shuffle_epi8(x, _mm_setr_epi8(0, 1, 2, 3, 8, 9, 10, 11, -1, -1, -1, -1, -1, -1, -1, -1));
                    else return scalar_fallback_convert_<ISA, false, From, To>(x);
                }
                else if constexpr (y_to_x && has_bwvl) {
                    return _mm256_cvtepi64_epi32(x);
                }
                else if constexpr (z_to_y && has_bw) {
                    return _mm512_cvtepi64_epi32(x);
                }
                else return scalar_fallback_convert_<ISA, false, From, To>(x);
            }
            else if constexpr (i64_to_i16) {
                if constexpr (x_to_x && has_bwvl) return _mm_cvtepi64_epi16(x);
                else if constexpr (y_to_x && has_bwvl) return _mm256_cvtepi64_epi16(x);
                else if constexpr (z_to_x && has_bw) return _mm512_cvtepi64_epi16(x);
                else return scalar_fallback_convert_<ISA, false, From, To>(x);
            }
            else if constexpr (i64_to_i8) {
                if constexpr (x_to_x && has_bwvl) return _mm_cvtepi64_epi8(x);
                else if constexpr (y_to_x && has_bwvl) return _mm256_cvtepi64_epi8(x);
                else if constexpr (z_to_x && has_bw)   return _mm512_cvtepi64_epi8(x);
                else return scalar_fallback_convert_<ISA, false, From, To>(x);
            }
            else return scalar_fallback_convert_<ISA, false, From, To>(x);
        }
        else if constexpr (f_to_i) {
            if constexpr (!to_sgn) {
                return scalar_fallback_convert_<ISA, false, From, To>(x);
            }

            if constexpr (sizeof(From) == 4) {
                if constexpr (sizeof(To) == 4) {
                    if constexpr (x_to_x) return _mm_cvttps_epi32(x);
                    else if constexpr (y_to_y && has_ymm_v) return _mm256_cvttps_epi32(x);
                    else if constexpr (z_to_z && has_zmm) return _mm512_cvttps_epi32(x);
                    else return scalar_fallback_convert_<ISA, false, From, To>(x);
                }
                else if constexpr (sizeof(To) == 8) {
                    if constexpr (x_to_y && has_dqvl) return _mm256_cvttps_epi64(x);
                    else if constexpr (y_to_z && has_dq) return _mm512_cvttps_epi64(x);
                    else return scalar_fallback_convert_<ISA, false, From, To>(x);
                }
                else if constexpr (sizeof(To) == 2) {
                    if constexpr (x_to_x) {
                        if constexpr (has_bwvl) return _mm_cvtepi32_epi16(_mm_cvttps_epi32(x));
                        else if constexpr (has_ssse3<ISA>) return _mm_shuffle_epi8(_mm_cvttps_epi32(x), _mm_setr_epi8(0, 1, 4, 5, 8, 9, 12, 13, -1, -1, -1, -1, -1, -1, -1, -1));
                        else return _mm_packs_epi32(_mm_srai_epi32(_mm_slli_epi32(_mm_cvttps_epi32(x), 16), 16), _mm_setzero_si128());
                    }
                    else if constexpr (y_to_x && has_avx2_v) {
                        if constexpr (has_bwvl) return _mm256_cvtepi32_epi16(_mm256_cvttps_epi32(x));
                        else return scalar_fallback_convert_<ISA, false, From, To>(x);
                    }
                    else if constexpr (z_to_y && has_bw) {
                        return _mm512_cvtepi32_epi16(_mm512_cvttps_epi32(x));
                    }
                    else return scalar_fallback_convert_<ISA, false, From, To>(x);
                }
                else if constexpr (sizeof(To) == 1) {
                    if constexpr (x_to_x) {
                        if constexpr (has_bwvl) return _mm_cvtepi32_epi8(_mm_cvttps_epi32(x));
                        else return scalar_fallback_convert_<ISA, false, From, To>(x);
                    }
                    else if constexpr (y_to_x && has_avx2_v) {
                        if constexpr (has_bwvl) return _mm256_cvtepi32_epi8(_mm256_cvttps_epi32(x));
                        else return scalar_fallback_convert_<ISA, false, From, To>(x);
                    }
                    else if constexpr (z_to_x && has_bw) {
                        return _mm512_cvtepi32_epi8(_mm512_cvttps_epi32(x));
                    }
                    else return scalar_fallback_convert_<ISA, false, From, To>(x);
                }
                else return scalar_fallback_convert_<ISA, false, From, To>(x);
            }
            else if constexpr (sizeof(From) == 8) {
                if constexpr (sizeof(To) == 4) {
                    if constexpr (x_to_x) return _mm_cvttpd_epi32(x);
                    else if constexpr (y_to_x && has_ymm_v) return _mm256_cvttpd_epi32(x);
                    else if constexpr (z_to_y && has_zmm)   return _mm512_cvttpd_epi32(x);
                    else return scalar_fallback_convert_<ISA, false, From, To>(x);
                }
                else if constexpr (sizeof(To) == 8) {
                    if constexpr (x_to_x && has_dqvl) return _mm_cvttpd_epi64(x);
                    else if constexpr (y_to_y && has_dqvl) return _mm256_cvttpd_epi64(x);
                    else if constexpr (z_to_z && has_dq)   return _mm512_cvttpd_epi64(x);
                    else return scalar_fallback_convert_<ISA, false, From, To>(x);
                }
                else if constexpr (sizeof(To) == 2) {
                    if constexpr (x_to_x && has_sse2<ISA>) {
                        if constexpr (has_bwvl) return _mm_cvtepi32_epi16(_mm_cvttpd_epi32(x));
                        else if constexpr (has_ssse3<ISA>) return _mm_shuffle_epi8(i32, _mm_setr_epi8(0, 1, 4, 5, 8, 9, 12, 13, -1, -1, -1, -1, -1, -1, -1, -1));
                        else return _mm_packs_epi32(_mm_srai_epi32(_mm_slli_epi32(i32, 16), 16), _mm_setzero_si128());
                    }
                    else if constexpr (y_to_x && has_ymm_v) {
                        if constexpr (has_bwvl) return _mm_cvtepi32_epi16(_mm256_cvttpd_epi32(x));
                        else if constexpr (has_ssse3<ISA>) return _mm_shuffle_epi8(_mm256_cvttpd_epi32(x), _mm_setr_epi8(0, 1, 4, 5, 8, 9, 12, 13, -1, -1, -1, -1, -1, -1, -1, -1));
                        else return _mm_packs_epi32(_mm_srai_epi32(_mm_slli_epi32(_mm256_cvttpd_epi32(x), 16), 16), _mm_setzero_si128());
                    }
                    else if constexpr (z_to_x && has_zmm) {
                        if constexpr (has_bwvl) return _mm256_cvtepi32_epi16(_mm512_cvttpd_epi32(x));
                        else return scalar_fallback_convert_<ISA, false, From, To>(x);
                    }
                    else return scalar_fallback_convert_<ISA, false, From, To>(x);
                }
                else if constexpr (sizeof(To) == 1) {
                    if constexpr (x_to_x && has_sse2<ISA>) {
                        if constexpr (has_bwvl) return _mm_cvtepi32_epi8(_mm_cvttpd_epi32(x));
                        else return scalar_fallback_convert_<ISA, false, From, To>(x);
                    }
                    else if constexpr (y_to_x && has_ymm_v) {
                        if constexpr (has_bwvl) return _mm_cvtepi32_epi8(_mm256_cvttpd_epi32(x));
                        else return scalar_fallback_convert_<ISA, false, From, To>(x);
                    }
                    else if constexpr (z_to_x && has_zmm) {
                        if constexpr (has_bwvl) return _mm256_cvtepi32_epi8(_mm512_cvttpd_epi32(x));
                        else return scalar_fallback_convert_<ISA, false, From, To>(x);
                    }
                    else return scalar_fallback_convert_<ISA, false, From, To>(x);
                }
                else return scalar_fallback_convert_<ISA, false, From, To>(x);
            }
            else return scalar_fallback_convert_<ISA, false, From, To>(x);
        }
        else if constexpr (i_to_f) {
            if constexpr (sizeof(To) == 4) {
                if constexpr (sizeof(From) == 1) {
                    if constexpr (x_to_z) {
                        if constexpr (has_zmm) {
                            if constexpr (frm_sgn) return _mm512_cvtepi32_ps(_mm512_cvtepi8_epi32(x));
                            else return _mm512_cvtepi32_ps(_mm512_cvtepu8_epi32(x));
                        }
                        else if constexpr (has_avx2_v) {
                            if constexpr (frm_sgn) 
                                return simd_tuple_make((_mm256_cvtepi8_epi32(x)),
                                    _mm256_cvtepi32_ps(_mm256_cvtepi8_epi32(_mm_srli_si128(x, 8))));
                            else 
                                return simd_tuple_make(_mm256_cvtepi32_ps(_mm256_cvtepu8_epi32(x)),
                                _mm256_cvtepi32_ps(_mm256_cvtepu8_epi32(_mm_srli_si128(x, 8))));
                        }
                        else if constexpr (has_avx<ISA> && has_s41) {
                            __m128i e0, e1, e2, e3;

                            if constexpr (frm_sgn) {
                                e0 = _mm_cvtepi8_epi32(x);
                                e1 = _mm_cvtepi8_epi32(_mm_srli_si128(x, 4));
                                e2 = _mm_cvtepi8_epi32(_mm_srli_si128(x, 8));
                                e3 = _mm_cvtepi8_epi32(_mm_srli_si128(x, 12));
                            }
                            else {
                                e0 = _mm_cvtepu8_epi32(x);
                                e1 = _mm_cvtepu8_epi32(_mm_srli_si128(x, 4));
                                e2 = _mm_cvtepu8_epi32(_mm_srli_si128(x, 8));
                                e3 = _mm_cvtepu8_epi32(_mm_srli_si128(x, 12));
                            }

                            return simd_tuple_make(_mm256_insertf128_ps(as<__m256>(_mm_cvtepi32_ps(e0)), _mm_cvtepi32_ps(e1), 1),
                                _mm256_insertf128_ps(as<__m256>(_mm_cvtepi32_ps(e2)), _mm_cvtepi32_ps(e3), 1));
                        }
                        else if constexpr (has_s41) {
                            if constexpr (frm_sgn) 
                                return simd_tuple_make(_mm_cvtepi32_ps(_mm_cvtepi8_epi32(x)),
                                    _mm_cvtepi32_ps(_mm_cvtepi8_epi32(_mm_srli_si128(x, 4))),
                                    _mm_cvtepi32_ps(_mm_cvtepi8_epi32(_mm_srli_si128(x, 8))),
                                    _mm_cvtepi32_ps(_mm_cvtepi8_epi32(_mm_srli_si128(x, 12))));
                            else 
                                return simd_tuple_make(_mm_cvtepi32_ps(_mm_cvtepu8_epi32(x)),
                                    _mm_cvtepi32_ps(_mm_cvtepu8_epi32(_mm_srli_si128(x, 4))),
                                    _mm_cvtepi32_ps(_mm_cvtepu8_epi32(_mm_srli_si128(x, 8))),
                                    _mm_cvtepi32_ps(_mm_cvtepu8_epi32(_mm_srli_si128(x, 12))));
                        }
                        else {
                            __m128i lo16, hi16;

                            if constexpr (frm_sgn) {
                                lo16 = _mm_srai_epi16(_mm_unpacklo_epi8(x, x), 8);
                                hi16 = _mm_srai_epi16(_mm_unpackhi_epi8(x, x), 8);
                            }
                            else {
                                __m128i zero = _mm_setzero_si128();
                                lo16 = _mm_unpacklo_epi8(x, zero);
                                hi16 = _mm_unpackhi_epi8(x, zero);
                            }

                            if constexpr (frm_sgn) 
                                return simd_tuple_make(_mm_cvtepi32_ps(_mm_srai_epi32(_mm_unpacklo_epi16(lo16, lo16), 16)),
                                    _mm_cvtepi32_ps(_mm_srai_epi32(_mm_unpackhi_epi16(lo16, lo16), 16)),
                                    _mm_cvtepi32_ps(_mm_srai_epi32(_mm_unpacklo_epi16(hi16, hi16), 16)),
                                    _mm_cvtepi32_ps(_mm_srai_epi32(_mm_unpackhi_epi16(hi16, hi16), 16)));
                            else {
                                return simd_tuple_make(_mm_cvtepi32_ps(_mm_unpacklo_epi16(lo16, _mm_setzero_si128())),
                                    _mm_cvtepi32_ps(_mm_unpackhi_epi16(lo16, _mm_setzero_si128())),
                                    _mm_cvtepi32_ps(_mm_unpacklo_epi16(hi16, _mm_setzero_si128())),
                                    _mm_cvtepi32_ps(_mm_unpackhi_epi16(hi16, _mm_setzero_si128())));
                            }
                        }
                    }
                    else if constexpr (sizeof(R) == 128 && has_zmm) {
                        if constexpr (frm_sgn)
                            return simd_tuple_make(_mm512_cvtepi32_ps(_mm512_cvtepi8_epi32(as<__m128i>(x))),
                                _mm512_cvtepi32_ps(_mm512_cvtepi8_epi32(_mm256_extracti128_si256(x, 1))));
                        else
                            return simd_tuple_make(_mm512_cvtepi32_ps(_mm512_cvtepu8_epi32(as<__m128i>(x))),
                                _mm512_cvtepi32_ps(_mm512_cvtepu8_epi32(_mm256_extracti128_si256(x, 1))));
                    }
                    else if constexpr (sizeof(R) == 128 && has_avx2_v) {
                        auto hi = _mm256_extracti128_si256(x, 1);
                        if constexpr (frm_sgn) 
                            return simd_tuple_make(_mm256_cvtepi32_ps(_mm256_cvtepi8_epi32(as<__m128i>(x))),
                                _mm256_cvtepi32_ps(_mm256_cvtepi8_epi32(_mm_srli_si128(as<__m128i>(x), 8))),
                                _mm256_cvtepi32_ps(_mm256_cvtepi8_epi32(hi)),
                                _mm256_cvtepi32_ps(_mm256_cvtepi8_epi32(_mm_srli_si128(hi, 8))));
                        else 
                            return simd_tuple_make(_mm256_cvtepi32_ps(_mm256_cvtepu8_epi32(as<__m128i>(x))),
                                _mm256_cvtepi32_ps(_mm256_cvtepu8_epi32(_mm_srli_si128(as<__m128i>(x), 8))),
                                _mm256_cvtepi32_ps(_mm256_cvtepu8_epi32(hi)),
                                _mm256_cvtepi32_ps(_mm256_cvtepu8_epi32(_mm_srli_si128(hi, 8))));
                    }
                    else return scalar_fallback_convert_<ISA, false, From, To>(x);
                }
                else if constexpr (sizeof(From) == 2) {
                    if constexpr (x_to_y && has_avx2_v) {
                        if constexpr (frm_sgn) return _mm256_cvtepi32_ps(_mm256_cvtepi16_epi32(x));
                        else return _mm256_cvtepi32_ps(_mm256_cvtepu16_epi32(x));
                    }
                    else if constexpr (x_to_y && has_avx<ISA> && has_s41) {
                        __m128i e0, e1;

                        if constexpr (frm_sgn) {
                            e0 = _mm_cvtepi16_epi32(x);
                            e1 = _mm_cvtepi16_epi32(_mm_srli_si128(x, 8));
                        }
                        else {
                            e0 = _mm_cvtepu16_epi32(x);
                            e1 = _mm_cvtepu16_epi32(_mm_srli_si128(x, 8));
                        }

                        return _mm256_insertf128_ps(as<__m256>(_mm_cvtepi32_ps(e0)), _mm_cvtepi32_ps(e1), 1);
                    }
                    else if constexpr (x_to_y && has_s41) {
                        if constexpr (frm_sgn) 
                            return simd_tuple_make(_mm_cvtepi32_ps(_mm_cvtepi16_epi32(x)),
                                _mm_cvtepi32_ps(_mm_cvtepi16_epi32(_mm_srli_si128(x, 8))));
                        else 
                            return simd_tuple_make(_mm_cvtepi32_ps(_mm_cvtepu16_epi32(x)),
                                _mm_cvtepi32_ps(_mm_cvtepu16_epi32(_mm_srli_si128(x, 8))));
                    }
                    else if constexpr (y_to_z) {
                        if constexpr (has_zmm) {
                            if constexpr (frm_sgn) return _mm512_cvtepi32_ps(_mm512_cvtepi16_epi32(x));
                            else return _mm512_cvtepi32_ps(_mm512_cvtepu16_epi32(x));
                        }
                        else if constexpr (has_avx2_v) {
                            if constexpr (frm_sgn) 
                                return simd_tuple_make(_mm256_cvtepi32_ps(_mm256_cvtepi16_epi32(as<__m128i>(x))),
                                    _mm256_cvtepi32_ps(_mm256_cvtepi16_epi32(_mm256_extracti128_si256(x, 1))));
                            else 
                                return simd_tuple_make(_mm256_cvtepi32_ps(_mm256_cvtepu16_epi32(as<__m128i>(x))),
                                    _mm256_cvtepi32_ps(_mm256_cvtepu16_epi32(_mm256_extracti128_si256(x, 1))));
                        }
                        else return scalar_fallback_convert_<ISA, false, From, To>(x);
                    }
                    else if constexpr (x_to_y) {
                        if constexpr (frm_sgn) 
                            return simd_tuple_make(_mm_cvtepi32_ps(_mm_srai_epi32(_mm_unpacklo_epi16(x, x), 16)),
                                _mm_cvtepi32_ps(_mm_srai_epi32(_mm_unpackhi_epi16(x, x), 16)));
                        else
                            return simd_tuple_make(_mm_cvtepi32_ps(_mm_unpacklo_epi16(x, _mm_setzero_si128())),
                                _mm_cvtepi32_ps(_mm_unpackhi_epi16(x, _mm_setzero_si128())));
                    }
                    else if constexpr (sizeof(R) == 128 && has_zmm) {
                        auto lo = as<__m256i>(x);
                        auto hi = _mm512_extracti64x4_epi64(x, 1);

                        if constexpr (frm_sgn) 
                            return simd_tuple_make(_mm512_cvtepi32_ps(_mm512_cvtepi16_epi32(lo)),
                                _mm512_cvtepi32_ps(_mm512_cvtepi16_epi32(hi)));
                        else 
                            return simd_tuple_make(_mm512_cvtepi32_ps(_mm512_cvtepu16_epi32(lo)),
                                _mm512_cvtepi32_ps(_mm512_cvtepu16_epi32(hi)));
                    }
                    else return scalar_fallback_convert_<ISA, false, From, To>(x);
                }
                else if constexpr (sizeof(From) == 4) {
                    if constexpr (frm_sgn) {
                        if constexpr (x_to_x) return _mm_cvtepi32_ps(x);
                        else if constexpr (y_to_y && has_ymm_v) return _mm256_cvtepi32_ps(x);
                        else if constexpr (z_to_z && has_zmm)   return _mm512_cvtepi32_ps(x);
                        else return scalar_fallback_convert_<ISA, false, From, To>(x);
                    }
                    else {
                        if constexpr (x_to_x && has_zmm && has_vl) return _mm_cvtepu32_ps(x);
                        else if constexpr (y_to_y && has_zmm && has_vl) return _mm256_cvtepu32_ps(x);
                        else if constexpr (z_to_z && has_zmm) return _mm512_cvtepu32_ps(x);
                        else return scalar_fallback_convert_<ISA, false, From, To>(x);
                    }
                }
                else if constexpr (sizeof(From) == 8) {
                    if constexpr (x_to_x && has_dqvl) {
                        if constexpr (frm_sgn) return _mm_cvtepi64_ps(x);
                        else return _mm_cvtepu64_ps(x);
                    }
                    else if constexpr (y_to_x && has_dqvl) {
                        if constexpr (frm_sgn) return _mm256_cvtepi64_ps(x);
                        else return _mm256_cvtepu64_ps(x);
                    }
                    else if constexpr (z_to_y && has_dq) {
                        if constexpr (frm_sgn) return _mm512_cvtepi64_ps(x);
                        else return _mm512_cvtepu64_ps(x);
                    }
                    else return scalar_fallback_convert_<ISA, false, From, To>(x);
                }
                else return scalar_fallback_convert_<ISA, false, From, To>(x);
            }
            else if constexpr (sizeof(To) == 8) {
                if constexpr (sizeof(From) == 1) {
                    if constexpr (sizeof(R) == 128 && has_zmm) {
                        __m512i e32;

                        if constexpr (frm_sgn) e32 = _mm512_cvtepi8_epi32(x);
                        else e32 = _mm512_cvtepu8_epi32(x);

                        return simd_tuple_make(_mm512_cvtepi32_pd(as<__m256i>(e32)),
                            _mm512_cvtepi32_pd(_mm512_extracti64x4_epi64(e32, 1)));
                    }
                    else if constexpr (sizeof(R) == 128 && has_avx2_v) {
                        __m256i lo32, hi32;

                        if constexpr (frm_sgn) {
                            lo32 = _mm256_cvtepi8_epi32(x);
                            hi32 = _mm256_cvtepi8_epi32(_mm_srli_si128(x, 8));
                        }
                        else {
                            lo32 = _mm256_cvtepu8_epi32(x);
                            hi32 = _mm256_cvtepu8_epi32(_mm_srli_si128(x, 8));
                        }

                        return simd_tuple_make(_mm256_cvtepi32_pd(as<__m128i>(lo32)),
                            _mm256_cvtepi32_pd(_mm256_extracti128_si256(lo32, 1)),
                            _mm256_cvtepi32_pd(as<__m128i>(hi32)),
                            _mm256_cvtepi32_pd(_mm256_extracti128_si256(hi32, 1)));
                    }
                    else if constexpr (sizeof(V) == 16 && sizeof(R) == 128 && has_avx<ISA> && has_s41) {
                        __m128i e0, e1, e2, e3;

                        if constexpr (frm_sgn) {
                            e0 = _mm_cvtepi8_epi32(x);
                            e1 = _mm_cvtepi8_epi32(_mm_srli_si128(x, 4));
                            e2 = _mm_cvtepi8_epi32(_mm_srli_si128(x, 8));
                            e3 = _mm_cvtepi8_epi32(_mm_srli_si128(x, 12));
                        }
                        else {
                            e0 = _mm_cvtepu8_epi32(x);
                            e1 = _mm_cvtepu8_epi32(_mm_srli_si128(x, 4));
                            e2 = _mm_cvtepu8_epi32(_mm_srli_si128(x, 8));
                            e3 = _mm_cvtepu8_epi32(_mm_srli_si128(x, 12));
                        }

                        return simd_tuple_make( _mm256_cvtepi32_pd(e0), _mm256_cvtepi32_pd(e1),
                            _mm256_cvtepi32_pd(e2), _mm256_cvtepi32_pd(e3));
                    }
                    else if constexpr (sizeof(V) == 16 && sizeof(R) == 128 && has_s41) {
                        __m128i e0, e1, e2, e3;

                        if constexpr (frm_sgn) {
                            e0 = _mm_cvtepi8_epi32(x);
                            e1 = _mm_cvtepi8_epi32(_mm_srli_si128(x, 4));
                            e2 = _mm_cvtepi8_epi32(_mm_srli_si128(x, 8));
                            e3 = _mm_cvtepi8_epi32(_mm_srli_si128(x, 12));
                        }
                        else {
                            e0 = _mm_cvtepu8_epi32(x);
                            e1 = _mm_cvtepu8_epi32(_mm_srli_si128(x, 4));
                            e2 = _mm_cvtepu8_epi32(_mm_srli_si128(x, 8));
                            e3 = _mm_cvtepu8_epi32(_mm_srli_si128(x, 12));
                        }

                        return simd_tuple_make(_mm_cvtepi32_pd(e0), _mm_cvtepi32_pd(_mm_srli_si128(e0, 8)),
                            _mm_cvtepi32_pd(e1), _mm_cvtepi32_pd(_mm_srli_si128(e1, 8)),
                            _mm_cvtepi32_pd(e2), _mm_cvtepi32_pd(_mm_srli_si128(e2, 8)),
                            _mm_cvtepi32_pd(e3), _mm_cvtepi32_pd(_mm_srli_si128(e3, 8)));
                    }
                    else if constexpr (sizeof(V) == 16 && sizeof(R) == 128 && has_sse2<ISA>) {
                        __m128i lo16, hi16;
                        __m128i e0, e1, e2, e3;

                        if constexpr (frm_sgn) {
                            lo16 = _mm_srai_epi16(_mm_unpacklo_epi8(x, x), 8);
                            hi16 = _mm_srai_epi16(_mm_unpackhi_epi8(x, x), 8);
                        }
                        else {
                            __m128i zero = _mm_setzero_si128();
                            lo16 = _mm_unpacklo_epi8(x, zero);
                            hi16 = _mm_unpackhi_epi8(x, zero);
                        }

                        if constexpr (frm_sgn) {
                            e0 = _mm_srai_epi32(_mm_unpacklo_epi16(lo16, lo16), 16);
                            e1 = _mm_srai_epi32(_mm_unpackhi_epi16(lo16, lo16), 16);
                            e2 = _mm_srai_epi32(_mm_unpacklo_epi16(hi16, hi16), 16);
                            e3 = _mm_srai_epi32(_mm_unpackhi_epi16(hi16, hi16), 16);
                        }
                        else {
                            e0 = _mm_unpacklo_epi16(lo16, _mm_setzero_si128());
                            e1 = _mm_unpackhi_epi16(lo16, _mm_setzero_si128());
                            e2 = _mm_unpacklo_epi16(hi16, _mm_setzero_si128());
                            e3 = _mm_unpackhi_epi16(hi16, _mm_setzero_si128());
                        }

                        return simd_tuple_make(_mm_cvtepi32_pd(e0), _mm_cvtepi32_pd(_mm_srli_si128(e0, 8)),
                            _mm_cvtepi32_pd(e1), _mm_cvtepi32_pd(_mm_srli_si128(e1, 8)),
                            _mm_cvtepi32_pd(e2), _mm_cvtepi32_pd(_mm_srli_si128(e2, 8)),
                            _mm_cvtepi32_pd(e3), _mm_cvtepi32_pd(_mm_srli_si128(e3, 8)));
                    }
                    else return scalar_fallback_convert_<ISA, false, From, To>(x);
                }
                else if constexpr (sizeof(From) == 2) {
                    if constexpr (x_to_z) {
                        if constexpr (has_zmm) {
                            if constexpr (frm_sgn) return _mm512_cvtepi32_pd(_mm256_cvtepi16_epi32(x));
                            else return _mm512_cvtepu32_pd(_mm256_cvtepu16_epi32(x));
                        }
                        else if constexpr (has_avx2_v) {
                            __m256i w32;

                            if constexpr (frm_sgn) w32 = _mm256_cvtepi16_epi32(x);
                            else w32 = _mm256_cvtepu16_epi32(x);

                            return simd_tuple_make(_mm256_cvtepi32_pd(as<__m128i>(w32)),
                                _mm256_cvtepi32_pd(_mm256_extracti128_si256(w32, 1)));
                        }
                        else if constexpr (sizeof(V) == 16 && has_avx<ISA> && has_s41) {
                            __m128i w0, w1;

                            if constexpr (frm_sgn) {
                                w0 = _mm_cvtepi16_epi32(x);
                                w1 = _mm_cvtepi16_epi32(_mm_srli_si128(x, 8));
                            }
                            else {
                                w0 = _mm_cvtepu16_epi32(x);
                                w1 = _mm_cvtepu16_epi32(_mm_srli_si128(x, 8));
                            }

                            return simd_tuple_make(_mm256_cvtepi32_pd(w0), _mm256_cvtepi32_pd(w1));
                        }
                        else if constexpr (sizeof(V) == 16 && has_s41) {
                            __m128i w0, w1;

                            if constexpr (frm_sgn) {
                                w0 = _mm_cvtepi16_epi32(x);
                                w1 = _mm_cvtepi16_epi32(_mm_srli_si128(x, 8));
                            }
                            else {
                                w0 = _mm_cvtepu16_epi32(x);
                                w1 = _mm_cvtepu16_epi32(_mm_srli_si128(x, 8));
                            }

                            return simd_tuple_make(_mm_cvtepi32_pd(w0), _mm_cvtepi32_pd(_mm_srli_si128(w0, 8)),
                                _mm_cvtepi32_pd(w1), _mm_cvtepi32_pd(_mm_srli_si128(w1, 8)));
                        }
                        else if constexpr (sizeof(V) == 16 && has_sse2<ISA>) {
                            __m128i w0, w1;

                            if constexpr (frm_sgn) {
                                w0 = _mm_srai_epi32(_mm_unpacklo_epi16(x, x), 16);
                                w1 = _mm_srai_epi32(_mm_unpackhi_epi16(x, x), 16);
                            }
                            else {
                                __m128i zero = _mm_setzero_si128();
                                w0 = _mm_unpacklo_epi16(x, zero);
                                w1 = _mm_unpackhi_epi16(x, zero);
                            }

                            return simd_tuple_make(_mm_cvtepi32_pd(w0), _mm_cvtepi32_pd(_mm_srli_si128(w0, 8)),
                                _mm_cvtepi32_pd(w1), _mm_cvtepi32_pd(_mm_srli_si128(w1, 8)));
                        }
                        else return scalar_fallback_convert_<ISA, false, From, To>(x);
                    }
                    else if constexpr (sizeof(R) == 128 && sizeof(V) == 32 && has_zmm) {
                        __m512i e32;

                        if constexpr (frm_sgn) e32 = _mm512_cvtepi16_epi32(x);
                        else e32 = _mm512_cvtepu16_epi32(x);

                        return simd_tuple_make(_mm512_cvtepi32_pd(as<__m256i>(e32)), _mm512_cvtepi32_pd(_mm512_extracti64x4_epi64(e32, 1)));
                    }
                    else if constexpr (sizeof(R) == 128 && has_avx2_v) {
                        __m256i w_lo, w_hi;

                        auto lo = as<__m128i>(x);
                        auto hi = _mm256_extracti128_si256(x, 1);

                        if constexpr (frm_sgn) {
                            w_lo = _mm256_cvtepi16_epi32(lo);
                            w_hi = _mm256_cvtepi16_epi32(hi);
                        }
                        else {
                            w_lo = _mm256_cvtepu16_epi32(lo);
                            w_hi = _mm256_cvtepu16_epi32(hi);
                        }

                        return simd_tuple_make(_mm256_cvtepi32_pd(as<__m128i>(w_lo)),
                            _mm256_cvtepi32_pd(_mm256_extracti128_si256(w_lo, 1)),
                            _mm256_cvtepi32_pd(as<__m128i>(w_hi)),
                            _mm256_cvtepi32_pd(_mm256_extracti128_si256(w_hi, 1)));
                    }
                    else return scalar_fallback_convert_<ISA, false, From, To>(x);
                }
                else if constexpr (sizeof(From) == 4) {
                    if constexpr (frm_sgn) {
                        if constexpr (x_to_y && has_ymm_v) return _mm256_cvtepi32_pd(x);
                        else if constexpr (y_to_z) {
                            if constexpr (has_zmm) return _mm512_cvtepi32_pd(x);
                            else if constexpr (has_avx2_v) return simd_tuple_make(_mm256_cvtepi32_pd(
                                as<__m128i>(x)), _mm256_cvtepi32_pd(_mm256_extracti128_si256(x, 1)));
                            else return scalar_fallback_convert_<ISA, false, From, To>(x);
                        }
                        else if constexpr (sizeof(V) == 64 && sizeof(R) == 128 && has_zmm) return simd_tuple_make(_mm512_cvtepi32_pd(
                            as<__m256i>(x)), _mm512_cvtepi32_pd(_mm512_extracti64x4_epi64(x, 1)));
                        else return scalar_fallback_convert_<ISA, false, From, To>(x);
                    }
                    else {
                        if constexpr (sizeof(V) == 64 && sizeof(R) == 128 && has_zmm) return simd_tuple_make(
                            _mm512_cvtepu32_pd(as<__m256i>(x)), _mm512_cvtepu32_pd(_mm512_extracti64x4_epi64(x, 1)));
                        else if constexpr (x_to_y && has_zmm && has_vl) return _mm256_cvtepu32_pd(x);
                        else if constexpr (y_to_z && has_zmm) return _mm512_cvtepu32_pd(x);
                        else return scalar_fallback_convert_<ISA, false, From, To>(x);
                    }
                }
                else if constexpr (sizeof(From) == 8) {
                    if constexpr (x_to_x && has_dqvl) {
                        if constexpr (frm_sgn) return _mm_cvtepi64_pd(x);
                        else return _mm_cvtepu64_pd(x);
                    }
                    else if constexpr (y_to_y && has_dqvl) {
                        if constexpr (frm_sgn) return _mm256_cvtepi64_pd(x);
                        else return _mm256_cvtepu64_pd(x);
                    }
                    else if constexpr (z_to_z && has_dq) {
                        if constexpr (frm_sgn) return _mm512_cvtepi64_pd(x);
                        else return _mm512_cvtepu64_pd(x);
                    }
                    else return scalar_fallback_convert_<ISA, false, From, To>(x);
                }
                else return scalar_fallback_convert_<ISA, false, From, To>(x);
            }
            else return scalar_fallback_convert_<ISA, false, From, To>(x);
        }
        else if constexpr (f_to_f) {
            if constexpr (sizeof(From) == 4) {
                if constexpr (x_to_y) {
                    if constexpr (has_ymm_v) return _mm256_cvtps_pd(x);
                    else return scalar_fallback_convert_<ISA, false, From, To>(x);
                }
                else if constexpr (y_to_z) {
                    if constexpr (has_zmm) return _mm512_cvtps_pd(x);
                    else return scalar_fallback_convert_<ISA, false, From, To>(x);
                }
                else if constexpr (x_to_x) {
                    return pack_or_tuple<ISA>(_mm_cvtps_pd(x), _mm_cvtps_pd(_mm_movehl_ps(x, x)));
                }
                else return scalar_fallback_convert_<ISA, false, From, To>(x);
            }
            else if constexpr (sizeof(From) == 8) {
                if constexpr (x_to_x) return _mm_cvtpd_ps(x);
                else if constexpr (y_to_x) {
                    if constexpr (has_ymm_v) return _mm256_cvtpd_ps(x);
                    else return scalar_fallback_convert_<ISA, false, From, To>(x);
                }
                else if constexpr (z_to_y) {
                    if constexpr (has_zmm) return _mm512_cvtpd_ps(x);
                    else return scalar_fallback_convert_<ISA, false, From, To>(x);
                }
                else return scalar_fallback_convert_<ISA, false, From, To>(x);
            }
            else return scalar_fallback_convert_<ISA, false, From, To>(x);
        }
        else return scalar_fallback_convert_<ISA, false, From, To>(x);
    }
}

template <arch::ISA ISA, arithmetic_type From, arithmetic_type To, intrin_or_arithmetic_type V>
raze_always_inline deduce_convert_return_type<ISA, From, To, V>
convert_saturated_(V x) noexcept {
    using R = deduce_convert_return_type<ISA, From, To, V>;

    if constexpr (std::same_as<To, From>) {
        return x;
    }
    else if constexpr (sizeof(V) < 16) {
        return scalar_fallback_convert_<ISA, true, From, To>(x);
    }
    else if constexpr (should_split_<ISA, From, To, V>()) {
        return split_and_recurse_<ISA>(x, [](auto chunk) raze_always_inline_lambda{ return convert_saturated_<ISA, From, To>(chunk); });
    }
    else {
        constexpr bool i_to_i = std::is_integral_v<From> && std::is_integral_v<To>;
        constexpr bool f_to_i = std::is_floating_point_v<From> && std::is_integral_v<To>;
        constexpr bool i_to_f = std::is_integral_v<From> && std::is_floating_point_v<To>;
        constexpr bool f_to_f = std::is_floating_point_v<From> && std::is_floating_point_v<To>;
        constexpr bool frm_sgn = std::is_signed_v<From>;
        constexpr bool to_sgn = std::is_signed_v<To>;

        constexpr bool has_ymm_v = has_avx<ISA>;
        constexpr bool has_avx2_v = has_avx2<ISA>;
        constexpr bool has_zmm = has_avx512f<ISA>;
        constexpr bool has_bw = has_avx512bw<ISA>;
        constexpr bool has_dq = has_avx512dq<ISA>;
        constexpr bool has_vl = has_avx512vl<ISA>;
        constexpr bool has_bwvl = has_bw && has_vl;
        constexpr bool has_dqvl = has_dq && has_vl;
        constexpr bool has_s41 = has_sse41<ISA>;

        constexpr bool x_to_x = sizeof(V) <= 16 && sizeof(R) <= 16;
        constexpr bool x_to_y = sizeof(V) <= 16 && sizeof(R) == 32;
        constexpr bool x_to_z = sizeof(V) <= 16 && sizeof(R) == 64;
        constexpr bool y_to_x = sizeof(V) == 32 && sizeof(R) <= 16;
        constexpr bool y_to_y = sizeof(V) == 32 && sizeof(R) == 32;
        constexpr bool y_to_z = sizeof(V) == 32 && sizeof(R) == 64;
        constexpr bool z_to_x = sizeof(V) == 64 && sizeof(R) <= 16;
        constexpr bool z_to_y = sizeof(V) == 64 && sizeof(R) == 32;
        constexpr bool z_to_z = sizeof(V) == 64 && sizeof(R) == 64;

        constexpr bool i8_to_i16 = i_to_i && sizeof(From) == 1 && sizeof(To) == 2;
        constexpr bool i8_to_i32 = i_to_i && sizeof(From) == 1 && sizeof(To) == 4;
        constexpr bool i8_to_i64 = i_to_i && sizeof(From) == 1 && sizeof(To) == 8;
        constexpr bool i16_to_i32 = i_to_i && sizeof(From) == 2 && sizeof(To) == 4;
        constexpr bool i16_to_i64 = i_to_i && sizeof(From) == 2 && sizeof(To) == 8;
        constexpr bool i32_to_i64 = i_to_i && sizeof(From) == 4 && sizeof(To) == 8;

        if constexpr (i_to_i) {
            constexpr bool need_scalar_narrowing = sizeof(To) < sizeof(From);
            if constexpr (need_scalar_narrowing) return scalar_fallback_convert_<ISA, true, From, To>(x);

            constexpr bool sgn_to_unsgn = frm_sgn && !to_sgn;

            if constexpr (i8_to_i16) {
                if constexpr (x_to_y && has_avx2_v) {
                    if constexpr (sgn_to_unsgn) return vertical_max_<ISA, int16_t>(_mm256_cvtepi8_epi16(x), _mm256_setzero_si256());
                    if constexpr (frm_sgn) return _mm256_cvtepi8_epi16(x);
                    else  return _mm256_cvtepu8_epi16(x);
                }
                else if constexpr (y_to_z && has_bw) {
                    if constexpr (sgn_to_unsgn) return vertical_max_<ISA, int16_t>(_mm512_cvtepi8_epi16(x), _mm512_setzero_si512());
                    if constexpr (frm_sgn) return _mm512_cvtepi8_epi16(x);
                    else return _mm512_cvtepu8_epi16(x);
                }
                else if constexpr (y_to_z && has_avx2_v) {
                    auto lo = as<__m128i>(x);
                    auto hi = _mm256_extracti128_si256(x, 1);

                    if constexpr (sgn_to_unsgn)
                        return pack_or_tuple<ISA>(vertical_max_<ISA, int16_t>(_mm256_cvtepi8_epi16(lo), _mm256_setzero_si256()),
                            vertical_max_<ISA, int16_t>(_mm256_cvtepi8_epi16(hi), _mm256_setzero_si256()));

                    if constexpr (frm_sgn) return pack_or_tuple<ISA>(_mm256_cvtepi8_epi16(lo), _mm256_cvtepi8_epi16(hi));
                    else return pack_or_tuple<ISA>(_mm256_cvtepu8_epi16(lo), _mm256_cvtepu8_epi16(hi));
                }
                else if constexpr (sizeof(R) == 128 && has_bw) {
                    auto x0 = as<__m128i>(x);
                    auto x1 = _mm512_extracti32x4_epi32(x, 1);
                    auto x2 = _mm512_extracti32x4_epi32(x, 2);
                    auto x3 = _mm512_extracti32x4_epi32(x, 3);

                    if constexpr (sgn_to_unsgn)
                        return simd_tuple_make(vertical_max_<ISA, int16_t>(_mm512_cvtepi8_epi16(_mm256_inserti128_si256(as<__m256i>(x0), x1, 1)), _mm512_setzero_si512()),
                            vertical_max_<ISA, int16_t>(_mm512_cvtepi8_epi16(_mm256_inserti128_si256(as<__m256i>(x2), x3, 1)), _mm512_setzero_si512()));

                    if constexpr (frm_sgn)
                        return simd_tuple_make(_mm512_cvtepi8_epi16(_mm256_inserti128_si256(as<__m256i>(x0), x1, 1)),
                            _mm512_cvtepi8_epi16(_mm256_inserti128_si256(as<__m256i>(x2), x3, 1)));
                    else 
                        return simd_tuple_make(_mm512_cvtepu8_epi16(_mm256_inserti128_si256(as<__m256i>(x0), x1, 1)),
                            _mm512_cvtepu8_epi16(_mm256_inserti128_si256(as<__m256i>(x2), x3, 1)));
                }
                else return scalar_fallback_convert_<ISA, true, From, To>(x);
            }
            else if constexpr (i8_to_i32) {
                if constexpr (x_to_z) {
                    if constexpr (has_zmm) {
                        if constexpr (sgn_to_unsgn) return vertical_max_<ISA, int32_t>(_mm512_cvtepi8_epi32(x), _mm512_setzero_si512());
                        if constexpr (frm_sgn) return _mm512_cvtepi8_epi32(x);
                        else return _mm512_cvtepu8_epi32(x);
                    }
                    else if constexpr (has_avx2_v) {
                        if constexpr (sgn_to_unsgn)
                            return simd_tuple_make(vertical_max_<ISA, int32_t>(_mm256_cvtepi8_epi32(x), _mm256_setzero_si256()),
                                vertical_max_<ISA, int32_t>(_mm256_cvtepi8_epi32(_mm_srli_si128(x, 8)), _mm256_setzero_si256()));

                        if constexpr (frm_sgn) 
                            return simd_tuple_make(_mm256_cvtepi8_epi32(x),
                                _mm256_cvtepi8_epi32(_mm_srli_si128(x, 8)));
                        else 
                            return simd_tuple_make(_mm256_cvtepu8_epi32(x),
                                _mm256_cvtepu8_epi32(_mm_srli_si128(x, 8)));
                    }
                    else if constexpr (has_avx<ISA> && has_s41) {
                        __m128i e0, e1, e2, e3;

                        if constexpr (frm_sgn) {
                            e0 = _mm_cvtepi8_epi32(x);
                            e1 = _mm_cvtepi8_epi32(_mm_srli_si128(x, 4));
                            e2 = _mm_cvtepi8_epi32(_mm_srli_si128(x, 8));
                            e3 = _mm_cvtepi8_epi32(_mm_srli_si128(x, 12));
                        }
                        else {
                            e0 = _mm_cvtepu8_epi32(x);
                            e1 = _mm_cvtepu8_epi32(_mm_srli_si128(x, 4));
                            e2 = _mm_cvtepu8_epi32(_mm_srli_si128(x, 8));
                            e3 = _mm_cvtepu8_epi32(_mm_srli_si128(x, 12));
                        }

                        if constexpr (sgn_to_unsgn) {
                            auto m0 = vertical_max_<ISA, int32_t>(e0, _mm_setzero_si128());
                            auto m1 = vertical_max_<ISA, int32_t>(e1, _mm_setzero_si128());
                            auto m2 = vertical_max_<ISA, int32_t>(e2, _mm_setzero_si128());
                            auto m3 = vertical_max_<ISA, int32_t>(e3, _mm_setzero_si128());

                            return simd_tuple_make(_mm256_insertf128_si256(as<__m256i>(m0), m1, 1),
                                _mm256_insertf128_si256(as<__m256i>(m2), m3, 1));
                        }

                        return simd_tuple_make(_mm256_insertf128_si256(as<__m256i>(e0), e1, 1),
                            _mm256_insertf128_si256(as<__m256i>(e2), e3, 1));
                    }
                    else if constexpr (has_s41) {
                        __m128i e0, e1, e2, e3;

                        if constexpr (frm_sgn) {
                            e0 = _mm_cvtepi8_epi32(x);
                            e1 = _mm_cvtepi8_epi32(_mm_srli_si128(x, 4));
                            e2 = _mm_cvtepi8_epi32(_mm_srli_si128(x, 8));
                            e3 = _mm_cvtepi8_epi32(_mm_srli_si128(x, 12));
                        }
                        else {
                            e0 = _mm_cvtepu8_epi32(x);
                            e1 = _mm_cvtepu8_epi32(_mm_srli_si128(x, 4));
                            e2 = _mm_cvtepu8_epi32(_mm_srli_si128(x, 8));
                            e3 = _mm_cvtepu8_epi32(_mm_srli_si128(x, 12));
                        }

                        if constexpr (sgn_to_unsgn)
                            return simd_tuple_make(vertical_max_<ISA, int32_t>(e0, _mm_setzero_si128()),
                                vertical_max_<ISA, int32_t>(e1, _mm_setzero_si128()),
                                vertical_max_<ISA, int32_t>(e2, _mm_setzero_si128()),
                                vertical_max_<ISA, int32_t>(e3, _mm_setzero_si128()));

                        return simd_tuple_make(e0, e1, e2, e3);
                    }
                    else {
                        __m128i lo16, hi16;
                        __m128i e0, e1, e2, e3;

                        if constexpr (frm_sgn) {
                            lo16 = _mm_srai_epi16(_mm_unpacklo_epi8(x, x), 8);
                            hi16 = _mm_srai_epi16(_mm_unpackhi_epi8(x, x), 8);
                        }
                        else {
                            __m128i zero = _mm_setzero_si128();
                            lo16 = _mm_unpacklo_epi8(x, zero);
                            hi16 = _mm_unpackhi_epi8(x, zero);
                        }

                        if constexpr (frm_sgn) {
                            e0 = _mm_srai_epi32(_mm_unpacklo_epi16(lo16, lo16), 16);
                            e1 = _mm_srai_epi32(_mm_unpackhi_epi16(lo16, lo16), 16);
                            e2 = _mm_srai_epi32(_mm_unpacklo_epi16(hi16, hi16), 16);
                            e3 = _mm_srai_epi32(_mm_unpackhi_epi16(hi16, hi16), 16);
                        }
                        else {
                            e0 = _mm_unpacklo_epi16(lo16, _mm_setzero_si128());
                            e1 = _mm_unpackhi_epi16(lo16, _mm_setzero_si128());
                            e2 = _mm_unpacklo_epi16(hi16, _mm_setzero_si128());
                            e3 = _mm_unpackhi_epi16(hi16, _mm_setzero_si128());
                        }

                        if constexpr (sgn_to_unsgn)
                            return simd_tuple_make(vertical_max_<ISA, int32_t>(e0, _mm_setzero_si128()),
                                vertical_max_<ISA, int32_t>(e1, _mm_setzero_si128()),
                                vertical_max_<ISA, int32_t>(e2, _mm_setzero_si128()),
                                vertical_max_<ISA, int32_t>(e3, _mm_setzero_si128()));

                        return simd_tuple_make(e0, e1, e2, e3);
                    }
                }
                else if constexpr (sizeof(R) == 128 && has_zmm) {
                    auto lo = as<__m128i>(x);
                    auto hi = _mm256_extracti128_si256(x, 1);

                    if constexpr (sgn_to_unsgn)
                        return simd_tuple_make(vertical_max_<ISA, int32_t>(_mm512_cvtepi8_epi32(lo), _mm512_setzero_si512()),
                            vertical_max_<ISA, int32_t>(_mm512_cvtepi8_epi32(hi), _mm512_setzero_si512()));

                    if constexpr (frm_sgn) return simd_tuple_make(_mm512_cvtepi8_epi32(lo), _mm512_cvtepi8_epi32(hi));
                    else return simd_tuple_make(_mm512_cvtepu8_epi32(lo), _mm512_cvtepu8_epi32(hi));
                }
                else if constexpr (sizeof(R) == 128 && has_avx2_v) {
                    auto lo = as<__m128i>(x);
                    auto hi = _mm256_extracti128_si256(x, 1);

                    if constexpr (sgn_to_unsgn)
                        return simd_tuple_make(vertical_max_<ISA, int32_t>(_mm256_cvtepi8_epi32(lo), _mm256_setzero_si256()),
                            vertical_max_<ISA, int32_t>(_mm256_cvtepi8_epi32(_mm_srli_si128(lo, 8)), _mm256_setzero_si256()),
                            vertical_max_<ISA, int32_t>(_mm256_cvtepi8_epi32(hi), _mm256_setzero_si256()),
                            vertical_max_<ISA, int32_t>(_mm256_cvtepi8_epi32(_mm_srli_si128(hi, 8)), _mm256_setzero_si256()));
                    
                    if constexpr (frm_sgn) 
                        return simd_tuple_make(_mm256_cvtepi8_epi32(lo), _mm256_cvtepi8_epi32(_mm_srli_si128(lo, 8)),
                            _mm256_cvtepi8_epi32(hi), _mm256_cvtepi8_epi32(_mm_srli_si128(hi, 8)));
                    else 
                        return simd_tuple_make(_mm256_cvtepu8_epi32(lo), _mm256_cvtepu8_epi32(_mm_srli_si128(lo, 8)),
                            _mm256_cvtepu8_epi32(hi), _mm256_cvtepu8_epi32(_mm_srli_si128(hi, 8)));
                }
                else return scalar_fallback_convert_<ISA, true, From, To>(x);
            }
            else if constexpr (i8_to_i64) {
                if constexpr (sizeof(R) == 128 && has_zmm) {
                    __m512i e32;

                    if constexpr (frm_sgn) e32 = _mm512_cvtepi8_epi32(x);
                    else e32 = _mm512_cvtepu8_epi32(x);

                    if constexpr (sgn_to_unsgn) e32 = vertical_max_<ISA, int32_t>(e32, _mm512_setzero_si512());

                    auto lo32 = as<__m256i>(e32);
                    auto hi32 = _mm512_extracti64x4_epi64(e32, 1);

                    if constexpr (sgn_to_unsgn) return simd_tuple_make(_mm512_cvtepu32_epi64(lo32), _mm512_cvtepu32_epi64(hi32));
                    if constexpr (frm_sgn) return simd_tuple_make(_mm512_cvtepi32_epi64(lo32), _mm512_cvtepi32_epi64(hi32));
                    else return simd_tuple_make(_mm512_cvtepu32_epi64(lo32), _mm512_cvtepu32_epi64(hi32));
                }
                else if constexpr (sizeof(R) == 128 && has_avx2_v) {
                    __m256i lo32, hi32;

                    if constexpr (frm_sgn) {
                        lo32 = _mm256_cvtepi8_epi32(x);
                        hi32 = _mm256_cvtepi8_epi32(_mm_srli_si128(x, 8));
                    }
                    else {
                        lo32 = _mm256_cvtepu8_epi32(x);
                        hi32 = _mm256_cvtepu8_epi32(_mm_srli_si128(x, 8));
                    }

                    if constexpr (sgn_to_unsgn) {
                        lo32 = vertical_max_<ISA, int32_t>(lo32, _mm256_setzero_si256());
                        hi32 = vertical_max_<ISA, int32_t>(hi32, _mm256_setzero_si256());
                    }

                    if constexpr (sgn_to_unsgn) 
                        return simd_tuple_make(_mm256_cvtepu32_epi64(as<__m128i>(lo32)),
                            _mm256_cvtepu32_epi64(_mm256_extracti128_si256(lo32, 1)),
                            _mm256_cvtepu32_epi64(as<__m128i>(hi32)),
                            _mm256_cvtepu32_epi64(_mm256_extracti128_si256(hi32, 1)));

                    if constexpr (frm_sgn) 
                        return simd_tuple_make(_mm256_cvtepi32_epi64(as<__m128i>(lo32)),
                            _mm256_cvtepi32_epi64(_mm256_extracti128_si256(lo32, 1)),
                            _mm256_cvtepi32_epi64(as<__m128i>(hi32)),
                            _mm256_cvtepi32_epi64(_mm256_extracti128_si256(hi32, 1)));
                    else 
                        return simd_tuple_make(_mm256_cvtepu32_epi64(as<__m128i>(lo32)),
                            _mm256_cvtepu32_epi64(_mm256_extracti128_si256(lo32, 1)),
                            _mm256_cvtepu32_epi64(as<__m128i>(hi32)),
                            _mm256_cvtepu32_epi64(_mm256_extracti128_si256(hi32, 1)));
                }
                else return scalar_fallback_convert_<ISA, true, From, To>(x);
            }
            else if constexpr (i16_to_i32) {
                if constexpr (x_to_y && has_avx2_v) {
                    if constexpr (sgn_to_unsgn) return vertical_max_<ISA, int32_t>(_mm256_cvtepi16_epi32(x), _mm256_setzero_si256());
                    if constexpr (frm_sgn) return _mm256_cvtepi16_epi32(x);
                    else return _mm256_cvtepu16_epi32(x);
                }
                else if constexpr (y_to_z) {
                    if constexpr (has_zmm) {
                        if constexpr (sgn_to_unsgn) return vertical_max_<ISA, int32_t>(_mm512_cvtepi16_epi32(x), _mm512_setzero_si512());
                        if constexpr (frm_sgn) return _mm512_cvtepi16_epi32(x);
                        else return _mm512_cvtepu16_epi32(x);
                    }
                    else if constexpr (has_avx2_v) {
                        auto lo = as<__m128i>(x);
                        auto hi = _mm256_extracti128_si256(x, 1);
                        if constexpr (sgn_to_unsgn) {
                            return simd_tuple_make(vertical_max_<ISA, int32_t>(_mm256_cvtepi16_epi32(lo), _mm256_setzero_si256()),
                                vertical_max_<ISA, int32_t>(_mm256_cvtepi16_epi32(hi), _mm256_setzero_si256()));
                        }
                        if constexpr (frm_sgn) return simd_tuple_make(_mm256_cvtepi16_epi32(lo), _mm256_cvtepi16_epi32(hi));
                        else return simd_tuple_make(_mm256_cvtepu16_epi32(lo), _mm256_cvtepu16_epi32(hi));
                    }
                    else return scalar_fallback_convert_<ISA, true, From, To>(x);
                }
                else if constexpr (x_to_y && has_avx<ISA> && has_s41) {
                    __m128i e0, e1;
                    if constexpr (frm_sgn) {
                        e0 = _mm_cvtepi16_epi32(x);
                        e1 = _mm_cvtepi16_epi32(_mm_srli_si128(x, 8));
                    }
                    else {
                        e0 = _mm_cvtepu16_epi32(x);
                        e1 = _mm_cvtepu16_epi32(_mm_srli_si128(x, 8));
                    }
                    if constexpr (sgn_to_unsgn) {
                        auto m0 = vertical_max_<ISA, int32_t>(e0, _mm_setzero_si128());
                        auto m1 = vertical_max_<ISA, int32_t>(e1, _mm_setzero_si128());
                        return _mm256_insertf128_si256(as<__m256i>(m0), m1, 1);
                    }
                    return _mm256_insertf128_si256(as<__m256i>(e0), e1, 1);
                }
                else if constexpr (x_to_y && has_s41) {
                    auto e0 = _mm_cvtepi16_epi32(x);
                    auto e1 = _mm_cvtepi16_epi32(_mm_srli_si128(x, 8));
                    if constexpr (sgn_to_unsgn) return simd_tuple_make(vertical_max_<ISA, int32_t>(e0, _mm_setzero_si128()), vertical_max_<ISA, int32_t>(e1, _mm_setzero_si128()));
                    if constexpr (frm_sgn) return simd_tuple_make(e0, e1);
                    else return simd_tuple_make(_mm_cvtepu16_epi32(x), _mm_cvtepu16_epi32(_mm_srli_si128(x, 8)));
                }
                else if constexpr (x_to_y) {
                    if constexpr (frm_sgn) {
                        __m128i e0 = _mm_srai_epi32(_mm_unpacklo_epi16(x, x), 16);
                        __m128i e1 = _mm_srai_epi32(_mm_unpackhi_epi16(x, x), 16);
                        if constexpr (sgn_to_unsgn) return simd_tuple_make(vertical_max_<ISA, int32_t>(e0, _mm_setzero_si128()), 
                            vertical_max_<ISA, int32_t>(e1, _mm_setzero_si128()));
                        return simd_tuple_make(e0, e1);
                    }
                    else return simd_tuple_make(_mm_unpacklo_epi16(x, _mm_setzero_si128()), _mm_unpackhi_epi16(x, _mm_setzero_si128()));
                }
                else if constexpr (sizeof(R) == 128 && has_zmm) {
                    auto lo = as<__m256i>(x);
                    auto hi = _mm512_extracti64x4_epi64(x, 1);
                    if constexpr (sgn_to_unsgn) return simd_tuple_make(vertical_max_<ISA, int32_t>(_mm512_cvtepi16_epi32(lo), _mm512_setzero_si512()),
                        vertical_max_<ISA, int32_t>(_mm512_cvtepi16_epi32(hi), _mm512_setzero_si512()));
                    if constexpr (frm_sgn) return simd_tuple_make(_mm512_cvtepi16_epi32(lo), _mm512_cvtepi16_epi32(hi));
                    else return simd_tuple_make(_mm512_cvtepu16_epi32(lo), _mm512_cvtepu16_epi32(hi));
                }
                else return scalar_fallback_convert_<ISA, true, From, To>(x);
            }
            else if constexpr (i16_to_i64) {
                if constexpr (x_to_z) {
                    if constexpr (has_zmm) {
                        if constexpr (sgn_to_unsgn) {
                            auto cx = vertical_max_<ISA, int16_t>(x, _mm_setzero_si128());
                            return _mm512_cvtepu16_epi64(cx);
                        }
                        if constexpr (frm_sgn) return _mm512_cvtepi16_epi64(x);
                        else return _mm512_cvtepu16_epi64(x);
                    }
                    else if constexpr (has_avx2_v) {
                        __m256i w32;

                        if constexpr (frm_sgn) w32 = _mm256_cvtepi16_epi32(x);
                        else w32 = _mm256_cvtepu16_epi32(x);

                        if constexpr (sgn_to_unsgn) {
                            w32 = vertical_max_<ISA, int32_t>(w32, _mm256_setzero_si256());
                            return simd_tuple_make(_mm256_cvtepu32_epi64(as<__m128i>(w32)),
                                _mm256_cvtepu32_epi64(_mm256_extracti128_si256(w32, 1)));
                        }

                        if constexpr (frm_sgn) 
                            return simd_tuple_make(_mm256_cvtepi32_epi64(as<__m128i>(w32)),
                                _mm256_cvtepi32_epi64(_mm256_extracti128_si256(w32, 1)));
                        else 
                            return simd_tuple_make(_mm256_cvtepu32_epi64(as<__m128i>(w32)),
                                _mm256_cvtepu32_epi64(_mm256_extracti128_si256(w32, 1)));
                    }
                    else return scalar_fallback_convert_<ISA, true, From, To>(x);
                }
                else if constexpr (sizeof(R) == 128 && has_zmm) {
                    auto lo = as<__m128i>(x);
                    auto hi = _mm256_extracti128_si256(x, 1);
                    if constexpr (sgn_to_unsgn) {
                        return simd_tuple_make(vertical_max_<ISA, int64_t>(_mm512_cvtepi16_epi64(lo), _mm512_setzero_si512()),
                            vertical_max_<ISA, int64_t>(_mm512_cvtepi16_epi64(hi), _mm512_setzero_si512()));
                    }
                    if constexpr (frm_sgn) return simd_tuple_make(_mm512_cvtepi16_epi64(lo), _mm512_cvtepi16_epi64(hi));
                    else return simd_tuple_make(_mm512_cvtepu16_epi64(lo), _mm512_cvtepu16_epi64(hi));
                }
                else if constexpr (sizeof(R) == 128 && has_avx2_v) {
                    __m256i w_lo, w_hi;

                    if constexpr (frm_sgn) {
                        w_lo = _mm256_cvtepi16_epi32(as<__m128i>(x));
                        w_hi = _mm256_cvtepi16_epi32(_mm256_extracti128_si256(x, 1));
                    }
                    else {
                        w_lo = _mm256_cvtepu16_epi32(as<__m128i>(x));
                        w_hi = _mm256_cvtepu16_epi32(_mm256_extracti128_si256(x, 1));
                    }

                    if constexpr (sgn_to_unsgn) {
                        w_lo = vertical_max_<ISA, int32_t>(w_lo, _mm256_setzero_si256());
                        w_hi = vertical_max_<ISA, int32_t>(w_hi, _mm256_setzero_si256());
                        return simd_tuple_make(_mm256_cvtepu32_epi64(as<__m128i>(w_lo)),
                            _mm256_cvtepu32_epi64(_mm256_extracti128_si256(w_lo, 1)),
                            _mm256_cvtepu32_epi64(as<__m128i>(w_hi)),
                            _mm256_cvtepu32_epi64(_mm256_extracti128_si256(w_hi, 1)));
                    }
                    if constexpr (frm_sgn) 
                        return simd_tuple_make(_mm256_cvtepi32_epi64(as<__m128i>(w_lo)),
                            _mm256_cvtepi32_epi64(_mm256_extracti128_si256(w_lo, 1)),
                            _mm256_cvtepi32_epi64(as<__m128i>(w_hi)),
                            _mm256_cvtepi32_epi64(_mm256_extracti128_si256(w_hi, 1)));
                    else 
                        return simd_tuple_make(_mm256_cvtepu32_epi64(as<__m128i>(w_lo)),
                            _mm256_cvtepu32_epi64(_mm256_extracti128_si256(w_lo, 1)),
                            _mm256_cvtepu32_epi64(as<__m128i>(w_hi)),
                            _mm256_cvtepu32_epi64(_mm256_extracti128_si256(w_hi, 1)));
                }
                else return scalar_fallback_convert_<ISA, true, From, To>(x);
            }
            else if constexpr (i32_to_i64) {
                if constexpr (x_to_y && has_avx2_v) {
                    if constexpr (sgn_to_unsgn) return _mm256_cvtepu32_epi64(vertical_max_<ISA, int32_t>(x, _mm_setzero_si128()));
                    if constexpr (frm_sgn) return _mm256_cvtepi32_epi64(x);
                    else return _mm256_cvtepu32_epi64(x);
                }
                else if constexpr (y_to_z) {
                    if constexpr (has_zmm) {
                        if constexpr (sgn_to_unsgn) return _mm512_cvtepu32_epi64(vertical_max_<ISA, int32_t>(x, _mm256_setzero_si256()));
                        if constexpr (frm_sgn) return _mm512_cvtepi32_epi64(x);
                        else return _mm512_cvtepu32_epi64(x);
                    }
                    else if constexpr (has_avx2_v) {
                        if constexpr (sgn_to_unsgn) {
                            auto cx = vertical_max_<ISA, int32_t>(x, _mm256_setzero_si256());
                            return simd_tuple_make(_mm256_cvtepu32_epi64(as<__m128i>(cx)),
                                _mm256_cvtepu32_epi64(_mm256_extracti128_si256(cx, 1)));
                        }
                        if constexpr (frm_sgn) return simd_tuple_make(_mm256_cvtepi32_epi64(as<__m128i>(x)), _mm256_cvtepi32_epi64(_mm256_extracti128_si256(x, 1)));
                        else return simd_tuple_make(_mm256_cvtepu32_epi64(as<__m128i>(x)), _mm256_cvtepu32_epi64(_mm256_extracti128_si256(x, 1)));
                    }
                    else return scalar_fallback_convert_<ISA, true, From, To>(x);
                }
                else if constexpr (x_to_y && has_avx<ISA> && has_s41) {
                    __m128i e0, e1;

                    if constexpr (sgn_to_unsgn) {
                        auto cx = vertical_max_<ISA, int32_t>(x, _mm_setzero_si128());
                        auto lo = _mm_cvtepu32_epi64(cx);
                        auto hi = _mm_cvtepu32_epi64(_mm_srli_si128(cx, 8));
                        return _mm256_insertf128_si256(as<__m256i>(lo), hi, 1);
                    }
                    if constexpr (frm_sgn) {
                        e0 = _mm_cvtepi32_epi64(x);
                        e1 = _mm_cvtepi32_epi64(_mm_srli_si128(x, 8));
                    }
                    else {
                        e0 = _mm_cvtepu32_epi64(x);
                        e1 = _mm_cvtepu32_epi64(_mm_srli_si128(x, 8));
                    }

                    return _mm256_insertf128_si256(as<__m256i>(e0), e1, 1);
                }
                else if constexpr (x_to_y && has_s41) {
                    if constexpr (sgn_to_unsgn) {
                        auto cx = vertical_max_<ISA, int32_t>(x, _mm_setzero_si128());
                        return simd_tuple_make(_mm_cvtepu32_epi64(cx),
                            _mm_cvtepu32_epi64(_mm_srli_si128(cx, 8)));
                    }
                    if constexpr (frm_sgn) return simd_tuple_make(_mm_cvtepi32_epi64(x), _mm_cvtepi32_epi64(_mm_srli_si128(x, 8)));
                    else return simd_tuple_make(_mm_cvtepu32_epi64(x), _mm_cvtepu32_epi64(_mm_srli_si128(x, 8)));
                }
                else if constexpr (x_to_y) {
                    if constexpr (sgn_to_unsgn) {
                        auto cx = vertical_max_<ISA, int32_t>(x, _mm_setzero_si128());
                        return simd_tuple_make(_mm_unpacklo_epi32(cx, _mm_setzero_si128()),
                            _mm_unpackhi_epi32(cx, _mm_setzero_si128()));
                    }
                    if constexpr (frm_sgn) {
                        auto s0 = _mm_srai_epi32(x, 31);
                        return simd_tuple_make(_mm_unpacklo_epi32(x, s0), _mm_unpackhi_epi32(x, s0));
                    }
                    else {
                        return simd_tuple_make(_mm_unpacklo_epi32(x, _mm_setzero_si128()),
                            _mm_unpackhi_epi32(x, _mm_setzero_si128()));
                    }
                }
                else if constexpr (sizeof(R) == 128 && has_zmm) {
                    auto lo = as<__m256i>(x);
                    auto hi = _mm512_extracti64x4_epi64(x, 1);
                    if constexpr (sgn_to_unsgn)
                        return simd_tuple_make(_mm512_cvtepu32_epi64(vertical_max_<ISA, int32_t>(lo, _mm256_setzero_si256())),
                            _mm512_cvtepu32_epi64(vertical_max_<ISA, int32_t>(hi, _mm256_setzero_si256())));
                    if constexpr (frm_sgn) return simd_tuple_make(_mm512_cvtepi32_epi64(lo), _mm512_cvtepi32_epi64(hi));
                    else return simd_tuple_make(_mm512_cvtepu32_epi64(lo), _mm512_cvtepu32_epi64(hi));
                }
                else return scalar_fallback_convert_<ISA, true, From, To>(x);
            }
            else return scalar_fallback_convert_<ISA, true, From, To>(x);
        }
        else if constexpr (f_to_i) {
            return scalar_fallback_convert_<ISA, true, From, To>(x);
        }
        else if constexpr (i_to_f) {
            if constexpr (sizeof(To) == 4) {
                if constexpr (sizeof(From) == 1) {
                    if constexpr (x_to_z) {
                        if constexpr (has_zmm) {
                            if constexpr (frm_sgn) return _mm512_cvtepi32_ps(_mm512_cvtepi8_epi32(x));
                            else return _mm512_cvtepi32_ps(_mm512_cvtepu8_epi32(x));
                        }
                        else if constexpr (has_avx2_v) {
                            if constexpr (frm_sgn) 
                                return simd_tuple_make(_mm256_cvtepi32_ps(_mm256_cvtepi8_epi32(x)),
                                    _mm256_cvtepi32_ps(_mm256_cvtepi8_epi32(_mm_srli_si128(x, 8))));
                            else 
                                return simd_tuple_make(_mm256_cvtepi32_ps(_mm256_cvtepu8_epi32(x)),
                                    _mm256_cvtepi32_ps(_mm256_cvtepu8_epi32(_mm_srli_si128(x, 8))));
                        }
                        else if constexpr (has_avx<ISA> && has_s41) {
                            __m128i e0, e1, e2, e3;

                            if constexpr (frm_sgn) {
                                e0 = _mm_cvtepi8_epi32(x);
                                e1 = _mm_cvtepi8_epi32(_mm_srli_si128(x, 4));
                                e2 = _mm_cvtepi8_epi32(_mm_srli_si128(x, 8));
                                e3 = _mm_cvtepi8_epi32(_mm_srli_si128(x, 12));
                            }
                            else {
                                e0 = _mm_cvtepu8_epi32(x);
                                e1 = _mm_cvtepu8_epi32(_mm_srli_si128(x, 4));
                                e2 = _mm_cvtepu8_epi32(_mm_srli_si128(x, 8));
                                e3 = _mm_cvtepu8_epi32(_mm_srli_si128(x, 12));
                            }

                            return simd_tuple_make(_mm256_insertf128_ps(as<__m256>(_mm_cvtepi32_ps(e0)), _mm_cvtepi32_ps(e1), 1),
                                _mm256_insertf128_ps(as<__m256>(_mm_cvtepi32_ps(e2)), _mm_cvtepi32_ps(e3), 1));
                        }
                        else if constexpr (has_s41) {
                            if constexpr (frm_sgn) 
                                return simd_tuple_make(_mm_cvtepi32_ps(_mm_cvtepi8_epi32(x)),
                                    _mm_cvtepi32_ps(_mm_cvtepi8_epi32(_mm_srli_si128(x, 4))),
                                    _mm_cvtepi32_ps(_mm_cvtepi8_epi32(_mm_srli_si128(x, 8))),
                                    _mm_cvtepi32_ps(_mm_cvtepi8_epi32(_mm_srli_si128(x, 12))));
                            else 
                                return simd_tuple_make(_mm_cvtepi32_ps(_mm_cvtepu8_epi32(x)),
                                    _mm_cvtepi32_ps(_mm_cvtepu8_epi32(_mm_srli_si128(x, 4))),
                                    _mm_cvtepi32_ps(_mm_cvtepu8_epi32(_mm_srli_si128(x, 8))),
                                    _mm_cvtepi32_ps(_mm_cvtepu8_epi32(_mm_srli_si128(x, 12))));
                        }
                        else {
                            __m128i lo16, hi16;

                            if constexpr (frm_sgn) {
                                lo16 = _mm_srai_epi16(_mm_unpacklo_epi8(x, x), 8);
                                hi16 = _mm_srai_epi16(_mm_unpackhi_epi8(x, x), 8);
                            }
                            else {
                                __m128i zero = _mm_setzero_si128();
                                lo16 = _mm_unpacklo_epi8(x, zero);
                                hi16 = _mm_unpackhi_epi8(x, zero);
                            }

                            if constexpr (frm_sgn) 
                                return simd_tuple_make(_mm_cvtepi32_ps(_mm_srai_epi32(_mm_unpacklo_epi16(lo16, lo16), 16)),
                                    _mm_cvtepi32_ps(_mm_srai_epi32(_mm_unpackhi_epi16(lo16, lo16), 16)),
                                    _mm_cvtepi32_ps(_mm_srai_epi32(_mm_unpacklo_epi16(hi16, hi16), 16)),
                                    _mm_cvtepi32_ps(_mm_srai_epi32(_mm_unpackhi_epi16(hi16, hi16), 16)));
                            else
                                return simd_tuple_make(_mm_cvtepi32_ps(_mm_unpacklo_epi16(lo16, _mm_setzero_si128())),
                                    _mm_cvtepi32_ps(_mm_unpackhi_epi16(lo16, _mm_setzero_si128())),
                                    _mm_cvtepi32_ps(_mm_unpacklo_epi16(hi16, _mm_setzero_si128())),
                                    _mm_cvtepi32_ps(_mm_unpackhi_epi16(hi16, _mm_setzero_si128())));
                        }
                    }
                    else if constexpr (sizeof(R) == 128 && has_zmm) {
                        auto lo = as<__m128i>(x);
                        auto hi = _mm256_extracti128_si256(x, 1);

                        if constexpr (frm_sgn) 
                            return simd_tuple_make(_mm512_cvtepi32_ps(_mm512_cvtepi8_epi32(lo)),
                                _mm512_cvtepi32_ps(_mm512_cvtepi8_epi32(hi)));
                        else 
                            return simd_tuple_make(_mm512_cvtepi32_ps(_mm512_cvtepu8_epi32(lo)),
                                _mm512_cvtepi32_ps(_mm512_cvtepu8_epi32(hi)));
                    }
                    else if constexpr (sizeof(R) == 128 && has_avx2_v) {
                        auto lo = as<__m128i>(x);
                        auto hi = _mm256_extracti128_si256(x, 1);

                        if constexpr (frm_sgn) 
                            return simd_tuple_make(_mm256_cvtepi32_ps(_mm256_cvtepi8_epi32(lo)),
                                _mm256_cvtepi32_ps(_mm256_cvtepi8_epi32(_mm_srli_si128(lo, 8))),
                                _mm256_cvtepi32_ps(_mm256_cvtepi8_epi32(hi)),
                                _mm256_cvtepi32_ps(_mm256_cvtepi8_epi32(_mm_srli_si128(hi, 8))));
                        else 
                            return simd_tuple_make(_mm256_cvtepi32_ps(_mm256_cvtepu8_epi32(lo)),
                                _mm256_cvtepi32_ps(_mm256_cvtepu8_epi32(_mm_srli_si128(lo, 8))),
                                _mm256_cvtepi32_ps(_mm256_cvtepu8_epi32(hi)),
                                _mm256_cvtepi32_ps(_mm256_cvtepu8_epi32(_mm_srli_si128(hi, 8))));
                    }
                    else return scalar_fallback_convert_<ISA, true, From, To>(x);
                }
                else if constexpr (sizeof(From) == 2) {
                    if constexpr (x_to_y && has_avx2_v) {
                        if constexpr (frm_sgn) return _mm256_cvtepi32_ps(_mm256_cvtepi16_epi32(x));
                        else return _mm256_cvtepi32_ps(_mm256_cvtepu16_epi32(x));
                    }
                    else if constexpr (x_to_y && has_avx<ISA> && has_s41) {
                        __m128i e0, e1;

                        if constexpr (frm_sgn) {
                            e0 = _mm_cvtepi16_epi32(x);
                            e1 = _mm_cvtepi16_epi32(_mm_srli_si128(x, 8));
                        }
                        else {
                            e0 = _mm_cvtepu16_epi32(x);
                            e1 = _mm_cvtepu16_epi32(_mm_srli_si128(x, 8));
                        }

                        return _mm256_insertf128_ps(as<__m256>(_mm_cvtepi32_ps(e0)), _mm_cvtepi32_ps(e1), 1);
                    }
                    else if constexpr (x_to_y && has_s41) {
                        if constexpr (frm_sgn) 
                            return simd_tuple_make(_mm_cvtepi32_ps(_mm_cvtepi16_epi32(x)),
                                _mm_cvtepi32_ps(_mm_cvtepi16_epi32(_mm_srli_si128(x, 8))));
                        else 
                            return simd_tuple_make(_mm_cvtepi32_ps(_mm_cvtepu16_epi32(x)),
                                _mm_cvtepi32_ps(_mm_cvtepu16_epi32(_mm_srli_si128(x, 8))));
                    }
                    else if constexpr (y_to_z) {
                        if constexpr (has_zmm) {
                            if constexpr (frm_sgn) return _mm512_cvtepi32_ps(_mm512_cvtepi16_epi32(x));
                            else return _mm512_cvtepi32_ps(_mm512_cvtepu16_epi32(x));
                        }
                        else if constexpr (has_avx2_v) {
                            if constexpr (frm_sgn)
                                return simd_tuple_make(_mm256_cvtepi32_ps(_mm256_cvtepi16_epi32(as<__m128i>(x))),
                                    _mm256_cvtepi32_ps(_mm256_cvtepi16_epi32(_mm256_extracti128_si256(x, 1))));
                            else 
                                return simd_tuple_make(_mm256_cvtepi32_ps(_mm256_cvtepu16_epi32(as<__m128i>(x))),
                                    _mm256_cvtepi32_ps(_mm256_cvtepu16_epi32(_mm256_extracti128_si256(x, 1))));
                        }
                        else return scalar_fallback_convert_<ISA, true, From, To>(x);
                    }
                    else if constexpr (x_to_y) {
                        if constexpr (frm_sgn) 
                            return simd_tuple_make(_mm_cvtepi32_ps(_mm_srai_epi32(_mm_unpacklo_epi16(x, x), 16)),
                                _mm_cvtepi32_ps(_mm_srai_epi32(_mm_unpackhi_epi16(x, x), 16)));
                        else
                            return simd_tuple_make(_mm_cvtepi32_ps(_mm_unpacklo_epi16(x, _mm_setzero_si128())),
                                _mm_cvtepi32_ps(_mm_unpackhi_epi16(x, _mm_setzero_si128())));
                    }
                    else if constexpr (sizeof(R) == 128 && has_zmm) {
                        if constexpr (frm_sgn) 
                            return simd_tuple_make(_mm512_cvtepi32_ps(_mm512_cvtepi16_epi32(as<__m256i>(x))),
                                _mm512_cvtepi32_ps(_mm512_cvtepi16_epi32(_mm512_extracti64x4_epi64(x, 1))));
                        else 
                            return simd_tuple_make(_mm512_cvtepi32_ps(_mm512_cvtepu16_epi32(as<__m256i>(x))),
                                _mm512_cvtepi32_ps(_mm512_cvtepu16_epi32(_mm512_extracti64x4_epi64(x, 1))));
                    }
                    else return scalar_fallback_convert_<ISA, true, From, To>(x);
                }
                else if constexpr (sizeof(From) == 4) {
                    if constexpr (frm_sgn) {
                        if constexpr (x_to_x) return _mm_cvtepi32_ps(x);
                        else if constexpr (y_to_y && has_ymm_v) return _mm256_cvtepi32_ps(x);
                        else if constexpr (z_to_z && has_zmm)   return _mm512_cvtepi32_ps(x);
                        else return scalar_fallback_convert_<ISA, true, From, To>(x);
                    }
                    else {
                        if constexpr (x_to_x && has_zmm && has_vl) return _mm_cvtepu32_ps(x);
                        else if constexpr (y_to_y && has_zmm && has_vl) return _mm256_cvtepu32_ps(x);
                        else if constexpr (z_to_z && has_zmm) return _mm512_cvtepu32_ps(x);
                        else return scalar_fallback_convert_<ISA, true, From, To>(x);
                    }
                }
                else if constexpr (sizeof(From) == 8) {
                    if constexpr (x_to_x && has_dqvl) {
                        if constexpr (frm_sgn) return _mm_cvtepi64_ps(x);
                        else return _mm_cvtepu64_ps(x);
                    }
                    else if constexpr (y_to_x && has_dqvl) {
                        if constexpr (frm_sgn) return _mm256_cvtepi64_ps(x);
                        else return _mm256_cvtepu64_ps(x);
                    }
                    else if constexpr (z_to_y && has_dq) {
                        if constexpr (frm_sgn) return _mm512_cvtepi64_ps(x);
                        else return _mm512_cvtepu64_ps(x);
                    }
                    else return scalar_fallback_convert_<ISA, true, From, To>(x);
                }
                else return scalar_fallback_convert_<ISA, true, From, To>(x);
            }
            else if constexpr (sizeof(To) == 8) {
                if constexpr (sizeof(From) == 1) {
                    if constexpr (sizeof(R) == 128 && has_zmm) {
                        __m512i e32;
                        if constexpr (frm_sgn) e32 = _mm512_cvtepi8_epi32(x);
                        else e32 = _mm512_cvtepu8_epi32(x);

                        return simd_tuple_make(_mm512_cvtepi32_pd(as<__m256i>(e32)),
                            _mm512_cvtepi32_pd(_mm512_extracti64x4_epi64(e32, 1)));
                    }
                    else if constexpr (sizeof(R) == 128 && has_avx2_v) {
                        __m256i lo32, hi32;

                        if constexpr (frm_sgn) {
                            lo32 = _mm256_cvtepi8_epi32(x);
                            hi32 = _mm256_cvtepi8_epi32(_mm_srli_si128(x, 8));
                        }
                        else {
                            lo32 = _mm256_cvtepu8_epi32(x);
                            hi32 = _mm256_cvtepu8_epi32(_mm_srli_si128(x, 8));
                        }

                        return simd_tuple_make(_mm256_cvtepi32_pd(as<__m128i>(lo32)),
                            _mm256_cvtepi32_pd(_mm256_extracti128_si256(lo32, 1)),
                            _mm256_cvtepi32_pd(as<__m128i>(hi32)),
                            _mm256_cvtepi32_pd(_mm256_extracti128_si256(hi32, 1)));
                    }
                    else if constexpr (sizeof(V) == 16 && sizeof(R) == 128 && has_avx<ISA> && has_s41) {
                        __m128i e0, e1, e2, e3;

                        if constexpr (frm_sgn) {
                            e0 = _mm_cvtepi8_epi32(x);
                            e1 = _mm_cvtepi8_epi32(_mm_srli_si128(x, 4));
                            e2 = _mm_cvtepi8_epi32(_mm_srli_si128(x, 8));
                            e3 = _mm_cvtepi8_epi32(_mm_srli_si128(x, 12));
                        }
                        else {
                            e0 = _mm_cvtepu8_epi32(x);
                            e1 = _mm_cvtepu8_epi32(_mm_srli_si128(x, 4));
                            e2 = _mm_cvtepu8_epi32(_mm_srli_si128(x, 8));
                            e3 = _mm_cvtepu8_epi32(_mm_srli_si128(x, 12));
                        }

                        return simd_tuple_make(_mm256_cvtepi32_pd(e0), _mm256_cvtepi32_pd(e1),
                            _mm256_cvtepi32_pd(e2), _mm256_cvtepi32_pd(e3));
                    }
                    else if constexpr (sizeof(V) == 16 && sizeof(R) == 128 && has_s41) {
                        __m128i e0, e1, e2, e3;

                        if constexpr (frm_sgn) {
                            e0 = _mm_cvtepi8_epi32(x);
                            e1 = _mm_cvtepi8_epi32(_mm_srli_si128(x, 4));
                            e2 = _mm_cvtepi8_epi32(_mm_srli_si128(x, 8));
                            e3 = _mm_cvtepi8_epi32(_mm_srli_si128(x, 12));
                        }
                        else {
                            e0 = _mm_cvtepu8_epi32(x);
                            e1 = _mm_cvtepu8_epi32(_mm_srli_si128(x, 4));
                            e2 = _mm_cvtepu8_epi32(_mm_srli_si128(x, 8));
                            e3 = _mm_cvtepu8_epi32(_mm_srli_si128(x, 12));
                        }

                        return simd_tuple_make(_mm_cvtepi32_pd(e0), _mm_cvtepi32_pd(_mm_srli_si128(e0, 8)),
                            _mm_cvtepi32_pd(e1), _mm_cvtepi32_pd(_mm_srli_si128(e1, 8)),
                            _mm_cvtepi32_pd(e2), _mm_cvtepi32_pd(_mm_srli_si128(e2, 8)),
                            _mm_cvtepi32_pd(e3), _mm_cvtepi32_pd(_mm_srli_si128(e3, 8)));
                    }
                    else if constexpr (sizeof(V) == 16 && sizeof(R) == 128 && has_sse2<ISA>) {
                        __m128i lo16, hi16;
                        __m128i e0, e1, e2, e3;

                        if constexpr (frm_sgn) {
                            lo16 = _mm_srai_epi16(_mm_unpacklo_epi8(x, x), 8);
                            hi16 = _mm_srai_epi16(_mm_unpackhi_epi8(x, x), 8);
                        }
                        else {
                            lo16 = _mm_unpacklo_epi8(x, _mm_setzero_si128());
                            hi16 = _mm_unpackhi_epi8(x, _mm_setzero_si128());
                        }

                        if constexpr (frm_sgn) {
                            e0 = _mm_srai_epi32(_mm_unpacklo_epi16(lo16, lo16), 16);
                            e1 = _mm_srai_epi32(_mm_unpackhi_epi16(lo16, lo16), 16);
                            e2 = _mm_srai_epi32(_mm_unpacklo_epi16(hi16, hi16), 16);
                            e3 = _mm_srai_epi32(_mm_unpackhi_epi16(hi16, hi16), 16);
                        }
                        else {
                            e0 = _mm_unpacklo_epi16(lo16, _mm_setzero_si128());
                            e1 = _mm_unpackhi_epi16(lo16, _mm_setzero_si128());
                            e2 = _mm_unpacklo_epi16(hi16, _mm_setzero_si128());
                            e3 = _mm_unpackhi_epi16(hi16, _mm_setzero_si128());
                        }

                        return simd_tuple_make(_mm_cvtepi32_pd(e0), _mm_cvtepi32_pd(_mm_srli_si128(e0, 8)),
                            _mm_cvtepi32_pd(e1), _mm_cvtepi32_pd(_mm_srli_si128(e1, 8)),
                            _mm_cvtepi32_pd(e2), _mm_cvtepi32_pd(_mm_srli_si128(e2, 8)),
                            _mm_cvtepi32_pd(e3), _mm_cvtepi32_pd(_mm_srli_si128(e3, 8)));
                    }
                    else return scalar_fallback_convert_<ISA, true, From, To>(x);
                }
                else if constexpr (sizeof(From) == 2) {
                    if constexpr (x_to_z) {
                        if constexpr (has_zmm) {
                            if constexpr (frm_sgn) return _mm512_cvtepi32_pd(_mm256_cvtepi16_epi32(x));
                            else return _mm512_cvtepu32_pd(_mm256_cvtepu16_epi32(x));
                        }
                        else if constexpr (has_avx2_v) {
                            __m256i w32;

                            if constexpr (frm_sgn) w32 = _mm256_cvtepi16_epi32(x);
                            else w32 = _mm256_cvtepu16_epi32(x);

                            return simd_tuple_make(_mm256_cvtepi32_pd(as<__m128i>(w32)),
                                _mm256_cvtepi32_pd(_mm256_extracti128_si256(w32, 1)));
                        }
                        else if constexpr (sizeof(V) == 16 && has_avx<ISA> && has_s41) {
                            __m128i w0, w1;

                            if constexpr (frm_sgn) {
                                w0 = _mm_cvtepi16_epi32(x);
                                w1 = _mm_cvtepi16_epi32(_mm_srli_si128(x, 8));
                            }
                            else {
                                w0 = _mm_cvtepu16_epi32(x);
                                w1 = _mm_cvtepu16_epi32(_mm_srli_si128(x, 8));
                            }

                            return simd_tuple_make(_mm256_cvtepi32_pd(w0), _mm256_cvtepi32_pd(w1));
                        }
                        else if constexpr (sizeof(V) == 16 && has_s41) {
                            __m128i w0, w1;

                            if constexpr (frm_sgn) {
                                w0 = _mm_cvtepi16_epi32(x);
                                w1 = _mm_cvtepi16_epi32(_mm_srli_si128(x, 8));
                            }
                            else {
                                w0 = _mm_cvtepu16_epi32(x);
                                w1 = _mm_cvtepu16_epi32(_mm_srli_si128(x, 8));
                            }

                            return simd_tuple_make(_mm_cvtepi32_pd(w0), _mm_cvtepi32_pd(_mm_srli_si128(w0, 8)),
                                _mm_cvtepi32_pd(w1), _mm_cvtepi32_pd(_mm_srli_si128(w1, 8)));
                        }
                        else if constexpr (sizeof(V) == 16 && has_sse2<ISA>) {
                            __m128i w0, w1;

                            if constexpr (frm_sgn) {
                                w0 = _mm_srai_epi32(_mm_unpacklo_epi16(x, x), 16);
                                w1 = _mm_srai_epi32(_mm_unpackhi_epi16(x, x), 16);
                            }
                            else {
                                __m128i zero = _mm_setzero_si128();
                                w0 = _mm_unpacklo_epi16(x, zero);
                                w1 = _mm_unpackhi_epi16(x, zero);
                            }

                            return simd_tuple_make(_mm_cvtepi32_pd(w0), _mm_cvtepi32_pd(_mm_srli_si128(w0, 8)),
                                _mm_cvtepi32_pd(w1), _mm_cvtepi32_pd(_mm_srli_si128(w1, 8)));
                        }
                        else return scalar_fallback_convert_<ISA, true, From, To>(x);
                    }
                    else if constexpr (sizeof(R) == 128 && sizeof(V) == 32 && has_zmm) {
                        __m512i e32;

                        if constexpr (frm_sgn) e32 = _mm512_cvtepi16_epi32(x);
                        else e32 = _mm512_cvtepu16_epi32(x);
                        
                        return simd_tuple_make(_mm512_cvtepi32_pd(as<__m256i>(e32)), _mm512_cvtepi32_pd(_mm512_extracti64x4_epi64(e32, 1)));
                    }
                    else if constexpr (sizeof(R) == 128 && has_avx2_v) {
                        __m256i w_lo, w_hi;
                        
                        if constexpr (frm_sgn) {
                            w_lo = _mm256_cvtepi16_epi32(as<__m128i>(x));
                            w_hi = _mm256_cvtepi16_epi32(_mm256_extracti128_si256(x, 1));
                        }
                        else {
                            w_lo = _mm256_cvtepu16_epi32(as<__m128i>(x));
                            w_hi = _mm256_cvtepu16_epi32(_mm256_extracti128_si256(x, 1));
                        }

                        return simd_tuple_make(_mm256_cvtepi32_pd(as<__m128i>(w_lo)), _mm256_cvtepi32_pd(_mm256_extracti128_si256(w_lo, 1)),
                            _mm256_cvtepi32_pd(as<__m128i>(w_hi)), _mm256_cvtepi32_pd(_mm256_extracti128_si256(w_hi, 1)));
                    }
                    else return scalar_fallback_convert_<ISA, true, From, To>(x);
                }
                else if constexpr (sizeof(From) == 4) {
                    if constexpr (frm_sgn) {
                        if constexpr (x_to_y && has_ymm_v) return _mm256_cvtepi32_pd(x);
                        else if constexpr (y_to_z) {
                            if constexpr (has_zmm) return _mm512_cvtepi32_pd(x);
                            else if constexpr (has_avx2_v) return simd_tuple_make(_mm256_cvtepi32_pd(
                                as<__m128i>(x)), _mm256_cvtepi32_pd(_mm256_extracti128_si256(x, 1)));
                            else return scalar_fallback_convert_<ISA, true, From, To>(x);
                        }
                        else if constexpr (sizeof(V) == 64 && sizeof(R) == 128 && has_zmm) return simd_tuple_make(_mm512_cvtepi32_pd(
                            as<__m256i>(x)), _mm512_cvtepi32_pd(_mm512_extracti64x4_epi64(x, 1)));
                        else return scalar_fallback_convert_<ISA, true, From, To>(x);
                    }
                    else {
                        if constexpr (sizeof(V) == 64 && sizeof(R) == 128 && has_zmm) return simd_tuple_make(_mm512_cvtepu32_pd(as<__m256i>(x)),
                            _mm512_cvtepu32_pd(_mm512_extracti64x4_epi64(x, 1)));
                        else if constexpr (x_to_y && has_zmm && has_vl) return _mm256_cvtepu32_pd(x);
                        else if constexpr (y_to_z && has_zmm) return _mm512_cvtepu32_pd(x);
                        else return scalar_fallback_convert_<ISA, true, From, To>(x);
                    }
                }
                else if constexpr (sizeof(From) == 8) {
                    if constexpr (x_to_x && has_dqvl) {
                        if constexpr (frm_sgn) return _mm_cvtepi64_pd(x);
                        else return _mm_cvtepu64_pd(x);
                    }
                    else if constexpr (y_to_y && has_dqvl) {
                        if constexpr (frm_sgn) return _mm256_cvtepi64_pd(x);
                        else return _mm256_cvtepu64_pd(x);
                    }
                    else if constexpr (z_to_z && has_dq) {
                        if constexpr (frm_sgn) return _mm512_cvtepi64_pd(x);
                        else return _mm512_cvtepu64_pd(x);
                    }
                    else return scalar_fallback_convert_<ISA, true, From, To>(x);
                }
                else return scalar_fallback_convert_<ISA, true, From, To>(x);
            }
            else return scalar_fallback_convert_<ISA, true, From, To>(x);
        }
        else if constexpr (f_to_f) {
            return scalar_fallback_convert_<ISA, true, From, To>(x);
        }
        else return scalar_fallback_convert_<ISA, true, From, To>(x);
    }
}

__RAZE_VX_NAMESPACE_END