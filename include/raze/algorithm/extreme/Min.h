#pragma once

#include <raze/vx/Algorithm.h>
#include <raze/math/BitCast.h>
#include <raze/math/Math.h>
#include <src/raze/algorithm/RangesSize.h>
#include <src/raze/algorithm/UncheckedAlgorithms.h>
#include <src/raze/algorithm/DataSource.h>
#include <src/raze/algorithm/VectorizablePredicate.h>
#include <src/raze/math/IntegralTypesConversions.h>
#include <optional>
#include <functional>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <class Traits>
struct min_t : Traits, dispatchable<min_t<Traits>> {
    template <source Source, class Comp, class Projection>
    struct kernel {
        using source_type = std::remove_cvref_t<Source>;
        using iterator_type = typename source_type::iterator_type;

        using unchecked_iterator_type = typename source_type::unchecked_iterator_type;
        using unchecked_sentinel_type = typename source_type::unchecked_sentinel_type;

        using value_type = std::iter_value_t<unchecked_iterator_type>;
        using vector_value_type = value_type;

        static consteval bool vectorizable() noexcept {
            return contiguous_source<source_type>
                && !traits::is_iterator_volatile_v<iterator_type>
                && vectorizable_binary_predicate<Comp, unchecked_iterator_type>
                && vectorizable_projection<Projection, unchecked_iterator_type>;
        }

        source_type _source;
        unchecked_iterator_type _iterator;
        unchecked_sentinel_type _sentinel;
        raze_no_unique_address Comp _comp;
        raze_no_unique_address Projection _proj;
        std::optional<value_type> _value;

        constexpr explicit kernel(Source&& source, Comp comp, Projection proj)
            : _source(std::forward<Source>(source)),
              _iterator(_source.ubegin()),
              _sentinel(_source.uend()),
              _comp(std::move(comp)),
              _proj(std::move(proj))
        {}

        raze_nodiscard constexpr raze_always_inline bool exit() const {
            return _iterator == _sentinel;
        }

        raze_nodiscard constexpr raze_always_inline std::optional<value_type> default_result() const noexcept {
            return std::nullopt;
        }

        raze_always_inline constexpr void operator()() {
            if (_iterator == _sentinel)
                return;

            if (!_value.has_value()) {
                _value.emplace(*_iterator);
                ++_iterator;
            }

            raze_disable_unrolling
            for (; _iterator != _sentinel; ++_iterator) {
                if (std::invoke(_comp, std::invoke(_proj, *_iterator), std::invoke(_proj, *_value)))
                    *_value = *_iterator;
            }
        }

        template <vectorizable_tag Tag>
        raze_always_inline void merge_vector(const Tag& vmin) {
            value_type candidate;

            if constexpr (is_less_comparator<Comp>)
                candidate = vx::horizontal_min(vmin);
            else
                candidate = vx::horizontal_max(vmin);

            if (!_value.has_value()) {
                _value.emplace(candidate);
            }
            else if (std::invoke(_comp, std::invoke(_proj, candidate), std::invoke(_proj, *_value))) {
                *_value = candidate;
            }
        }

        template <vectorizable_tag Tag>
        raze_always_inline void operator()(Tag, sizetype aligned_size) {
            auto* ptr = std::to_address(_iterator);

            if constexpr (is_less_comparator<Comp> || is_greater_comparator<Comp>) {
                const auto* const aligned_end = bytes_pointer_offset(ptr, aligned_size);

                auto vmin = _proj(vx::load<Tag>(ptr));
                advance_bytes(ptr, sizeof(Tag));

                raze_disable_unrolling
                while (ptr != aligned_end) {
                    const auto loaded = _proj(vx::load<Tag>(ptr));

                    if constexpr (is_less_comparator<Comp>)
                        vmin = vx::vertical_min(loaded, vmin);
                    else
                        vmin = vx::vertical_max(loaded, vmin);

                    advance_bytes(ptr, sizeof(Tag));
                }

                source_type::from_ptr(_iterator, ptr);
                merge_vector(vmin);
            }
            else {
                using unsigned_t = typename IntegerForSizeof<value_type>::Unsigned;
                using index_simd_t = vx::simd<unsigned_t, vx::abi_t<Tag>>;
                using index_mask_t = typename index_simd_t::mask_type;
                using mask_t = typename Tag::mask_type;

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

                    auto best_vals = _proj(vx::load<Tag>(ptr));
                    auto best_idxs = vx::zero<index_simd_t>();
                    auto curr_idxs = vx::zero<index_simd_t>();

                    advance_bytes(ptr, sizeof(Tag));

                    raze_disable_unrolling
                    while (ptr != stop_at) {
                        ++curr_idxs;
                        const auto curr_vals = _proj(vx::load<Tag>(ptr));
                        const auto better = _comp(curr_vals, best_vals);

                        best_idxs = vx::select[math::bit_cast<index_mask_t>(better), best_idxs](curr_idxs);
                        best_vals = vx::select[better, best_vals](curr_vals);

                        advance_bytes(ptr, sizeof(Tag));
                    }

                    const value_type portion_min = vx::fold(best_vals,
                        [&](const auto& x, const auto& y) raze_always_inline_lambda {
                            return vx::select[_comp(y, x), x](y);
                        });

                    if (!_value.has_value() ||
                        std::invoke(_comp, std::invoke(_proj, portion_min), std::invoke(_proj, *_value)))
                    {
                        const mask_t eq_mask = (!_comp(best_vals, portion_min)) & (!_comp(portion_min, best_vals));

                        if (vx::any_of(eq_mask)) {
                            const auto idx_eq_mask = math::bit_cast<index_mask_t>(eq_mask);
                            const auto masked_idxs = vx::select[idx_eq_mask, vx::all_bits<index_simd_t>()](best_idxs);
                            const auto min_idx = vx::horizontal_min(masked_idxs);
                            const auto match_mask = math::bit_cast<mask_t>(masked_idxs == min_idx) & eq_mask;

                            const auto h_pos = vx::find_first_set[vx::not_null](match_mask);
                            const auto v_pos = static_cast<sizetype>(best_idxs[h_pos]);

                            const auto candidate = *bytes_pointer_offset(portion_begin,
                                v_pos * sizeof(Tag) + static_cast<sizetype>(h_pos) * sizeof(value_type));

                            if (!_value.has_value())
                                _value.emplace(candidate);
                            else if (std::invoke(_comp, std::invoke(_proj, candidate), std::invoke(_proj, *_value)))
                                *_value = candidate;
                        }
                        else if (!_value.has_value()) {
                            _value.emplace(*portion_begin);
                        }
                    }
                }

                source_type::from_ptr(_iterator, ptr);
            }
        }

        template <vectorizable_tag Tag>
        raze_always_inline void operator()(Tag, tail_mask_type auto const& ignore) {
            if (ignore.tail_bytes() == 0)
                return;

            if constexpr (is_less_comparator<Comp> || is_greater_comparator<Comp>) {
                auto* ptr = std::to_address(_iterator);
                const auto mask = ignore.mask();
                const Tag fallback(*ptr);
                const auto loaded = _proj(vx::select[mask, fallback](vx::load<Tag>[mask](ptr)));

                advance_bytes(ptr, ignore.tail_bytes());
                source_type::from_ptr(_iterator, ptr);
                merge_vector(loaded);
            }
            else {
                (*this)();
            }
        }

        raze_nodiscard constexpr raze_always_inline std::optional<value_type> result() const {
            return _value;
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

    template <std::input_iterator It, std::sentinel_for<It> Sent,
        class Comp = std::less<>, class Projection = std::identity>
    raze_nodiscard constexpr raze_always_inline std::optional<std::iter_value_t<It>>
    operator()(It first, Sent last, Comp comp = {}, Projection proj = {}) const
        requires(std::indirect_strict_weak_order<Comp, std::projected<It, Projection>>)
    {
        return this->dispatch(get_source(std::move(first), std::move(last)),
            traits::fwd_fn(comp), traits::fwd_fn(proj));
    }

    template <std::ranges::input_range R,
        class Comp = std::less<>, class Projection = std::identity>
    raze_nodiscard constexpr raze_always_inline std::optional<std::ranges::range_value_t<R>>
    operator()(R&& r, Comp comp = {}, Projection proj = {}) const
        requires(std::indirect_strict_weak_order<Comp, std::projected<std::ranges::iterator_t<R>, Projection>>)
    {
        return this->dispatch(get_source(std::forward<R>(r)),
            traits::fwd_fn(comp), traits::fwd_fn(proj));
    }
};

constexpr inline auto min = options::function_with_traits<min_t>;

__RAZE_ALGORITHM_NAMESPACE_END