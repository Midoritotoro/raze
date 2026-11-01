#pragma once 

#include <src/raze/vx/hw/common/ShufflePattern.h>
#include <array>

__RAZE_VX_NAMESPACE_BEGIN

/**
 * @ingroup raze_vx_shuffle_patterns
 * @brief A special index value indicating that a target lane should be zero-filled.
 *
 * Used inside `shuffle_pattern` and slide operations. If a lane in a pattern is set to this value,
 * `raze::vx::shuffle` will write `0` into that lane instead of reading from the source vector.
*/
inline constexpr sizetype shuffle_zero = std::numeric_limits<sizetype>::max();


/**
 * @ingroup raze_vx_shuffle_patterns
 * @brief Checks if the upper half of a shuffle pattern is a shifted copy of the lower half.
 * @param p The shuffle pattern to evaluate.
 * @return `true` if `p[i + N/2] == p[i] + N/2` for all `i` in the lower half.
*/
template <class Pattern>
consteval raze_always_inline  bool is_halfs_equal(Pattern p) noexcept {
	constexpr auto h = Pattern::size() / 2;

	for (auto i = 0; i < h; ++i)
		if (p[i] != (p[i + h] - h))
			return false;

	return true;
}

/**
 * @ingroup raze_vx_shuffle_patterns
 * @brief Checks if the pattern perfectly reverses the vector elements.
 * @param p The shuffle pattern to evaluate.
 * @return `true` if `p[i] == N - 1 - i`.
*/
template <class Pattern>
consteval raze_always_inline bool is_reverse(Pattern p) noexcept {
	for (auto i = 0; i < Pattern::size(); ++i)
		if (p[i] != (Pattern::size() - i - 1))
			return false;

	return true;
}

/**
 * @ingroup raze_vx_shuffle_patterns
 * @brief Checks if the pattern represents an identity permutation (no changes).
 * @param p The shuffle pattern to evaluate.
 * @return `true` if `p[i] == i`.
*/
template <class Pattern>
consteval raze_always_inline bool is_identity(Pattern p) noexcept {
	for (auto i = 0; i < Pattern::size(); ++i)
		if (p[i] != i)
			return false;

	return true;
}

/**
 * @ingroup raze_vx_shuffle_patterns
 * @brief Checks if the pattern broadcasts a single element to all lanes.
 * @param p The shuffle pattern to evaluate.
 * @return `true` if all indices in the pattern are equal (`p[0] == p[i]`).
*/
template <class Pattern>
consteval raze_always_inline bool is_splat(Pattern p) noexcept {
	const auto v = p[0];

	for (auto i = 1; i < Pattern::size(); ++i)
		if (p[i] != v)
			return false;

	return true;
}

/**
 * @ingroup raze_vx_shuffle_patterns
 * @brief Checks if the pattern represents a left circular rotation.
 * @param p The shuffle pattern to evaluate.
 * @return `true` if `p[i] == (i + shift) % N`.
*/
template <class Pattern>
consteval raze_always_inline bool is_rotate_left(Pattern p) noexcept {
	constexpr auto n = Pattern::size();

	const auto shift = p[0];

	for (auto i = 0; i < n; ++i)
		if (p[i] != ((i + shift) % n))
			return false;

	return true;
}

/**
 * @ingroup raze_vx_shuffle_patterns
 * @brief Checks if the pattern represents a right circular rotation.
 * @param p The shuffle pattern to evaluate.
 * @return `true` if `p[i] == (i + N - shift) % N`.
*/
template <class Pattern>
consteval raze_always_inline bool is_rotate_right(Pattern p) noexcept {
	constexpr auto n = Pattern::size();
	const auto shift = (n - p[0]) % n;

	for (auto i = 0; i < n; ++i)
		if (p[i] != ((i + n - shift) % n))
			return false;

	return true;
}

/**
 * @ingroup raze_vx_shuffle_patterns
 * @brief Checks if the pattern represents a left slide (with zero-fill).
 * @param p The shuffle pattern to evaluate.
 * @return `true` if the pattern shifts elements left by a constant amount and fills the rightmost lanes with `shuffle_zero`.
*/
template <class Pattern>
consteval raze_always_inline bool is_slide_left(Pattern p) noexcept {
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

/**
 * @ingroup raze_vx_shuffle_patterns
 * @brief Checks if the pattern represents a right slide (with zero-fill).
 * @param p The shuffle pattern to evaluate.
 * @return `true` if the pattern shifts elements right by a constant amount and fills the leftmost lanes with `shuffle_zero`.
*/
template <class Pattern>
consteval raze_always_inline bool is_slide_right(Pattern p) noexcept {
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

/**
 * @ingroup raze_vx_shuffle_patterns
 * @brief Checks if the pattern interleaves the lower half of the vector.
 * @param p The shuffle pattern to evaluate.
 * @return `true` if the pattern alternates elements from the first and second quarter of the vector.
*/
template <class Pattern>
consteval raze_always_inline bool is_interleave_low(Pattern p) noexcept {
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

/**
 * @ingroup raze_vx_shuffle_patterns
 * @brief Checks if the pattern interleaves the upper half of the vector.
 * @param p The shuffle pattern to evaluate.
 * @return `true` if the pattern alternates elements from the third and fourth quarter of the vector.
*/
template <class Pattern>
consteval raze_always_inline bool is_interleave_high(Pattern p) noexcept {
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
consteval raze_always_inline auto across_halfs(Pattern p) noexcept
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
consteval raze_always_inline auto across_quads(Pattern p) noexcept
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
consteval raze_always_inline bool can_widen_shuffle(Pattern p) noexcept
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

/**
 * @ingroup raze_vx_shuffle_patterns
 * @brief Checks if the pattern only references indices from the lower half of the vector.
 * @param p The shuffle pattern to evaluate.
 * @return `true` if all indices are `< N / 2`.
*/
template <class Pattern>
consteval raze_always_inline bool is_low_half(Pattern p) noexcept {
	for (auto i = 0; i < p.size(); ++i)
		if (p[i] >= (p.size() / 2))
			return false;

	return true;
}

/**
 * @ingroup raze_vx_shuffle_patterns
 * @brief Checks if the pattern only references indices from the upper half of the vector.
 * @param p The shuffle pattern to evaluate.
 * @return `true` if all indices are `>= N / 2`.
*/
template <class Pattern>
consteval raze_always_inline bool is_high_half(Pattern p) noexcept {
	for (auto i = 0; i < p.size(); ++i)
		if (p[i] < (p.size() / 2))
			return false;

	return true;
}

/**
 * @ingroup raze_vx_shuffle_patterns
 * @brief Checks if the pattern duplicates the lower half of the vector into both halves.
 * @param p The shuffle pattern to evaluate.
 * @return `true` if `p[i] == p[i + N/2]` and `p[i] < N/2`.
*/
template <class Pattern>
consteval raze_always_inline bool is_dup_low(Pattern p) noexcept {
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

/**
 * @ingroup raze_vx_shuffle_patterns
 * @brief Checks if the pattern duplicates the upper half of the vector into both halves.
 * @param p The shuffle pattern to evaluate.
 * @return `true` if `p[i] == p[i + N/2]` and `p[i] >= N/2`.
*/
template <class Pattern>
consteval raze_always_inline bool is_dup_high(Pattern p) noexcept {
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

/**
 * @ingroup raze_vx_shuffle_patterns
 * @brief Checks if the pattern is an identity over the lower half, duplicated into the upper half.
 * @param p The shuffle pattern to evaluate.
 * @return `true` if `p[i] == i` for the lower half, and `p[i + N/2] == p[i]`.
*/
template <class Pattern>
consteval raze_always_inline bool is_dup_low_identity(Pattern p) noexcept {
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

/**
 * @ingroup raze_vx_shuffle_patterns
 * @brief Checks if the pattern is an identity over the upper half, duplicated into the lower half.
 * @param p The shuffle pattern to evaluate.
 * @return `true` if `p[i] == i + N/2` for the lower half, and `p[i + N/2] == p[i]`.
*/
template <class Pattern>
consteval raze_always_inline bool is_dup_high_identity(Pattern p) noexcept {
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

/**
 * @ingroup raze_vx_shuffle_patterns
 * @brief Checks if the pattern interleaves the two halves of the vector.
 * @param p The shuffle pattern to evaluate.
 * @return `true` if `p[2i] == i` and `p[2i+1] == i + N/2`.
*/
template <class Pattern>
consteval raze_always_inline bool is_zip(Pattern p) noexcept {
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

/**
 * @ingroup raze_vx_shuffle_patterns
 * @brief Checks if the pattern de-interleaves the vector into two consecutive halves.
 * @param p The shuffle pattern to evaluate.
 * @return `true` if `p[i] == 2i` and `p[i + N/2] == 2i + 1`.
*/
template <class Pattern>
consteval raze_always_inline bool is_unzip(Pattern p) noexcept {
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

/**
 * @ingroup raze_vx_shuffle_patterns
 * @brief Extracts the shift amount from a left-rotation shuffle pattern.
 * @param p A pattern verified by `is_rotate_left`.
 * @return The shift amount.
*/
template <class Pattern>
consteval raze_always_inline auto get_rotate_left_shift(Pattern p) noexcept
	requires (is_rotate_left(Pattern{}))
{
	return p[0];
}

/**
 * @ingroup raze_vx_shuffle_patterns
 * @brief Extracts the shift amount from a right-rotation shuffle pattern.
 * @param p A pattern verified by `is_rotate_right`.
 * @return The shift amount.
*/
template <class Pattern>
consteval raze_always_inline auto get_rotate_right_shift(Pattern p) noexcept
	requires (is_rotate_right(Pattern{}))
{
	constexpr auto n = Pattern::size();
	return (n - p[0]) % n;
}

/**
 * @ingroup raze_vx_shuffle_patterns
 * @brief Extracts the shift amount from a left-slide shuffle pattern.
 * @param p A pattern verified by `is_slide_left`.
 * @return The shift amount.
*/
template <class Pattern>
consteval raze_always_inline auto get_slide_left_shift(Pattern p) noexcept
	requires (is_slide_left(Pattern{}))
{
	for (sizetype i = 0; i < Pattern::size(); ++i)
		if (p[i] != shuffle_zero)
			return p[i] - i;

	return Pattern::size();
}

/**
 * @ingroup raze_vx_shuffle_patterns
 * @brief Extracts the shift amount from a right-slide shuffle pattern.
 * @param p A pattern verified by `is_slide_right`.
 * @return The shift amount.
*/
template <class Pattern>
consteval raze_always_inline auto get_slide_right_shift(Pattern p) noexcept
	requires (is_slide_right(Pattern{}))
{
	for (sizetype i = 0; i < Pattern::size(); ++i)
		if (p[i] != shuffle_zero)
			return i - p[i];

	return Pattern::size();
}

template <class Pattern>
consteval raze_always_inline u8 to_pshufd_mask(Pattern p) noexcept {
	return ((p[0] & 0x03) | ((p[1] & 0x03) << 2) | ((p[2] & 0x03) << 4) | ((p[3] & 0x03) << 6));
}

template <class Pattern>
consteval raze_always_inline u8 shufpd_to_pshufd_mask(Pattern p) noexcept {
	return (((2 * p[0]) & 0x03) | (((2 * p[0] + 1) & 0x03) << 2)
		| (((2 * p[1]) & 0x03) << 4) | (((2 * p[1] + 1) & 0x03) << 6));
}

template <simd_type V, auto F, sizetype ... Idxs>
consteval auto make_shuffle_pattern_impl(std::integer_sequence<sizetype, Idxs...>) noexcept {
	return shuffle_pattern<V, F(Idxs)...>{};
}

/**
 * @ingroup raze_vx_shuffle_patterns
 * @brief Generates a shuffle pattern from a compile-time lambda.
 * @tparam V The target SIMD type.
 * @tparam F The lambda function that takes `sizetype i` and returns the source index.
*/
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

/**
 * @ingroup raze_vx_shuffle_patterns
 * @brief Creates a shuffle pattern directly from a sequence of indices.
 * @tparam V The target SIMD type.
 * @tparam Idxs The index sequence.
*/
template <simd_type V, sizetype ... Idxs>
using make_pattern = shuffle_pattern<V, Idxs...>;

/**
 * @ingroup raze_vx_shuffle_patterns
 * @brief Generates a pattern that reverses the vector.
 * @tparam V The target SIMD type.
*/
template <simd_type V>
using make_reversed_pattern = make_shuffle_pattern<V,
	[] (sizetype i) { return V::size() - 1 - i; }>;

/**
 * @ingroup raze_vx_shuffle_patterns
 * @brief Generates an identity pattern (no-op shuffle).
 * @tparam V The target SIMD type.
*/
template <simd_type V>
using make_identity_pattern = make_shuffle_pattern<V,
	[] (sizetype i) { return i; }>;

/**
 * @ingroup raze_vx_shuffle_patterns
 * @brief Generates a pattern for left circular rotation.
 * @tparam V The target SIMD type.
 * @tparam Shift The number of elements to rotate.
*/
template <simd_type V, sizetype Shift>
using make_rotate_left_pattern = make_shuffle_pattern<V, 
	[] (sizetype i) { return (i + Shift) % V::size(); }>;

/**
 * @ingroup raze_vx_shuffle_patterns
 * @brief Generates a pattern for right circular rotation.
 * @tparam V The target SIMD type.
 * @tparam Shift The number of elements to rotate.
*/
template <simd_type V, sizetype Shift>
using make_rotate_right_pattern = make_shuffle_pattern<V, 
	[] (sizetype i) { return (i + V::size() - (Shift % V::size())) % V::size(); }>;

/**
 * @ingroup raze_vx_shuffle_patterns
 * @brief Generates a pattern that broadcasts a single element to all lanes.
 * @tparam V The target SIMD type.
 * @tparam Idx The index of the element to broadcast.
*/
template <simd_type V, sizetype Idx>
using make_splat_pattern = make_shuffle_pattern<V, [] (sizetype) { return Idx; }>;

/**
 * @ingroup raze_vx_shuffle_patterns
 * @brief Generates a pattern for left sliding with zero fill.
 * @tparam V The target SIMD type.
 * @tparam Shift The number of elements to slide.
*/
template <simd_type V, sizetype Shift>
using make_slide_left_pattern = make_shuffle_pattern<V,
	[] (sizetype i) {
		const auto src = i + Shift;

		if (src >= V::size())
			return shuffle_zero;

		return src;
	}>;

/**
 * @ingroup raze_vx_shuffle_patterns
 * @brief Generates a pattern for right sliding with zero fill.
 * @tparam V The target SIMD type.
 * @tparam Shift The number of elements to slide.
*/
template <simd_type V, sizetype Shift>
using make_slide_right_pattern = make_shuffle_pattern<V,
	[] (sizetype i) {
		if (i < Shift)
			return shuffle_zero;

		return i - Shift;
	}>;

/**
 * @ingroup raze_vx_shuffle_patterns
 * @brief Generates an interleaving pattern (zip).
 * @tparam V The target SIMD type.
*/
template <simd_type V>
using make_zip_pattern = make_shuffle_pattern<V,
	[] (sizetype i) {
		if ((i & 1) == 0)
			return i / 2;

		return V::size() / 2 + i / 2;
	}>;

/**
 * @ingroup raze_vx_shuffle_patterns
 * @brief Generates a de-interleaving pattern (unzip).
 * @tparam V The target SIMD type.
*/
template <simd_type V>
using make_unzip_pattern = make_shuffle_pattern<V,
	[] (sizetype i) {
		if (i < V::size() / 2)
			return 2 * i;

		return 2 * (i - V::size() / 2) + 1;
	}>;

/**
 * @ingroup raze_vx_shuffle_patterns
 * @brief Generates a pattern that swaps adjacent blocks of elements.
 * @tparam V The target SIMD type.
 * @tparam GroupSize The number of elements in each block to swap.
*/
template <simd_type V, sizetype GroupSize>
using make_swap_adjacent_pattern = make_shuffle_pattern<V,
	[] (sizetype i) {
		constexpr auto block = 2 * GroupSize;

		const auto block_begin = (i / block) * block;
		const auto offset = i % block;

		auto index = 0;

		if (offset < GroupSize) index = block_begin + offset + GroupSize;
		else index = block_begin + offset - GroupSize;

		return index % V::size();
	}>;

__RAZE_VX_NAMESPACE_END
