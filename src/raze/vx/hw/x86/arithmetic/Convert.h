#pragma once 

#include <src/raze/vx/hw/x86/memory/Load.h>
#include <src/raze/vx/hw/x86/memory/Store.h>
#include <tuple>
#include <utility>

__RAZE_VX_NAMESPACE_BEGIN

template <sizetype I, class V>
using identity_v_ = V;

template <class T>
struct is_tuple_ : std::false_type {};

template <class... Ts>
struct is_tuple_<std::tuple<Ts...>> : std::true_type {};

template <arch::ISA ISA, class From, class To, class V>
consteval auto deduce_convert_return_type_() {
	if constexpr (sizeof(V) < 16) {
		return To{};
	} else if constexpr (sizeof(From) == sizeof(To)) {
		return traits::deduce_simd_vector_type<To, sizeof(V) * 8>{};
	} else {
		constexpr sizetype num_elements = sizeof(V) / sizeof(From);
		constexpr sizetype total_bytes = num_elements * sizeof(To);

		if constexpr (total_bytes <= 16) {
			return traits::deduce_simd_vector_type<To, 128>{};
		}
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

			return [] <sizetype ... Elements> (std::integer_sequence<sizetype, Elements...>) {
				return std::tuple{ identity_v_<Elements, traits::deduce_simd_vector_type<To, chunk_size * 8>>{}... };
			}(std::make_integer_sequence<sizetype, num_chunks>{});
		}
	}
}

template <arch::ISA ISA, class From, class To, class V>
using deduce_convert_return_type = decltype(deduce_convert_return_type_<ISA, From, To, V>());

template <class V>
raze_always_inline auto combine_ymm(V lo, V hi) noexcept {
	if constexpr (std::same_as<V, __m128>) return _mm256_insertf128_ps(_mm256_castps128_ps256(lo), hi, 1);
	else if constexpr (std::same_as<V, __m128d>) return _mm256_insertf128_pd(_mm256_castpd128_pd256(lo), hi, 1);
	else return _mm256_insertf128_si256(_mm256_castsi128_si256(lo), hi, 1);
}

template <class V>
raze_always_inline auto combine_zmm(V lo, V hi) noexcept {
	if constexpr (std::same_as<V, __m256>) return _mm512_insertf32x8(_mm512_castps256_ps512(lo), hi, 1);
	else if constexpr (std::same_as<V, __m256d>) return _mm512_insertf64x4(_mm512_castpd256_pd512(lo), hi, 1);
	else return _mm512_inserti64x4(_mm512_castsi256_si512(lo), hi, 1);
}

template <arch::ISA ISA, class To, class Part>
raze_always_inline auto pack_or_tuple(Part p0, Part p1) noexcept {
	if constexpr (has_avx<ISA> && sizeof(Part) == 16) return combine_ymm(p0, p1);
	else if constexpr (has_avx512f<ISA> && sizeof(Part) == 32) return combine_zmm(p0, p1);
	else return std::tuple{ p0, p1 };
}

template <arch::ISA ISA, class To, class Part>
raze_always_inline auto pack_or_tuple(Part p0, Part p1, Part p2, Part p3) noexcept {
	if constexpr (has_avx512f<ISA>) return combine_zmm(combine_ymm(p0, p1), combine_ymm(p2, p3));
	else if constexpr (has_avx<ISA>) return std::tuple{ combine_ymm(p0, p1), combine_ymm(p2, p3) };
	else return std::tuple{ p0, p1, p2, p3 };
}

template <arch::ISA ISA, class Target, class To, sizetype N>
raze_always_inline Target load_target_from_buffer_(const To* buf) noexcept {
	if constexpr (is_tuple_<Target>::value) {
		return[] <sizetype... Is> (const To* b, std::index_sequence<Is...>) {
			using FirstChunk = std::tuple_element_t<0, Target>;
			constexpr sizetype elems_per_chunk = sizeof(FirstChunk) / sizeof(To);
			return Target{ load_<ISA, std::tuple_element_t<Is, Target>>(b + Is * elems_per_chunk)... };
		}(buf, std::make_index_sequence<std::tuple_size_v<Target>>{});
	}
	else {
		return load_<ISA, Target>(buf);
	}
}

template <arch::ISA ISA, class From, class To, class V>
raze_always_inline deduce_convert_return_type<ISA, From, To, V> scalar_fallback_convert_(V x) noexcept {
	constexpr sizetype count = sizeof(V) / sizeof(From);
	alignas(64) From arr[count];
	store_(arr, x);

	using Ret = deduce_convert_return_type<ISA, From, To, V>;
	constexpr sizetype ret_bytes = sizeof(Ret);
	constexpr sizetype out_count = (ret_bytes > count * sizeof(To)) ? (ret_bytes / sizeof(To)) : count;

	alignas(64) To out[out_count]{};
	for (sizetype i = 0; i < count; ++i)
		out[i] = static_cast<To>(arr[i]);

	return load_target_from_buffer_<ISA, Ret, To, count>(out);
}

template <arch::ISA ISA, arithmetic_type From, arithmetic_type To, intrin_or_arithmetic_type V>
raze_always_inline deduce_convert_return_type<ISA, From, To, V> convert_(V x) noexcept {
	constexpr auto has_ymm = has_avx<ISA>;
	constexpr auto has_zmm = has_avx512f<ISA>;

	if constexpr (std::same_as<To, From>) return x;

	if constexpr (sizeof(V) == 16) {
		if constexpr (std::same_as<From, float>) {
			if constexpr (std::same_as<To, double>) {
				if constexpr (has_ymm) return _mm256_cvtps_pd(x);
				else if constexpr (has_sse2<ISA>) {
					auto low = _mm_cvtps_pd(x);
					auto high = _mm_cvtps_pd(_mm_movehl_ps(x, x));
					return pack_or_tuple<ISA, To>(low, high);
				}
				else return scalar_fallback_convert_<ISA, From, To>(x);
			}
			else if constexpr (epi32<To>) return _mm_cvttps_epi32(x);
			else if constexpr (epu32<To>) {
				if constexpr (has_avx512vl<ISA> && has_avx512f<ISA>) return _mm_cvttps_epu32(x);
				else return scalar_fallback_convert_<ISA, From, To>(x);
			}
			else if constexpr (epi16<To> || epu16<To>) {
				if constexpr (has_avx512vl<ISA> && has_avx512f<ISA>) return _mm_cvtepi32_epi16(_mm_cvttps_epi32(x));
				else if constexpr (epi16<To>) return _mm_packs_epi32(_mm_cvttps_epi32(x), _mm_setzero_si128());
				else if constexpr (has_sse41<ISA>) return _mm_packus_epi32(_mm_cvttps_epi32(x), _mm_setzero_si128());
				else return scalar_fallback_convert_<ISA, From, To>(x);
			}
			else if constexpr (epi8<To> || epu8<To>) {
				if constexpr (has_avx512vl<ISA> && has_avx512f<ISA>) return _mm_cvtepi32_epi8(_mm_cvttps_epi32(x));
				else return scalar_fallback_convert_<ISA, From, To>(x);
			}
			else return scalar_fallback_convert_<ISA, From, To>(x);
		}
		else if constexpr (std::same_as<From, double>) {
			if constexpr (std::same_as<To, float>) return _mm_cvtpd_ps(x);
			else if constexpr (epi32<To>) return _mm_cvttpd_epi32(x);
			else if constexpr (epu32<To>) {
				if constexpr (has_avx512vl<ISA> && has_avx512f<ISA>) return _mm_cvttpd_epu32(x);
				else return scalar_fallback_convert_<ISA, From, To>(x);
			}
			else if constexpr (epi64<To>) {
				if constexpr (has_avx512vl<ISA> && has_avx512dq<ISA>) return _mm_cvttpd_epi64(x);
				else return scalar_fallback_convert_<ISA, From, To>(x);
			}
			else if constexpr (epu64<To>) {
				if constexpr (has_avx512vl<ISA> && has_avx512dq<ISA>) return _mm_cvttpd_epu64(x);
				else return scalar_fallback_convert_<ISA, From, To>(x);
			}
			else return scalar_fallback_convert_<ISA, From, To>(x);
		}
		else if constexpr (epi8<From> || epu8<From>) {
			constexpr bool from_signed = epi8<From>;

			if constexpr (epi16<To> || epu16<To>) {
				if constexpr (has_avx2<ISA>) {
					if constexpr (from_signed) return _mm256_cvtepi8_epi16(x);
					else return _mm256_cvtepu8_epi16(x);
				} else if constexpr (has_sse41<ISA>) {
					if constexpr (from_signed) {
						auto low = _mm_cvtepi8_epi16(x);
						auto high = _mm_cvtepi8_epi16(_mm_shuffle_epi32(x, 0xEE));
						return pack_or_tuple<ISA, To>(low, high);
					} else {
						auto low = _mm_cvtepu8_epi16(x);
						auto high = _mm_cvtepu8_epi16(_mm_shuffle_epi32(x, 0xEE));
						return pack_or_tuple<ISA, To>(low, high);
					}
				} else {
					if constexpr (from_signed) {
						auto low = _mm_srai_epi16(_mm_unpacklo_epi8(x, x), 8);
						auto high = _mm_srai_epi16(_mm_unpackhi_epi8(x, x), 8);
						return pack_or_tuple<ISA, To>(low, high);
					} else {
						auto zero = _mm_setzero_si128();
						auto low = _mm_unpacklo_epi8(x, zero);
						auto high = _mm_unpackhi_epi8(x, zero);
						return pack_or_tuple<ISA, To>(low, high);
					}
				}
			}
			else if constexpr (epi32<To> || epu32<To>) {
				if constexpr (has_zmm) {
					if constexpr (from_signed) return _mm512_cvtepi8_epi32(x);
					else return _mm512_cvtepu8_epi32(x);
				} else if constexpr (has_avx2<ISA>) {
					if constexpr (from_signed) {
						auto y0 = _mm256_cvtepi8_epi32(x);
						auto y1 = _mm256_cvtepi8_epi32(_mm_srli_si128(x, 8));
						return std::tuple{ y0, y1 };
					} else {
						auto y0 = _mm256_cvtepu8_epi32(x);
						auto y1 = _mm256_cvtepu8_epi32(_mm_srli_si128(x, 8));
						return std::tuple{ y0, y1 };
					}
				} else if constexpr (has_sse41<ISA>) {
					if constexpr (from_signed) {
						auto p0 = _mm_cvtepi8_epi32(x);
						auto p1 = _mm_cvtepi8_epi32(_mm_srli_si128(x, 4));
						auto p2 = _mm_cvtepi8_epi32(_mm_srli_si128(x, 8));
						auto p3 = _mm_cvtepi8_epi32(_mm_srli_si128(x, 12));
						return pack_or_tuple<ISA, To>(p0, p1, p2, p3);
					} else {
						auto p0 = _mm_cvtepu8_epi32(x);
						auto p1 = _mm_cvtepu8_epi32(_mm_srli_si128(x, 4));
						auto p2 = _mm_cvtepu8_epi32(_mm_srli_si128(x, 8));
						auto p3 = _mm_cvtepu8_epi32(_mm_srli_si128(x, 12));
						return pack_or_tuple<ISA, To>(p0, p1, p2, p3);
					}
				} else {
					return scalar_fallback_convert_<ISA, From, To>(x);
				}
			}
			else if constexpr (std::same_as<To, float>) {
				if constexpr (has_zmm) {
					if constexpr (from_signed) return _mm512_cvtepi32_ps(_mm512_cvtepi8_epi32(x));
					else return _mm512_cvtepi32_ps(_mm512_cvtepu8_epi32(x));
				} else if constexpr (has_avx2<ISA>) {
					if constexpr (from_signed) {
						auto y0 = _mm256_cvtepi32_ps(_mm256_cvtepi8_epi32(x));
						auto y1 = _mm256_cvtepi32_ps(_mm256_cvtepi8_epi32(_mm_srli_si128(x, 8)));
						return std::tuple{ y0, y1 };
					} else {
						auto y0 = _mm256_cvtepi32_ps(_mm256_cvtepu8_epi32(x));
						auto y1 = _mm256_cvtepi32_ps(_mm256_cvtepu8_epi32(_mm_srli_si128(x, 8)));
						return std::tuple{ y0, y1 };
					}
				} else if constexpr (has_sse41<ISA>) {
					if constexpr (from_signed) {
						auto p0 = _mm_cvtepi32_ps(_mm_cvtepi8_epi32(x));
						auto p1 = _mm_cvtepi32_ps(_mm_cvtepi8_epi32(_mm_srli_si128(x, 4)));
						auto p2 = _mm_cvtepi32_ps(_mm_cvtepi8_epi32(_mm_srli_si128(x, 8)));
						auto p3 = _mm_cvtepi32_ps(_mm_cvtepi8_epi32(_mm_srli_si128(x, 12)));
						return pack_or_tuple<ISA, To>(p0, p1, p2, p3);
					} else {
						auto p0 = _mm_cvtepi32_ps(_mm_cvtepu8_epi32(x));
						auto p1 = _mm_cvtepi32_ps(_mm_cvtepu8_epi32(_mm_srli_si128(x, 4)));
						auto p2 = _mm_cvtepi32_ps(_mm_cvtepu8_epi32(_mm_srli_si128(x, 8)));
						auto p3 = _mm_cvtepi32_ps(_mm_cvtepu8_epi32(_mm_srli_si128(x, 12)));
						return pack_or_tuple<ISA, To>(p0, p1, p2, p3);
					}
				} else {
					return scalar_fallback_convert_<ISA, From, To>(x);
				}
			}
			else if constexpr (epi64<To> || epu64<To>) {
				if constexpr (has_zmm) {
					if constexpr (from_signed) {
						auto z0 = _mm512_cvtepi8_epi64(x);
						auto z1 = _mm512_cvtepi8_epi64(_mm_srli_si128(x, 8));
						return std::tuple{ z0, z1 };
					} else {
						auto z0 = _mm512_cvtepu8_epi64(x);
						auto z1 = _mm512_cvtepu8_epi64(_mm_srli_si128(x, 8));
						return std::tuple{ z0, z1 };
					}
				} else if constexpr (has_avx2<ISA>) {
					if constexpr (from_signed) {
						auto y0 = _mm256_cvtepi8_epi64(x);
						auto y1 = _mm256_cvtepi8_epi64(_mm_srli_si128(x, 4));
						auto y2 = _mm256_cvtepi8_epi64(_mm_srli_si128(x, 8));
						auto y3 = _mm256_cvtepi8_epi64(_mm_srli_si128(x, 12));
						return std::tuple{ y0, y1, y2, y3 };
					} else {
						auto y0 = _mm256_cvtepu8_epi64(x);
						auto y1 = _mm256_cvtepu8_epi64(_mm_srli_si128(x, 4));
						auto y2 = _mm256_cvtepu8_epi64(_mm_srli_si128(x, 8));
						auto y3 = _mm256_cvtepu8_epi64(_mm_srli_si128(x, 12));
						return std::tuple{ y0, y1, y2, y3 };
					}
				} else {
					return scalar_fallback_convert_<ISA, From, To>(x);
				}
			}
			else if constexpr (std::same_as<To, double>) {
				if constexpr (has_zmm) {
					if constexpr (from_signed) {
						auto z0 = _mm512_cvtepi32_pd(_mm256_cvtepi8_epi32(x));
						auto z1 = _mm512_cvtepi32_pd(_mm256_cvtepi8_epi32(_mm_srli_si128(x, 8)));
						return std::tuple{ z0, z1 };
					} else {
						auto z0 = _mm512_cvtepi32_pd(_mm256_cvtepu8_epi32(x));
						auto z1 = _mm512_cvtepi32_pd(_mm256_cvtepu8_epi32(_mm_srli_si128(x, 8)));
						return std::tuple{ z0, z1 };
					}
				} else if constexpr (has_avx2<ISA>) {
					if constexpr (from_signed) {
						auto y0 = _mm256_cvtepi32_pd(_mm_cvtepi8_epi32(x));
						auto y1 = _mm256_cvtepi32_pd(_mm_cvtepi8_epi32(_mm_srli_si128(x, 4)));
						auto y2 = _mm256_cvtepi32_pd(_mm_cvtepi8_epi32(_mm_srli_si128(x, 8)));
						auto y3 = _mm256_cvtepi32_pd(_mm_cvtepi8_epi32(_mm_srli_si128(x, 12)));
						return std::tuple{ y0, y1, y2, y3 };
					} else {
						auto y0 = _mm256_cvtepi32_pd(_mm_cvtepu8_epi32(x));
						auto y1 = _mm256_cvtepi32_pd(_mm_cvtepu8_epi32(_mm_srli_si128(x, 4)));
						auto y2 = _mm256_cvtepi32_pd(_mm_cvtepu8_epi32(_mm_srli_si128(x, 8)));
						auto y3 = _mm256_cvtepi32_pd(_mm_cvtepu8_epi32(_mm_srli_si128(x, 12)));
						return std::tuple{ y0, y1, y2, y3 };
					}
				} else {
					return scalar_fallback_convert_<ISA, From, To>(x);
				}
			} else {
				return scalar_fallback_convert_<ISA, From, To>(x);
			}
		}
		else if constexpr (epi16<From> || epu16<From>) {
			constexpr bool from_signed = epi16<From>;

			if constexpr (epi8<To> || epu8<To>) {
				if constexpr (has_avx512vl<ISA> && has_avx512bw<ISA>) return _mm_cvtepi16_epi8(x);
				else if constexpr (from_signed) return _mm_packs_epi16(x, _mm_setzero_si128());
				else return _mm_packus_epi16(x, _mm_setzero_si128());
			}
			else if constexpr (epi32<To> || epu32<To>) {
				if constexpr (has_avx2<ISA>) {
					if constexpr (from_signed) return _mm256_cvtepi16_epi32(x);
					else return _mm256_cvtepu16_epi32(x);
				} else if constexpr (has_sse41<ISA>) {
					if constexpr (from_signed) {
						auto low = _mm_cvtepi16_epi32(x);
						auto high = _mm_cvtepi16_epi32(_mm_shuffle_epi32(x, 0xEE));
						return pack_or_tuple<ISA, To>(low, high);
					} else {
						auto low = _mm_cvtepu16_epi32(x);
						auto high = _mm_cvtepu16_epi32(_mm_shuffle_epi32(x, 0xEE));
						return pack_or_tuple<ISA, To>(low, high);
					}
				} else {
					return scalar_fallback_convert_<ISA, From, To>(x);
				}
			}
			else if constexpr (std::same_as<To, float>) {
				if constexpr (has_avx2<ISA>) {
					if constexpr (from_signed) return _mm256_cvtepi32_ps(_mm256_cvtepi16_epi32(x));
					else return _mm256_cvtepi32_ps(_mm256_cvtepu16_epi32(x));
				} else if constexpr (has_sse41<ISA>) {
					if constexpr (from_signed) {
						auto low = _mm_cvtepi32_ps(_mm_cvtepi16_epi32(x));
						auto high = _mm_cvtepi32_ps(_mm_cvtepi16_epi32(_mm_shuffle_epi32(x, 0xEE)));
						return pack_or_tuple<ISA, To>(low, high);
					} else {
						auto low = _mm_cvtepi32_ps(_mm_cvtepu16_epi32(x));
						auto high = _mm_cvtepi32_ps(_mm_cvtepu16_epi32(_mm_shuffle_epi32(x, 0xEE)));
						return pack_or_tuple<ISA, To>(low, high);
					}
				} else {
					return scalar_fallback_convert_<ISA, From, To>(x);
				}
			}
			else if constexpr (epi64<To> || epu64<To>) {
				if constexpr (has_zmm) {
					if constexpr (from_signed) return _mm512_cvtepi16_epi64(x);
					else return _mm512_cvtepu16_epi64(x);
				} else if constexpr (has_avx2<ISA>) {
					if constexpr (from_signed) {
						auto y0 = _mm256_cvtepi16_epi64(x);
						auto y1 = _mm256_cvtepi16_epi64(_mm_shuffle_epi32(x, 0xEE));
						return std::tuple{ y0, y1 };
					} else {
						auto y0 = _mm256_cvtepu16_epi64(x);
						auto y1 = _mm256_cvtepu16_epi64(_mm_shuffle_epi32(x, 0xEE));
						return std::tuple{ y0, y1 };
					}
				} else if constexpr (has_sse41<ISA>) {
					if constexpr (from_signed) {
						auto p0 = _mm_cvtepi16_epi64(x);
						auto p1 = _mm_cvtepi16_epi64(_mm_srli_si128(x, 4));
						auto p2 = _mm_cvtepi16_epi64(_mm_srli_si128(x, 8));
						auto p3 = _mm_cvtepi16_epi64(_mm_srli_si128(x, 12));
						return pack_or_tuple<ISA, To>(p0, p1, p2, p3);
					} else {
						auto p0 = _mm_cvtepu16_epi64(x);
						auto p1 = _mm_cvtepu16_epi64(_mm_srli_si128(x, 4));
						auto p2 = _mm_cvtepu16_epi64(_mm_srli_si128(x, 8));
						auto p3 = _mm_cvtepu16_epi64(_mm_srli_si128(x, 12));
						return pack_or_tuple<ISA, To>(p0, p1, p2, p3);
					}
				} else {
					return scalar_fallback_convert_<ISA, From, To>(x);
				}
			}
			else if constexpr (std::same_as<To, double>) {
				if constexpr (has_zmm) {
					if constexpr (from_signed) return _mm512_cvtepi32_pd(_mm256_cvtepi16_epi32(x));
					else return _mm512_cvtepu32_pd(_mm256_cvtepu16_epi32(x));
				} else if constexpr (has_avx2<ISA>) {
					if constexpr (from_signed) {
						auto y0 = _mm256_cvtepi32_pd(_mm_cvtepi16_epi32(x));
						auto y1 = _mm256_cvtepi32_pd(_mm_cvtepi16_epi32(_mm_shuffle_epi32(x, 0xEE)));
						return std::tuple{ y0, y1 };
					} else {
						auto y0 = _mm256_cvtepi32_pd(_mm_cvtepu16_epi32(x));
						auto y1 = _mm256_cvtepi32_pd(_mm_cvtepu16_epi32(_mm_shuffle_epi32(x, 0xEE)));
						return std::tuple{ y0, y1 };
					}
				} else {
					return scalar_fallback_convert_<ISA, From, To>(x);
				}
			} else {
				return scalar_fallback_convert_<ISA, From, To>(x);
			}
		}
		else if constexpr (epi32<From> || epu32<From>) {
			constexpr bool from_signed = epi32<From>;

			if constexpr (epi8<To> || epu8<To>) {
				if constexpr (has_avx512vl<ISA> && has_avx512f<ISA>) return _mm_cvtepi32_epi8(x);
				else if constexpr (has_ssse3<ISA>) return _mm_shuffle_epi8(x, _mm_set_epi8(-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 12, 8, 4, 0));
				else return scalar_fallback_convert_<ISA, From, To>(x);
			}
			else if constexpr (epi16<To> || epu16<To>) {
				if constexpr (has_avx512vl<ISA> && has_avx512f<ISA>) return _mm_cvtepi32_epi16(x);
				else if constexpr (from_signed) return _mm_packs_epi32(x, _mm_setzero_si128());
				else if constexpr (has_sse41<ISA>) return _mm_packus_epi32(x, _mm_setzero_si128());
				else return scalar_fallback_convert_<ISA, From, To>(x);
			}
			else if constexpr (std::same_as<To, float>) {
				if constexpr (from_signed) return _mm_cvtepi32_ps(x);
				else if constexpr (has_avx512vl<ISA> && has_avx512f<ISA>) return _mm_cvtepu32_ps(x);
				else return scalar_fallback_convert_<ISA, From, To>(x);
			}
			else if constexpr (epi64<To> || epu64<To>) {
				if constexpr (has_avx2<ISA>) {
					if constexpr (from_signed) return _mm256_cvtepi32_epi64(x);
					else return _mm256_cvtepu32_epi64(x);
				} else if constexpr (has_sse41<ISA>) {
					if constexpr (from_signed) {
						auto low = _mm_cvtepi32_epi64(x);
						auto high = _mm_cvtepi32_epi64(_mm_shuffle_epi32(x, 0xEE));
						return pack_or_tuple<ISA, To>(low, high);
					} else {
						auto low = _mm_cvtepu32_epi64(x);
						auto high = _mm_cvtepu32_epi64(_mm_shuffle_epi32(x, 0xEE));
						return pack_or_tuple<ISA, To>(low, high);
					}
				} else {
					return scalar_fallback_convert_<ISA, From, To>(x);
				}
			}
			else if constexpr (std::same_as<To, double>) {
				if constexpr (from_signed) {
					if constexpr (has_ymm) return _mm256_cvtepi32_pd(x);
					else if constexpr (has_sse2<ISA>) {
						auto low = _mm_cvtepi32_pd(x);
						auto high = _mm_cvtepi32_pd(_mm_srli_si128(x, 8));
						return pack_or_tuple<ISA, To>(low, high);
					} else return scalar_fallback_convert_<ISA, From, To>(x);
				} else {
					if constexpr (has_avx512vl<ISA> && has_avx512f<ISA>) return _mm256_cvtepu32_pd(x);
					else return scalar_fallback_convert_<ISA, From, To>(x);
				}
			} else {
				return scalar_fallback_convert_<ISA, From, To>(x);
			}
		}
		else if constexpr (epi64<From> || epu64<From>) {
			constexpr bool from_signed = epi64<From>;

			if constexpr (epi8<To> || epu8<To>) {
				if constexpr (has_avx512vl<ISA> && has_avx512f<ISA>) return _mm_cvtepi64_epi8(x);
				else return scalar_fallback_convert_<ISA, From, To>(x);
			}
			else if constexpr (epi16<To> || epu16<To>) {
				if constexpr (has_avx512vl<ISA> && has_avx512f<ISA>) return _mm_cvtepi64_epi16(x);
				else return scalar_fallback_convert_<ISA, From, To>(x);
			}
			else if constexpr (epi32<To> || epu32<To>) {
				if constexpr (has_avx512vl<ISA> && has_avx512f<ISA>) return _mm_cvtepi64_epi32(x);
				else return scalar_fallback_convert_<ISA, From, To>(x);
			}
			else if constexpr (std::same_as<To, float>) {
				if constexpr (has_avx512vl<ISA> && has_avx512dq<ISA>) {
					if constexpr (from_signed) return _mm_cvtepi64_ps(x);
					else return _mm_cvtepu64_ps(x);
				} else return scalar_fallback_convert_<ISA, From, To>(x);
			}
			else if constexpr (std::same_as<To, double>) {
				if constexpr (has_avx512vl<ISA> && has_avx512dq<ISA>) {
					if constexpr (from_signed) return _mm_cvtepi64_pd(x);
					else return _mm_cvtepu64_pd(x);
				} else return scalar_fallback_convert_<ISA, From, To>(x);
			}
			else return scalar_fallback_convert_<ISA, From, To>(x);
		}
		else {
			return scalar_fallback_convert_<ISA, From, To>(x);
		}
	}
	else if constexpr (sizeof(V) == 32) {
		if constexpr (std::same_as<From, float>) {
			if constexpr (std::same_as<To, double>) {
				if constexpr (has_zmm) return _mm512_cvtps_pd(x);
				else if constexpr (has_ymm) {
					auto lo = _mm256_cvtps_pd(_mm256_castps256_ps128(x));
					auto hi = _mm256_cvtps_pd(_mm256_extractf128_ps(x, 1));
					return pack_or_tuple<ISA, To>(lo, hi);
				} else return scalar_fallback_convert_<ISA, From, To>(x);
			}
			else if constexpr (epi32<To>) return _mm256_cvttps_epi32(x);
			else if constexpr (epu32<To>) {
				if constexpr (has_avx512vl<ISA> && has_avx512f<ISA>) return _mm256_cvttps_epu32(x);
				else return scalar_fallback_convert_<ISA, From, To>(x);
			}
			else if constexpr (epi16<To> || epu16<To>) {
				if constexpr (has_avx512vl<ISA> && has_avx512f<ISA>) return _mm256_cvtepi32_epi16(_mm256_cvttps_epi32(x));
				else if constexpr (has_avx2<ISA>) {
					auto lo = _mm256_castsi256_si128(_mm256_cvttps_epi32(x));
					auto hi = _mm256_extracti128_si256(_mm256_cvttps_epi32(x), 1);
					if constexpr (epi16<To>) return _mm_packs_epi32(lo, hi);
					else return _mm_packus_epi32(lo, hi);
				}
				else return scalar_fallback_convert_<ISA, From, To>(x);
			}
			else if constexpr (epi8<To> || epu8<To>) {
				if constexpr (has_avx512vl<ISA> && has_avx512f<ISA>) return _mm256_cvtepi32_epi8(_mm256_cvttps_epi32(x));
				else return scalar_fallback_convert_<ISA, From, To>(x);
			}
			else return scalar_fallback_convert_<ISA, From, To>(x);
		}
		else if constexpr (std::same_as<From, double>) {
			if constexpr (std::same_as<To, float>) return _mm256_cvtpd_ps(x);
			else if constexpr (epi32<To>) return _mm256_cvttpd_epi32(x);
			else if constexpr (epu32<To>) {
				if constexpr (has_avx512vl<ISA> && has_avx512f<ISA>) return _mm256_cvttpd_epu32(x);
				else return scalar_fallback_convert_<ISA, From, To>(x);
			}
			else if constexpr (epi64<To>) {
				if constexpr (has_avx512vl<ISA> && has_avx512dq<ISA>) return _mm256_cvttpd_epi64(x);
				else return scalar_fallback_convert_<ISA, From, To>(x);
			}
			else if constexpr (epu64<To>) {
				if constexpr (has_avx512vl<ISA> && has_avx512dq<ISA>) return _mm256_cvttpd_epu64(x);
				else return scalar_fallback_convert_<ISA, From, To>(x);
			}
			else return scalar_fallback_convert_<ISA, From, To>(x);
		}
		else if constexpr (epi8<From> || epu8<From>) {
			constexpr bool from_signed = epi8<From>;

			if constexpr (epi16<To> || epu16<To>) {
				if constexpr (has_avx512bw<ISA>) {
					if constexpr (from_signed) return _mm512_cvtepi8_epi16(x);
					else return _mm512_cvtepu8_epi16(x);
				} else if constexpr (has_avx2<ISA>) {
					auto x_lo = _mm256_castsi256_si128(x);
					auto x_hi = _mm256_extracti128_si256(x, 1);
					if constexpr (from_signed) {
						auto y_lo = _mm256_cvtepi8_epi16(x_lo);
						auto y_hi = _mm256_cvtepi8_epi16(x_hi);
						return pack_or_tuple<ISA, To>(y_lo, y_hi);
					} else {
						auto y_lo = _mm256_cvtepu8_epi16(x_lo);
						auto y_hi = _mm256_cvtepu8_epi16(x_hi);
						return pack_or_tuple<ISA, To>(y_lo, y_hi);
					}
				} else return scalar_fallback_convert_<ISA, From, To>(x);
			}
			else if constexpr (epi32<To> || epu32<To>) {
				if constexpr (has_zmm) {
					auto x_lo = _mm256_castsi256_si128(x);
					auto x_hi = _mm256_extracti128_si256(x, 1);
					auto z0 = from_signed ? _mm512_cvtepi8_epi32(x_lo) : _mm512_cvtepu8_epi32(x_lo);
					auto z1 = from_signed ? _mm512_cvtepi8_epi32(x_hi) : _mm512_cvtepu8_epi32(x_hi);
					return std::tuple{ z0, z1 };
				} else if constexpr (has_avx2<ISA>) {
					auto x_lo = _mm256_castsi256_si128(x);
					auto x_hi = _mm256_extracti128_si256(x, 1);
					auto y0 = from_signed ? _mm256_cvtepi8_epi32(x_lo) : _mm256_cvtepu8_epi32(x_lo);
					auto y1 = from_signed ? _mm256_cvtepi8_epi32(_mm_srli_si128(x_lo, 8)) : _mm256_cvtepu8_epi32(_mm_srli_si128(x_lo, 8));
					auto y2 = from_signed ? _mm256_cvtepi8_epi32(x_hi) : _mm256_cvtepu8_epi32(x_hi);
					auto y3 = from_signed ? _mm256_cvtepi8_epi32(_mm_srli_si128(x_hi, 8)) : _mm256_cvtepu8_epi32(_mm_srli_si128(x_hi, 8));
					return std::tuple{ y0, y1, y2, y3 };
				} else return scalar_fallback_convert_<ISA, From, To>(x);
			}
			else if constexpr (std::same_as<To, float>) {
				if constexpr (has_zmm) {
					auto x_lo = _mm256_castsi256_si128(x);
					auto x_hi = _mm256_extracti128_si256(x, 1);
					auto z0 = _mm512_cvtepi32_ps(from_signed ? _mm512_cvtepi8_epi32(x_lo) : _mm512_cvtepu8_epi32(x_lo));
					auto z1 = _mm512_cvtepi32_ps(from_signed ? _mm512_cvtepi8_epi32(x_hi) : _mm512_cvtepu8_epi32(x_hi));
					return std::tuple{ z0, z1 };
				} else if constexpr (has_avx2<ISA>) {
					auto x_lo = _mm256_castsi256_si128(x);
					auto x_hi = _mm256_extracti128_si256(x, 1);
					auto y0 = _mm256_cvtepi32_ps(from_signed ? _mm256_cvtepi8_epi32(x_lo) : _mm256_cvtepu8_epi32(x_lo));
					auto y1 = _mm256_cvtepi32_ps(from_signed ? _mm256_cvtepi8_epi32(_mm_srli_si128(x_lo, 8)) : _mm256_cvtepu8_epi32(_mm_srli_si128(x_lo, 8)));
					auto y2 = _mm256_cvtepi32_ps(from_signed ? _mm256_cvtepi8_epi32(x_hi) : _mm256_cvtepu8_epi32(x_hi));
					auto y3 = _mm256_cvtepi32_ps(from_signed ? _mm256_cvtepi8_epi32(_mm_srli_si128(x_hi, 8)) : _mm256_cvtepu8_epi32(_mm_srli_si128(x_hi, 8)));
					return std::tuple{ y0, y1, y2, y3 };
				} else return scalar_fallback_convert_<ISA, From, To>(x);
			}
			else if constexpr (epi64<To> || epu64<To>) {
				if constexpr (has_zmm) {
					auto x_lo = _mm256_castsi256_si128(x);
					auto x_hi = _mm256_extracti128_si256(x, 1);
					auto z0 = from_signed ? _mm512_cvtepi8_epi64(x_lo) : _mm512_cvtepu8_epi64(x_lo);
					auto z1 = from_signed ? _mm512_cvtepi8_epi64(_mm_srli_si128(x_lo, 8)) : _mm512_cvtepu8_epi64(_mm_srli_si128(x_lo, 8));
					auto z2 = from_signed ? _mm512_cvtepi8_epi64(x_hi) : _mm512_cvtepu8_epi64(x_hi);
					auto z3 = from_signed ? _mm512_cvtepi8_epi64(_mm_srli_si128(x_hi, 8)) : _mm512_cvtepu8_epi64(_mm_srli_si128(x_hi, 8));
					return std::tuple{ z0, z1, z2, z3 };
				} else if constexpr (has_avx2<ISA>) {
					auto x_lo = _mm256_castsi256_si128(x);
					auto x_hi = _mm256_extracti128_si256(x, 1);
					auto y0 = from_signed ? _mm256_cvtepi8_epi64(x_lo) : _mm256_cvtepu8_epi64(x_lo);
					auto y1 = from_signed ? _mm256_cvtepi8_epi64(_mm_srli_si128(x_lo, 4)) : _mm256_cvtepu8_epi64(_mm_srli_si128(x_lo, 4));
					auto y2 = from_signed ? _mm256_cvtepi8_epi64(_mm_srli_si128(x_lo, 8)) : _mm256_cvtepu8_epi64(_mm_srli_si128(x_lo, 8));
					auto y3 = from_signed ? _mm256_cvtepi8_epi64(_mm_srli_si128(x_lo, 12)) : _mm256_cvtepu8_epi64(_mm_srli_si128(x_lo, 12));
					auto y4 = from_signed ? _mm256_cvtepi8_epi64(x_hi) : _mm256_cvtepu8_epi64(x_hi);
					auto y5 = from_signed ? _mm256_cvtepi8_epi64(_mm_srli_si128(x_hi, 4)) : _mm256_cvtepu8_epi64(_mm_srli_si128(x_hi, 4));
					auto y6 = from_signed ? _mm256_cvtepi8_epi64(_mm_srli_si128(x_hi, 8)) : _mm256_cvtepu8_epi64(_mm_srli_si128(x_hi, 8));
					auto y7 = from_signed ? _mm256_cvtepi8_epi64(_mm_srli_si128(x_hi, 12)) : _mm256_cvtepu8_epi64(_mm_srli_si128(x_hi, 12));
					return std::tuple{ y0, y1, y2, y3, y4, y5, y6, y7 };
				} else return scalar_fallback_convert_<ISA, From, To>(x);
			}
			else if constexpr (std::same_as<To, double>) {
				if constexpr (has_zmm) {
					auto x_lo = _mm256_castsi256_si128(x);
					auto x_hi = _mm256_extracti128_si256(x, 1);
					auto z0 = _mm512_cvtepi32_pd(from_signed ? _mm256_cvtepi8_epi32(x_lo) : _mm256_cvtepu8_epi32(x_lo));
					auto z1 = _mm512_cvtepi32_pd(from_signed ? _mm256_cvtepi8_epi32(_mm_srli_si128(x_lo, 8)) : _mm256_cvtepu8_epi32(_mm_srli_si128(x_lo, 8)));
					auto z2 = _mm512_cvtepi32_pd(from_signed ? _mm256_cvtepi8_epi32(x_hi) : _mm256_cvtepu8_epi32(x_hi));
					auto z3 = _mm512_cvtepi32_pd(from_signed ? _mm256_cvtepi8_epi32(_mm_srli_si128(x_hi, 8)) : _mm256_cvtepu8_epi32(_mm_srli_si128(x_hi, 8)));
					return std::tuple{ z0, z1, z2, z3 };
				} else if constexpr (has_avx2<ISA>) {
					auto x_lo = _mm256_castsi256_si128(x);
					auto x_hi = _mm256_extracti128_si256(x, 1);
					auto y0 = _mm256_cvtepi32_pd(from_signed ? _mm_cvtepi8_epi32(x_lo) : _mm_cvtepu8_epi32(x_lo));
					auto y1 = _mm256_cvtepi32_pd(from_signed ? _mm_cvtepi8_epi32(_mm_srli_si128(x_lo, 4)) : _mm_cvtepu8_epi32(_mm_srli_si128(x_lo, 4)));
					auto y2 = _mm256_cvtepi32_pd(from_signed ? _mm_cvtepi8_epi32(_mm_srli_si128(x_lo, 8)) : _mm_cvtepu8_epi32(_mm_srli_si128(x_lo, 8)));
					auto y3 = _mm256_cvtepi32_pd(from_signed ? _mm_cvtepi8_epi32(_mm_srli_si128(x_lo, 12)) : _mm_cvtepu8_epi32(_mm_srli_si128(x_lo, 12)));
					auto y4 = _mm256_cvtepi32_pd(from_signed ? _mm_cvtepi8_epi32(x_hi) : _mm_cvtepu8_epi32(x_hi));
					auto y5 = _mm256_cvtepi32_pd(from_signed ? _mm_cvtepi8_epi32(_mm_srli_si128(x_hi, 4)) : _mm_cvtepu8_epi32(_mm_srli_si128(x_hi, 4)));
					auto y6 = _mm256_cvtepi32_pd(from_signed ? _mm_cvtepi8_epi32(_mm_srli_si128(x_hi, 8)) : _mm_cvtepu8_epi32(_mm_srli_si128(x_hi, 8)));
					auto y7 = _mm256_cvtepi32_pd(from_signed ? _mm_cvtepi8_epi32(_mm_srli_si128(x_hi, 12)) : _mm_cvtepu8_epi32(_mm_srli_si128(x_hi, 12)));
					return std::tuple{ y0, y1, y2, y3, y4, y5, y6, y7 };
				} else return scalar_fallback_convert_<ISA, From, To>(x);
			} else return scalar_fallback_convert_<ISA, From, To>(x);
		}
		else if constexpr (epi16<From> || epu16<From>) {
			constexpr bool from_signed = epi16<From>;

			if constexpr (epi8<To> || epu8<To>) {
				if constexpr (has_avx512vl<ISA> && has_avx512bw<ISA>) return _mm256_cvtepi16_epi8(x);
				else if constexpr (has_avx2<ISA>) {
					auto lo = _mm256_castsi256_si128(x);
					auto hi = _mm256_extracti128_si256(x, 1);
					if constexpr (epi8<To>) return _mm_packs_epi16(lo, hi);
					else return _mm_packus_epi16(lo, hi);
				}
				else return scalar_fallback_convert_<ISA, From, To>(x);
			}
			else if constexpr (epi32<To> || epu32<To>) {
				if constexpr (has_zmm) {
					if constexpr (from_signed) return _mm512_cvtepi16_epi32(x);
					else return _mm512_cvtepu16_epi32(x);
				} else if constexpr (has_avx2<ISA>) {
					auto x_lo = _mm256_castsi256_si128(x);
					auto x_hi = _mm256_extracti128_si256(x, 1);
					if constexpr (from_signed) {
						auto y_lo = _mm256_cvtepi16_epi32(x_lo);
						auto y_hi = _mm256_cvtepi16_epi32(x_hi);
						return pack_or_tuple<ISA, To>(y_lo, y_hi);
					} else {
						auto y_lo = _mm256_cvtepu16_epi32(x_lo);
						auto y_hi = _mm256_cvtepu16_epi32(x_hi);
						return pack_or_tuple<ISA, To>(y_lo, y_hi);
					}
				} else return scalar_fallback_convert_<ISA, From, To>(x);
			} 
			else if constexpr (std::same_as<To, float>) {
				if constexpr (has_zmm) {
					if constexpr (from_signed) return _mm512_cvtepi32_ps(_mm512_cvtepi16_epi32(x));
					else return _mm512_cvtepi32_ps(_mm512_cvtepu16_epi32(x));
				} else if constexpr (has_avx2<ISA>) {
					auto x_lo = _mm256_castsi256_si128(x);
					auto x_hi = _mm256_extracti128_si256(x, 1);
					if constexpr (from_signed) {
						auto y_lo = _mm256_cvtepi32_ps(_mm256_cvtepi16_epi32(x_lo));
						auto y_hi = _mm256_cvtepi32_ps(_mm256_cvtepi16_epi32(x_hi));
						return pack_or_tuple<ISA, To>(y_lo, y_hi);
					} else {
						auto y_lo = _mm256_cvtepi32_ps(_mm256_cvtepu16_epi32(x_lo));
						auto y_hi = _mm256_cvtepi32_ps(_mm256_cvtepu16_epi32(x_hi));
						return pack_or_tuple<ISA, To>(y_lo, y_hi);
					}
				} else return scalar_fallback_convert_<ISA, From, To>(x);
			}
			else if constexpr (epi64<To> || epu64<To>) {
				if constexpr (has_zmm) {
					auto x_lo = _mm256_castsi256_si128(x);
					auto x_hi = _mm256_extracti128_si256(x, 1);
					if constexpr (from_signed) {
						auto z0 = _mm512_cvtepi16_epi64(x_lo);
						auto z1 = _mm512_cvtepi16_epi64(x_hi);
						return std::tuple{ z0, z1 };
					} else {
						auto z0 = _mm512_cvtepu16_epi64(x_lo);
						auto z1 = _mm512_cvtepu16_epi64(x_hi);
						return std::tuple{ z0, z1 };
					}
				} else if constexpr (has_avx2<ISA>) {
					auto x_lo = _mm256_castsi256_si128(x);
					auto x_hi = _mm256_extracti128_si256(x, 1);
					if constexpr (from_signed) {
						auto y0 = _mm256_cvtepi16_epi64(x_lo);
						auto y1 = _mm256_cvtepi16_epi64(_mm_shuffle_epi32(x_lo, 0xEE));
						auto y2 = _mm256_cvtepi16_epi64(x_hi);
						auto y3 = _mm256_cvtepi16_epi64(_mm_shuffle_epi32(x_hi, 0xEE));
						return std::tuple{ y0, y1, y2, y3 };
					} else {
						auto y0 = _mm256_cvtepu16_epi64(x_lo);
						auto y1 = _mm256_cvtepu16_epi64(_mm_shuffle_epi32(x_lo, 0xEE));
						auto y2 = _mm256_cvtepu16_epi64(x_hi);
						auto y3 = _mm256_cvtepu16_epi64(_mm_shuffle_epi32(x_hi, 0xEE));
						return std::tuple{ y0, y1, y2, y3 };
					}
				} else return scalar_fallback_convert_<ISA, From, To>(x);
			}
			else if constexpr (std::same_as<To, double>) {
				if constexpr (has_zmm) {
					auto x_lo = _mm256_castsi256_si128(x);
					auto x_hi = _mm256_extracti128_si256(x, 1);
					if constexpr (from_signed) {
						auto z0 = _mm512_cvtepi32_pd(_mm256_cvtepi16_epi32(x_lo));
						auto z1 = _mm512_cvtepi32_pd(_mm256_cvtepi16_epi32(x_hi));
						return std::tuple{ z0, z1 };
					} else {
						auto z0 = _mm512_cvtepu32_pd(_mm256_cvtepu16_epi32(x_lo));
						auto z1 = _mm512_cvtepu32_pd(_mm256_cvtepu16_epi32(x_hi));
						return std::tuple{ z0, z1 };
					}
				} else if constexpr (has_avx2<ISA>) {
					auto x_lo = _mm256_castsi256_si128(x);
					auto x_hi = _mm256_extracti128_si256(x, 1);
					if constexpr (from_signed) {
						auto y0 = _mm256_cvtepi32_pd(_mm_cvtepi16_epi32(x_lo));
						auto y1 = _mm256_cvtepi32_pd(_mm_cvtepi16_epi32(_mm_shuffle_epi32(x_lo, 0xEE)));
						auto y2 = _mm256_cvtepi32_pd(_mm_cvtepi16_epi32(x_hi));
						auto y3 = _mm256_cvtepi32_pd(_mm_cvtepi16_epi32(_mm_shuffle_epi32(x_hi, 0xEE)));
						return std::tuple{ y0, y1, y2, y3 };
					} else {
						auto y0 = _mm256_cvtepi32_pd(_mm_cvtepu16_epi32(x_lo));
						auto y1 = _mm256_cvtepi32_pd(_mm_cvtepu16_epi32(_mm_shuffle_epi32(x_lo, 0xEE)));
						auto y2 = _mm256_cvtepi32_pd(_mm_cvtepu16_epi32(x_hi));
						auto y3 = _mm256_cvtepi32_pd(_mm_cvtepu16_epi32(_mm_shuffle_epi32(x_hi, 0xEE)));
						return std::tuple{ y0, y1, y2, y3 };
					}
				} else return scalar_fallback_convert_<ISA, From, To>(x);
			} else return scalar_fallback_convert_<ISA, From, To>(x);
		}
		else if constexpr (epi32<From> || epu32<From>) {
			constexpr bool from_signed = epi32<From>;

			if constexpr (epi8<To> || epu8<To>) {
				if constexpr (has_avx512vl<ISA> && has_avx512f<ISA>) return _mm256_cvtepi32_epi8(x);
				else return scalar_fallback_convert_<ISA, From, To>(x);
			}
			else if constexpr (epi16<To> || epu16<To>) {
				if constexpr (has_avx512vl<ISA> && has_avx512f<ISA>) return _mm256_cvtepi32_epi16(x);
				else if constexpr (has_avx2<ISA>) {
					auto lo = _mm256_castsi256_si128(x);
					auto hi = _mm256_extracti128_si256(x, 1);
					if constexpr (epi16<To>) return _mm_packs_epi32(lo, hi);
					else if constexpr (has_sse41<ISA>) return _mm_packus_epi32(lo, hi);
					else return scalar_fallback_convert_<ISA, From, To>(x);
				}
				else return scalar_fallback_convert_<ISA, From, To>(x);
			}
			else if constexpr (std::same_as<To, float>) {
				if constexpr (from_signed) return _mm256_cvtepi32_ps(x);
				else if constexpr (has_avx512vl<ISA> && has_avx512f<ISA>) return _mm256_cvtepu32_ps(x);
				else return scalar_fallback_convert_<ISA, From, To>(x);
			}
			else if constexpr (epi64<To> || epu64<To>) {
				if constexpr (has_zmm) {
					if constexpr (from_signed) return _mm512_cvtepi32_epi64(x);
					else return _mm512_cvtepu32_epi64(x);
				} else if constexpr (has_avx2<ISA>) {
					auto x_lo = _mm256_castsi256_si128(x);
					auto x_hi = _mm256_extracti128_si256(x, 1);
					if constexpr (from_signed) {
						auto y_lo = _mm256_cvtepi32_epi64(x_lo);
						auto y_hi = _mm256_cvtepi32_epi64(x_hi);
						return pack_or_tuple<ISA, To>(y_lo, y_hi);
					} else {
						auto y_lo = _mm256_cvtepu32_epi64(x_lo);
						auto y_hi = _mm256_cvtepu32_epi64(x_hi);
						return pack_or_tuple<ISA, To>(y_lo, y_hi);
					}
				} else return scalar_fallback_convert_<ISA, From, To>(x);
			}
			else if constexpr (std::same_as<To, double>) {
				if constexpr (from_signed) {
					if constexpr (has_zmm) return _mm512_cvtepi32_pd(x);
					else if constexpr (has_ymm) {
						auto x_lo = _mm256_castsi256_si128(x);
						auto x_hi = _mm256_extractf128_si256(x, 1);
						auto y_lo = _mm256_cvtepi32_pd(x_lo);
						auto y_hi = _mm256_cvtepi32_pd(x_hi);
						return pack_or_tuple<ISA, To>(y_lo, y_hi);
					} else return scalar_fallback_convert_<ISA, From, To>(x);
				} else {
					if constexpr (has_zmm) return _mm512_cvtepu32_pd(x);
					else return scalar_fallback_convert_<ISA, From, To>(x);
				}
			} else return scalar_fallback_convert_<ISA, From, To>(x);
		}
		else if constexpr (epi64<From> || epu64<From>) {
			constexpr bool from_signed = epi64<From>;

			if constexpr (epi8<To> || epu8<To>) {
				if constexpr (has_avx512vl<ISA> && has_avx512f<ISA>) return _mm256_cvtepi64_epi8(x);
				else return scalar_fallback_convert_<ISA, From, To>(x);
			}
			else if constexpr (epi16<To> || epu16<To>) {
				if constexpr (has_avx512vl<ISA> && has_avx512f<ISA>) return _mm256_cvtepi64_epi16(x);
				else return scalar_fallback_convert_<ISA, From, To>(x);
			}
			else if constexpr (epi32<To> || epu32<To>) {
				if constexpr (has_avx512vl<ISA> && has_avx512f<ISA>) return _mm256_cvtepi64_epi32(x);
				else return scalar_fallback_convert_<ISA, From, To>(x);
			}
			else if constexpr (std::same_as<To, float>) {
				if constexpr (has_avx512vl<ISA> && has_avx512dq<ISA>) {
					if constexpr (from_signed) return _mm256_cvtepi64_ps(x);
					else return _mm256_cvtepu64_ps(x);
				} else return scalar_fallback_convert_<ISA, From, To>(x);
			}
			else if constexpr (std::same_as<To, double>) {
				if constexpr (has_avx512vl<ISA> && has_avx512dq<ISA>) {
					if constexpr (from_signed) return _mm256_cvtepi64_pd(x);
					else return _mm256_cvtepu64_pd(x);
				} else return scalar_fallback_convert_<ISA, From, To>(x);
			}
			else return scalar_fallback_convert_<ISA, From, To>(x);
		}
		else return scalar_fallback_convert_<ISA, From, To>(x);
	}
	else if constexpr (sizeof(V) == 64) {
		if constexpr (std::same_as<From, float>) {
			if constexpr (std::same_as<To, double>) {
				if constexpr (has_zmm) {
					auto lo = _mm512_cvtps_pd(_mm512_castps512_ps256(x));
					auto hi = _mm512_cvtps_pd(_mm512_extractf32x8_ps(x, 1));
					return std::tuple{ lo, hi };
				} else return scalar_fallback_convert_<ISA, From, To>(x);
			}
			else if constexpr (epi32<To>) return _mm512_cvttps_epi32(x);
			else if constexpr (epu32<To>) return _mm512_cvttps_epu32(x);
			else if constexpr (epi16<To> || epu16<To>) return _mm512_cvtepi32_epi16(_mm512_cvttps_epi32(x));
			else if constexpr (epi8<To> || epu8<To>) return _mm512_cvtepi32_epi8(_mm512_cvttps_epi32(x));
			else return scalar_fallback_convert_<ISA, From, To>(x);
		}
		else if constexpr (std::same_as<From, double>) {
			if constexpr (std::same_as<To, float>) return _mm512_cvtpd_ps(x);
			else if constexpr (epi32<To>) return _mm512_cvttpd_epi32(x);
			else if constexpr (epu32<To>) return _mm512_cvttpd_epu32(x);
			else if constexpr (epi64<To>) {
				if constexpr (has_avx512dq<ISA>) return _mm512_cvttpd_epi64(x);
				else return scalar_fallback_convert_<ISA, From, To>(x);
			}
			else if constexpr (epu64<To>) {
				if constexpr (has_avx512dq<ISA>) return _mm512_cvttpd_epu64(x);
				else return scalar_fallback_convert_<ISA, From, To>(x);
			}
			else return scalar_fallback_convert_<ISA, From, To>(x);
		}
		else if constexpr (epi8<From> || epu8<From>) {
			constexpr bool from_signed = epi8<From>;

			if constexpr (epi16<To> || epu16<To>) {
				if constexpr (has_avx512bw<ISA>) {
					auto x_lo = _mm512_castsi512_si256(x);
					auto x_hi = _mm512_extracti64x4_epi64(x, 1);
					auto z0 = from_signed ? _mm512_cvtepi8_epi16(x_lo) : _mm512_cvtepu8_epi16(x_lo);
					auto z1 = from_signed ? _mm512_cvtepi8_epi16(x_hi) : _mm512_cvtepu8_epi16(x_hi);
					return std::tuple{ z0, z1 };
				} else return scalar_fallback_convert_<ISA, From, To>(x);
			}
			else if constexpr (epi32<To> || epu32<To>) {
				if constexpr (has_zmm) {
					auto x0 = _mm512_castsi512_si128(x);
					auto x1 = _mm512_extracti32x4_epi32(x, 1);
					auto x2 = _mm512_extracti32x4_epi32(x, 2);
					auto x3 = _mm512_extracti32x4_epi32(x, 3);
					auto z0 = from_signed ? _mm512_cvtepi8_epi32(x0) : _mm512_cvtepu8_epi32(x0);
					auto z1 = from_signed ? _mm512_cvtepi8_epi32(x1) : _mm512_cvtepu8_epi32(x1);
					auto z2 = from_signed ? _mm512_cvtepi8_epi32(x2) : _mm512_cvtepu8_epi32(x2);
					auto z3 = from_signed ? _mm512_cvtepi8_epi32(x3) : _mm512_cvtepu8_epi32(x3);
					return std::tuple{ z0, z1, z2, z3 };
				} else return scalar_fallback_convert_<ISA, From, To>(x);
			}
			else if constexpr (std::same_as<To, float>) {
				if constexpr (has_zmm) {
					auto x0 = _mm512_castsi512_si128(x);
					auto x1 = _mm512_extracti32x4_epi32(x, 1);
					auto x2 = _mm512_extracti32x4_epi32(x, 2);
					auto x3 = _mm512_extracti32x4_epi32(x, 3);
					auto z0 = _mm512_cvtepi32_ps(from_signed ? _mm512_cvtepi8_epi32(x0) : _mm512_cvtepu8_epi32(x0));
					auto z1 = _mm512_cvtepi32_ps(from_signed ? _mm512_cvtepi8_epi32(x1) : _mm512_cvtepu8_epi32(x1));
					auto z2 = _mm512_cvtepi32_ps(from_signed ? _mm512_cvtepi8_epi32(x2) : _mm512_cvtepu8_epi32(x2));
					auto z3 = _mm512_cvtepi32_ps(from_signed ? _mm512_cvtepi8_epi32(x3) : _mm512_cvtepu8_epi32(x3));
					return std::tuple{ z0, z1, z2, z3 };
				} else return scalar_fallback_convert_<ISA, From, To>(x);
			} else return scalar_fallback_convert_<ISA, From, To>(x);
		}
		else if constexpr (epi16<From> || epu16<From>) {
			constexpr bool from_signed = epi16<From>;

			if constexpr (epi8<To> || epu8<To>) {
				if constexpr (has_avx512bw<ISA>) return _mm512_cvtepi16_epi8(x);
				else return scalar_fallback_convert_<ISA, From, To>(x);
			}
			else if constexpr (epi32<To> || epu32<To>) {
				auto x_lo = _mm512_castsi512_si256(x);
				auto x_hi = _mm512_extracti64x4_epi64(x, 1);
				if constexpr (from_signed) {
					auto z0 = _mm512_cvtepi16_epi32(x_lo);
					auto z1 = _mm512_cvtepi16_epi32(x_hi);
					return std::tuple{ z0, z1 };
				} else {
					auto z0 = _mm512_cvtepu16_epi32(x_lo);
					auto z1 = _mm512_cvtepu16_epi32(x_hi);
					return std::tuple{ z0, z1 };
				}
			}
			else if constexpr (std::same_as<To, float>) {
				auto x_lo = _mm512_castsi512_si256(x);
				auto x_hi = _mm512_extracti64x4_epi64(x, 1);
				if constexpr (from_signed) {
					auto z0 = _mm512_cvtepi32_ps(_mm512_cvtepi16_epi32(x_lo));
					auto z1 = _mm512_cvtepi32_ps(_mm512_cvtepi16_epi32(x_hi));
					return std::tuple{ z0, z1 };
				} else {
					auto z0 = _mm512_cvtepi32_ps(_mm512_cvtepu16_epi32(x_lo));
					auto z1 = _mm512_cvtepi32_ps(_mm512_cvtepu16_epi32(x_hi));
					return std::tuple{ z0, z1 };
				}
			}
			else return scalar_fallback_convert_<ISA, From, To>(x);
		}
		else if constexpr (epi32<From> || epu32<From>) {
			constexpr bool from_signed = epi32<From>;

			if constexpr (epi8<To> || epu8<To>) return _mm512_cvtepi32_epi8(x);
			else if constexpr (epi16<To> || epu16<To>) return _mm512_cvtepi32_epi16(x);
			else if constexpr (std::same_as<To, float>) {
				if constexpr (from_signed) return _mm512_cvtepi32_ps(x);
				else return _mm512_cvtepu32_ps(x);
			}
			else if constexpr (epi64<To> || epu64<To>) {
				auto x_lo = _mm512_castsi512_si256(x);
				auto x_hi = _mm512_extracti64x4_epi64(x, 1);
				if constexpr (from_signed) {
					auto z0 = _mm512_cvtepi32_epi64(x_lo);
					auto z1 = _mm512_cvtepi32_epi64(x_hi);
					return std::tuple{ z0, z1 };
				} else {
					auto z0 = _mm512_cvtepu32_epi64(x_lo);
					auto z1 = _mm512_cvtepu32_epi64(x_hi);
					return std::tuple{ z0, z1 };
				}
			}
			else if constexpr (std::same_as<To, double>) {
				auto x_lo = _mm512_castsi512_si256(x);
				auto x_hi = _mm512_extracti64x4_epi64(x, 1);
				if constexpr (from_signed) {
					auto z0 = _mm512_cvtepi32_pd(x_lo);
					auto z1 = _mm512_cvtepi32_pd(x_hi);
					return std::tuple{ z0, z1 };
				} else {
					auto z0 = _mm512_cvtepu32_pd(x_lo);
					auto z1 = _mm512_cvtepu32_pd(x_hi);
					return std::tuple{ z0, z1 };
				}
			}
			else return scalar_fallback_convert_<ISA, From, To>(x);
		}
		else if constexpr (epi64<From> || epu64<From>) {
			constexpr bool from_signed = epi64<From>;

			if constexpr (epi8<To> || epu8<To>) return _mm512_cvtepi64_epi8(x);
			else if constexpr (epi16<To> || epu16<To>) return _mm512_cvtepi64_epi16(x);
			else if constexpr (epi32<To> || epu32<To>) return _mm512_cvtepi64_epi32(x);
			else if constexpr (std::same_as<To, float>) {
				if constexpr (has_avx512dq<ISA>) {
					if constexpr (from_signed) return _mm512_cvtepi64_ps(x);
					else return _mm512_cvtepu64_ps(x);
				} else return scalar_fallback_convert_<ISA, From, To>(x);
			}
			else if constexpr (std::same_as<To, double>) {
				if constexpr (has_avx512dq<ISA>) {
					if constexpr (from_signed) return _mm512_cvtepi64_pd(x);
					else return _mm512_cvtepu64_pd(x);
				} else return scalar_fallback_convert_<ISA, From, To>(x);
			}
			else return scalar_fallback_convert_<ISA, From, To>(x);
		}
		else return scalar_fallback_convert_<ISA, From, To>(x);
	}

	return scalar_fallback_convert_<ISA, From, To>(x);
}

template <arch::ISA ISA, arithmetic_type From, arithmetic_type To, intrin_or_arithmetic_type V, raw_mask_type M>
raze_always_inline auto convert_(V x, M mask) noexcept {
	return convert_<ISA, From, To>(x);
}

template <arch::ISA ISA, arithmetic_type From, arithmetic_type To, intrin_or_arithmetic_type V, raw_mask_type M>
raze_always_inline auto convert_(V x, M mask, V src) noexcept {
	return convert_<ISA, From, To>(x);
}

__RAZE_VX_NAMESPACE_END