#pragma once

#include <src/raze/vx/hw/x86/memory/Load.h>
#include <src/raze/vx/hw/x86/memory/Store.h>
#include <tuple>
#include <utility>

__RAZE_VX_NAMESPACE_BEGIN

template <class T>
struct is_tuple_ : std::false_type {};

template <class... Ts>
struct is_tuple_<std::tuple<Ts...>> : std::true_type {};

template <sizetype I, class V>
using identity_v_ = V;

template <arch::ISA ISA, class From, class To, class V>
consteval auto deduce_convert_return_type_() {
    if constexpr (sizeof(V) < 16) return To{};
    else if constexpr (sizeof(From) == sizeof(To)) return traits::deduce_simd_vector_type<To, sizeof(V) * 8>{};
    else {
        constexpr sizetype total_bytes = sizeof(V) / sizeof(From) * sizeof(To);
        if constexpr (total_bytes <= 16) return traits::deduce_simd_vector_type<To, 128>{};
        else if constexpr (total_bytes <= 32) {
            if constexpr (has_avx<ISA>) return traits::deduce_simd_vector_type<To, 256>{};
            else return std::tuple<traits::deduce_simd_vector_type<To, 128>, traits::deduce_simd_vector_type<To, 128>>{};
        }
        else if constexpr (total_bytes <= 64) {
            if constexpr (has_avx512f<ISA>) return traits::deduce_simd_vector_type<To, 512>{};
            else if constexpr (has_avx<ISA>) return std::tuple<traits::deduce_simd_vector_type<To, 256>, traits::deduce_simd_vector_type<To, 256>>{};
            else return std::tuple<traits::deduce_simd_vector_type<To, 128>, traits::deduce_simd_vector_type<To, 128>,
                traits::deduce_simd_vector_type<To, 128>, traits::deduce_simd_vector_type<To, 128>>{};
        }
        else {
            constexpr sizetype chunk_size = has_avx512f<ISA> ? 64 : (has_avx<ISA> ? 32 : 16);
            constexpr sizetype num_chunks = total_bytes / chunk_size;
            return[] <sizetype... E> (std::integer_sequence<sizetype, E...>) {
                return std::tuple{ identity_v_<E, traits::deduce_simd_vector_type<To, chunk_size * 8>>{}... };
            }(std::make_integer_sequence<sizetype, num_chunks>{});
        }
    }
}

template <arch::ISA ISA, class From, class To, class V>
using deduce_convert_return_type = decltype(deduce_convert_return_type_<ISA, From, To, V>());

template <arch::ISA ISA, class Part>
raze_always_inline auto pack_or_tuple(Part p0, Part p1) noexcept {
    if constexpr (has_avx<ISA> && sizeof(Part) == 16) {
        if constexpr (std::same_as<Part, __m128>) return _mm256_insertf128_ps(_mm256_castps128_ps256(p0), p1, 1);
        else if constexpr (std::same_as<Part, __m128d>) return _mm256_insertf128_pd(_mm256_castpd128_pd256(p0), p1, 1);
        else return _mm256_insertf128_si256(_mm256_castsi128_si256(p0), p1, 1);
    }
    else if constexpr (has_avx512f<ISA> && sizeof(Part) == 32) {
        if constexpr (std::same_as<Part, __m256>) return _mm512_insertf32x8(_mm512_castps256_ps512(p0), p1, 1);
        else if constexpr (std::same_as<Part, __m256d>) return _mm512_insertf64x4(_mm512_castpd256_pd512(p0), p1, 1);
        else return _mm512_inserti64x4(_mm512_castsi256_si512(p0), p1, 1);
    }
    else return std::tuple{ p0, p1 };
}

template <arch::ISA ISA, class Part>
raze_always_inline auto pack_or_tuple(Part p0, Part p1, Part p2, Part p3) noexcept {
    if constexpr (has_avx512f<ISA> && sizeof(Part) == 16) {
        if constexpr (std::same_as<Part, __m128>) {
            auto lo = _mm256_insertf128_ps(_mm256_castps128_ps256(p0), p1, 1);
            auto hi = _mm256_insertf128_ps(_mm256_castps128_ps256(p2), p3, 1);
            return _mm512_insertf32x8(_mm512_castps256_ps512(lo), hi, 1);
        }
        else if constexpr (std::same_as<Part, __m128d>) {
            auto lo = _mm256_insertf128_pd(_mm256_castpd128_pd256(p0), p1, 1);
            auto hi = _mm256_insertf128_pd(_mm256_castpd128_pd256(p2), p3, 1);
            return _mm512_insertf64x4(_mm512_castpd256_pd512(lo), hi, 1);
        }
        else {
            auto lo = _mm256_insertf128_si256(_mm256_castsi128_si256(p0), p1, 1);
            auto hi = _mm256_insertf128_si256(_mm256_castsi128_si256(p2), p3, 1);
            return _mm512_inserti64x4(_mm512_castsi256_si512(lo), hi, 1);
        }
    }
    else if constexpr (has_avx<ISA> && sizeof(Part) == 16) {
        if constexpr (std::same_as<Part, __m128>) return std::tuple{ _mm256_insertf128_ps(_mm256_castps128_ps256(p0), p1, 1), _mm256_insertf128_ps(_mm256_castps128_ps256(p2), p3, 1) };
        else if constexpr (std::same_as<Part, __m128d>) return std::tuple{ _mm256_insertf128_pd(_mm256_castpd128_pd256(p0), p1, 1), _mm256_insertf128_pd(_mm256_castpd128_pd256(p2), p3, 1) };
        else return std::tuple{ _mm256_insertf128_si256(_mm256_castsi128_si256(p0), p1, 1), _mm256_insertf128_si256(_mm256_castsi128_si256(p2), p3, 1) };
    }
    else return std::tuple{ p0, p1, p2, p3 };
}

template <bool Saturated, class To, class From>
raze_always_inline To scalar_narrow_(From v) noexcept {
    if constexpr (!Saturated) return static_cast<To>(v);
    else if constexpr (std::is_floating_point_v<From>) {
        long double d = static_cast<long double>(v);
        long double mn = static_cast<long double>(std::numeric_limits<To>::lowest());
        long double mx = static_cast<long double>(std::numeric_limits<To>::max());
        if (!(d == d)) return To{};
        if (d < mn) return static_cast<To>(mn);
        if (d > mx) return static_cast<To>(mx);
        return static_cast<To>(d);
    }
    else {
        using Wide = std::conditional_t<std::is_signed_v<From>, int64_t, uint64_t>;
        Wide w = static_cast<Wide>(v);
        Wide mn = static_cast<Wide>(std::numeric_limits<To>::lowest());
        Wide mx = static_cast<Wide>(std::numeric_limits<To>::max());
        if (w < mn) return std::numeric_limits<To>::lowest();
        if (w > mx) return std::numeric_limits<To>::max();
        return static_cast<To>(w);
    }
}

template <arch::ISA ISA, class Target, class To, sizetype N>
raze_always_inline Target load_target_from_buffer_(const To* buf) noexcept {
    if constexpr (is_tuple_<Target>::value) {
        return[] <sizetype... Is> (const To * b, std::index_sequence<Is...>) {
            using FirstChunk = std::tuple_element_t<0, Target>;
            constexpr sizetype step = sizeof(FirstChunk) / sizeof(To);
            return Target{ load_<ISA, std::tuple_element_t<Is, Target>>(b + Is * step)... };
        }(buf, std::make_index_sequence<std::tuple_size_v<Target>>{});
    }
    else return load_<ISA, Target>(buf);
}

template <arch::ISA ISA, bool Saturated, class From, class To, class V>
raze_always_inline deduce_convert_return_type<ISA, From, To, V> scalar_fallback_convert_(V x) noexcept {
    constexpr sizetype count = sizeof(V) / sizeof(From);
    alignas(64) From arr[count];
    store_(arr, x);
    using Ret = deduce_convert_return_type<ISA, From, To, V>;
    constexpr sizetype ret_bytes = sizeof(Ret);
    constexpr sizetype out_count = (ret_bytes > count * sizeof(To)) ? (ret_bytes / sizeof(To)) : count;
    alignas(64) To out[out_count]{};
    for (sizetype i = 0; i < count; ++i) out[i] = scalar_narrow_<Saturated, To, From>(arr[i]);
    return load_target_from_buffer_<ISA, Ret, To, count>(out);
}

template <arch::ISA ISA, bool Saturated, arithmetic_type From, arithmetic_type To, intrin_or_arithmetic_type V>
raze_always_inline deduce_convert_return_type<ISA, From, To, V> convert_(V x) noexcept {
    using R = deduce_convert_return_type<ISA, From, To, V>;

    if constexpr (std::same_as<To, From>) {
        return x;
    }
    else if constexpr (sizeof(V) < 16) {
        return scalar_fallback_convert_<ISA, Saturated, From, To>(x);
    }
    else if constexpr (sizeof(R) > 128 && sizeof(V) >= 32) {
        if constexpr (sizeof(V) == 32 && has_avx<ISA>) {
            if constexpr (std::same_as<V, __m256d>) {
                auto r_lo = convert_<ISA, Saturated, From, To>(_mm256_castpd256_pd128(x));
                auto r_hi = convert_<ISA, Saturated, From, To>(_mm256_extractf128_pd(x, 1));
                return std::tuple_cat(std::move(r_lo), std::move(r_hi));
            }
            else if constexpr (std::same_as<V, __m256>) {
                auto r_lo = convert_<ISA, Saturated, From, To>(_mm256_castps256_ps128(x));
                auto r_hi = convert_<ISA, Saturated, From, To>(_mm256_extractf128_ps(x, 1));
                return std::tuple_cat(std::move(r_lo), std::move(r_hi));
            }
            else if constexpr (has_avx2<ISA>) {
                auto r_lo = convert_<ISA, Saturated, From, To>(_mm256_castsi256_si128(x));
                auto r_hi = convert_<ISA, Saturated, From, To>(_mm256_extracti128_si256(x, 1));
                return std::tuple_cat(std::move(r_lo), std::move(r_hi));
            }
            else if constexpr (has_avx<ISA>) {
                auto r_lo = convert_<ISA, Saturated, From, To>(_mm256_castsi256_si128(x));
                auto r_hi = convert_<ISA, Saturated, From, To>(_mm256_extractf128_si256(x, 1));
                return std::tuple_cat(std::move(r_lo), std::move(r_hi));
            }
            else return scalar_fallback_convert_<ISA, Saturated, From, To>(x);
        }
        else if constexpr (sizeof(V) == 64 && has_avx512f<ISA>) {
            if constexpr (std::same_as<V, __m512d>) {
                auto r_lo = convert_<ISA, Saturated, From, To>(_mm512_castpd512_pd256(x));
                auto r_hi = convert_<ISA, Saturated, From, To>(_mm512_extractf64x4_pd(x, 1));
                return std::tuple_cat(std::move(r_lo), std::move(r_hi));
            }
            else if constexpr (std::same_as<V, __m512>) {
                auto r_lo = convert_<ISA, Saturated, From, To>(_mm512_castps512_ps256(x));
                auto r_hi = convert_<ISA, Saturated, From, To>(_mm512_extractf32x8_ps(x, 1));
                return std::tuple_cat(std::move(r_lo), std::move(r_hi));
            }
            else {
                auto r_lo = convert_<ISA, Saturated, From, To>(_mm512_castsi512_si256(x));
                auto r_hi = convert_<ISA, Saturated, From, To>(_mm512_extracti64x4_epi64(x, 1));
                return std::tuple_cat(std::move(r_lo), std::move(r_hi));
            }
        }
        else return scalar_fallback_convert_<ISA, Saturated, From, To>(x);
    }
    else if constexpr (sizeof(R) == 128 && sizeof(V) == 32 && has_avx<ISA> && !has_avx2<ISA> && !has_avx512f<ISA>) {
        if constexpr (std::same_as<V, __m256d>) {
            auto r_lo = convert_<ISA, Saturated, From, To>(_mm256_castpd256_pd128(x));
            auto r_hi = convert_<ISA, Saturated, From, To>(_mm256_extractf128_pd(x, 1));
            return std::tuple_cat(std::move(r_lo), std::move(r_hi));
        }
        else if constexpr (std::same_as<V, __m256>) {
            auto r_lo = convert_<ISA, Saturated, From, To>(_mm256_castps256_ps128(x));
            auto r_hi = convert_<ISA, Saturated, From, To>(_mm256_extractf128_ps(x, 1));
            return std::tuple_cat(std::move(r_lo), std::move(r_hi));
        }
        else {
            auto r_lo = convert_<ISA, Saturated, From, To>(_mm256_castsi256_si128(x));
            auto r_hi = convert_<ISA, Saturated, From, To>(_mm256_extractf128_si256(x, 1));
            return std::tuple_cat(std::move(r_lo), std::move(r_hi));
        }
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
                    auto lo = _mm256_castsi256_si128(x);
                    auto hi = _mm256_extracti128_si256(x, 1);
                    if constexpr (frm_sgn) return pack_or_tuple<ISA>(_mm256_cvtepi8_epi16(lo), _mm256_cvtepi8_epi16(hi));
                    else return pack_or_tuple<ISA>(_mm256_cvtepu8_epi16(lo), _mm256_cvtepu8_epi16(hi));
                }
                else if constexpr (sizeof(R) == 128 && has_bw) {
                    auto x0 = _mm512_castsi512_si128(x);
                    auto x1 = _mm512_extracti32x4_epi32(x, 1);
                    auto x2 = _mm512_extracti32x4_epi32(x, 2);
                    auto x3 = _mm512_extracti32x4_epi32(x, 3);
                    if constexpr (frm_sgn) return std::tuple{
                        _mm512_cvtepi8_epi16(_mm256_inserti128_si256(_mm256_castsi128_si256(x0), x1, 1)),
                        _mm512_cvtepi8_epi16(_mm256_inserti128_si256(_mm256_castsi128_si256(x2), x3, 1)) };
                    else return std::tuple{
                        _mm512_cvtepu8_epi16(_mm256_inserti128_si256(_mm256_castsi128_si256(x0), x1, 1)),
                        _mm512_cvtepu8_epi16(_mm256_inserti128_si256(_mm256_castsi128_si256(x2), x3, 1)) };
                }
                else return scalar_fallback_convert_<ISA, Saturated, From, To>(x);
            }
            else if constexpr (i8_to_i32) {
                if constexpr (x_to_z) {
                    if constexpr (has_zmm) {
                        if constexpr (frm_sgn) return _mm512_cvtepi8_epi32(x);
                        else return _mm512_cvtepu8_epi32(x);
                    }
                    else if constexpr (has_avx2_v) {
                        if constexpr (frm_sgn) return std::tuple{
                            _mm256_cvtepi8_epi32(x),
                            _mm256_cvtepi8_epi32(_mm_srli_si128(x, 8)) };
                        else return std::tuple{
                            _mm256_cvtepu8_epi32(x),
                            _mm256_cvtepu8_epi32(_mm_srli_si128(x, 8)) };
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
                        return std::tuple{
                            _mm256_insertf128_si256(_mm256_castsi128_si256(e0), e1, 1),
                            _mm256_insertf128_si256(_mm256_castsi128_si256(e2), e3, 1) };
                    }
                    else if constexpr (has_s41) {
                        if constexpr (frm_sgn) return std::tuple{
                            _mm_cvtepi8_epi32(x),
                            _mm_cvtepi8_epi32(_mm_srli_si128(x, 4)),
                            _mm_cvtepi8_epi32(_mm_srli_si128(x, 8)),
                            _mm_cvtepi8_epi32(_mm_srli_si128(x, 12)) };
                        else return std::tuple{
                            _mm_cvtepu8_epi32(x),
                            _mm_cvtepu8_epi32(_mm_srli_si128(x, 4)),
                            _mm_cvtepu8_epi32(_mm_srli_si128(x, 8)),
                            _mm_cvtepu8_epi32(_mm_srli_si128(x, 12)) };
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
                        if constexpr (frm_sgn) return std::tuple{
                            _mm_srai_epi32(_mm_unpacklo_epi16(lo16, lo16), 16),
                            _mm_srai_epi32(_mm_unpackhi_epi16(lo16, lo16), 16),
                            _mm_srai_epi32(_mm_unpacklo_epi16(hi16, hi16), 16),
                            _mm_srai_epi32(_mm_unpackhi_epi16(hi16, hi16), 16) };
                        else {
                            __m128i zero = _mm_setzero_si128();
                            return std::tuple{
                                _mm_unpacklo_epi16(lo16, zero), _mm_unpackhi_epi16(lo16, zero),
                                _mm_unpacklo_epi16(hi16, zero), _mm_unpackhi_epi16(hi16, zero) };
                        }
                    }
                }
                else if constexpr (sizeof(R) == 128 && has_zmm) {
                    auto lo = _mm256_castsi256_si128(x);
                    auto hi = _mm256_extracti128_si256(x, 1);
                    if constexpr (frm_sgn) return std::tuple{ _mm512_cvtepi8_epi32(lo), _mm512_cvtepi8_epi32(hi) };
                    else return std::tuple{ _mm512_cvtepu8_epi32(lo), _mm512_cvtepu8_epi32(hi) };
                }
                else if constexpr (sizeof(R) == 128 && has_avx2_v) {
                    auto lo = _mm256_castsi256_si128(x);
                    auto hi = _mm256_extracti128_si256(x, 1);
                    if constexpr (frm_sgn) return std::tuple{
                        _mm256_cvtepi8_epi32(lo), _mm256_cvtepi8_epi32(_mm_srli_si128(lo, 8)),
                        _mm256_cvtepi8_epi32(hi), _mm256_cvtepi8_epi32(_mm_srli_si128(hi, 8)) };
                    else return std::tuple{
                        _mm256_cvtepu8_epi32(lo), _mm256_cvtepu8_epi32(_mm_srli_si128(lo, 8)),
                        _mm256_cvtepu8_epi32(hi), _mm256_cvtepu8_epi32(_mm_srli_si128(hi, 8)) };
                }
                else return scalar_fallback_convert_<ISA, Saturated, From, To>(x);
            }
            else if constexpr (i8_to_i64) {
                if constexpr (sizeof(R) == 128 && has_zmm) {
                    __m512i e32;
                    if constexpr (frm_sgn) e32 = _mm512_cvtepi8_epi32(x);
                    else e32 = _mm512_cvtepu8_epi32(x);
                    auto lo32 = _mm512_castsi512_si256(e32);
                    auto hi32 = _mm512_extracti64x4_epi64(e32, 1);
                    if constexpr (frm_sgn) return std::tuple{ _mm512_cvtepi32_epi64(lo32), _mm512_cvtepi32_epi64(hi32) };
                    else return std::tuple{ _mm512_cvtepu32_epi64(lo32), _mm512_cvtepu32_epi64(hi32) };
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
                    if constexpr (frm_sgn) return std::tuple{
                        _mm256_cvtepi32_epi64(_mm256_castsi256_si128(lo32)),
                        _mm256_cvtepi32_epi64(_mm256_extracti128_si256(lo32, 1)),
                        _mm256_cvtepi32_epi64(_mm256_castsi256_si128(hi32)),
                        _mm256_cvtepi32_epi64(_mm256_extracti128_si256(hi32, 1)) };
                    else return std::tuple{
                        _mm256_cvtepu32_epi64(_mm256_castsi256_si128(lo32)),
                        _mm256_cvtepu32_epi64(_mm256_extracti128_si256(lo32, 1)),
                        _mm256_cvtepu32_epi64(_mm256_castsi256_si128(hi32)),
                        _mm256_cvtepu32_epi64(_mm256_extracti128_si256(hi32, 1)) };
                }
                //else if constexpr (sizeof(R) == 128 && has_s41) {
                //    __m128i e0, e1, e2, e3;
                //    if constexpr (frm_sgn) {
                //        e0 = _mm_cvtepi8_epi32(x);
                //        e1 = _mm_cvtepi8_epi32(_mm_srli_si128(x, 4));
                //        e2 = _mm_cvtepi8_epi32(_mm_srli_si128(x, 8));
                //        e3 = _mm_cvtepi8_epi32(_mm_srli_si128(x, 12));
                //    }
                //    else {
                //        e0 = _mm_cvtepu8_epi32(x);
                //        e1 = _mm_cvtepu8_epi32(_mm_srli_si128(x, 4));
                //        e2 = _mm_cvtepu8_epi32(_mm_srli_si128(x, 8));
                //        e3 = _mm_cvtepu8_epi32(_mm_srli_si128(x, 12));
                //    }
                //    if constexpr (frm_sgn) return std::tuple{
                //        _mm_cvtepi32_epi64(e0), _mm_cvtepi32_epi64(_mm_srli_si128(e0, 8)),
                //        _mm_cvtepi32_epi64(e1), _mm_cvtepi32_epi64(_mm_srli_si128(e1, 8)),
                //        _mm_cvtepi32_epi64(e2), _mm_cvtepi32_epi64(_mm_srli_si128(e2, 8)),
                //        _mm_cvtepi32_epi64(e3), _mm_cvtepi32_epi64(_mm_srli_si128(e3, 8)) };
                //    else return std::tuple{
                //        _mm_cvtepu32_epi64(e0), _mm_cvtepu32_epi64(_mm_srli_si128(e0, 8)),
                //        _mm_cvtepu32_epi64(e1), _mm_cvtepu32_epi64(_mm_srli_si128(e1, 8)),
                //        _mm_cvtepu32_epi64(e2), _mm_cvtepu32_epi64(_mm_srli_si128(e2, 8)),
                //        _mm_cvtepu32_epi64(e3), _mm_cvtepu32_epi64(_mm_srli_si128(e3, 8)) };
                //}
               /* else if constexpr (sizeof(R) == 128) {
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
                    __m128i e0, e1, e2, e3;
                    if constexpr (frm_sgn) {
                        e0 = _mm_srai_epi32(_mm_unpacklo_epi16(lo16, lo16), 16);
                        e1 = _mm_srai_epi32(_mm_unpackhi_epi16(lo16, lo16), 16);
                        e2 = _mm_srai_epi32(_mm_unpacklo_epi16(hi16, hi16), 16);
                        e3 = _mm_srai_epi32(_mm_unpackhi_epi16(hi16, hi16), 16);
                        return std::tuple{
                            _mm_unpacklo_epi32(e0, _mm_srai_epi32(e0, 31)),
                            _mm_unpackhi_epi32(e0, _mm_srai_epi32(e0, 31)),
                            _mm_unpacklo_epi32(e1, _mm_srai_epi32(e1, 31)),
                            _mm_unpackhi_epi32(e1, _mm_srai_epi32(e1, 31)),
                            _mm_unpacklo_epi32(e2, _mm_srai_epi32(e2, 31)),
                            _mm_unpackhi_epi32(e2, _mm_srai_epi32(e2, 31)),
                            _mm_unpacklo_epi32(e3, _mm_srai_epi32(e3, 31)),
                            _mm_unpackhi_epi32(e3, _mm_srai_epi32(e3, 31)) };
                    }
                    else {
                        __m128i zero = _mm_setzero_si128();
                        e0 = _mm_unpacklo_epi16(lo16, zero);
                        e1 = _mm_unpackhi_epi16(lo16, zero);
                        e2 = _mm_unpacklo_epi16(hi16, zero);
                        e3 = _mm_unpackhi_epi16(hi16, zero);
                        return std::tuple{
                            _mm_unpacklo_epi32(e0, zero), _mm_unpackhi_epi32(e0, zero),
                            _mm_unpacklo_epi32(e1, zero), _mm_unpackhi_epi32(e1, zero),
                            _mm_unpacklo_epi32(e2, zero), _mm_unpackhi_epi32(e2, zero),
                            _mm_unpacklo_epi32(e3, zero), _mm_unpackhi_epi32(e3, zero) };
                    }
                }*/
                else return scalar_fallback_convert_<ISA, Saturated, From, To>(x);
            }
            else if constexpr (i16_to_i8) {
                if constexpr (x_to_x) {
                    if constexpr (has_bwvl) {
                        if constexpr (Saturated) {
                            if constexpr (epu8<To>) return _mm_cvtusepi16_epi8(x);
                            else return _mm_cvtsepi16_epi8(x);
                        }
                        else return _mm_cvtepi16_epi8(x);
                    }
                    else if constexpr (!Saturated) {
                        if constexpr (has_ssse3<ISA>) {
                            const __m128i m = _mm_setr_epi8(0, 2, 4, 6, 8, 10, 12, 14, -1, -1, -1, -1, -1, -1, -1, -1);
                            return _mm_shuffle_epi8(x, m);
                        }
                        else {
                            auto m = _mm_srai_epi16(_mm_slli_epi16(x, 8), 8);
                            return _mm_packs_epi16(m, _mm_setzero_si128());
                        }
                    }
                    else {
                        const __m128i zero = _mm_setzero_si128();
                        if constexpr (epu8<To>) return _mm_packus_epi16(x, zero);
                        else return _mm_packs_epi16(x, zero);
                    }
                }
                else if constexpr (y_to_x) {
                    if constexpr (has_bwvl) {
                        if constexpr (Saturated) {
                            if constexpr (epu8<To>) return _mm256_cvtusepi16_epi8(x);
                            else return _mm256_cvtsepi16_epi8(x);
                        }
                        else return _mm256_cvtepi16_epi8(x);
                    }
                    else if constexpr (!Saturated && has_avx2_v) {
                        const __m256i m = _mm256_setr_epi8(0, 2, 4, 6, 8, 10, 12, 14, -1, -1, -1, -1, -1, -1, -1, -1,
                            0, 2, 4, 6, 8, 10, 12, 14, -1, -1, -1, -1, -1, -1, -1, -1);
                        auto a = _mm256_shuffle_epi8(x, m);
                        auto b = _mm256_permute4x64_epi64(a, 0xF8);
                        return _mm256_castsi256_si128(b);
                    }
                    else if constexpr (Saturated && has_avx2_v) {
                        auto lo = _mm256_castsi256_si128(x);
                        auto hi = _mm256_extracti128_si256(x, 1);
                        if constexpr (epu8<To>) return _mm_packus_epi16(lo, hi);
                        else return _mm_packs_epi16(lo, hi);
                    }
                    else return scalar_fallback_convert_<ISA, Saturated, From, To>(x);
                }
                else if constexpr (z_to_y && has_bw) {
                    if constexpr (Saturated) {
                        if constexpr (epu8<To>) return _mm512_cvtusepi16_epi8(x);
                        else return _mm512_cvtsepi16_epi8(x);
                    }
                    else return _mm512_cvtepi16_epi8(x);
                }
                else return scalar_fallback_convert_<ISA, Saturated, From, To>(x);
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
                        auto lo = _mm256_castsi256_si128(x);
                        auto hi = _mm256_extracti128_si256(x, 1);
                        if constexpr (frm_sgn) return std::tuple{ _mm256_cvtepi16_epi32(lo), _mm256_cvtepi16_epi32(hi) };
                        else return std::tuple{ _mm256_cvtepu16_epi32(lo), _mm256_cvtepu16_epi32(hi) };
                    }
                    else return scalar_fallback_convert_<ISA, Saturated, From, To>(x);
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
                    return _mm256_insertf128_si256(_mm256_castsi128_si256(e0), e1, 1);
                }
                else if constexpr (x_to_y && has_s41) {
                    if constexpr (frm_sgn) return std::tuple{ _mm_cvtepi16_epi32(x), _mm_cvtepi16_epi32(_mm_srli_si128(x, 8)) };
                    else return std::tuple{ _mm_cvtepu16_epi32(x), _mm_cvtepu16_epi32(_mm_srli_si128(x, 8)) };
                }
                else if constexpr (x_to_y) {
                    if constexpr (frm_sgn) return std::tuple{
                        _mm_srai_epi32(_mm_unpacklo_epi16(x, x), 16),
                        _mm_srai_epi32(_mm_unpackhi_epi16(x, x), 16) };
                    else {
                        __m128i zero = _mm_setzero_si128();
                        return std::tuple{ _mm_unpacklo_epi16(x, zero), _mm_unpackhi_epi16(x, zero) };
                    }
                }
                else if constexpr (sizeof(R) == 128 && has_zmm) {
                    auto lo = _mm512_castsi512_si256(x);
                    auto hi = _mm512_extracti64x4_epi64(x, 1);
                    if constexpr (frm_sgn) return std::tuple{ _mm512_cvtepi16_epi32(lo), _mm512_cvtepi16_epi32(hi) };
                    else return std::tuple{ _mm512_cvtepu16_epi32(lo), _mm512_cvtepu16_epi32(hi) };
                }
                else return scalar_fallback_convert_<ISA, Saturated, From, To>(x);
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
                        if constexpr (frm_sgn) return std::tuple{
                            _mm256_cvtepi32_epi64(_mm256_castsi256_si128(w32)),
                            _mm256_cvtepi32_epi64(_mm256_extracti128_si256(w32, 1)) };
                        else return std::tuple{
                            _mm256_cvtepu32_epi64(_mm256_castsi256_si128(w32)),
                            _mm256_cvtepu32_epi64(_mm256_extracti128_si256(w32, 1)) };
                    }
                    else if constexpr (has_avx<ISA> && has_s41) {
                        __m128i w0, w1;
                        if constexpr (frm_sgn) {
                            w0 = _mm_cvtepi16_epi32(x);
                            w1 = _mm_cvtepi16_epi32(_mm_srli_si128(x, 8));
                        }
                        else {
                            w0 = _mm_cvtepu16_epi32(x);
                            w1 = _mm_cvtepu16_epi32(_mm_srli_si128(x, 8));
                        }
                        __m128i q0, q1, q2, q3;
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
                        return std::tuple{
                            _mm256_insertf128_si256(_mm256_castsi128_si256(q0), q1, 1),
                            _mm256_insertf128_si256(_mm256_castsi128_si256(q2), q3, 1) };
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
                        if constexpr (frm_sgn) return std::tuple{
                            _mm_cvtepi32_epi64(w0), _mm_cvtepi32_epi64(_mm_srli_si128(w0, 8)),
                            _mm_cvtepi32_epi64(w1), _mm_cvtepi32_epi64(_mm_srli_si128(w1, 8)) };
                        else return std::tuple{
                            _mm_cvtepu32_epi64(w0), _mm_cvtepu32_epi64(_mm_srli_si128(w0, 8)),
                            _mm_cvtepu32_epi64(w1), _mm_cvtepu32_epi64(_mm_srli_si128(w1, 8)) };
                    }
                    else {
                        if constexpr (frm_sgn) {
                            __m128i w0 = _mm_srai_epi32(_mm_unpacklo_epi16(x, x), 16);
                            __m128i w1 = _mm_srai_epi32(_mm_unpackhi_epi16(x, x), 16);
                            return std::tuple{
                                _mm_unpacklo_epi32(w0, _mm_srai_epi32(w0, 31)),
                                _mm_unpackhi_epi32(w0, _mm_srai_epi32(w0, 31)),
                                _mm_unpacklo_epi32(w1, _mm_srai_epi32(w1, 31)),
                                _mm_unpackhi_epi32(w1, _mm_srai_epi32(w1, 31)) };
                        }
                        else {
                            __m128i zero = _mm_setzero_si128();
                            __m128i w0 = _mm_unpacklo_epi16(x, zero);
                            __m128i w1 = _mm_unpackhi_epi16(x, zero);
                            return std::tuple{
                                _mm_unpacklo_epi32(w0, zero), _mm_unpackhi_epi32(w0, zero),
                                _mm_unpacklo_epi32(w1, zero), _mm_unpackhi_epi32(w1, zero) };
                        }
                    }
                }
                else if constexpr (sizeof(R) == 128 && has_zmm) {
                    auto lo = _mm256_castsi256_si128(x);
                    auto hi = _mm256_extracti128_si256(x, 1);
                    if constexpr (frm_sgn) return std::tuple{ _mm512_cvtepi16_epi64(lo), _mm512_cvtepi16_epi64(hi) };
                    else return std::tuple{ _mm512_cvtepu16_epi64(lo), _mm512_cvtepu16_epi64(hi) };
                }
                else if constexpr (sizeof(R) == 128 && has_avx2_v) {
                    auto lo = _mm256_castsi256_si128(x);
                    auto hi = _mm256_extracti128_si256(x, 1);
                    __m256i w_lo, w_hi;
                    if constexpr (frm_sgn) {
                        w_lo = _mm256_cvtepi16_epi32(lo);
                        w_hi = _mm256_cvtepi16_epi32(hi);
                    }
                    else {
                        w_lo = _mm256_cvtepu16_epi32(lo);
                        w_hi = _mm256_cvtepu16_epi32(hi);
                    }
                    if constexpr (frm_sgn) return std::tuple{
                        _mm256_cvtepi32_epi64(_mm256_castsi256_si128(w_lo)),
                        _mm256_cvtepi32_epi64(_mm256_extracti128_si256(w_lo, 1)),
                        _mm256_cvtepi32_epi64(_mm256_castsi256_si128(w_hi)),
                        _mm256_cvtepi32_epi64(_mm256_extracti128_si256(w_hi, 1)) };
                    else return std::tuple{
                        _mm256_cvtepu32_epi64(_mm256_castsi256_si128(w_lo)),
                        _mm256_cvtepu32_epi64(_mm256_extracti128_si256(w_lo, 1)),
                        _mm256_cvtepu32_epi64(_mm256_castsi256_si128(w_hi)),
                        _mm256_cvtepu32_epi64(_mm256_extracti128_si256(w_hi, 1)) };
                }
                else if constexpr (sizeof(R) == 128 && has_avx<ISA> && has_s41) {
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
                    __m128i q0, q1, q2, q3, q4, q5, q6, q7;
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
                    return std::tuple{
                        _mm256_insertf128_si256(_mm256_castsi128_si256(q0), q1, 1),
                        _mm256_insertf128_si256(_mm256_castsi128_si256(q2), q3, 1),
                        _mm256_insertf128_si256(_mm256_castsi128_si256(q4), q5, 1),
                        _mm256_insertf128_si256(_mm256_castsi128_si256(q6), q7, 1) };
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
                    if constexpr (frm_sgn) return std::tuple{
                        _mm_cvtepi32_epi64(w0), _mm_cvtepi32_epi64(_mm_srli_si128(w0, 8)),
                        _mm_cvtepi32_epi64(w1), _mm_cvtepi32_epi64(_mm_srli_si128(w1, 8)),
                        _mm_cvtepi32_epi64(w2), _mm_cvtepi32_epi64(_mm_srli_si128(w2, 8)),
                        _mm_cvtepi32_epi64(w3), _mm_cvtepi32_epi64(_mm_srli_si128(w3, 8)) };
                    else return std::tuple{
                        _mm_cvtepu32_epi64(w0), _mm_cvtepu32_epi64(_mm_srli_si128(w0, 8)),
                        _mm_cvtepu32_epi64(w1), _mm_cvtepu32_epi64(_mm_srli_si128(w1, 8)),
                        _mm_cvtepu32_epi64(w2), _mm_cvtepu32_epi64(_mm_srli_si128(w2, 8)),
                        _mm_cvtepu32_epi64(w3), _mm_cvtepu32_epi64(_mm_srli_si128(w3, 8)) };
                }
                else if constexpr (sizeof(R) == 128) {
                    if constexpr (frm_sgn) {
                        __m128i w0 = _mm_srai_epi32(_mm_unpacklo_epi16(x, x), 16);
                        __m128i w1 = _mm_srai_epi32(_mm_unpackhi_epi16(x, x), 16);
                        return std::tuple{
                            _mm_unpacklo_epi32(w0, _mm_srai_epi32(w0, 31)),
                            _mm_unpackhi_epi32(w0, _mm_srai_epi32(w0, 31)),
                            _mm_unpacklo_epi32(w1, _mm_srai_epi32(w1, 31)),
                            _mm_unpackhi_epi32(w1, _mm_srai_epi32(w1, 31)) };
                    }
                    else {
                        __m128i zero = _mm_setzero_si128();
                        __m128i w0 = _mm_unpacklo_epi16(x, zero);
                        __m128i w1 = _mm_unpackhi_epi16(x, zero);
                        return std::tuple{
                            _mm_unpacklo_epi32(w0, zero), _mm_unpackhi_epi32(w0, zero),
                            _mm_unpacklo_epi32(w1, zero), _mm_unpackhi_epi32(w1, zero) };
                    }
                }
                else if constexpr (sizeof(R) == 256 && has_zmm) {
                    auto x0 = _mm512_castsi512_si128(x);
                    auto x1 = _mm512_extracti32x4_epi32(x, 1);
                    auto x2 = _mm512_extracti32x4_epi32(x, 2);
                    auto x3 = _mm512_extracti32x4_epi32(x, 3);
                    if constexpr (frm_sgn) return std::tuple{
                        _mm512_cvtepi16_epi64(x0), _mm512_cvtepi16_epi64(x1),
                        _mm512_cvtepi16_epi64(x2), _mm512_cvtepi16_epi64(x3) };
                    else return std::tuple{
                        _mm512_cvtepu16_epi64(x0), _mm512_cvtepu16_epi64(x1),
                        _mm512_cvtepu16_epi64(x2), _mm512_cvtepu16_epi64(x3) };
                }
                else return scalar_fallback_convert_<ISA, Saturated, From, To>(x);
            }
            else if constexpr (i32_to_i8) {
                if constexpr (x_to_x) {
                    if constexpr (has_bwvl) {
                        if constexpr (Saturated) {
                            if constexpr (epu8<To>) return _mm_cvtusepi32_epi8(x);
                            else return _mm_cvtsepi32_epi8(x);
                        }
                        else return _mm_cvtepi32_epi8(x);
                    }
                    else if constexpr (!Saturated) {
                        if constexpr (has_ssse3<ISA>) {
                            const __m128i m = _mm_setr_epi8(0, 4, 8, 12, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1);
                            return _mm_shuffle_epi8(x, m);
                        }
                        else {
                            auto m = _mm_srai_epi32(_mm_slli_epi32(x, 24), 24);
                            auto p = _mm_packs_epi32(m, _mm_setzero_si128());
                            return _mm_packs_epi16(p, _mm_setzero_si128());
                        }
                    }
                    else {
                        if constexpr (epu8<To>) {
                            const __m128i zero = _mm_setzero_si128();
                            const __m128i maxv = _mm_set1_epi32(255);
                            auto c = _mm_and_si128(x, _mm_cmpgt_epi32(x, zero));
                            auto e = _mm_cmpgt_epi32(c, maxv);
                            auto r = _mm_or_si128(_mm_andnot_si128(e, c), _mm_and_si128(e, maxv));
                            if constexpr (has_ssse3<ISA>) {
                                const __m128i m = _mm_setr_epi8(0, 4, 8, 12, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1);
                                return _mm_shuffle_epi8(r, m);
                            }
                            else {
                                auto m = _mm_srai_epi32(_mm_slli_epi32(r, 24), 24);
                                auto p = _mm_packs_epi32(m, _mm_setzero_si128());
                                return _mm_packs_epi16(p, _mm_setzero_si128());
                            }
                        }
                        else {
                            const __m128i lo_v = _mm_set1_epi32(-128);
                            const __m128i hi_v = _mm_set1_epi32(127);
                            auto lt = _mm_cmplt_epi32(x, lo_v);
                            auto gt = _mm_cmpgt_epi32(x, hi_v);
                            auto r = _mm_or_si128(_mm_andnot_si128(lt, x), _mm_and_si128(lt, lo_v));
                            r = _mm_or_si128(_mm_andnot_si128(gt, r), _mm_and_si128(gt, hi_v));
                            if constexpr (has_ssse3<ISA>) {
                                const __m128i m = _mm_setr_epi8(0, 4, 8, 12, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1);
                                return _mm_shuffle_epi8(r, m);
                            }
                            else {
                                auto m = _mm_srai_epi32(_mm_slli_epi32(r, 24), 24);
                                auto p = _mm_packs_epi32(m, _mm_setzero_si128());
                                return _mm_packs_epi16(p, _mm_setzero_si128());
                            }
                        }
                    }
                }
                else if constexpr (y_to_x && has_bwvl) {
                    if constexpr (Saturated) {
                        if constexpr (epu8<To>) return _mm256_cvtusepi32_epi8(x);
                        else return _mm256_cvtsepi32_epi8(x);
                    }
                    else return _mm256_cvtepi32_epi8(x);
                }
                else if constexpr (y_to_x && has_avx2_v) {
                    auto lo = _mm256_castsi256_si128(x);
                    auto hi = _mm256_extracti128_si256(x, 1);
                    __m128i r_lo, r_hi;
                    if constexpr (!Saturated) {
                        r_lo = _mm_srai_epi32(_mm_slli_epi32(lo, 24), 24);
                        r_hi = _mm_srai_epi32(_mm_slli_epi32(hi, 24), 24);
                        auto p16 = _mm_packs_epi32(r_lo, r_hi);
                        return _mm_packs_epi16(p16, _mm_setzero_si128());
                    }
                    else {
                        if constexpr (epu8<To>) {
                            const __m128i zero = _mm_setzero_si128();
                            const __m128i maxv = _mm_set1_epi32(255);
                            auto c_lo = _mm_and_si128(lo, _mm_cmpgt_epi32(lo, zero));
                            auto c_hi = _mm_and_si128(hi, _mm_cmpgt_epi32(hi, zero));
                            auto e_lo = _mm_cmpgt_epi32(c_lo, maxv);
                            auto e_hi = _mm_cmpgt_epi32(c_hi, maxv);
                            r_lo = _mm_or_si128(_mm_andnot_si128(e_lo, c_lo), _mm_and_si128(e_lo, maxv));
                            r_hi = _mm_or_si128(_mm_andnot_si128(e_hi, c_hi), _mm_and_si128(e_hi, maxv));
                            auto p16 = _mm_packs_epi32(r_lo, r_hi);
                            return _mm_packus_epi16(p16, zero);
                        }
                        else {
                            const __m128i lo_v = _mm_set1_epi32(-128);
                            const __m128i hi_v = _mm_set1_epi32(127);
                            auto lt_lo = _mm_cmplt_epi32(lo, lo_v);
                            auto gt_lo = _mm_cmpgt_epi32(lo, hi_v);
                            auto lt_hi = _mm_cmplt_epi32(hi, lo_v);
                            auto gt_hi = _mm_cmpgt_epi32(hi, hi_v);
                            r_lo = _mm_or_si128(_mm_andnot_si128(lt_lo, lo), _mm_and_si128(lt_lo, lo_v));
                            r_lo = _mm_or_si128(_mm_andnot_si128(gt_lo, r_lo), _mm_and_si128(gt_lo, hi_v));
                            r_hi = _mm_or_si128(_mm_andnot_si128(lt_hi, hi), _mm_and_si128(lt_hi, lo_v));
                            r_hi = _mm_or_si128(_mm_andnot_si128(gt_hi, r_hi), _mm_and_si128(gt_hi, hi_v));
                            auto p16 = _mm_packs_epi32(r_lo, r_hi);
                            return _mm_packs_epi16(p16, _mm_setzero_si128());
                        }
                    }
                }
                else if constexpr (z_to_x && has_bw) {
                    if constexpr (Saturated) {
                        if constexpr (epu8<To>) return _mm512_cvtusepi32_epi8(x);
                        else return _mm512_cvtsepi32_epi8(x);
                    }
                    else return _mm512_cvtepi32_epi8(x);
                }
                else return scalar_fallback_convert_<ISA, Saturated, From, To>(x);
            }
            else if constexpr (i32_to_i16) {
                if constexpr (x_to_x) {
                    if constexpr (has_bwvl) {
                        if constexpr (Saturated) {
                            if constexpr (epu16<To>) return _mm_cvtusepi32_epi16(x);
                            else return _mm_cvtsepi32_epi16(x);
                        }
                        else return _mm_cvtepi32_epi16(x);
                    }
                    else if constexpr (!Saturated) {
                        if constexpr (has_ssse3<ISA>) {
                            const __m128i m = _mm_setr_epi8(0, 1, 4, 5, 8, 9, 12, 13, -1, -1, -1, -1, -1, -1, -1, -1);
                            return _mm_shuffle_epi8(x, m);
                        }
                        else {
                            auto m = _mm_srai_epi32(_mm_slli_epi32(x, 16), 16);
                            return _mm_packs_epi32(m, _mm_setzero_si128());
                        }
                    }
                    else {
                        const __m128i zero = _mm_setzero_si128();
                        if constexpr (epu16<To>) {
                            if constexpr (has_s41) return _mm_packus_epi32(x, zero);
                            else {
                                const __m128i maxv = _mm_set1_epi32(65535);
                                auto c = _mm_and_si128(x, _mm_cmpgt_epi32(x, zero));
                                auto e = _mm_cmpgt_epi32(c, maxv);
                                auto r = _mm_or_si128(_mm_andnot_si128(e, c), _mm_and_si128(e, maxv));
                                auto m = _mm_srai_epi32(_mm_slli_epi32(r, 16), 16);
                                return _mm_packs_epi32(m, zero);
                            }
                        }
                        else return _mm_packs_epi32(x, zero);
                    }
                }
                else if constexpr (y_to_x) {
                    if constexpr (has_bwvl) {
                        if constexpr (Saturated) {
                            if constexpr (epu16<To>) return _mm256_cvtusepi32_epi16(x);
                            else return _mm256_cvtsepi32_epi16(x);
                        }
                        else return _mm256_cvtepi32_epi16(x);
                    }
                    else if constexpr (!Saturated && has_avx2_v) {
                        const __m256i m = _mm256_setr_epi8(0, 1, 4, 5, 8, 9, 12, 13, -1, -1, -1, -1, -1, -1, -1, -1,
                            0, 1, 4, 5, 8, 9, 12, 13, -1, -1, -1, -1, -1, -1, -1, -1);
                        auto a = _mm256_shuffle_epi8(x, m);
                        auto b = _mm256_permute4x64_epi64(a, 0xF8);
                        return _mm256_castsi256_si128(b);
                    }
                    else if constexpr (Saturated && has_avx2_v) {
                        auto lo = _mm256_castsi256_si128(x);
                        auto hi = _mm256_extracti128_si256(x, 1);
                        if constexpr (epu16<To>) {
                            if constexpr (has_s41) return _mm_packus_epi32(lo, hi);
                            else return scalar_fallback_convert_<ISA, Saturated, From, To>(x);
                        }
                        else return _mm_packs_epi32(lo, hi);
                    }
                    else return scalar_fallback_convert_<ISA, Saturated, From, To>(x);
                }
                else if constexpr (z_to_y && has_bw) {
                    if constexpr (Saturated) {
                        if constexpr (epu16<To>) return _mm512_cvtusepi32_epi16(x);
                        else return _mm512_cvtsepi32_epi16(x);
                    }
                    else return _mm512_cvtepi32_epi16(x);
                }
                else return scalar_fallback_convert_<ISA, Saturated, From, To>(x);
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
                        auto lo = _mm256_castsi256_si128(x);
                        auto hi = _mm256_extracti128_si256(x, 1);
                        if constexpr (frm_sgn) return std::tuple{ _mm256_cvtepi32_epi64(lo), _mm256_cvtepi32_epi64(hi) };
                        else return std::tuple{ _mm256_cvtepu32_epi64(lo), _mm256_cvtepu32_epi64(hi) };
                    }
                    else return scalar_fallback_convert_<ISA, Saturated, From, To>(x);
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
                    return _mm256_insertf128_si256(_mm256_castsi128_si256(e0), e1, 1);
                }
                else if constexpr (x_to_y && has_s41) {
                    if constexpr (frm_sgn) return std::tuple{ _mm_cvtepi32_epi64(x), _mm_cvtepi32_epi64(_mm_srli_si128(x, 8)) };
                    else return std::tuple{ _mm_cvtepu32_epi64(x), _mm_cvtepu32_epi64(_mm_srli_si128(x, 8)) };
                }
                else if constexpr (x_to_y) {
                    if constexpr (frm_sgn) return std::tuple{
                        _mm_unpacklo_epi32(x, _mm_srai_epi32(x, 31)),
                        _mm_unpackhi_epi32(x, _mm_srai_epi32(x, 31)) };
                    else {
                        __m128i zero = _mm_setzero_si128();
                        return std::tuple{ _mm_unpacklo_epi32(x, zero), _mm_unpackhi_epi32(x, zero) };
                    }
                }
                else if constexpr (sizeof(R) == 128 && has_zmm) {
                    auto lo = _mm512_castsi512_si256(x);
                    auto hi = _mm512_extracti64x4_epi64(x, 1);
                    if constexpr (frm_sgn) return std::tuple{ _mm512_cvtepi32_epi64(lo), _mm512_cvtepi32_epi64(hi) };
                    else return std::tuple{ _mm512_cvtepu32_epi64(lo), _mm512_cvtepu32_epi64(hi) };
                }
                else return scalar_fallback_convert_<ISA, Saturated, From, To>(x);
            }
            else if constexpr (i64_to_i32) {
                if constexpr (x_to_x) {
                    if constexpr (has_bwvl) {
                        if constexpr (Saturated) {
                            if constexpr (epu32<To>) return _mm_cvtusepi64_epi32(x);
                            else return _mm_cvtsepi64_epi32(x);
                        }
                        else return _mm_cvtepi64_epi32(x);
                    }
                    else if constexpr (has_ssse3<ISA> && !Saturated) {
                        const __m128i m = _mm_setr_epi8(0, 1, 2, 3, 8, 9, 10, 11, -1, -1, -1, -1, -1, -1, -1, -1);
                        return _mm_shuffle_epi8(x, m);
                    }
                    else return scalar_fallback_convert_<ISA, Saturated, From, To>(x);
                }
                else if constexpr (y_to_x && has_bwvl) {
                    if constexpr (Saturated) {
                        if constexpr (epu32<To>) return _mm256_cvtusepi64_epi32(x);
                        else return _mm256_cvtsepi64_epi32(x);
                    }
                    else return _mm256_cvtepi64_epi32(x);
                }
                else if constexpr (z_to_y && has_bw) {
                    if constexpr (Saturated) {
                        if constexpr (epu32<To>) return _mm512_cvtusepi64_epi32(x);
                        else return _mm512_cvtsepi64_epi32(x);
                    }
                    else return _mm512_cvtepi64_epi32(x);
                }
                else return scalar_fallback_convert_<ISA, Saturated, From, To>(x);
            }
            else if constexpr (i64_to_i16) {
                if constexpr (x_to_x && has_bwvl) {
                    if constexpr (Saturated) {
                        if constexpr (epu16<To>) return _mm_cvtusepi64_epi16(x);
                        else return _mm_cvtsepi64_epi16(x);
                    }
                    else return _mm_cvtepi64_epi16(x);
                }
                else if constexpr (y_to_x && has_bwvl) {
                    if constexpr (Saturated) {
                        if constexpr (epu16<To>) return _mm256_cvtusepi64_epi16(x);
                        else return _mm256_cvtsepi64_epi16(x);
                    }
                    else return _mm256_cvtepi64_epi16(x);
                }
                else if constexpr (z_to_x && has_bw) {
                    if constexpr (Saturated) {
                        if constexpr (epu16<To>) return _mm512_cvtusepi64_epi16(x);
                        else return _mm512_cvtsepi64_epi16(x);
                    }
                    else return _mm512_cvtepi64_epi16(x);
                }
                else return scalar_fallback_convert_<ISA, Saturated, From, To>(x);
            }
            else if constexpr (i64_to_i8) {
                if constexpr (x_to_x && has_bwvl) {
                    if constexpr (Saturated) {
                        if constexpr (epu8<To>) return _mm_cvtusepi64_epi8(x);
                        else return _mm_cvtsepi64_epi8(x);
                    }
                    else return _mm_cvtepi64_epi8(x);
                }
                else if constexpr (y_to_x && has_bwvl) {
                    if constexpr (Saturated) {
                        if constexpr (epu8<To>) return _mm256_cvtusepi64_epi8(x);
                        else return _mm256_cvtsepi64_epi8(x);
                    }
                    else return _mm256_cvtepi64_epi8(x);
                }
                else if constexpr (z_to_x && has_bw) {
                    if constexpr (Saturated) {
                        if constexpr (epu8<To>) return _mm512_cvtusepi64_epi8(x);
                        else return _mm512_cvtsepi64_epi8(x);
                    }
                    else return _mm512_cvtepi64_epi8(x);
                }
                else return scalar_fallback_convert_<ISA, Saturated, From, To>(x);
            }
            else return scalar_fallback_convert_<ISA, Saturated, From, To>(x);
        }
        else if constexpr (f_to_i) {
            if constexpr (sizeof(From) == 4) {
                if constexpr (sizeof(To) == 4) {
                    if constexpr (to_sgn) {
                        if constexpr (x_to_x) return _mm_cvttps_epi32(x);
                        else if constexpr (y_to_y && has_ymm_v) return _mm256_cvttps_epi32(x);
                        else if constexpr (z_to_z && has_zmm) return _mm512_cvttps_epi32(x);
                        else return scalar_fallback_convert_<ISA, Saturated, From, To>(x);
                    }
                    else {
                        if constexpr (x_to_x && has_zmm && has_vl) return _mm_cvttps_epu32(x);
                        else if constexpr (y_to_y && has_zmm && has_vl) return _mm256_cvttps_epu32(x);
                        else if constexpr (z_to_z && has_zmm) return _mm512_cvttps_epu32(x);
                        else return scalar_fallback_convert_<ISA, Saturated, From, To>(x);
                    }
                }
                else if constexpr (sizeof(To) == 8) {
                    if constexpr (x_to_y && has_dqvl) {
                        if constexpr (to_sgn) return _mm256_cvttps_epi64(x);
                        else return _mm256_cvttps_epu64(x);
                    }
                    else if constexpr (y_to_z && has_dq) {
                        if constexpr (to_sgn) return _mm512_cvttps_epi64(x);
                        else return _mm512_cvttps_epu64(x);
                    }
                    else return scalar_fallback_convert_<ISA, Saturated, From, To>(x);
                }
                else if constexpr (sizeof(To) == 2) {
                    if constexpr (x_to_x) {
                        auto i32 = _mm_cvttps_epi32(x);
                        if constexpr (has_bwvl) {
                            if constexpr (Saturated) {
                                if constexpr (epu16<To>) return _mm_cvtusepi32_epi16(i32);
                                else return _mm_cvtsepi32_epi16(i32);
                            }
                            else return _mm_cvtepi32_epi16(i32);
                        }
                        else if constexpr (!Saturated) {
                            if constexpr (has_ssse3<ISA>) {
                                const __m128i m = _mm_setr_epi8(0, 1, 4, 5, 8, 9, 12, 13, -1, -1, -1, -1, -1, -1, -1, -1);
                                return _mm_shuffle_epi8(i32, m);
                            }
                            else {
                                auto m = _mm_srai_epi32(_mm_slli_epi32(i32, 16), 16);
                                return _mm_packs_epi32(m, _mm_setzero_si128());
                            }
                        }
                        else {
                            const __m128i zero = _mm_setzero_si128();
                            if constexpr (epu16<To>) {
                                if constexpr (has_s41) return _mm_packus_epi32(i32, zero);
                                else return scalar_fallback_convert_<ISA, Saturated, From, To>(x);
                            }
                            else return _mm_packs_epi32(i32, zero);
                        }
                    }
                    else if constexpr (y_to_x && has_avx2_v) {
                        auto i32 = _mm256_cvttps_epi32(x);
                        if constexpr (has_bwvl) {
                            if constexpr (Saturated) {
                                if constexpr (epu16<To>) return _mm256_cvtusepi32_epi16(i32);
                                else return _mm256_cvtsepi32_epi16(i32);
                            }
                            else return _mm256_cvtepi32_epi16(i32);
                        }
                        else return scalar_fallback_convert_<ISA, Saturated, From, To>(x);
                    }
                    else if constexpr (z_to_y && has_bw) {
                        auto i32 = _mm512_cvttps_epi32(x);
                        if constexpr (Saturated) {
                            if constexpr (epu16<To>) return _mm512_cvtusepi32_epi16(i32);
                            else return _mm512_cvtsepi32_epi16(i32);
                        }
                        else return _mm512_cvtepi32_epi16(i32);
                    }
                    else return scalar_fallback_convert_<ISA, Saturated, From, To>(x);
                }
                else if constexpr (sizeof(To) == 1) {
                    if constexpr (x_to_x) {
                        auto i32 = _mm_cvttps_epi32(x);
                        if constexpr (has_bwvl) {
                            if constexpr (Saturated) {
                                if constexpr (epu8<To>) return _mm_cvtusepi32_epi8(i32);
                                else return _mm_cvtsepi32_epi8(i32);
                            }
                            else return _mm_cvtepi32_epi8(i32);
                        }
                        else return scalar_fallback_convert_<ISA, Saturated, From, To>(x);
                    }
                    else if constexpr (y_to_x && has_avx2_v) {
                        auto i32 = _mm256_cvttps_epi32(x);
                        if constexpr (has_bwvl) {
                            if constexpr (Saturated) {
                                if constexpr (epu8<To>) return _mm256_cvtusepi32_epi8(i32);
                                else return _mm256_cvtsepi32_epi8(i32);
                            }
                            else return _mm256_cvtepi32_epi8(i32);
                        }
                        else return scalar_fallback_convert_<ISA, Saturated, From, To>(x);
                    }
                    else if constexpr (z_to_x && has_bw) {
                        auto i32 = _mm512_cvttps_epi32(x);
                        if constexpr (Saturated) {
                            if constexpr (epu8<To>) return _mm512_cvtusepi32_epi8(i32);
                            else return _mm512_cvtsepi32_epi8(i32);
                        }
                        else return _mm512_cvtepi32_epi8(i32);
                    }
                    else return scalar_fallback_convert_<ISA, Saturated, From, To>(x);
                }
                else return scalar_fallback_convert_<ISA, Saturated, From, To>(x);
            }
            else if constexpr (sizeof(From) == 8) {
                if constexpr (sizeof(To) == 4) {
                    if constexpr (to_sgn) {
                        if constexpr (x_to_x) return _mm_cvttpd_epi32(x);
                        else if constexpr (y_to_x && has_ymm_v) return _mm256_cvttpd_epi32(x);
                        else if constexpr (z_to_y && has_zmm) return _mm512_cvttpd_epi32(x);
                        else return scalar_fallback_convert_<ISA, Saturated, From, To>(x);
                    }
                    else {
                        if constexpr (x_to_x && has_zmm && has_vl) return _mm_cvttpd_epu32(x);
                        else if constexpr (y_to_x && has_zmm && has_vl) return _mm256_cvttpd_epu32(x);
                        else if constexpr (z_to_y && has_zmm) return _mm512_cvttpd_epu32(x);
                        else return scalar_fallback_convert_<ISA, Saturated, From, To>(x);
                    }
                }
                else if constexpr (sizeof(To) == 8) {
                    if constexpr (x_to_x && has_dqvl) {
                        if constexpr (to_sgn) {
                            if constexpr (Saturated) return _mm_cvtpd_epi64(x);
                            else return _mm_cvttpd_epi64(x);
                        }
                        else {
                            if constexpr (Saturated) return _mm_cvtpd_epu64(x);
                            else return _mm_cvttpd_epu64(x);
                        }
                    }
                    else if constexpr (y_to_y && has_dqvl) {
                        if constexpr (to_sgn) {
                            if constexpr (Saturated) return _mm256_cvtpd_epi64(x);
                            else return _mm256_cvttpd_epi64(x);
                        }
                        else {
                            if constexpr (Saturated) return _mm256_cvtpd_epu64(x);
                            else return _mm256_cvttpd_epu64(x);
                        }
                    }
                    else if constexpr (z_to_z && has_dq) {
                        if constexpr (to_sgn) {
                            if constexpr (Saturated) return _mm512_cvtpd_epi64(x);
                            else return _mm512_cvttpd_epi64(x);
                        }
                        else {
                            if constexpr (Saturated) return _mm512_cvtpd_epu64(x);
                            else return _mm512_cvttpd_epu64(x);
                        }
                    }
                    else return scalar_fallback_convert_<ISA, Saturated, From, To>(x);
                }
                else if constexpr (sizeof(To) == 2) {
                    if constexpr (x_to_x && has_sse2<ISA>) {
                        auto i32 = _mm_cvttpd_epi32(x);
                        if constexpr (has_bwvl) {
                            if constexpr (Saturated) {
                                if constexpr (epu16<To>) return _mm_cvtusepi32_epi16(i32);
                                else return _mm_cvtsepi32_epi16(i32);
                            }
                            else return _mm_cvtepi32_epi16(i32);
                        }
                        else if constexpr (!Saturated) {
                            if constexpr (has_ssse3<ISA>) {
                                const __m128i m = _mm_setr_epi8(0, 1, 4, 5, 8, 9, 12, 13, -1, -1, -1, -1, -1, -1, -1, -1);
                                return _mm_shuffle_epi8(i32, m);
                            }
                            else {
                                auto m = _mm_srai_epi32(_mm_slli_epi32(i32, 16), 16);
                                return _mm_packs_epi32(m, _mm_setzero_si128());
                            }
                        }
                        else {
                            const __m128i zero = _mm_setzero_si128();
                            if constexpr (epu16<To>) {
                                if constexpr (has_s41) return _mm_packus_epi32(i32, zero);
                                else return scalar_fallback_convert_<ISA, Saturated, From, To>(x);
                            }
                            else return _mm_packs_epi32(i32, zero);
                        }
                    }
                    else if constexpr (y_to_x && has_ymm_v) {
                        auto i32 = _mm256_cvttpd_epi32(x);
                        if constexpr (has_bwvl) {
                            if constexpr (Saturated) {
                                if constexpr (epu16<To>) return _mm_cvtusepi32_epi16(i32);
                                else return _mm_cvtsepi32_epi16(i32);
                            }
                            else return _mm_cvtepi32_epi16(i32);
                        }
                        else if constexpr (!Saturated) {
                            if constexpr (has_ssse3<ISA>) {
                                const __m128i m = _mm_setr_epi8(0, 1, 4, 5, 8, 9, 12, 13, -1, -1, -1, -1, -1, -1, -1, -1);
                                return _mm_shuffle_epi8(i32, m);
                            }
                            else {
                                auto m = _mm_srai_epi32(_mm_slli_epi32(i32, 16), 16);
                                return _mm_packs_epi32(m, _mm_setzero_si128());
                            }
                        }
                        else {
                            const __m128i zero = _mm_setzero_si128();
                            if constexpr (epu16<To>) {
                                if constexpr (has_s41) return _mm_packus_epi32(i32, zero);
                                else return scalar_fallback_convert_<ISA, Saturated, From, To>(x);
                            }
                            else return _mm_packs_epi32(i32, zero);
                        }
                    }
                    else if constexpr (z_to_x && has_zmm) {
                        auto i32 = _mm512_cvttpd_epi32(x);
                        if constexpr (has_bwvl) {
                            if constexpr (Saturated) {
                                if constexpr (epu16<To>) return _mm256_cvtusepi32_epi16(i32);
                                else return _mm256_cvtsepi32_epi16(i32);
                            }
                            else return _mm256_cvtepi32_epi16(i32);
                        }
                        else return scalar_fallback_convert_<ISA, Saturated, From, To>(x);
                    }
                    else return scalar_fallback_convert_<ISA, Saturated, From, To>(x);
                }
                else if constexpr (sizeof(To) == 1) {
                    if constexpr (x_to_x && has_sse2<ISA>) {
                        auto i32 = _mm_cvttpd_epi32(x);
                        if constexpr (has_bwvl) {
                            if constexpr (Saturated) {
                                if constexpr (epu8<To>) return _mm_cvtusepi32_epi8(i32);
                                else return _mm_cvtsepi32_epi8(i32);
                            }
                            else return _mm_cvtepi32_epi8(i32);
                        }
                        else return scalar_fallback_convert_<ISA, Saturated, From, To>(x);
                    }
                    else if constexpr (y_to_x && has_ymm_v) {
                        auto i32 = _mm256_cvttpd_epi32(x);
                        if constexpr (has_bwvl) {
                            if constexpr (Saturated) {
                                if constexpr (epu8<To>) return _mm_cvtusepi32_epi8(i32);
                                else return _mm_cvtsepi32_epi8(i32);
                            }
                            else return _mm_cvtepi32_epi8(i32);
                        }
                        else return scalar_fallback_convert_<ISA, Saturated, From, To>(x);
                    }
                    else if constexpr (z_to_x && has_zmm) {
                        auto i32 = _mm512_cvttpd_epi32(x);
                        if constexpr (has_bwvl) {
                            if constexpr (Saturated) {
                                if constexpr (epu8<To>) return _mm256_cvtusepi32_epi8(i32);
                                else return _mm256_cvtsepi32_epi8(i32);
                            }
                            else return _mm256_cvtepi32_epi8(i32);
                        }
                        else return scalar_fallback_convert_<ISA, Saturated, From, To>(x);
                    }
                    else return scalar_fallback_convert_<ISA, Saturated, From, To>(x);
                }
                else return scalar_fallback_convert_<ISA, Saturated, From, To>(x);
            }
            else return scalar_fallback_convert_<ISA, Saturated, From, To>(x);
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
                            if constexpr (frm_sgn) return std::tuple{
                                _mm256_cvtepi32_ps(_mm256_cvtepi8_epi32(x)),
                                _mm256_cvtepi32_ps(_mm256_cvtepi8_epi32(_mm_srli_si128(x, 8))) };
                            else return std::tuple{
                                _mm256_cvtepi32_ps(_mm256_cvtepu8_epi32(x)),
                                _mm256_cvtepi32_ps(_mm256_cvtepu8_epi32(_mm_srli_si128(x, 8))) };
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
                            __m128 f0 = _mm_cvtepi32_ps(e0);
                            __m128 f1 = _mm_cvtepi32_ps(e1);
                            __m128 f2 = _mm_cvtepi32_ps(e2);
                            __m128 f3 = _mm_cvtepi32_ps(e3);
                            return std::tuple{
                                _mm256_insertf128_ps(_mm256_castps128_ps256(f0), f1, 1),
                                _mm256_insertf128_ps(_mm256_castps128_ps256(f2), f3, 1) };
                        }
                        else if constexpr (has_s41) {
                            if constexpr (frm_sgn) return std::tuple{
                                _mm_cvtepi32_ps(_mm_cvtepi8_epi32(x)),
                                _mm_cvtepi32_ps(_mm_cvtepi8_epi32(_mm_srli_si128(x, 4))),
                                _mm_cvtepi32_ps(_mm_cvtepi8_epi32(_mm_srli_si128(x, 8))),
                                _mm_cvtepi32_ps(_mm_cvtepi8_epi32(_mm_srli_si128(x, 12))) };
                            else return std::tuple{
                                _mm_cvtepi32_ps(_mm_cvtepu8_epi32(x)),
                                _mm_cvtepi32_ps(_mm_cvtepu8_epi32(_mm_srli_si128(x, 4))),
                                _mm_cvtepi32_ps(_mm_cvtepu8_epi32(_mm_srli_si128(x, 8))),
                                _mm_cvtepi32_ps(_mm_cvtepu8_epi32(_mm_srli_si128(x, 12))) };
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
                            if constexpr (frm_sgn) return std::tuple{
                                _mm_cvtepi32_ps(_mm_srai_epi32(_mm_unpacklo_epi16(lo16, lo16), 16)),
                                _mm_cvtepi32_ps(_mm_srai_epi32(_mm_unpackhi_epi16(lo16, lo16), 16)),
                                _mm_cvtepi32_ps(_mm_srai_epi32(_mm_unpacklo_epi16(hi16, hi16), 16)),
                                _mm_cvtepi32_ps(_mm_srai_epi32(_mm_unpackhi_epi16(hi16, hi16), 16)) };
                            else {
                                __m128i zero = _mm_setzero_si128();
                                return std::tuple{
                                    _mm_cvtepi32_ps(_mm_unpacklo_epi16(lo16, zero)),
                                    _mm_cvtepi32_ps(_mm_unpackhi_epi16(lo16, zero)),
                                    _mm_cvtepi32_ps(_mm_unpacklo_epi16(hi16, zero)),
                                    _mm_cvtepi32_ps(_mm_unpackhi_epi16(hi16, zero)) };
                            }
                        }
                    }
                    else if constexpr (sizeof(R) == 128 && has_zmm) {
                        auto lo = _mm256_castsi256_si128(x);
                        auto hi = _mm256_extracti128_si256(x, 1);
                        if constexpr (frm_sgn) return std::tuple{
                            _mm512_cvtepi32_ps(_mm512_cvtepi8_epi32(lo)),
                            _mm512_cvtepi32_ps(_mm512_cvtepi8_epi32(hi)) };
                        else return std::tuple{
                            _mm512_cvtepi32_ps(_mm512_cvtepu8_epi32(lo)),
                            _mm512_cvtepi32_ps(_mm512_cvtepu8_epi32(hi)) };
                    }
                    else if constexpr (sizeof(R) == 128 && has_avx2_v) {
                        auto lo = _mm256_castsi256_si128(x);
                        auto hi = _mm256_extracti128_si256(x, 1);
                        if constexpr (frm_sgn) return std::tuple{
                            _mm256_cvtepi32_ps(_mm256_cvtepi8_epi32(lo)),
                            _mm256_cvtepi32_ps(_mm256_cvtepi8_epi32(_mm_srli_si128(lo, 8))),
                            _mm256_cvtepi32_ps(_mm256_cvtepi8_epi32(hi)),
                            _mm256_cvtepi32_ps(_mm256_cvtepi8_epi32(_mm_srli_si128(hi, 8))) };
                        else return std::tuple{
                            _mm256_cvtepi32_ps(_mm256_cvtepu8_epi32(lo)),
                            _mm256_cvtepi32_ps(_mm256_cvtepu8_epi32(_mm_srli_si128(lo, 8))),
                            _mm256_cvtepi32_ps(_mm256_cvtepu8_epi32(hi)),
                            _mm256_cvtepi32_ps(_mm256_cvtepu8_epi32(_mm_srli_si128(hi, 8))) };
                    }
                    else return scalar_fallback_convert_<ISA, Saturated, From, To>(x);
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
                        return _mm256_insertf128_ps(_mm256_castps128_ps256(_mm_cvtepi32_ps(e0)), _mm_cvtepi32_ps(e1), 1);
                    }
                    else if constexpr (x_to_y && has_s41) {
                        if constexpr (frm_sgn) return std::tuple{
                            _mm_cvtepi32_ps(_mm_cvtepi16_epi32(x)),
                            _mm_cvtepi32_ps(_mm_cvtepi16_epi32(_mm_srli_si128(x, 8))) };
                        else return std::tuple{
                            _mm_cvtepi32_ps(_mm_cvtepu16_epi32(x)),
                            _mm_cvtepi32_ps(_mm_cvtepu16_epi32(_mm_srli_si128(x, 8))) };
                    }
                    else if constexpr (y_to_z) {
                        if constexpr (has_zmm) {
                            if constexpr (frm_sgn) return _mm512_cvtepi32_ps(_mm512_cvtepi16_epi32(x));
                            else return _mm512_cvtepi32_ps(_mm512_cvtepu16_epi32(x));
                        }
                        else if constexpr (has_avx2_v) {
                            auto lo = _mm256_castsi256_si128(x);
                            auto hi = _mm256_extracti128_si256(x, 1);
                            if constexpr (frm_sgn) return std::tuple{
                                _mm256_cvtepi32_ps(_mm256_cvtepi16_epi32(lo)),
                                _mm256_cvtepi32_ps(_mm256_cvtepi16_epi32(hi)) };
                            else return std::tuple{
                                _mm256_cvtepi32_ps(_mm256_cvtepu16_epi32(lo)),
                                _mm256_cvtepi32_ps(_mm256_cvtepu16_epi32(hi)) };
                        }
                        else return scalar_fallback_convert_<ISA, Saturated, From, To>(x);
                    }
                    else if constexpr (x_to_y) {
                        if constexpr (frm_sgn) return std::tuple{
                            _mm_cvtepi32_ps(_mm_srai_epi32(_mm_unpacklo_epi16(x, x), 16)),
                            _mm_cvtepi32_ps(_mm_srai_epi32(_mm_unpackhi_epi16(x, x), 16)) };
                        else {
                            __m128i zero = _mm_setzero_si128();
                            return std::tuple{
                                _mm_cvtepi32_ps(_mm_unpacklo_epi16(x, zero)),
                                _mm_cvtepi32_ps(_mm_unpackhi_epi16(x, zero)) };
                        }
                    }
                    else if constexpr (sizeof(R) == 128 && has_zmm) {
                        auto lo = _mm512_castsi512_si256(x);
                        auto hi = _mm512_extracti64x4_epi64(x, 1);
                        if constexpr (frm_sgn) return std::tuple{
                            _mm512_cvtepi32_ps(_mm512_cvtepi16_epi32(lo)),
                            _mm512_cvtepi32_ps(_mm512_cvtepi16_epi32(hi)) };
                        else return std::tuple{
                            _mm512_cvtepi32_ps(_mm512_cvtepu16_epi32(lo)),
                            _mm512_cvtepi32_ps(_mm512_cvtepu16_epi32(hi)) };
                    }
                    else return scalar_fallback_convert_<ISA, Saturated, From, To>(x);
                }
                else if constexpr (sizeof(From) == 4) {
                    if constexpr (frm_sgn) {
                        if constexpr (x_to_x) return _mm_cvtepi32_ps(x);
                        else if constexpr (y_to_y && has_ymm_v) return _mm256_cvtepi32_ps(x);
                        else if constexpr (z_to_z && has_zmm) return _mm512_cvtepi32_ps(x);
                        else return scalar_fallback_convert_<ISA, Saturated, From, To>(x);
                    }
                    else {
                        if constexpr (x_to_x && has_zmm && has_vl) return _mm_cvtepu32_ps(x);
                        else if constexpr (y_to_y && has_zmm && has_vl) return _mm256_cvtepu32_ps(x);
                        else if constexpr (z_to_z && has_zmm) return _mm512_cvtepu32_ps(x);
                        else return scalar_fallback_convert_<ISA, Saturated, From, To>(x);
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
                    else return scalar_fallback_convert_<ISA, Saturated, From, To>(x);
                }
                else return scalar_fallback_convert_<ISA, Saturated, From, To>(x);
            }
            else if constexpr (sizeof(To) == 8) {
                if constexpr (sizeof(From) == 1) {
                    if constexpr (sizeof(R) == 128 && has_zmm) {
                        __m512i e32;
                        if constexpr (frm_sgn) e32 = _mm512_cvtepi8_epi32(x);
                        else e32 = _mm512_cvtepu8_epi32(x);
                        return std::tuple{
                            _mm512_cvtepi32_pd(_mm512_castsi512_si256(e32)),
                            _mm512_cvtepi32_pd(_mm512_extracti64x4_epi64(e32, 1)) };
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
                        return std::tuple{
                            _mm256_cvtepi32_pd(_mm256_castsi256_si128(lo32)),
                            _mm256_cvtepi32_pd(_mm256_extracti128_si256(lo32, 1)),
                            _mm256_cvtepi32_pd(_mm256_castsi256_si128(hi32)),
                            _mm256_cvtepi32_pd(_mm256_extracti128_si256(hi32, 1)) };
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
                        return std::tuple{ _mm256_cvtepi32_pd(e0), _mm256_cvtepi32_pd(e1), _mm256_cvtepi32_pd(e2), _mm256_cvtepi32_pd(e3) };
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
                        return std::tuple{
                            _mm_cvtepi32_pd(e0), _mm_cvtepi32_pd(_mm_srli_si128(e0, 8)),
                            _mm_cvtepi32_pd(e1), _mm_cvtepi32_pd(_mm_srli_si128(e1, 8)),
                            _mm_cvtepi32_pd(e2), _mm_cvtepi32_pd(_mm_srli_si128(e2, 8)),
                            _mm_cvtepi32_pd(e3), _mm_cvtepi32_pd(_mm_srli_si128(e3, 8)) };
                    }
                   else if constexpr (sizeof(V) && sizeof(R) == 128 && has_sse2<ISA>) {
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
                        __m128i e0, e1, e2, e3;
                        if constexpr (frm_sgn) {
                            e0 = _mm_srai_epi32(_mm_unpacklo_epi16(lo16, lo16), 16);
                            e1 = _mm_srai_epi32(_mm_unpackhi_epi16(lo16, lo16), 16);
                            e2 = _mm_srai_epi32(_mm_unpacklo_epi16(hi16, hi16), 16);
                            e3 = _mm_srai_epi32(_mm_unpackhi_epi16(hi16, hi16), 16);
                        }
                        else {
                            __m128i zero = _mm_setzero_si128();
                            e0 = _mm_unpacklo_epi16(lo16, zero);
                            e1 = _mm_unpackhi_epi16(lo16, zero);
                            e2 = _mm_unpacklo_epi16(hi16, zero);
                            e3 = _mm_unpackhi_epi16(hi16, zero);
                        }
                        return std::tuple{
                            _mm_cvtepi32_pd(e0), _mm_cvtepi32_pd(_mm_srli_si128(e0, 8)),
                            _mm_cvtepi32_pd(e1), _mm_cvtepi32_pd(_mm_srli_si128(e1, 8)),
                            _mm_cvtepi32_pd(e2), _mm_cvtepi32_pd(_mm_srli_si128(e2, 8)),
                            _mm_cvtepi32_pd(e3), _mm_cvtepi32_pd(_mm_srli_si128(e3, 8)) };
                    }
                    else return scalar_fallback_convert_<ISA, Saturated, From, To>(x);
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
                            return pack_or_tuple<ISA>(_mm256_cvtepi32_pd(_mm256_castsi256_si128(w32)),
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
                            return pack_or_tuple<ISA>(_mm256_cvtepi32_pd(w0), _mm256_cvtepi32_pd(w1));
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
                            return pack_or_tuple<ISA>(_mm_cvtepi32_pd(w0), _mm_cvtepi32_pd(_mm_srli_si128(w0, 8)),
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
                            return pack_or_tuple<ISA>(_mm_cvtepi32_pd(w0), _mm_cvtepi32_pd(_mm_srli_si128(w0, 8)),
                                _mm_cvtepi32_pd(w1), _mm_cvtepi32_pd(_mm_srli_si128(w1, 8)));
                        }
                        else return scalar_fallback_convert_<ISA, Saturated, From, To>(x);
                    }
                    else if constexpr (sizeof(R) == 128 && sizeof(V) == 32 && has_zmm) {
                        __m512i e32;
                        if constexpr (frm_sgn) e32 = _mm512_cvtepi16_epi32(x);
                        else e32 = _mm512_cvtepu16_epi32(x);
                        auto lo = _mm512_castsi512_si256(e32);
                        auto hi = _mm512_extracti64x4_epi64(e32, 1);
                        return std::tuple{ _mm512_cvtepi32_pd(lo), _mm512_cvtepi32_pd(hi) };
                    }
                    else if constexpr (sizeof(R) == 128 && has_avx2_v) {
                        auto lo = _mm256_castsi256_si128(x);
                        auto hi = _mm256_extracti128_si256(x, 1);
                        __m256i w_lo, w_hi;
                        if constexpr (frm_sgn) {
                            w_lo = _mm256_cvtepi16_epi32(lo);
                            w_hi = _mm256_cvtepi16_epi32(hi);
                        }
                        else {
                            w_lo = _mm256_cvtepu16_epi32(lo);
                            w_hi = _mm256_cvtepu16_epi32(hi);
                        }
                        return std::tuple{
                            _mm256_cvtepi32_pd(_mm256_castsi256_si128(w_lo)),
                            _mm256_cvtepi32_pd(_mm256_extracti128_si256(w_lo, 1)),
                            _mm256_cvtepi32_pd(_mm256_castsi256_si128(w_hi)),
                            _mm256_cvtepi32_pd(_mm256_extracti128_si256(w_hi, 1)) };
                    }
                    else return scalar_fallback_convert_<ISA, Saturated, From, To>(x);
                }
                else if constexpr (sizeof(From) == 4) {
                    if constexpr (frm_sgn) {
                        if constexpr (x_to_y && has_ymm_v) return _mm256_cvtepi32_pd(x);
                        else if constexpr (y_to_z) {
                            if constexpr (has_zmm) return _mm512_cvtepi32_pd(x);
                            else if constexpr (has_avx2_v) {
                                auto lo = _mm256_castsi256_si128(x);
                                auto hi = _mm256_extracti128_si256(x, 1);
                                return std::tuple{ _mm256_cvtepi32_pd(lo), _mm256_cvtepi32_pd(hi) };
                            }
                            else return scalar_fallback_convert_<ISA, Saturated, From, To>(x);
                        }
                        else if constexpr (sizeof(V) == 64 && sizeof(R) == 128 && has_zmm) {
                            auto lo = _mm512_castsi512_si256(x);
                            auto hi = _mm512_extracti64x4_epi64(x, 1);
                            return std::tuple{ _mm512_cvtepi32_pd(lo), _mm512_cvtepi32_pd(hi) };
                        }
                        else return scalar_fallback_convert_<ISA, Saturated, From, To>(x);
                    }
                    else {
                        if constexpr (sizeof(V) == 64 && sizeof(R) == 128 && has_zmm) {
                            auto lo = _mm512_castsi512_si256(x);
                            auto hi = _mm512_extracti64x4_epi64(x, 1);
                            return std::tuple{ _mm512_cvtepu32_pd(lo), _mm512_cvtepu32_pd(hi) };
                        }
                        else if constexpr (x_to_y && has_zmm && has_vl) return _mm256_cvtepu32_pd(x);
                        else if constexpr (y_to_z && has_zmm) return _mm512_cvtepu32_pd(x);
                        else return scalar_fallback_convert_<ISA, Saturated, From, To>(x);
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
                    else return scalar_fallback_convert_<ISA, Saturated, From, To>(x);
                }
                else return scalar_fallback_convert_<ISA, Saturated, From, To>(x);
            }
            else return scalar_fallback_convert_<ISA, Saturated, From, To>(x);
        }
        else if constexpr (f_to_f) {
            if constexpr (sizeof(From) == 4) {
                if constexpr (x_to_y) {
                    if constexpr (has_ymm_v) return _mm256_cvtps_pd(x);
                    else return scalar_fallback_convert_<ISA, Saturated, From, To>(x);
                }
                else if constexpr (y_to_z) {
                    if constexpr (has_zmm) return _mm512_cvtps_pd(x);
                    else return scalar_fallback_convert_<ISA, Saturated, From, To>(x);
                }
                else if constexpr (x_to_x) {
                    auto lo = _mm_cvtps_pd(x);
                    auto hi = _mm_cvtps_pd(_mm_movehl_ps(x, x));
                    return pack_or_tuple<ISA>(lo, hi);
                }
                else return scalar_fallback_convert_<ISA, Saturated, From, To>(x);
            }
            else if constexpr (sizeof(From) == 8) {
                if constexpr (x_to_x) return _mm_cvtpd_ps(x);
                else if constexpr (y_to_x) {
                    if constexpr (has_ymm_v) return _mm256_cvtpd_ps(x);
                    else return scalar_fallback_convert_<ISA, Saturated, From, To>(x);
                }
                else if constexpr (z_to_y) {
                    if constexpr (has_zmm) return _mm512_cvtpd_ps(x);
                    else return scalar_fallback_convert_<ISA, Saturated, From, To>(x);
                }
                else return scalar_fallback_convert_<ISA, Saturated, From, To>(x);
            }
            else return scalar_fallback_convert_<ISA, Saturated, From, To>(x);
        }
        else return scalar_fallback_convert_<ISA, Saturated, From, To>(x);
    }
}

__RAZE_VX_NAMESPACE_END