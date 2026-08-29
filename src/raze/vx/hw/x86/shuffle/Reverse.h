#pragma once 

#include <src/raze/vx/hw/x86/cast/As.h>
#include <src/raze/vx/hw/common/PatternsCheck.h>
#include <src/raze/vx/hw/x86/shuffle/GenericShuffle.h>

__RAZE_VX_NAMESPACE_BEGIN

template <arch::ISA ISA, arithmetic_type T, intrin_or_arithmetic_type V, class Pattern>
raze_always_inline V reverse_native_(V x, Pattern p) noexcept {
	if constexpr (!has_ssse3<ISA> && sizeof(V) == 16) {
		if constexpr (sizeof(T) == 2) {
			x = as<V>(_mm_shuffle_pd(as<__m128d>(x), as<__m128d>(x), 1));
			x = as<V>(_mm_shufflehi_epi16(as<__m128i>(x), 0x1B));
			return as<V>(_mm_shufflelo_epi16(as<__m128i>(x), 0x1B));
		}
		else if constexpr (sizeof(T) == 1) {
			x = as<V>(_mm_or_si128(_mm_srli_epi16(as<__m128i>(x), 8), _mm_slli_epi16(as<__m128i>(x), 8)));
			x = as<V>(_mm_shufflelo_epi16(as<__m128i>(x), 0x1B));
			x = as<V>(_mm_shufflehi_epi16(as<__m128i>(x), 0x1B));
			return as<V>(_mm_shuffle_epi32(as<__m128i>(x), 0x4E));
		}
	}
	else if constexpr (sizeof(V) == 32) {
		if constexpr (sizeof(T) == 2 && !(has_avx512bw<ISA> && has_avx512vl<ISA>)) {
			const auto reversed_lanes = _mm256_shuffle_epi8(as<__m256i>(x), 
				(p % std::integral_constant<sizetype, 16>{}).template expand<u16, u8>().template as_native<__m256i>());
			return as<V>(_mm256_permute2x128_si256(reversed_lanes, reversed_lanes, 0x01));
		}
		else if constexpr (sizeof(T) == 1 && !(has_avx512vbmi<ISA> && has_avx512vl<ISA>)) {
			const auto reversed_lanes = _mm256_shuffle_epi8(as<__m256i>(x),
				(p % std::integral_constant<sizetype, 16>{}).template as_native<__m256i>());
			return as<V>(_mm256_permute2x128_si256(reversed_lanes, reversed_lanes, 0x01));
		}
	}
	else if constexpr (sizeof(V) == 64) {
		if constexpr ((sizeof(T) == 2 && !has_avx512bw<ISA>) || (sizeof(T) == 1 && !has_avx512vbmi<ISA>)) {
			constexpr auto p_offset = p.offset(std::integral_constant<sizetype, p.size() / 2>{});
			__m256i native;
			
			if constexpr (sizeof(T) == 2) native = p_offset.template expand<u16, u8>().template as_native<__m256i>();
			else native = p_offset.template as_native<__m256i>();

			const auto low_half = _mm512_extracti64x4_epi64(as<__m512i>(x), 0);
			const auto high_half = _mm512_extracti64x4_epi64(as<__m512i>(x), 1);

			const auto low = _mm256_shuffle_epi8(low_half, native);
			const auto high = _mm256_shuffle_epi8(high_half, native);

			return as<V>(_mm512_shuffle_i64x2(as<__m512i>(high), as<__m512i>(low), 0x11));
		}
	}
		
	if constexpr (arithmetic_type<V>) return x;
	else return generic_shuffle_native_<ISA, T>(x, p);
}

template <class Pattern>
raze_always_inline pattern_vector_t<Pattern> reverse_native_size_(const pattern_vector_t<Pattern>& x, Pattern p) noexcept {
	using V = pattern_vector_t<Pattern>;

	V result = x;
	result.__for_each_chunk([&] <class Chunk> (Chunk& chunk) raze_always_inline_lambda {
		chunk = reverse_native_<abi_t<V>::isa, typename V::value_type>(ustorage(chunk), p);
	});

	return result;
}

template <class Pattern>
raze_always_inline pattern_vector_t<Pattern> reverse_(pattern_vector_t<Pattern> x, Pattern p) noexcept {
	using V = pattern_vector_t<Pattern>;
	using T = typename V::value_type;

	if constexpr (native<V>) {
		return reverse_native_size_(x, p);
	}
	else if constexpr (trivially_chunk_swappable<V>) {
		[&] <sizetype... Indices> (std::integer_sequence<sizetype, Indices...>) raze_always_inline_lambda {
			([&](auto i) raze_always_inline_lambda {
				auto& c1 = x.template __get<i>();
				auto& c2 = x.template __get<V::__chunks_count() - i - 1>();

				auto c1_in = ustorage(c1);
				auto c2_in = ustorage(c2);

				using Chunk1 = std::remove_cvref_t<decltype(c1)>;
				using Chunk2 = std::remove_cvref_t<decltype(c2)>;

				const auto r1 = reverse_native_<abi_t<V>::isa, T>(
					c2_in, make_reversed_pattern<typename Chunk2::as_simd>{});

				const auto r2 = reverse_native_<abi_t<V>::isa, T>(
					c1_in, make_reversed_pattern<typename Chunk1::as_simd>{});

				c1 = r1;
				c2 = r2;
			}(std::integral_constant<sizetype, Indices>{}), ...);
		}(std::make_integer_sequence<sizetype, V::__chunks_count() / 2>{});

		return x;
	}
	else {
		return generic_shuffle_(x, p);
	}
}

__RAZE_VX_NAMESPACE_END
