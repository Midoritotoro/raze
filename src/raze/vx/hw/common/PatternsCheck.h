#pragma once 

#include <src/raze/vx/hw/common/ShufflePattern.h>
#include <array>

__RAZE_VX_NAMESPACE_BEGIN

inline constexpr sizetype shuffle_zero = std::numeric_limits<sizetype>::max();

template <class Pattern>
consteval bool is_halfs_equal(P p) noexcept {
	constexpr auto h = Pattern::size() / 2;

	for (auto i = 0; i < h; ++i)
		if (p[i] != (p[i + h] - h))
			return false;

	return true;
}

template <class Pattern>
consteval bool is_reverse(Pattern p) noexcept {
	for (auto i = 0; i < Pattern::size(); ++i)
		if (p[i] != (Pattern::size() - i - 1))
			return false;

	return true;
}

template <class Pattern>
consteval bool is_identity(Pattern p) noexcept {
	for (auto i = 0; i < Pattern::size(); ++i)
		if (p[i] != i)
			return false;

	return true;
}

template <class Pattern>
consteval bool is_splat(Pattern p) noexcept {
	const auto v = p[0];

	for (auto i = 1; i < Pattern::size(); ++i)
		if (p[i] != v)
			return false;

	return true;
}

template <class Pattern>
consteval bool is_rotate_left(Pattern p) noexcept {
	constexpr auto n = Pattern::size();

	const auto shift = p[0];

	for (auto i = 0; i < n; ++i)
		if (p[i] != ((i + shift) % n))
			return false;

	return true;
}

template <class Pattern>
consteval bool is_rotate_right(Pattern p) noexcept {
	constexpr auto n = Pattern::size();
	const auto shift = (n - p[0]) % n;

	for (auto i = 0; i < n; ++i)
		if (p[i] != ((i + n - shift) % n))
			return false;

	return true;
}

template <class Pattern>
consteval bool is_slide_left(Pattern p) noexcept {
	constexpr auto n = Pattern::size();

	bool found_shift = false;
	sizetype shift = 0;

	for (sizetype i = 0; i < n; ++i) {
		const auto v = p[i];

		if (v == shuffle_zero)
			continue;

		if (!found_shift) {
			shift = v - i;
			found_shift = true;
		}

		if (v != i + shift)
			return false;

		if (v >= n)
			return false;
	}

	return true;
}

template <class Pattern>
consteval bool is_slide_right(Pattern p) noexcept {
	constexpr auto n = Pattern::size();

	bool found_shift = false;
	sizetype shift = 0;

	for (sizetype i = 0; i < n; ++i) {
		const auto v = p[i];

		if (v == shuffle_zero || v < 0)
			continue;

		if (!found_shift) {
			shift = i - v;
			found_shift = true;
		}
		else {
			if (i - v != shift)
				return false;
		}

		if (v != i - shift)
			return false;

		if (v >= n)
			return false;
	}

	return true;
}

template <class Pattern>
consteval bool is_interleave_low(Pattern p) noexcept {
	constexpr auto n = Pattern::size();

	if ((n & 1) != 0)
		return false;

	constexpr auto h = n / 2;

	for (auto i = 0; i < h; ++i) {
		if (p[2 * i] != i)
			return false;

		if (p[2 * i + 1] != (h + i))
			return false;
	}

	return true;
}

template <class Pattern>
consteval bool is_interleave_high(Pattern p) noexcept {
	constexpr auto n = Pattern::size();

	if ((n & 1) != 0)
		return false;

	constexpr auto h = n / 2;

	for (auto i = 0; i < h; ++i) {
		if (p[2 * i] != (h + i))
			return false;

		if (p[2 * i + 1] != i)
			return false;
	}

	return true;
}

template <class Pattern>
consteval auto across_halfs(Pattern p) noexcept 
	requires (pattern_vector_t<Pattern>::is_native())
{
	constexpr auto size = sizeof(typename Pattern::vector_type);

	for (auto i = 0; i < Pattern::size() / 2; ++i)
		if (p[i] >= (Pattern::size() / 2))
			return true;

	for (auto i = Pattern::size() / 2; i < Pattern::size(); ++i)
		if (p[i] < (Pattern::size() / 2))
			return true;

	return false;
}

template <class Pattern>
consteval auto across_quads(Pattern p) noexcept
	requires (pattern_vector_t<Pattern>::is_native())
{
	constexpr auto q = Pattern::size() / 4;

	for (auto i = 0; i < q; ++i)
		if (p[i] >= q)
			return true;

	for (auto i = q; i < 2 * q; ++i)
		if (p[i] < q || p[i] >= 2 * q)
			return true;

	for (auto i = 2 * q; i < 3 * q; ++i)
		if (p[i] < 2 * q || p[i] >= 3 * q)
			return true;

	for (auto i = 3 * q; i < 4 * q; ++i)
		if (p[i] < 3 * q)
			return true;

	return false;
}

template <class Pattern>
consteval bool can_widen_shuffle(Pattern p) noexcept
	requires((Pattern::size() & 1) == 0)
{
	for (auto i = 0; i < Pattern::size(); i += 2) {
		const auto a = p[i];
		const auto b = p[i + 1];

		if ((a & 1) != 0)
			return false;

		if (b != a + 1)
			return false;
	}

	return true;
}

template <class Pattern>
consteval bool is_low_half(Pattern p) noexcept {
	for (auto i = 0; i < p.size(); ++i)
		if (p[i] >= (p.size() / 2))
			return false;

	return true;
}

template <class Pattern>
consteval bool is_high_half(Pattern p) noexcept {
	for (auto i = 0; i < p.size(); ++i)
		if (p[i] < (p.size() / 2))
			return false;

	return true;
}

template <class Pattern>
consteval bool is_dup_low(Pattern p) noexcept {
	constexpr auto n = Pattern::size();

	if ((n & 1) != 0)
		return false;

	constexpr auto h = n / 2;

	for (auto i = 0; i < h; ++i) {
		if (p[i] >= h)
			return false;

		if (p[i + h] != p[i])
			return false;
	}

	return true;
}

template <class Pattern>
consteval bool is_dup_high(Pattern p) noexcept {
	constexpr auto n = Pattern::size();

	if ((n & 1) != 0)
		return false;

	constexpr auto h = n / 2;

	for (auto i = 0; i < h; ++i) {
		if (p[i] < h || p[i] >= n)
			return false;

		if (p[i + h] != p[i])
			return false;
	}

	return true;
}

template <class Pattern>
consteval bool is_dup_low_identity(Pattern p) noexcept {
	constexpr auto n = Pattern::size();

	if ((n & 1) != 0)
		return false;

	constexpr auto h = n / 2;

	for (auto i = 0; i < h; ++i) {
		if (p[i] >= h || p[i] != i) return false;
		if (p[i + h] != p[i]) return false;
	}

	return true;
}

template <class Pattern>
consteval bool is_dup_high_identity(Pattern p) noexcept {
	constexpr auto n = Pattern::size();

	if ((n & 1) != 0)
		return false;

	constexpr auto h = n / 2;

	for (auto i = 0; i < h; ++i) {
		if (p[i] < h || p[i] >= n || p[i] != i) return false;
		if (p[i + h] != p[i]) return false;
	}

	return true;
}

template <class Pattern>
consteval bool is_zip(Pattern p) noexcept {
	constexpr auto n = Pattern::size();

	if ((n & 1) != 0)
		return false;

	constexpr auto h = n / 2;

	for (sizetype i = 0; i < h; ++i) {
		if (p[2 * i] != i)
			return false;

		if (p[2 * i + 1] != (h + i))
			return false;
	}

	return true;
}

template <class Pattern>
consteval bool is_unzip(Pattern p) noexcept {
	constexpr auto n = Pattern::size();

	if ((n & 1) != 0)
		return false;

	constexpr auto h = n / 2;

	for (sizetype i = 0; i < h; ++i) {
		if (p[i] != (2 * i))
			return false;

		if (p[i + h] != (2 * i + 1))
			return false;
	}

	return true;
}

template <class Pattern>
consteval auto get_rotate_left_shift(Pattern p) noexcept
	requires (is_rotate_left(Pattern{}))
{
	return p[0];
}

template <class Pattern>
consteval auto get_rotate_right_shift(Pattern p) noexcept
	requires (is_rotate_right(Pattern{}))
{
	constexpr auto n = Pattern::size();
	return (n - p[0]) % n;
}

template <class Pattern>
consteval auto get_slide_left_shift(Pattern p) noexcept
	requires (is_slide_left(Pattern{}))
{
	for (sizetype i = 0; i < Pattern::size(); ++i)
		if (p[i] != shuffle_zero)
			return p[i] - i;

	return Pattern::size();
}

template <class Pattern>
consteval auto get_slide_right_shift(Pattern p) noexcept
	requires (is_slide_right(Pattern{}))
{
	for (sizetype i = 0; i < Pattern::size(); ++i)
		if (p[i] != shuffle_zero)
			return i - p[i];

	return Pattern::size();
}

template <class Pattern>
consteval u8 to_pshufd_mask(Pattern p) noexcept {
	return ((p[0] & 0x03) | ((p[1] & 0x03) << 2) | ((p[2] & 0x03) << 4) | ((p[3] & 0x03) << 6));
}

template <class Pattern>
consteval u8 shufpd_to_pshufd_mask(Pattern p) noexcept {
	return (((2 * p[0]) & 0x03) | (((2 * p[0] + 1) & 0x03) << 2)
		| (((2 * p[1]) & 0x03) << 4) | (((2 * p[1] + 1) & 0x03) << 6));
}

template <simd_type V, auto F, sizetype ... Idxs>
consteval auto make_shuffle_pattern_impl(std::integer_sequence<sizetype, Idxs...>) noexcept {
	return shuffle_pattern<V, F(Idxs)...>{};
}

template <simd_type V, auto F>
using make_shuffle_pattern = decltype(make_shuffle_pattern_impl<V, F>(
		std::make_integer_sequence<sizetype, V::size()>{}));

template <simd_type V, auto _Fn_, sizetype Offset, sizetype ... Idxs>
consteval auto make_shuffle_pattern_with_offset_impl(std::integer_sequence<sizetype, Idxs...>) noexcept {
	return shuffle_pattern<V, _Fn_(Idxs + Offset)...>{};
}

template <simd_type V, auto F, sizetype Offset>
using make_shufflePatternwith_offset = decltype(make_shuffle_pattern_with_offset_impl<V, F, Offset>(
	std::make_integer_sequence<sizetype, V::size()>{}));

template <simd_type V, sizetype ... Idxs>
using make_pattern = shuffle_pattern<V, Idxs...>;

template <simd_type V>
using make_reversed_pattern = make_shuffle_pattern<V,
	[] (sizetype i) { return V::size() - 1 - i; }>;

template <simd_type V>
using make_identity_pattern = make_shuffle_pattern<V,
	[] (sizetype i) { return i; }>;

template <simd_type V, sizetype Shift>
using make_rotate_left_pattern = make_shuffle_pattern<V, 
	[] (sizetype i) { return (i + Shift) % V::size(); }>;

template <simd_type V, sizetype Shift>
using make_rotate_right_pattern = make_shuffle_pattern<V, 
	[] (sizetype i) { return (i + V::size() - (Shift % V::size())) % V::size(); }>;

template <simd_type V, sizetype Idx>
using make_splat_pattern = make_shuffle_pattern<V, [] (sizetype) { return Idx; }>;

template <simd_type V, sizetype Shift>
using make_slide_left_pattern = make_shuffle_pattern<V,
	[] (sizetype i) {
		const auto src = i + Shift;

		if (src >= V::size())
			return shuffle_zero;

		return src;
	}>;

template <simd_type V, sizetype Shift>
using make_slide_right_pattern = make_shuffle_pattern<V,
	[] (sizetype i) {
		if (i < Shift)
			return shuffle_zero;

		return i - Shift;
	}>;

template <simd_type V>
using make_zip_pattern = make_shuffle_pattern<V,
	[] (sizetype i) {
		if ((i & 1) == 0)
			return i / 2;

		return V::size() / 2 + i / 2;
	}>;

template <simd_type V>
using make_unzip_pattern = make_shuffle_pattern<V,
	[] (sizetype i) {
		if (i < V::size() / 2)
			return 2 * i;

		return 2 * (i - V::size() / 2) + 1;
	}>;

template <simd_type V, sizetype GroupSize>
using make_swap_adjacent_pattern = make_shuffle_pattern<V,
	[] (sizetype i) {
		constexpr auto block = 2 * GroupSize;

		const auto block_begin = (i / block) * block;
		const auto offset = i % block;

		if (offset < GroupSize) return block_begin + offset + GroupSize;
		else return block_begin + offset - GroupSize;
	}>;

__RAZE_VX_NAMESPACE_END
