#pragma once

#include <src/raze/algorithm/RangesSize.h>
#include <src/raze/algorithm/UncheckedAlgorithms.h>
#include <src/raze/algorithm/DataSource.h>
#include <raze/algorithm/order/Rotate.h>
#include <raze/options/Options.h>
#include <utility>
#include <functional>
#include <iterator>
#include <memory>
#include <new>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

namespace detail {

template <class T>
struct temporary_buffer {
	T* ptr = nullptr;
	std::size_t capacity = 0;
	std::size_t constructed = 0;

	temporary_buffer() noexcept = default;

	explicit temporary_buffer(std::size_t count) noexcept {
		if (count == 0) return;
		if (count > (static_cast<std::size_t>(-1) / sizeof(T))) return;
		if constexpr (alignof(T) > __STDCPP_DEFAULT_NEW_ALIGNMENT__) {
			ptr = static_cast<T*>(::operator new(count * sizeof(T), std::align_val_t{ alignof(T) }, std::nothrow));
		}
		else {
			ptr = static_cast<T*>(::operator new(count * sizeof(T), std::nothrow));
		}
		if (ptr) capacity = count;
	}

	temporary_buffer(const temporary_buffer&) = delete;
	temporary_buffer& operator=(const temporary_buffer&) = delete;

	~temporary_buffer() noexcept {
		if (!ptr) return;
		for (std::size_t i = constructed; i > 0; --i)
			std::destroy_at(ptr + (i - 1));
		if constexpr (alignof(T) > __STDCPP_DEFAULT_NEW_ALIGNMENT__) {
			::operator delete(ptr, std::align_val_t{ alignof(T) });
		}
		else {
			::operator delete(ptr);
		}
	}

	template <class... Args>
	T* emplace_back(Args&&... args) {
		T* p = std::construct_at(ptr + constructed, std::forward<Args>(args)...);
		++constructed;
		return p;
	}
};

template <class Traits, class It, class Diff, class Pred, class Proj>
constexpr It stable_partition_inplace(It first, It last, Diff len, Pred& pred, Proj& proj) {
	if (len == 1)
		return std::invoke(pred, std::invoke(proj, *first)) ? last : first;

	if (len == 2) {
		auto second = std::ranges::next(first);
		if (std::invoke(pred, std::invoke(proj, *first)))
			return std::invoke(pred, std::invoke(proj, *second)) ? last : second;
		if (std::invoke(pred, std::invoke(proj, *second))) {
			std::ranges::iter_swap(first, second);
			return second;
		}
		return first;
	}

	const Diff half = len / 2;
	auto mid = std::ranges::next(first, half);

	auto left_cut = stable_partition_inplace<Traits>(first, mid, half, pred, proj);
	auto right_cut = stable_partition_inplace<Traits>(mid, last, len - half, pred, proj);

	return rotate[options::drop_alignment<Traits>()](left_cut, mid, right_cut).begin();
}

} // namespace detail

template <class Traits>
struct stable_partition_t : Traits, dispatchable<stable_partition_t<Traits>> {
	template <source Source, class Predicate, class Projection>
	struct kernel {
		using source_type = std::remove_cvref_t<Source>;
		using iterator_type = typename source_type::iterator_type;
		using unchecked_iterator_type = typename source_type::unchecked_iterator_type;
		using unchecked_sentinel_type = typename source_type::unchecked_sentinel_type;
		using value_type = std::iter_value_t<unchecked_iterator_type>;

		using vector_value_type = void;

		static consteval bool vectorizable() noexcept {
			return false;
		}

		raze_no_unique_address source_type _source;
		unchecked_iterator_type _first;
		raze_no_unique_address unchecked_sentinel_type _sent;
		unchecked_iterator_type _part_point;
		unchecked_iterator_type _end_it;
		raze_no_unique_address Predicate _pred;
		raze_no_unique_address Projection _proj;

		constexpr explicit kernel(Source&& src, Predicate pred, Projection proj)
			: _source(std::forward<Source>(src)),
			  _first(_source.ubegin()),
			  _sent(_source.uend()),
			  _part_point(_first),
			  _end_it(_first),
			  _pred(std::move(pred)),
			  _proj(std::move(proj))
		{}

		raze_always_inline constexpr void operator()() {
			auto last = std::ranges::next(_first, _sent);
			_end_it = last;

			auto lo = _first;
			while (lo != last && std::invoke(_pred, std::invoke(_proj, *lo)))
				++lo;

			if (lo == last) {
				_part_point = last;
				return;
			}

			auto hi = last;
			do {
				--hi;
				if (hi == lo) {
					_part_point = lo;
					return;
				}
			} while (!std::invoke(_pred, std::invoke(_proj, *hi)));
			++hi;

			const auto len = std::ranges::distance(lo, hi);

			if !consteval {
				if constexpr (std::is_move_constructible_v<value_type> && std::is_move_assignable_v<value_type>) {
					if (len >= 16) {
						detail::temporary_buffer<value_type> buf(static_cast<std::size_t>(len));
						if (buf.ptr != nullptr) {
							auto write = lo;
							buf.emplace_back(std::ranges::iter_move(lo));
							for (auto it = std::ranges::next(lo); it != hi; ++it) {
								if (std::invoke(_pred, std::invoke(_proj, *it))) {
									*write = std::ranges::iter_move(it);
									++write;
								}
								else {
									buf.emplace_back(std::ranges::iter_move(it));
								}
							}

							_part_point = write;
							for (std::size_t i = 0; i < buf.constructed; ++i, ++write)
								*write = std::move(buf.ptr[i]);
							return;
						}
					}
				}
			}

			_part_point = detail::stable_partition_inplace<Traits>(lo, hi, len, _pred, _proj);
		}

		raze_nodiscard constexpr raze_always_inline std::ranges::subrange<iterator_type> result() {
			return { _source.wrap(std::move(_part_point)), _source.wrap(std::move(_end_it)) };
		}
	};

	template <std::bidirectional_iterator It, std::sentinel_for<It> Sent,
		class Predicate, class Proj = std::identity>
	constexpr raze_always_inline std::ranges::subrange<It>
	operator()(It first, Sent last, Predicate pred, Proj proj = {}) const
		requires(std::permutable<It> &&
			std::indirect_unary_predicate<Predicate, std::projected<It, Proj>>)
	{
		return this->dispatch(get_source(std::move(first), std::move(last)),
			traits::fwd_fn(pred), traits::fwd_fn(proj));
	}

	template <std::ranges::bidirectional_range R, class Predicate, class Proj = std::identity>
	constexpr raze_always_inline std::ranges::borrowed_subrange_t<R>
	operator()(R&& r, Predicate pred, Proj proj = {}) const
		requires(std::permutable<std::ranges::iterator_t<R>> &&
			std::indirect_unary_predicate<Predicate,
				std::projected<std::ranges::iterator_t<R>, Proj>>)
	{
		return this->dispatch(get_source(std::forward<R>(r)),
			traits::fwd_fn(pred), traits::fwd_fn(proj));
	}
};

/**
 * @ingroup raze_algo_partition
 * @memberof raze::algorithm
 * @hideinitializer
 * @brief Partitions a range by a predicate while preserving relative element order (C++26 P2562 `constexpr`).
 *
 * 1) Reorders `[first, last)` such that all elements for which `std::invoke(pred, std::invoke(proj, *i))`
 *    is `true` precede all elements for which it is `false`, preserving the relative order within each group.
 *
 * 2) Same as (1), but uses `r` as the range, as if by:
 *    `stable_partition(std::ranges::begin(r), std::ranges::end(r), pred, proj)`.
 *
 * ### Declarations
 * ```cpp
 * // (1) Iterator-sentinel overload
 * template< std::bidirectional_iterator It, std::sentinel_for<It> Sent,
 *           class Predicate, class Proj = std::identity >
 *   requires(std::permutable<It> &&
 *            std::indirect_unary_predicate<Predicate, std::projected<It, Proj>>)
 * constexpr std::ranges::subrange<It>
 * stable_partition( It first, Sent last, Predicate pred, Proj proj = {} );
 *
 * // (2) Range overload
 * template< std::ranges::bidirectional_range R, class Predicate, class Proj = std::identity >
 *   requires(std::permutable<std::ranges::iterator_t<R>> &&
 *            std::indirect_unary_predicate<Predicate, std::projected<std::ranges::iterator_t<R>, Proj>>)
 * constexpr std::ranges::borrowed_subrange_t<R>
 * stable_partition( R&& r, Predicate pred, Proj proj = {} );
 * ```
 *
 * ### Parameters
 * - `first`, `last` - the bidirectional range of elements to partition
 * - `r`             - the bidirectional range of elements to partition
 * - `pred`          - unary predicate returning `true` for elements placed in the first partition
 * - `proj`          - projection applied to elements (defaults to `std::identity`)
 *
 * ### Return value
 * A `std::ranges::subrange` `{pivot, last}` spanning the second group (elements for which `pred` is `false`).
 *
 * ### Complexity
 * Exactly `N` applications of `pred` and `proj` when a temporary buffer is available (`O(N)` moves);
 * falls back to `O(N log N)` swaps in-place during constant evaluation (`if consteval`) or if allocation fails.
 *
 * ### Options
 * Accepts standard modifiers from @ref raze_options (see @ref dynamic_dispatch and @ref vectorization_semantics).
 *
 * ### Notes
 * - Implements C++26 P2562 `constexpr` support in C++23.
 *
 * ### Example
 * ```cpp
 * #include <iostream>
 * #include <vector>
 * #include <raze/algorithm/partition/StablePartition.h>
 *
 * int main() {
 *     std::vector<int> v{1, 2, 3, 4, 5, 6};
 *     raze::algorithm::stable_partition(v, [](int x) { return x % 2 == 0; });
 *     for (int x : v) std::cout << x << ' ';
 *     std::cout << '\n';
 * }
 * ```
 *
 * ### See also
 * - @ref raze_options
 * - @ref dynamic_dispatch
 * - @ref vectorization_semantics
 * - `raze::algorithm::partition`
 * - `raze::algorithm::stable_sort`
 */
constexpr inline auto stable_partition = options::function_with_traits<stable_partition_t>;

__RAZE_ALGORITHM_NAMESPACE_END
