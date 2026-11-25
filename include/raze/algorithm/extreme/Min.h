#pragma once

#include <raze/vx/Algorithm.h>
#include <raze/math/Math.h>
#include <src/raze/algorithm/RangesSize.h>
#include <src/raze/algorithm/UncheckedAlgorithms.h>
#include <src/raze/algorithm/DataSource.h>
#include <src/raze/algorithm/VectorizablePredicate.h>
#include <src/raze/math/IntegralTypesConversions.h>
#include <initializer_list>
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
        using align = options::alignments<Traits, 1>;

        static consteval bool vectorizable() noexcept {
            return contiguous_source<source_type>
                && !traits::is_iterator_volatile_v<iterator_type>
                && vectorizable_binary_predicate<Comp, unchecked_iterator_type>
                && vectorizable_projection<Projection, unchecked_iterator_type>;
        }

        struct trivial_storage {
            value_type val{};
            bool has = false;

            constexpr raze_always_inline bool has_value() const noexcept { return has; }
            constexpr raze_always_inline value_type& operator*() noexcept { return val; }
            constexpr raze_always_inline const value_type& operator*() const noexcept { return val; }

            template <class... Args>
            constexpr raze_always_inline value_type& emplace(Args&&... args) {
                val = value_type(std::forward<Args>(args)...);
                has = true;
                return val;
            }
        };

        static constexpr bool use_trivial_storage =
            std::is_trivially_default_constructible_v<value_type> &&
            std::is_trivially_copyable_v<value_type>;

        using storage_type = std::conditional_t<use_trivial_storage, trivial_storage, std::optional<value_type>>;

        raze_no_unique_address source_type _source;
        unchecked_iterator_type _iterator;
        raze_no_unique_address unchecked_sentinel_type _sentinel;
        raze_no_unique_address Comp _comp;
        raze_no_unique_address Projection _proj;
        storage_type _value{};

        constexpr explicit kernel(Source&& source, Comp comp, Projection proj)
            : _source(std::forward<Source>(source)),
              _iterator(_source.ubegin()),
              _sentinel(_source.uend()),
              _comp(std::move(comp)),
              _proj(std::move(proj))
        {}

        constexpr void expects() const {
            align::verify(_source);
        }

        raze_nodiscard constexpr raze_always_inline bool exit() const {
            return _iterator == _sentinel;
        }

        raze_nodiscard constexpr raze_always_inline std::optional<value_type> default_result() const noexcept {
            return std::nullopt;
        }

        raze_always_inline constexpr void operator()(autovectorizable_tag)
            requires(contiguous_source<source_type> && !traits::is_iterator_volatile_v<iterator_type>)
        {
            auto [first, last] = source_type::to_raw_range(_iterator, _sentinel);
            if (first == last)
                return;

            first = align::template assume<0>(first);

            std::conditional_t<traits::lightweight_callable<Comp>, Comp, Comp&> comp = _comp;
            std::conditional_t<traits::lightweight_callable<Projection>, Projection, Projection&> proj = _proj;

            auto start = first;
            if (!_value.has_value()) {
                _value.emplace(*start);
                ++start;
            }

            auto best = std::move(*_value);
            const auto len = last - start;
            for (std::remove_const_t<decltype(len)> i = 0; i < len; ++i) {
                if (std::invoke(comp, std::invoke(proj, start[i]), std::invoke(proj, best)))
                    best = start[i];
            }

            *_value = std::move(best);
            source_type::from_ptr(_iterator, last);
        }

        raze_always_inline constexpr void operator()() {
            auto it = _iterator;
            if (it == _sentinel)
                return;

            if (!_value.has_value()) {
                _value.emplace(*it);
                ++it;
            }

            auto best = std::move(*_value);
            raze_novec_loop((; it != _sentinel; ++it), {
                if (std::invoke(_comp, std::invoke(_proj, *it), std::invoke(_proj, best)))
                    best = *it;
            })
            *_value = std::move(best);
            _iterator = std::move(it);
        }

        template <vectorizable_tag Tag>
        raze_always_inline void merge_vector(const Tag& vmin) {
            value_type candidate;

            if constexpr (is_less_comparator<Comp, value_type>)
                candidate = vx::horizontal_min(vmin);
            else
                candidate = vx::horizontal_max(vmin);

            if (!_value.has_value()) {
                _value.emplace(candidate);
            }
            else if (std::invoke(_comp, candidate, *_value)) {
                *_value = candidate;
            }
        }

        template <vectorizable_tag Tag>
        raze_always_inline void operator()(Tag, sizetype aligned_size) {
            constexpr auto alignment = align::template policy_for<0, Tag>;
            auto* ptr = std::to_address(_iterator);

            if constexpr (is_less_comparator<Comp, value_type> || is_greater_comparator<Comp, value_type>) {
                const auto* const aligned_end = bytes_pointer_offset(ptr, aligned_size);

                auto vmin = vx::load<Tag>[alignment](ptr);
                advance_bytes(ptr, sizeof(Tag));

                raze_disable_unrolling
                while (ptr != aligned_end) {
                    const auto loaded = vx::load<Tag>[alignment](ptr);

                    if constexpr (is_less_comparator<Comp, value_type>)
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
                        best_vals = vx::select(better, curr_vals, best_vals);

                        advance_bytes(ptr, sizeof(Tag));
                    }

                    const value_type portion_min = vx::fold(best_vals,
                        [&](const auto& x, const auto& y) raze_always_inline_lambda {
                            return vx::select(_comp(y, x), y, x);
                        });

                    auto extract_portion_min = [&]() raze_always_inline_lambda -> trivial_storage {
                        trivial_storage res{};
                        const auto eq_mask = (!_comp(best_vals, portion_min)) & (!_comp(portion_min, best_vals));

                        if (vx::any_of(eq_mask)) {
                            const auto idx_eq_mask = vx::convert<unsigned_t>(eq_mask);
                            const auto masked_idxs = vx::select(idx_eq_mask, best_idxs, vx::all_bits<index_simd_t>());
                            const auto min_idx = vx::horizontal_min(masked_idxs);
                            const auto h_pos = vx::find_first_set[vx::not_null](masked_idxs == min_idx);
                            const auto v_pos = static_cast<sizetype>(best_idxs[h_pos]);

                            res.emplace(*bytes_pointer_offset(portion_begin,
                                v_pos * sizeof(Tag) + static_cast<sizetype>(h_pos) * sizeof(value_type)));
                        }

                        return res;
                    };

                    if (!_value.has_value()) {
                        if (auto candidate = extract_portion_min(); candidate.has_value())
                            _value.emplace(*candidate);
                        else
                            _value.emplace(*portion_begin);
                    }
                    else if (std::invoke(_comp, portion_min, *_value)) {
                        if (auto candidate = extract_portion_min(); candidate.has_value()) {
                            if (std::invoke(_comp, *candidate, *_value))
                                *_value = *candidate;
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

            if constexpr (is_less_comparator<Comp, value_type> || is_greater_comparator<Comp, value_type>) {
                constexpr auto alignment = align::template policy_for<0, Tag>;
                auto* ptr = std::to_address(_iterator);
                const auto mask = ignore.mask();
                const Tag fallback(*ptr);
                const auto loaded = vx::select(mask, vx::load<Tag>[ignore][alignment](ptr), fallback);

                advance_bytes(ptr, ignore.tail_bytes());
                source_type::from_ptr(_iterator, ptr);
                merge_vector(loaded);
            }
            else {
                (*this)();
            }
        }

        raze_nodiscard constexpr raze_always_inline std::optional<value_type> result() const {
            if constexpr (use_trivial_storage)
                return _value.has_value() ? std::optional<value_type>(*_value) : std::nullopt;
            else
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
        requires(!std::same_as<std::remove_cvref_t<R>, std::initializer_list<std::ranges::range_value_t<R>>> &&
            std::indirect_strict_weak_order<Comp, std::projected<std::ranges::iterator_t<R>, Projection>>)
    {
        return this->dispatch(get_source(std::forward<R>(r)),
            traits::fwd_fn(comp), traits::fwd_fn(proj));
    }

    template <class T, class Comp = std::less<>, class Projection = std::identity>
    raze_nodiscard constexpr raze_always_inline std::optional<T>
    operator()(std::initializer_list<T> ilist, Comp comp = {}, Projection proj = {}) const
        requires(std::indirect_strict_weak_order<Comp, std::projected<const T*, Projection>>)
    {
        return this->dispatch(get_source(ilist.begin(), ilist.end()),
            traits::fwd_fn(comp), traits::fwd_fn(proj));
    }
};

/**
 * @ingroup raze_algo
 * @memberof raze::algorithm
 * @hideinitializer
 * @brief Returns the smallest element in a range, or `std::nullopt` if the range is empty.
 *
 * 1) Finds the smallest element in `[first, last)` according to `comp` after applying `proj`.
 * 2) Same as (1), operating on range `r`.
 * 3) Same as (1), operating on `std::initializer_list<T>`.
 *
 * If several elements are equivalent to the smallest element, returns a copy of the first such element.
 *
 * ### Decorators and Options
 * Configured with `raze::options::unroll<4>` by default and supports `aligned<N>`, `fscalar`,
 * `fstatic`, `fautovec`, and `unroll<N>` via `operator[]`.
 */
constexpr inline auto min = options::function_with_traits<min_t>[options::unroll<4>];

__RAZE_ALGORITHM_NAMESPACE_END
