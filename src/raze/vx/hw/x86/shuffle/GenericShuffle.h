#pragma once 

#include <src/raze/vx/hw/common/PatternsCheck.h>
#include <src/raze/vx/hw/x86/cast/As.h>
#include <src/raze/vx/hw/x86/merge/Select.h>
#include <src/raze/vx/hw/x86/mask/operations/ToMask.h>
#include <src/raze/vx/hw/configurable/memory/Load.h>
#include <src/raze/vx/hw/configurable/memory/Store.h>
#include <src/raze/vx/hw/x86/compare/Greater.h>
#include <src/raze/vx/hw/configurable/merge/Select.h>
#include <src/raze/vx/hw/configurable/shape/Split.h>
#include <src/raze/vx/hw/configurable/shape/SplitBy.h>


__RAZE_VX_NAMESPACE_BEGIN

template <class V>
struct fallback_result {
	static constexpr auto fallback = true;

	fallback_result(V x) noexcept : _data(x) {}

	fallback_result(fallback_result&&) noexcept = default;
	fallback_result(const fallback_result&) noexcept = default;

	fallback_result& operator=(const fallback_result&) noexcept = default;
	fallback_result& operator=(fallback_result&&) noexcept = default;

	raze_always_inline operator V() const noexcept {
		return _data;
	}

	V _data;
};


template <intrin_type V, class Idx>
struct rotate_indices {
	using index_type = Idx;
	rotate_indices(V x) noexcept : _data(x) {}

	rotate_indices(const rotate_indices&) noexcept = default;
	rotate_indices(rotate_indices&&) noexcept = default;

	rotate_indices& operator=(const rotate_indices&) noexcept = default;
	rotate_indices& operator=(rotate_indices&&) noexcept = default;

	raze_always_inline operator V() const noexcept {
		return _data;
	}

	raze_always_inline V data() const noexcept {
		return _data;
	}

	V _data;
};

template <arch::ISA ISA, class T, intrin_type V, sizetype ... Indices>
raze_always_inline fallback_result<V> shuffle_fallback(
	V x, std::integer_sequence<sizetype, Indices...>) noexcept
{
	constexpr auto length = sizeof(V) / sizeof(T);
	using IndexType = typename IntegerForSizeof<T>::Unsigned;

	alignas(sizeof(V)) static constexpr IndexType indices_array[] = { Indices... };

	alignas(sizeof(V)) T src[length];
	alignas(sizeof(V)) T dst[length];

	store_(src, x, aligned_policy{});

	for (auto i = 0; i < length; ++i)
		dst[i] = src[indices_array[i]];

	return load_<ISA, V>(dst, aligned_policy{});
}

template <arch::ISA ISA, class T, intrin_type V, intrin_type Index>
raze_always_inline fallback_result<V> shuffle_fallback(V x, Index idx) noexcept {
	constexpr auto length = sizeof(V) / sizeof(T);
	using IndexType = typename IntegerForSizeof<T>::Unsigned;

	alignas(sizeof(V)) T src[length];
	alignas(sizeof(V)) T dst[length];
	alignas(sizeof(V)) IndexType idxs[length];

	store_(src, vector, aligned_policy{});
	store_(idxs, idx, aligned_policy{});

	for (auto i = 0; i < length; ++i)
		dst[i] = src[idxs[i]];

	return load_<ISA, V>(dst, aligned_policy{});
}

template <class Pattern>
raze_always_inline fallback_result<pattern_vector_t<Pattern>> 
generic_shuffle_scalar_fallback_(const pattern_vector_t<Pattern>& x, Pattern p) noexcept 
{
	using V = pattern_vector_t<Pattern>;

	alignas(64) typename V::value_type dst[V::size()];
	alignas(64) typename V::value_type src[V::size()];

	vx::store[vx::aligned](src, x);

	[&] <sizetype ... I> (std::integer_sequence<sizetype, I...>) raze_always_inline_lambda {
		([&](auto i) raze_always_inline_lambda{
			dst[i] = src[p[i]];
		}(std::integral_constant<sizetype, I>{}), ...);
	}(std::make_integer_sequence<sizetype, V::size()>{});

	return vx::load<V>[vx::aligned](dst);
}

template <simd_type V, index_simd_type Index>
raze_always_inline fallback_result<V> 
generic_shuffle_scalar_fallback_(const V& x, const Index& idx) noexcept 
{
	alignas(64) typename V::value_type dst[V::size()];
	alignas(64) typename V::value_type src[V::size()];
	alignas(64) typename Index::value_type idxs[Index::size()];

	vx::store[vx::aligned](src, x);
	vx::store[vx::aligned](idxs, idx);

	if constexpr (V::size() <= 32) {
		[&] <sizetype ... I> (std::integer_sequence<sizetype, I...>) raze_always_inline_lambda {
			([&](auto i) raze_always_inline_lambda{
				dst[i] = src[idxs[i]];
			}(std::integral_constant<sizetype, I>{}), ...);
		}(std::make_integer_sequence<sizetype, V::size()>{});
	}
	else {
		for (auto i = 0; i < V::size(); ++i)
			dst[i] = src[idxs[i]];
	}

	return vx::load<V>[vx::aligned](dst);
}

template <arch::ISA ISA, intrin_type V>
raze_always_inline V byte_shuffle_mask_avx2(V x) noexcept {
	static_assert(has_avx2<ISA>);
	// Multiplication by 0x0202 (= {0x02, 0x02} as a pair of bytes) creates pairs {2*i, 2*i}
	// Adding 0x0100 increments the high byte of each pair (resulting in {2*i, 2*i+1}).
	return as<V>(_mm256_add_epi16(_mm256_mullo_epi16(as<__m256i>(x), 
		broadcast_<ISA, __m256i>(std::integral_constant<i16, 0x202>{})),
		broadcast_<ISA, __m256i>(std::integral_constant<i16, 0x0100>{})));
}

template <arch::ISA ISA, intrin_type V, intrin_type Index>
raze_always_inline V shuffle_i8x64_avx512(V x, Index idx) noexcept {
	static_assert(sizeof(V) == sizeof(__m512i));

	if constexpr (has_avx512vbmi<ISA>) {
		return as<V>(_mm512_permutexvar_epi8(as<__m512i>(idx), as<__m512i>(x)));
	}
	else if constexpr (has_avx512bw<ISA>) {
		auto src_part = _mm512_shuffle_i64x2(as<__m512i>(x), as<__m512i>(x), 0);
		auto result = _mm512_shuffle_epi8(src_part, as<__m512i>(idx));

		src_part = _mm512_shuffle_i64x2(as<__m512i>(x), as<__m512i>(x), 0x55);
		auto candidate = _mm512_shuffle_epi8(src_part, as<__m512i>(idx));
		auto threshold = broadcast_<ISA, __m512i>(std::integral_constant<i8, 0x0F>{});
		result = _mm512_mask_mov_epi8(result, _mm512_cmpgt_epi8_mask(as<__m512i>(idx), threshold), candidate);

		src_part = _mm512_shuffle_i64x2(as<__m512i>(x), as<__m512i>(x), 0xAA);
		candidate = _mm512_shuffle_epi8(src_part, as<__m512i>(idx));
		threshold = broadcast_<ISA, __m512i>(std::integral_constant<i8, 0x1F>{});
		result = _mm512_mask_mov_epi8(result, _mm512_cmpgt_epi8_mask(as<__m512i>(idx), threshold), candidate);

		src_part = _mm512_shuffle_i64x2(as<__m512i>(x), as<__m512i>(x), 0xFF);
		candidate = _mm512_shuffle_epi8(src_part, as<__m512i>(idx));
		threshold = broadcast_<ISA, __m512i>(std::integral_constant<i8, 0x2F>{});
		result = _mm512_mask_mov_epi8(result, _mm512_cmpgt_epi8_mask(as<__m512i>(idx), threshold), candidate);

		return as<V>(result);
	}
	else {
		const auto idx_high = _mm512_extracti64x4_epi64(as<__m512i>(idx), 1);
		auto src_part = as<__m256i>(_mm512_shuffle_i64x2(as<__m512i>(x), as<__m512i>(x), 0));

		auto result_low = _mm256_shuffle_epi8(src_part, as<__m256i>(idx));
		auto result_high = _mm256_shuffle_epi8(src_part, idx_high);

		src_part = as<__m256i>(_mm512_shuffle_i64x2(as<__m512i>(x), as<__m512i>(x), 0x05));
		auto candidate_low = _mm256_shuffle_epi8(src_part, as<__m256i>(idx));
		auto candidate_high = _mm256_shuffle_epi8(src_part, idx_high);

		auto threshold = broadcast_<ISA, __m256i>(std::integral_constant<i8, 0x0F>{});
		result_low = _mm256_blendv_epi8(result_low, candidate_low, _mm256_cmpgt_epi8(as<__m256i>(idx), threshold));
		result_high = _mm256_blendv_epi8(result_high, candidate_high, _mm256_cmpgt_epi8(idx_high, threshold));

		src_part = as<__m256i>(_mm512_shuffle_i64x2(as<__m512i>(x), as<__m512i>(x), 0xA));

		candidate_low = _mm256_shuffle_epi8(src_part, as<__m256i>(idx));
		candidate_high = _mm256_shuffle_epi8(src_part, idx_high);

		threshold = broadcast_<ISA, __m256i>(std::integral_constant<i8, 0x1F>{});
		result_low = _mm256_blendv_epi8(result_low, candidate_low, _mm256_cmpgt_epi8(as<__m256i>(idx), threshold));
		result_high = _mm256_blendv_epi8(result_high, candidate_high, _mm256_cmpgt_epi8(idx_high, threshold));

		src_part = as<__m256i>(_mm512_shuffle_i64x2(as<__m512i>(x), as<__m512i>(x), 0x1F));

		candidate_low = _mm256_shuffle_epi8(src_part, as<__m256i>(idx));
		candidate_high = _mm256_shuffle_epi8(src_part, idx_high);

		threshold = broadcast_<ISA, __m256i>(std::integral_constant<i8, 0x2F>{});
		result_low = _mm256_blendv_epi8(result_low, candidate_low, _mm256_cmpgt_epi8(as<__m256i>(idx), threshold));
		result_high = _mm256_blendv_epi8(result_high, candidate_high, _mm256_cmpgt_epi8(idx_high, threshold));

		return as<V>(_mm512_inserti64x4(as<__m512i>(result_low), result_high, 1));
	}
}

template <arch::ISA ISA, intrin_type V, intrin_type Index>
raze_always_inline V shuffle_i16x32_avx512(V x, Index idx) noexcept {
	static_assert(sizeof(V) == sizeof(__m512i));

	if constexpr (has_avx512bw<ISA>) {
		return as<V>(_mm512_permutexvar_epi16(as<__m512i>(idx), as<__m512i>(x)));
	}
	else {
		// shuffle_i64x2 layouts:
		// 0x00 : {L0,L0,L0,L0}
		// 0x05 : {L1,L1,L1,L1}
		// 0x0A : {L2,L2,L2,L2}
		// 0x0F : {L3,L3,L3,L3}

		const auto idx_high = _mm512_extracti64x4_epi64(as<__m512i>(idx), 1);

		const auto byte_shuffle_mask_low = byte_shuffle_mask_avx2<ISA>(as<__m256i>(idx));
		const auto byte_shuffle_mask_high = byte_shuffle_mask_avx2<ISA>(idx_high);

		auto src_part = as<__m256i>(_mm512_shuffle_i64x2(as<__m512i>(x), as<__m512i>(x), 0));

		auto result_low = _mm256_shuffle_epi8(src_part, byte_shuffle_mask_low);
		auto result_high = _mm256_shuffle_epi8(src_part, byte_shuffle_mask_high);

		src_part = as<__m256i>(_mm512_shuffle_i64x2(as<__m512i>(x), as<__m512i>(x), 0x05));
		auto candidate_low = _mm256_shuffle_epi8(src_part, byte_shuffle_mask_low);
		auto candidate_high = _mm256_shuffle_epi8(src_part, byte_shuffle_mask_high);

		auto threshold = broadcast_<ISA, __m256i>(std::integral_constant<i16, 0x07>{});
		result_low = _mm256_blendv_epi8(result_low, candidate_low, _mm256_cmpgt_epi16(as<__m256i>(idx), threshold));
		result_high = _mm256_blendv_epi8(result_high, candidate_high, _mm256_cmpgt_epi16(idx_high, threshold));

		src_part = as<__m256i>(_mm512_shuffle_i64x2(as<__m512i>(x), as<__m512i>(x), 0xA));

		candidate_low = _mm256_shuffle_epi8(src_part, byte_shuffle_mask_low);
		candidate_high = _mm256_shuffle_epi8(src_part, byte_shuffle_mask_high);

		threshold = broadcast_<ISA, __m256i>(std::integral_constant<i16, 0x0F>{});
		result_low = _mm256_blendv_epi8(result_low, candidate_low, _mm256_cmpgt_epi16(as<__m256i>(idx), threshold));
		result_high = _mm256_blendv_epi8(result_high, candidate_high, _mm256_cmpgt_epi16(idx_high, threshold));

		src_part = as<__m256i>(_mm512_shuffle_i64x2(as<__m512i>(x), as<__m512i>(x), 0xF));

		candidate_low = _mm256_shuffle_epi8(src_part, byte_shuffle_mask_low);
		candidate_high = _mm256_shuffle_epi8(src_part, byte_shuffle_mask_high);

		threshold = broadcast_<ISA, __m256i>(std::integral_constant<i16, 0x17>{});
		result_low = _mm256_blendv_epi8(result_low, candidate_low, _mm256_cmpgt_epi16(as<__m256i>(idx), threshold));
		result_high = _mm256_blendv_epi8(result_high, candidate_high, _mm256_cmpgt_epi16(idx_high, threshold));

		return as<V>(_mm512_inserti64x4(as<__m512i>(result_low), result_high, 1));
	}
}

template <arch::ISA ISA, arithmetic_type T, intrin_type V, class Pattern>
raze_always_inline auto generic_shuffle_native_(V x, Pattern p) noexcept {
	if constexpr (sizeof(V) == 16) {
		if constexpr (sizeof(T) == 8) {
			constexpr auto mask = shufpd_to_pshufd_mask(p);
			return as<V>(_mm_shuffle_epi32(as<__m128i>(x), mask));
		}
		else if constexpr (sizeof(T) == 4) {
			constexpr auto mask = to_pshufd_mask(p);
			return as<V>(_mm_shuffle_epi32(as<__m128i>(x), mask));
		}
		else if constexpr (sizeof(T) == 2) {
			constexpr auto low_shuf = to_pshufd_mask(p);
			constexpr auto high_shuf = (p[4] & 0x03) | ((p[5] & 0x03) << 2) | ((p[6] & 0x03) << 4) | ((p[7] & 0x03) << 6);

			if constexpr (can_widen_shuffle(p)) {
				return as<V>(_mm_shuffle_epi32(as<__m128i>(x), to_pshufd_mask(p.widen())));
			}
			else if constexpr (!across_halfs(p)) {
				auto shuffled = _mm_shufflelo_epi16(as<__m128i>(x), low_shuf);
				return as<V>(_mm_shufflehi_epi16(shuffled, high_shuf));
			}
			else if constexpr (has_ssse3<ISA>) {
				return as<V>(_mm_shuffle_epi8(as<__m128i>(x), p.template expand<u16, u8>().template as_native<__m128i>()));
			}
			else if constexpr (is_dup_low(p)) {
				const auto low = _mm_shufflelo_epi16(as<__m128i>(x), low_shuf);
				return as<V>(_mm_unpacklo_epi64(low, low));
			}
			else if constexpr (is_dup_high(p)) {
				const auto high = _mm_shufflehi_epi16(as<__m128i>(x), high_shuf);
				return as<V>(_mm_unpackhi_epi64(high, high));
			}
			else if constexpr (is_low_half(p)) {
				const auto low1 = _mm_shufflelo_epi16(as<__m128i>(x), low_shuf);
				const auto low2 = _mm_shufflelo_epi16(as<__m128i>(x), high_shuf);
				return as<V>(_mm_unpacklo_epi64(low1, low2));
			}
			else if constexpr (__is_high_half(__p)) {
				const auto high1 = _mm_shufflehi_epi16(as<__m128i>(x), low_shuf);
				const auto high2 = _mm_shufflehi_epi16(as<__m128i>(x), high_shuf);
				return as<V>(_mm_unpacklo_epi64(high1, high2));
			}
			else {
				const auto low1 = _mm_shufflelo_epi16(as<__m128i>(x), low_shuf);
				const auto low2 = _mm_shufflelo_epi16(as<__m128i>(x), high_shuf);
				const auto high1 = _mm_shufflehi_epi16(as<__m128i>(x), low_shuf);
				const auto high2 = _mm_shufflehi_epi16(as<__m128i>(x), high_shuf);

				const auto low = _mm_unpacklo_epi64(low1, low2);
				const auto high = _mm_unpackhi_epi64(high1, high2);

				return select_<ISA, T>(low, high, to_mask_<ISA, T>(p < std::integral_constant<sizetype, 4>{}));
			}
		}
		else if constexpr (sizeof(T) == 1) {
			if constexpr (can_widen_shuffle(p) && !across_halfs(p)) {
				constexpr auto widen = p.widen();

				constexpr auto low_shuf = to_pshufd_mask(widen);
				constexpr auto high_shuf = (widen[4] & 0x03) | ((widen[5] & 0x03) << 2)
					| ((widen[6] & 0x03) << 4) | ((widen[7] & 0x03) << 6);

				auto shuffled = _mm_shufflelo_epi16(as<__m128i>(x), low_shuf);
				return as<V>(_mm_shufflehi_epi16(shuffled, high_shuf));
			}
			else if constexpr (has_ssse3<ISA>) {
				return as<V>(_mm_shuffle_epi8(as<__m128i>(x), p.template as_native<__m128i>()));
			}
			else return shuffle_fallback_<ISA, T>(x, p.get());
		}
		else return shuffle_fallback_<ISA, T>(x, p.get());
	}
	else if constexpr (sizeof(V) == 32) {
		if constexpr (sizeof(T) == 8) {
			constexpr auto mask = ((p[0] % 2)) | ((p[1] % 2) << 1) | ((p[2] % 2) << 2) | ((p[3] % 2) << 3);

			if constexpr (!across_halfs(p)) {
				return as<V>(_mm256_permute_pd(as<__m256d>(x), mask));
			}
			else if constexpr (has_avx2<ISA>) return as<V>(_mm256_permute4x64_epi64(as<__m256i>(x), to_pshufd_mask(p)));
			else if constexpr (is_low_half(p)) {
				const auto_broadcasted_low_lane = _mm256_permute2f128_pd(as<__m256d>(x), as<__m256d>(x), 0);
				return as<V>(_mm256_permute_pd(broadcasted_low_lane, mask));
			}
			else if constexpr (is_high_half(p)) {
				const auto broadcasted_high_lane = _mm256_permute2f128_pd(as<__m256d>(x), as<__m256d>(x), 0x11);
				return as<V>(_mm256_permute_pd(broadcasted_high_lane, mask));
			}
			else {
				constexpr auto blend_mask = (p / std::integral_constant<sizetype, 2>{}) != shuffle_pattern<pattern_vector_t<Pattern>, 0, 0, 1, 1>{};
				const auto swapped_lanes = _mm256_permute2f128_pd(as<__m256d>(x), as<__m256d>(x), 0x01);

				const auto first = _mm256_permute_pd(as<__m256d>(x), mask);
				const auto second = _mm256_permute_pd(swapped_lanes, mask);
				
				return as<V>(_mm256_blend_pd(first, second, blend_mask));
			}
		}
		else if constexpr (sizeof(T) == 4) {
			if constexpr (!across_halfs(p) && is_halfs_equal(p))
				return as<V>(_mm256_shuffle_ps(as<__m256>(x), as<__m256>(x), to_pshufd_mask(p)));
			else if constexpr (can_widen_shuffle(p) && has_avx2<ISA>)
				return as<V>(_mm256_permute4x64_epi64(as<__m256i>(x), to_pshufd_mask(p.widen())));
			else if constexpr (has_avx2<ISA>)
				return as<V>(_mm256_permutevar8x32_epi32(as<__m256i>(x), p.template as_native<__m256i>()));
			else if constexpr (is_dup_low(p)) {
				auto split = _mm256_permute2f128_ps(as<__m256>(x), as<__m256>(x), 0);
				
				if constexpr (!is_dup_low_identity(p))
					split = _mm256_permute_ps(split, to_pshufd_mask(p));

				return as<V>(split);
			}
			else if constexpr (is_dup_high(p)) {
				constexpr auto mask = (((p[4] - 4) & 0x03) | (((p[5] - 4) & 0x03) << 2)
					| (((p[6] - 4) & 0x03) << 4) | (((p[7] - 4) & 0x03) << 6));

				auto split = _mm256_permute2f128_ps(as<__m256>(x), as<__m256>(x), 0x11);

				if constexpr (!is_dup_low_identity(p))
					split = _mm256_permute_ps(split,_mask);

				return as<V>(split);
			}
			else if constexpr (is_low_half(p)) {
				const auto broadcasted_low_lane = _mm256_permute2f128_ps(as<__m256>(x), as<__m256>(x), 0);
				return as<V>(_mm256_permutevar_ps(broadcasted_low_lane, (p % std::integral_constant<sizetype, 4>{}).template as_native<__m256i>()));
			}
			else if constexpr (is_high_half(p)) {
				const auto broadcasted_low_lane = _mm256_permute2f128_ps(as<__m256>(x), as<__m256>(x), 0x11);
				return as<V>(_mm256_permutevar_ps(broadcasted_low_lane, (p % std::integral_constant<sizetype, 4>{}).template as_native<__m256i>()));
			}
			else {
				constexpr auto low_mask = (p % std::integral_constant<sizetype, 4>{});
				constexpr auto blend_mask = (p / std::integral_constant<sizetype, 4>{}) != shuffle_pattern<pattern_vector_t<Pattern>, 0, 0, 0, 0, 1, 1, 1, 1>{};
				const auto swapped_lanes = _mm256_permute2f128_ps(as<__m256>(x), as<__m256>(x), 0x01);

				const auto native_mask = p.template as_native<__m256i>();

				const auto first = _mm256_permutevar_ps(as<__m256>(x), native_mask);
				const auto second = _mm256_permutevar_ps(swapped_lanes, native_mask);

				return as<V>(_mm256_blend_ps(first, second, blend_mask));
			}
		}
		else if constexpr (sizeof(T) == 2 && has_avx2<ISA>) {
			constexpr auto expanded = p.template expand<u16, u8>();
			constexpr auto mask = expanded % std::integral_constant<sizetype, 16>{};

			if constexpr (!across_halfs(p)) {
				return as<V>(_mm256_shuffle_epi8(as<__m256i>(x), p.template expand<u16, u8>().template as_native<__m256i>()));
			}
			else if constexpr (has_avx512bw<ISA> && has_avx512vl<ISA>) {
				return as<V>(_mm256_permutexvar_epi16(p.template as_native<__m256i>(), as<__m256i>(x)));
			}
			else if constexpr (is_low_half(p)) { 
				const auto broadcasted_low = _mm256_permute2x128_si256(as<__m256i>(x), as<__m256i>(x), 0);
				return as<V>(_mm256_shuffle_epi8(broadcasted_low, mask.template as_native<__m256i>()));
			}
			else if constexpr (__is_high_half(__p)) { 
				const auto broadcasted_low = _mm256_permute2x128_si256(as<__m256i>(x), as<__m256i>(x), 0x11);
				return as<V>(_mm256_shuffle_epi8(broadcasted_low, mask.template as_native<__m256i>()));
			}
			else {
				const auto swapped = _mm256_permute2x128_si256(as<__m256i>(x), as<__m256i>(x), 0x01);

				const auto shuffled1 = _mm256_shuffle_epi8(as<__m256i>(x), expanded.crossing_lanes().template as_native<__m256i>());
				const auto shuffled2 = _mm256_shuffle_epi8(swapped, expanded.non_crossing_lanes().template as_native<__m256i>());

				return as<V>(_mm256_or_si256(shuffled1, shuffled2));
			}
		}
		else if constexpr (sizeof(T) == 1 && has_avx2<ISA>) {
			constexpr auto mask = (p % std::integral_constant<sizetype, 16>{});

			if constexpr (!across_halfs(p)) {
				return as<V>(_mm256_shuffle_epi8(as<__m256i>(x), p.template as_native<__m256i>()));
			}
			else if constexpr (has_avx512vbmi<ISA> && has_avx512vl<ISA>) {
				return as<V>(_mm256_permutexvar_epi8(p.template as_native<__m256i>(), as<__m256i>(x)));
			}
			else if constexpr (is_low_half(p)) {
				const auto broadcasted_low = _mm256_permute2x128_si256(as<__m256i>(x), as<__m256i>(x), 0);
				return as<V>(_mm256_shuffle_epi8(broadcasted_low, mask.template as_native<__m256i>()));
			}
			else if constexpr (is_high_half(p)) {
				const auto broadcasted_low = _mm256_permute2x128_si256(as<__m256i>(x), as<__m256i>(x), 0x11);
				return as<V>(_mm256_shuffle_epi8(broadcasted_low, mask.template as_native<__m256i>()));
			}
			else {
				const auto shuffled1 = _mm256_shuffle_epi8(as<__m256i>(x), p.crossing_lanes().template as_native<__m256i>());
				const auto shuffled2 = _mm256_shuffle_epi8(_mm256_permute2x128_si256(as<__m256i>(x), as<__m256i>(x), 0x01),
					p.non_crossing_lanes().template as_native<__m256i>());

				return as<V>(_mm256_or_si256(shuffled1, shuffled2));
			}
		}
		else return shuffle_fallback_<ISA, T>(x, p.get());
	}
	else if constexpr (sizeof(V) == 64) {
		if constexpr (sizeof(T) == 8) {
			if constexpr (!across_quads(p)) {
				constexpr auto mask = ((p[0] & 1) << 0) | ((p[1] & 1) << 1) | ((p[2] & 1) << 2) | ((p[3] & 1) << 3) |
					((p[4] & 1) << 4) | ((p[5] & 1) << 5) | ((p[6] & 1) << 6) | ((p[7] & 1) << 7);

				return as<V>(_mm512_permute_pd(as<__m512d>(x), mask));
			}
			return as<V>(_mm512_permutexvar_epi64(p.template as_native<__m512i>(), as<__m512i>(x)));
		}
		else if constexpr (sizeof(T) == 4) {
			if constexpr (!across_quads(p)) {
				constexpr auto q0 = to_pshufd_mask(p.offset(std::integral_constant<sizetype, 0>{}));
				constexpr auto q1 = to_pshufd_mask(p.offset(std::integral_constant<sizetype, 4>{}));
				constexpr auto q2 = to_pshufd_mask(p.offset(std::integral_constant<sizetype, 8>{}));
				constexpr auto q3 = to_pshufd_mask(p.offset(std::integral_constant<sizetype, 12>{}));

				if constexpr (q0 == q1 && q1 == q2 && q2 == q3) return as<V>(_mm512_permute_ps(as<__m512>(x), q0));
				else return as<V>(_mm512_permutevar_ps(as<__m512>(x), p.template as_native<__m512i>()));
			}
			else return as<V>(_mm512_permutevar_epi32(p.template as_native<__m512i>(), as<__m512i>(x)));
		}
		else if constexpr (sizeof(T) == 2) {
			if constexpr (has_avx512bw<ISA>) return as<V>(_mm512_permutexvar_epi16(p.template as_native<__m512i>(), as<__m512i>(x)));
			else if constexpr (!across_quads(p)) {
				const auto shuffle = p.template expand<u16, u8>().template as_native<__m512i>();

				const auto low = _mm256_shuffle_epi8(as<__m256i>(x), as<__m256i>(shuffle));
				const auto high = _mm256_shuffle_epi8(_mm512_extracti64x4_epi64(as<__m512i>(x), 1), _mm512_extracti64x4_epi64(as<__m512i>(shuffle), 1));

				return as<V>(_mm512_inserti64x4(as<__m512i>(low), high, 1));
			}
			else return shuffle_i8x64_avx512<ISA>(x, p.template expand<u16, u8>().template as_native<__m512i>());
		}
		else if constexpr (sizeof(T) == 1) {
			const auto shuffle = p.template as_native<__m512i>();

			if constexpr (!across_quads(p) && (!has_avx512vbmi<ISA> || has_avx512bw<ISA>)) {
				if constexpr (has_avx512bw<ISA>) return as<V>(_mm512_shuffle_epi8(as<__m512i>(x), shuffle));
				else {
					const auto low = _mm256_shuffle_epi8(as<__m256i>(x), as<__m256i>(shuffle));
					const auto high = _mm256_shuffle_epi8(_mm512_extracti64x4_epi64(as<__m512i>(x), 1), _mm512_extracti64x4_epi64(as<__m512i>(shuffle), 1));

					return as<V>(_mm512_inserti64x4(as<__m512i>(low), high, 1));
				}
			}
			else return shuffle_i8x64_avx512<ISA>(x, p.template as_native<__m512i>());
		}
		else return shuffle_fallback_<ISA, T>(x, p.get());
	}
}

template <arch::ISA ISA, arithmetic_type T, intrin_type V, intrin_type Index>
raze_always_inline auto generic_shuffle_native_(V x, Index idx) noexcept {
	if constexpr (sizeof(V) == 16) {
		if constexpr (sizeof(T) == 8) {
			if constexpr (has_avx512vl<ISA>) {
				return as<V>(_mm_permutex2var_epi64(as<__m128i>(x), as<__m128i>(idx), as<__m128i>(x)));
			}
			else if constexpr (has_ssse3<ISA>) {
				const auto dupe_mask = _mm_setr_epi8(0, 0, 0, 0, 0, 0, 0, 0, 8, 8, 8, 8, 8, 8, 8, 8);
				const auto add_seq = _mm_setr_epi8(0, 1, 2, 3, 4, 5, 6, 7, 0, 1, 2, 3, 4, 5, 6, 7);

				const auto byte_offsets = _mm_slli_epi64(as<__m128i>(idx), 3);
				const auto duplicated = _mm_shuffle_epi8(byte_offsets, dupe_mask);
				const auto mask = _mm_add_epi8(duplicated, add_seq);

				return as<V>(_mm_shuffle_epi8(as<__m128i>(x), mask));
			}
			else {
				alignas(16) T t[2];
				store_(t, x, aligned_policy{});

				const auto* i = reinterpret_cast<const unsigned long long*>(&idx);
				
				if constexpr (std::is_floating_point_v<T>) return fallback_result { as<V>(_mm_set_pd(t[i[1]], t[i[0]])) };
				else return fallback_result{ as<V>(_mm_set_epi64x(t[i[1]], t[i[0]])) };
			}
		}
		else if constexpr (sizeof(T) == 4) {
			if constexpr (has_avx512vl<ISA>) {
				return as<V>(_mm_permutex2var_epi32(as<__m128i>(x), as<__m128i>(idx), as<__m128i>(x)));
			}
			else if constexpr (has_ssse3<ISA>) {
				const auto dupe_mask = _mm_setr_epi8(0, 0, 0, 0, 4, 4, 4, 4, 8, 8, 8, 8, 12, 12, 12, 12);
				const auto add_seq = _mm_setr_epi8(0, 1, 2, 3, 0, 1, 2, 3, 0, 1, 2, 3, 0, 1, 2, 3);

				const auto byte_offsets = _mm_slli_epi32(as<__m128i>(idx), 2);
				const auto duplicated = _mm_shuffle_epi8(byte_offsets,_dupe_mask);
				const auto pshufb_mask = _mm_add_epi8(duplicated, add_seq);

				return as<V>(_mm_shuffle_epi8(as<__m128i>(x), pshufb_mask));
			}
			else {
				alignas(16) T t[4];
				store_(t, __x, aligned_policy{});

				const auto* i = reinterpret_cast<const unsigned int*>(&idx);
				
				if constexpr (std::is_floating_point_v<T>) return fallback_result{ as<V>(_mm_set_ps(t[i[3]], t[i[2]], t[i[1]], t[i[0]])) };
				else return fallback_result{ as<V>(_mm_set_epi32(t[i[3]], t[i[2]], t[i[1]], t[i[0]])) };
			}
		}
		else if constexpr (sizeof(T) == 2) {
			if constexpr (has_avx512vl<ISA> && has_avx512bw<ISA>) {
				return as<V>(_mm_permutexvar_epi16(as<__m128i>(idx), as<__m128i>(x)));
			}
			else if constexpr (has_ssse3<ISA>) {
				const auto dupe_mask = _mm_setr_epi8(0, 0, 2, 2, 4, 4, 6, 6, 8, 8, 10, 10, 12, 12, 14, 14);
				const auto add_seq = _mm_setr_epi8(0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1);

				const auto byte_offsets = _mm_slli_epi16(as<__m128i>(idx), 1);
				const auto duplicated = _mm_shuffle_epi8(byte_offsets, dupe_mask);
				const auto pshufb_mask = _mm_add_epi8(duplicated, add_seq);

				return as<V>(_mm_shuffle_epi8(as<__m128i>(x), pshufb_mask));
			}
			else {
				alignas(16) T t[8];
				store_(t, x, aligned_policy{});

				const auto* i = reinterpret_cast<const unsigned short*>(&idx);
				return fallback_result{ as<V>(_mm_set_epi16(t[i[7]], t[i[6]], t[i[5]],
					t[i[4]], t[i[3]], t[i[2]], t[i[1]], t[i[0]])) };
			}
		}
		else if constexpr (sizeof(T) == 1) {
			if constexpr (has_ssse3<ISA>) {
				return as<V>(_mm_shuffle_epi8(as<__m128i>(x), as<__m128i>(idx)));
			}
			else {
				alignas(16) T t[16];
				store_(t, x, aligned_policy{});

				const auto* i = reinterpret_cast<const unsigned char*>(&idx);
				return fallback_result { as<V>(_mm_set_epi8(t[i[15]], t[i[14]], t[i[13]],
					t[i[12]], t[i[11]], t[i[10]], t[i[9]], t[i[8]],
					t[i[7]], t[i[6]], t[i[5]], t[i[4]], t[i[3]],
					t[i[2]], t[i[1]], t[i[0]])) };
			}
		}
	}
	else if constexpr (sizeof(V) == 32) {
		if constexpr (sizeof(T) == 8) {
			if constexpr (has_avx512vl<ISA>) {
				return as<V>(_mm256_permutexvar_epi64(as<__m256i>(idx), as<__m256i>(x)));
			}
			else if constexpr (has_avx2<ISA>) {
				const auto dupe_mask = _mm256_setr_epi32(0, 0, 2, 2, 4, 4, 6, 6);
				const auto add_seq = _mm256_setr_epi32(0, 1, 0, 1, 0, 1, 0, 1);

				const auto byte_offsets = _mm256_slli_epi64(as<__m256i>(idx), 1);
				const auto duplicated = _mm256_permutevar8x32_epi32(byte_offsets, dupe_mask);
				const auto vpermd_indices = _mm256_add_epi32(duplicated, add_seq);

				return as<V>(_mm256_permutevar8x32_epi32(as<__m256i>(x), vpermd_indices));
			}
			else return shuffle_fallback_<ISA, T>(x, idx);
		}
		else if constexpr (sizeof(T) == 4 && has_avx2<ISA>) {
			return as<V>(_mm256_permutevar8x32_epi32(as<__m256i>(x), as<__m256i>(idx)));
		}
		else if constexpr (sizeof(T) == 2) {
			if constexpr (has_avx512vl<ISA> && has_avx512bw<ISA>) {
				return as<V>(_mm256_permutexvar_epi16(as<__m256i>(idx), as<__m256i>(x)));
			}
			else if constexpr (has_avx2<ISA>) {
				const auto byte_shuffle_mask = byte_shuffle_mask_avx2<ISA>(idx);

				// If the index in __byte_shuffle_mask is > 15, then it is located at the top of the ymm register 
				const auto is_upper_half = _mm256_cmpgt_epi8(byte_shuffle_mask, _mm256_set1_epi8(15));

				// __upper: {__x[2], __x[3], __x[2], __x[3]}
				const auto upper = _mm256_shuffle_epi8(_mm256_permute4x64_epi64(as<__m256i>(x), 0xEE), byte_shuffle_mask);
				// __lower: {__x[0], __x[1], __x[0], __x[1]}
				const auto lower = _mm256_shuffle_epi8(_mm256_inserti128_si256(as<__m256i>(x), as<__m128i>(x), 1), byte_shuffle_mask);

				return as<V>(select_<ISA, T>(upper, lower, is_upper_half));
			}
			else return shuffle_fallback_<ISA, T>(x, idx);
		}
		else if constexpr (sizeof(T) == 1) {
			if constexpr (has_avx512vl<ISA> && has_avx512vbmi<ISA>) {
				return as<V>(_mm256_permutexvar_epi8(as<__m256i>(idx), as<__m256i>(x)));
			}
			else if constexpr (has_avx2<ISA>) {
				const auto is_upper_half = _mm256_cmpgt_epi8(as<__m256i>(idx), _mm256_set1_epi8(15));
				
				const auto upper = _mm256_shuffle_epi8(_mm256_permute4x64_epi64(as<__m256i>(x), 0xEE), as<__m256i>(idx));
				const auto lower = _mm256_shuffle_epi8(_mm256_inserti128_si256(as<__m256i>(x), as<__m128i>(x), 1), as<__m256i>(idx));

				return as<V>(select_<ISA, T>(upper, lower, is_upper_half));
			}
			else return shuffle_fallback_<ISA, T>(x, idx);
		}
		else return shuffle_fallback_<ISA, T>(x, idx);
	}
	else if constexpr (sizeof(V) == 64) {
		if constexpr (sizeof(T) == 8) return as<V>(_mm512_permutexvar_epi64(as<__m512i>(idx), as<__m512i>(x)));
		else if constexpr (sizeof(T) == 4) return as<V>(_mm512_permutexvar_epi32(as<__m512i>(idx), as<__m512i>(x)));
		else if constexpr (sizeof(T) == 2) return shuffle_i16x32_avx512<ISA>(x, idx);
		else if constexpr (sizeof(T) == 1) return shuffle_i8x64_avx512<ISA>(x, idx);
	}
	else return shuffle_fallback_<ISA, T>(x, idx);
}

template <class Return>
concept is_fallback = requires { { Return::fallback } -> std::convertible_to<bool>; } && Return::fallback;

template <class MaybeWrapped>
raze_always_inline auto unwrap_fallback(const MaybeWrapped& x) noexcept {
	if constexpr (is_fallback<MaybeWrapped>) return x._data;
	else return x;
}

template <class Pattern>
raze_always_inline auto generic_shuffle_native_size_(const pattern_vector_t<Pattern>& x, Pattern p) noexcept {
	using V = pattern_vector_t<Pattern>;
	V result = x;

	auto& storage = result.template __get<0>();
	
	using Ret = decltype(generic_shuffle_native_<abi_t<V>::isa, typename V::value_type>(ustorage(storage), p));

	if constexpr (is_fallback<Ret>) {
#if defined(raze_cpp_clang) || defined(raze_cpp_gnu)
		storage = Pattern::builtin_shufflevector(ustorage(storage));
#else
		storage = generic_shuffle_native_<abi_t<V>::isa, typename V::value_type>(ustorage(storage), p)._data;
		return fallback_result{ result };
#endif
	}
	else {
		storage = generic_shuffle_native_<abi_t<V>::isa, typename V::value_type>(ustorage(storage), p);
		return result;
	}
}

template <simd_type V, index_simd_type Index>
raze_always_inline auto generic_shuffle_native_size_(const V& x, const Index& idx) noexcept {
	V result = x;

	auto& storage = result.template __get<0>();
	auto idx_native = ustorage(idx.template __get<0>());
	
	using Ret = decltype(generic_shuffle_native_<abi_t<V>::isa, typename V::value_type>(ustorage(storage), idx_native));

	if constexpr (is_fallback<Ret>) {
		storage = generic_shuffle_native_<abi_t<V>::isa, typename V::value_type>(ustorage(storage), idx_native)._data;
		return fallback_result{ result };
	}
	else {
		storage = generic_shuffle_native_<abi_t<V>::isa, typename V::value_type>(ustorage(storage), idx_native);
		return result;
	}
}

template <class Pattern>
raze_always_inline auto generic_shuffle_(const pattern_vector_t<Pattern>& x, Pattern p) noexcept {
	using V = pattern_vector_t<Pattern>;

	if constexpr (is_identity(p)) return x;
	else if constexpr (native<V>) return generic_shuffle_native_size_(x, p);
	else if constexpr (!has_any_scalar_chunks<V> && V::__chunks_count() == 2 && trivially_chunk_swappable<V>) {
#if defined (raze_cpp_clang) || defined(raze_cpp_gnu)
		auto& storage1 = x.template __get<0>();
		auto& storage2 = x.template __get<1>();

		auto vec = Pattern::builtin_shufflevector(ustorage(storage1), ustorage(storage2));
		alignas(sizeof(V)) typename V::value_type arr[V::size()];
		using VecType = decltype(vec);
		*reinterpret_cast<VecType*>(arr) = vec;
		return raze::vx::load<V>[raze::vx::aligned](arr);
#else
		V dup_1;
		dup_1.template __get<0>() = x.template __get<0>();
		dup_1.template __get<1>() = x.template __get<0>();

		V dup_2;
		dup_2.template __get<0>() = x.template __get<1>();
		dup_2.template __get<1>() = x.template __get<1>();

		static constexpr auto new_p = p.split();

		[&] <sizetype... _Indices_> (std::integer_sequence<sizetype, _Indices_...>) raze_always_inline_lambda {
			([&](auto i) raze_always_inline_lambda {
				auto& c1 = dup_1.template __get<i>();
				auto& c2 = dup_2.template __get<i>();

				auto c1_in = ustorage(c1);
				auto c2_in = ustorage(c2);

				using Chunk1 = std::remove_cvref_t<decltype(c1)>;
				using Chunk2 = std::remove_cvref_t<decltype(c2)>;

				const auto r1 = generic_shuffle_native_<abi_t<V>::isa, typename V::value_type>(c1_in,
					make_shuffle_pattern<typename Chunk1::as_simd, [&](auto k) { return new_p.first[k] % (V::size() / 2); } > {});

				const auto r2 = generic_shuffle_native_<abi_t<V>::isa, typename V::value_type>(c2_in,
					make_shuffle_pattern<typename Chunk2::as_simd, [&](auto k) { return new_p.second[k] % (V::size() / 2); } > {});

				c1 = unwrap_fallback(r1);
				c2 = unwrap_fallback(r2);
				}(std::integral_constant<sizetype, Indices>{}), ...);
		}(std::make_integer_sequence<sizetype, V::__chunks_count()>{});

		const auto mask = p.to_mask([](auto idx) { return idx >= (V::size() / 2); });
		return vx::select[dup_1, mask](dup_2);
#endif
	}
	else return generic_shuffle_scalar_fallback_(x, p);
}

template <simd_type V, index_simd_type Index>
raze_always_inline auto generic_shuffle_(const V& x, const Index& idx) noexcept
	requires (index_type_for<Index, V>)
{
	if constexpr (native<V>) return __generic_shuffle_native_size(x, idx);
	else return generic_shuffle_scalar_fallback_(x, idx);
}

__RAZE_VX_NAMESPACE_END 
