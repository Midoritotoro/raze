#pragma once 

#include <src/raze/vx/hw/common/PatternsCheck.h>
#include <src/raze/vx/hw/x86/access/Extract.h>
#include <src/raze/vx/hw/x86/construct/Broadcast.h>
#include <src/raze/vx/hw/x86/shuffle/GenericShuffle.h>
#include <src/raze/vx/hw/x86/access/Extract.h>


__RAZE_VX_NAMESPACE_BEGIN

template <arch::ISA ISA, arithmetic_type T, intrin_type V>
raze_always_inline auto zmm_broadcast_low_(V x) noexcept {
	if constexpr (sizeof(T) == 8) return _mm512_broadcastq_epi64(as<__m128i>(x));
	else if constexpr (sizeof(T) == 4) return _mm512_broadcastd_epi32(as<__m128i>(x));
	else if constexpr (sizeof(T) == 2 && has_avx512bw<ISA>) return _mm512_broadcastw_epi16(as<__m128i>(x));
	else if constexpr (sizeof(T) == 1 && has_avx512bw<ISA>) return _mm512_broadcastb_epi8(as<__m128i>(x));
	return brodcast_<ISA, __m512i>()(extract_<ISA, T>(x, std::integral_constant<sizetype, 0>{}));
}

template <arch::ISA ISA, arithmetic_type T, intrin_type V, sizetype I>
raze_always_inline V splat_native_(V x, std::integral_constant<sizetype, I> i) noexcept
	requires(i >= 0 && i < (sizeof(V) / sizeof(T)))
{
	constexpr auto size = sizeof(V) / sizeof(T);

	if constexpr (sizeof(V) == 16) {
		if constexpr (sizeof(T) == 8) {
			if constexpr (i == 0) return as<V>(_mm_shuffle_pd(as<__m128d>(x), as<__m128d>(x), 0));
			else return as<V>(_mm_shuffle_pd(as<__m128d>(x), as<__m128d>(x), 0x03));
		}
		else if constexpr (sizeof(T) == 4) return as<V>(_mm_shuffle_epi32(as<__m128i>(x), __broadcast_pshufd_index(i)));
		else if constexpr (sizeof(T) == 2 && has_avx2<ISA> && i == 0) return as<V>(_mm_broadcastw_epi16(as<__m128i>(x)));
		else if constexpr (sizeof(T) == 1) {
			if constexpr (has_avx2<ISA> && i == 0) return as<V>(_mm_broadcastb_epi8(as<__m128i>(x)));
			else if constexpr (!has_ssse3<ISA>) {
				__m128i combined;

				if constexpr (i < 8) combined = _mm_unpacklo_epi8(as<__m128i>(x), as<__m128i>(x));
				else combined = _mm_unpackhi_epi8(as<__m128i>(x), as<__m128i>(x));

				return as<V>(generic_shuffle_native_<ISA, i16>(combined, make_splat_pattern<simd<i16, runtime_abi<ISA, 8>>, i % 8>{}));
			}
		}
		return generic_shuffle_native_<ISA, T>(x, make_splat_pattern<simd<T, runtime_abi<ISA, size>>, i>{});
	}
	else if constexpr (sizeof(V) == 32) {
		constexpr auto index = std::integral_constant<sizetype, i % (size / 2)>{};

		if constexpr (i == 0 && has_avx2<ISA>) {
			if constexpr (sizeof(T) == 8)	   return as<V>(_mm256_broadcastq_epi64(as<__m128i>(x)));
			else if constexpr (sizeof(T) == 4) return as<V>(_mm256_broadcastd_epi32(as<__m128i>(x)));
			else if constexpr (sizeof(T) == 2) return as<V>(_mm256_broadcastw_epi16(as<__m128i>(x)));
			else if constexpr (sizeof(T) == 1) return as<V>(_mm256_broadcastb_epi8(as<__m128i>(x)));
		}
		else if constexpr (sizeof(T) == 8) return generic_shuffle_native_<ISA, T, V>(x,
			make_splat_pattern<simd<T, runtime_abi<ISA, 4>>, i>{});
		else if constexpr (sizeof(T) == 4) {
			constexpr auto index = std::integral_constant<sizetype, i % (size / 2)>{};

			if constexpr (i < (size / 2)) {
				const auto half = splat_native_<ISA, T>(as<__m128i>(x), index);
				return as<V>(_mm256_insertf128_si256(as<__m256i>(half), as<__m128i>(half), 1));
			}
			else {
				const auto half = _mm256_permute_ps(as<__m256>(x), broadcast_pshufd_index_(std::integral_constant<sizetype, i % (size / 2)>{}));
				return as<V>(_mm256_permute2f128_si256(as<__m256i>(half), as<__m256i>(half), 0x31));
			}
		}
		else {
			__m128i half;

			if constexpr (i < (size / 2)) half = splat_native_<ISA, T>(as<__m128i>(x), index);
			else half = splat_native_<ISA, T>(_mm256_extractf128_si256(as<__m256i>(x), 1), index);

			return as<V>(_mm256_insertf128_si256(as<__m256i>(half), as<__m128i>(half), 1));
		}
	}
	else if constexpr (sizeof(V) == 64) {
		if constexpr (i == 0) return as<V>(__zmm_broadcast_low<ISA, T>(x));
		else {
			constexpr auto index = std::integral_constant<sizetype, i % (size / 4)>{};
			constexpr auto lane = i / (size / 4);

			return as<V>(zmm_broadcast_low_<ISA, T>(splat_native_<ISA, T>(
				_mm512_extracti32x4_epi32(as<__m512i>(x), lane), index)));
		}
	}
}

template <class Pattern>
raze_always_inline pattern_vector_t<Pattern> splat_(const pattern_vector_t<Pattern>& x, Pattern p) noexcept {
	using V = pattern_vector_t<Pattern>;

	if constexpr (native<V>) {
		using Ret = decltype(splat_native_<abi_t<V>::isa, typename V::value_type>(ustorage(x.template __get<0>()), p.template at<0>()));

		if constexpr (!std::is_void_v<_Ret>) {
			auto r = x;

			auto& storage = r.template __get<0>();
			storage = splat_native_<abi_t<V>::isa, typename V::value_type>(ustorage(storage), p.template at<0>());

			return r;
		}
		else return generic_shuffle_(x, p);
	}

	return x[p.template at<0>()];
}

__RAZE_VX_NAMESPACE_END
