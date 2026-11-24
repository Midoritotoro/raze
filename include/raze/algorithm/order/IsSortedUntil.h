#pragma once

#include <src/raze/algorithm/RangesSize.h>
#include <src/raze/algorithm/VectorizablePredicate.h>
#include <src/raze/algorithm/UncheckedAlgorithms.h>
#include <src/raze/algorithm/DataSource.h>
#include <algorithm>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <class Traits>
struct is_sorted_until_t : Traits, dispatchable<is_sorted_until_t<Traits>> {
	template <source Source, class Comp, class Projection>
	struct kernel {
		using source_type = std::remove_cvref_t<Source>;

		using iterator_type = typename source_type::iterator_type;
		using unchecked_iterator_type = typename source_type::unchecked_iterator_type;
		using unchecked_sentinel_type = typename source_type::unchecked_sentinel_type;

		using vector_value_type = std::iter_value_t<unchecked_iterator_type>;

		static consteval bool vectorizable() noexcept {
			return contiguous_source<Source> &&
				!traits::is_iterator_volatile_v<iterator_type> &&
				vectorizable_binary_predicate<Comp, iterator_type> &&
				vectorizable_projection<Projection, iterator_type>;
		}

		source_type _source;
		raze_no_unique_address Comp _comp;
		raze_no_unique_address Projection _proj;
		unchecked_iterator_type _in_it;
		unchecked_sentinel_type _in_sent;
		bool _sorted = true;

		constexpr explicit kernel(Source&& source, Comp comp, Projection proj)
			: _source(std::forward<Source>(source)), _comp(comp), _proj(proj),
			  _in_it(_source.ubegin()), _in_sent(_source.uend())
		{}

		raze_always_inline constexpr void operator()() {
			if (!_sorted || _in_it == _in_sent) return;

			auto prev = _in_it;
			raze_disable_unrolling
			for (++_in_it; _in_it != _in_sent; ++_in_it, ++prev) {
				if (std::invoke(_comp, std::invoke(_proj, *_in_it), std::invoke(_proj, *prev))) {
					_sorted = false;
					return;
				}
			}
		}

		template <vectorizable_tag Tag>
		raze_always_inline void operator()(Tag, sizetype aligned_size) {
			if (!_sorted || aligned_size < sizeof(Tag) * 2) return;

			auto* ptr = std::to_address(_in_it);
			const auto aligned_end = bytes_pointer_offset(ptr, aligned_size);

			raze_disable_unrolling do {
				const auto current = _proj(vx::load<Tag>(ptr));
				const auto next = _proj(vx::load<Tag>(ptr + 1));
				const auto mask = _comp(next, current);

				if (vx::any_of(mask)) {
					_sorted = false;
					source_type::from_ptr(_in_it, ptr + 1 + vx::find_first_set[vx::not_null](mask));
					return;
				}

				advance_bytes(ptr, sizeof(Tag));
			} while (ptr + Tag::size() != aligned_end);

			source_type::from_ptr(_in_it, ptr);
		}

		constexpr raze_always_inline iterator_type result() const {
			return _source.wrap(_in_it);
		}

		raze_nodiscard static constexpr raze_always_inline decltype(auto) static_size()
			requires(constexpr_sized_source<Source>)
		{
			return Source::static_size();
		}

		raze_nodiscard constexpr raze_always_inline auto size() const {
			return _source.size();
		}
	};

	template <std::input_iterator It, std::sentinel_for<It> Sent,
		class Comp = std::less<>, class Proj = std::identity>
	constexpr raze_always_inline It operator()(It first, Sent last, Comp comp = {}, Proj proj = {}) const
		requires(std::indirect_strict_weak_order<Comp, std::projected<It, Proj>>)
	{
		return this->dispatch(get_source(std::move(first), std::move(last)), traits::fwd_fn(comp), traits::fwd_fn(proj));
	}

	template <std::ranges::input_range R, class Comp = std::less<>, class Proj = std::identity>
	constexpr raze_always_inline std::ranges::borrowed_iterator_t<R> operator()(R&& r, Comp comp = {}, Proj proj = {}) const
		requires(std::indirect_strict_weak_order<Comp, std::projected<std::ranges::iterator_t<R>, Proj>>)
	{
		return this->dispatch(get_source(std::forward<R>(r)), traits::fwd_fn(comp), traits::fwd_fn(proj));
	}
};

constexpr inline auto is_sorted_until = options::function_with_traits<is_sorted_until_t>;

__RAZE_ALGORITHM_NAMESPACE_END