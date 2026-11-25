#pragma once

#include <raze/vx/Algorithm.h>
#include <raze/math/Math.h>
#include <src/raze/algorithm/RangesSize.h>
#include <src/raze/algorithm/UncheckedAlgorithms.h>
#include <src/raze/algorithm/DataSource.h>
#include <src/raze/algorithm/VectorizablePredicate.h>
#include <src/raze/math/IntegralTypesConversions.h>
#include <functional>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <class Traits>
struct min_element_t : Traits, dispatchable<min_element_t<Traits>> {
    template <source Source, class Comp, class Projection>
    struct kernel {
        using source_type = std::remove_cvref_t<Source>;
        using iterator_type = typename source_type::iterator_type;

        using unchecked_iterator_type = typename source_type::unchecked_iterator_type;
        using unchecked_sentinel_type = typename source_type::unchecked_sentinel_type;

        using value_type = std::iter_value_t<unchecked_iterator_type>;
        using vector_value_type = value_type;
        using align = options::alignments<Traits, 1>;

        static consteval bool vectorizable() noexcept {
            return contiguous_source<source_type>
                && !traits::is_iterator_volatile_v<iterator_type>
                && vectorizable_binary_predicate<Comp, unchecked_iterator_type>
                && vectorizable_projection<Projection, unchecked_iterator_type>;
        }

        raze_no_unique_address source_type _source;
        unchecked_iterator_type _iterator;
        raze_no_unique_address unchecked_sentinel_type _sentinel;
        raze_no_unique_address Comp _comp;
        raze_no_unique_address Projection _proj;
        unchecked_iterator_type _best;
        bool _started = false;

        constexpr explicit kernel(Source&& source, Comp comp, Projection proj)
            : _source(std::forward<Source>(source)),
              _iterator(_source.ubegin()),
              _sentinel(_source.uend()),
              _comp(std::move(comp)),
              _proj(std::move(proj)),
              _best(_iterator)
        {}

        constexpr void expects() const {
            align::verify(_source);
        }

        raze_nodiscard constexpr raze_always_inline bool exit() const {
            return _iterator == _sentinel;
        }

        raze_nodiscard constexpr raze_always_inline iterator_type default_result() const {
            return _source.wrap(_iterator);
        }

        raze_always_inline constexpr void operator()() {
            auto it = _iterator;
            if (it == _sentinel)
                return;

            auto best = _best;
            if (!_started) {
                best = it;
                ++it;
                _started = true;
            }

            raze_novec_loop((; it != _sentinel; ++it), {
                if (std::invoke(_comp, std::invoke(_proj, *it), std::invoke(_proj, *best)))
                    best = it;
            })

            _best = std::move(best);
            _iterator = std::move(it);
        }

        template <vectorizable_tag Tag>
        raze_always_inline void operator()(Tag, sizetype aligned_size) {
            using unsigned_t = typename IntegerForSizeof<value_type>::Unsigned;
            using index_simd_t = vx::simd<unsigned_t, vx::abi_t<Tag>>;

            constexpr auto alignment = align::template policy_for<0, Tag>;
            auto* ptr = std::to_address(_iterator);
            auto* best_ptr = std::to_address(_best);
            bool started = _started;

            constexpr bool needs_chunking = sizeof(unsigned_t) < sizeof(sizetype);
            constexpr sizetype max_portion_bytes = needs_chunking
                ? static_cast<sizetype>(math::max_limit<unsigned_t>()) * sizeof(Tag)
                : math::max_limit<sizetype>();

            while (aligned_size > 0) {
                const auto portion_bytes = needs_chunking
                    ? math::min(max_portion_bytes, aligned_size)
                    : aligned_size;
                aligned_size -= portion_bytes;

                auto* const portion_begin = ptr;
                const auto* const stop_at = bytes_pointer_offset(ptr, portion_bytes);

                auto best_vals = vx::load<Tag>[alignment](ptr);
                auto best_idxs = vx::zero<index_simd_t>();
                auto curr_idxs = vx::zero<index_simd_t>();

                advance_bytes(ptr, sizeof(Tag));

                raze_disable_unrolling
                while (ptr != stop_at) {
                    ++curr_idxs;
                    const auto curr_vals = vx::load<Tag>[alignment](ptr);
                    const auto better = _comp(curr_vals, best_vals);

                    best_idxs = vx::select(vx::convert<unsigned_t>(better), curr_idxs, best_idxs);

                    if constexpr (is_less_comparator<Comp, value_type> && has_native_vertical_minmax<Tag>())
                        best_vals = vx::vertical_min(curr_vals, best_vals);
                    else if constexpr (is_greater_comparator<Comp, value_type> && has_native_vertical_minmax<Tag>())
                        best_vals = vx::vertical_max(curr_vals, best_vals);
                    else
                        best_vals = vx::select(better, curr_vals, best_vals);

                    advance_bytes(ptr, sizeof(Tag));
                }

                value_type portion_min;
                if constexpr (is_less_comparator<Comp, value_type>) {
                    portion_min = vx::horizontal_min(best_vals);
                }
                else if constexpr (is_greater_comparator<Comp, value_type>) {
                    portion_min = vx::horizontal_max(best_vals);
                }
                else {
                    portion_min = vx::fold(best_vals, [&](const auto& x, const auto& y) raze_always_inline_lambda {
                        return vx::select(_comp(y, x), y, x);
                    });
                }

                if (!started || std::invoke(_comp, portion_min, *best_ptr)) {
                    typename Tag::mask_type eq_mask;
                    if constexpr (is_less_comparator<Comp, value_type> || is_greater_comparator<Comp, value_type>)
                        eq_mask = (best_vals == portion_min);
                    else
                        eq_mask = (!_comp(best_vals, portion_min)) & (!_comp(portion_min, best_vals));

                    if (vx::any_of(eq_mask)) {
                        const auto idx_eq_mask = vx::convert<unsigned_t>(eq_mask);
                        const auto masked_idxs = vx::select(idx_eq_mask, best_idxs, vx::all_bits<index_simd_t>());
                        const auto min_idx = vx::horizontal_min(masked_idxs);
                        const auto h_pos = vx::find_first_set[vx::not_null](masked_idxs == min_idx);
                        const auto v_pos = static_cast<sizetype>(best_idxs[h_pos]);

                        auto* const candidate = bytes_pointer_offset(portion_begin,
                            v_pos * sizeof(Tag) + static_cast<sizetype>(h_pos) * sizeof(value_type));

                        if (!started || std::invoke(_comp, *candidate, *best_ptr))
                            best_ptr = candidate;
                    }
                    started = true;
                }
            }

            _started = started;
            source_type::from_ptr(_iterator, ptr);
            source_type::from_ptr(_best, best_ptr);
        }

        raze_nodiscard constexpr raze_always_inline iterator_type result() const {
            return _source.wrap(_best);
        }

        raze_nodiscard static constexpr raze_always_inline decltype(auto) static_size()
            requires(constexpr_sized_source<source_type>)
        {
            return source_type::static_size();
        }

        raze_nodiscard constexpr raze_always_inline auto size() const {
            return _source.size();
        }
    };

    template <std::forward_iterator It, std::sentinel_for<It> Sent,
        class Comp = std::less<>, class Projection = std::identity>
    raze_nodiscard constexpr raze_always_inline It
    operator()(It first, Sent last, Comp comp = {}, Projection proj = {}) const
        requires(std::indirect_strict_weak_order<Comp, std::projected<It, Projection>>)
    {
        return this->dispatch(get_source(std::move(first), std::move(last)),
            traits::fwd_fn(comp), traits::fwd_fn(proj));
    }

    template <std::ranges::forward_range R,
        class Comp = std::less<>, class Projection = std::identity>
    raze_nodiscard constexpr raze_always_inline std::ranges::borrowed_iterator_t<R>
    operator()(R&& r, Comp comp = {}, Projection proj = {}) const
        requires(std::indirect_strict_weak_order<Comp, std::projected<std::ranges::iterator_t<R>, Projection>>)
    {
        return this->dispatch(get_source(std::forward<R>(r)),
            traits::fwd_fn(comp), traits::fwd_fn(proj));
    }
};

/**
 * @ingroup raze_algo
 * @memberof raze::algorithm
 * @hideinitializer
 * @brief Finds an iterator to the smallest element in a range.
 *
 * 1) Finds the first iterator `i` in `[first, last)` such that no element is smaller than `*i`
 *    under `comp` after applying `proj` (returns `last` if the range is empty).
 * 2) Same as (1), operating on range `r`.
 *
 * ### Decorators and Options
 * Supports `aligned<N>`, `fscalar`, `fstatic`, `fautovec`, and `unroll<N>` via `operator[]`.
 */
constexpr inline auto min_element = options::function_with_traits<min_element_t>;

__RAZE_ALGORITHM_NAMESPACE_END
