#pragma once

#include <raze/vx/Algorithm.h>
#include <src/raze/algorithm/RangesSize.h>
#include <src/raze/algorithm/UncheckedAlgorithms.h>
#include <src/raze/algorithm/DataSource.h>
#include <src/raze/algorithm/EqualTo.h>
#include <src/raze/algorithm/NotFn.h>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <class Traits>
struct count_if_t : Traits, dispatchable<count_if_t<Traits>> {
    template <source Source, class Predicate, class Projection>
    struct kernel {
        using source_type = std::remove_cvref_t<Source>;
        using iterator_type = typename source_type::iterator_type;
        using difference_type = std::iter_difference_t<iterator_type>;

        using unchecked_iterator_type = typename source_type::unchecked_iterator_type;
        using unchecked_sentinel_type = typename source_type::unchecked_sentinel_type;

        using vector_value_type = std::iter_value_t<unchecked_iterator_type>;

        source_type _source;
        unchecked_iterator_type _iterator;
        unchecked_sentinel_type _sentinel;
        Predicate _predicate;
        Projection _proj;
        difference_type _count = 0;

        static consteval bool vectorizable() noexcept {
            return contiguous_source<source_type>
                && !traits::is_iterator_volatile_v<iterator_type>
                && vectorizable_unary_predicate<Predicate, unchecked_iterator_type>
                && vectorizable_projection<Projection, unchecked_iterator_type>;
        }

        constexpr explicit kernel(Source&& src, Predicate pred, Projection proj) noexcept
            : _source(std::forward<Source>(src)),
              _iterator(_source.ubegin()),
              _sentinel(_source.uend()),
              _predicate(std::move(pred)),
              _proj(std::move(proj))
        {}

        raze_always_inline constexpr void operator()() noexcept {
            raze_disable_unrolling
            for (; _iterator != _sentinel; ++_iterator)
                _count += static_cast<difference_type>(static_cast<bool>(std::invoke(_predicate, std::invoke(_proj, *_iterator))));
        }

        template <vectorizable_tag Tag>
        raze_always_inline void operator()(Tag, sizetype aligned_size) noexcept {
            vx::counter<Tag> counter;

            auto* ptr = std::to_address(_iterator);
            const auto aligned_end = bytes_pointer_offset(ptr, aligned_size);

            raze_disable_unrolling while (ptr != aligned_end) {
                auto current_portion_size = counter.portion_size();

                raze_disable_unrolling
                while (ptr != aligned_end && current_portion_size > 0) {
                    counter.count(_predicate(_proj(vx::load<Tag>(ptr))));
                    advance_bytes(ptr, sizeof(Tag));
                    --current_portion_size;
                }

                _count += counter.result();
                counter.reset();
            }

            source_type::from_ptr(_iterator, ptr);
        }

        template <vectorizable_tag Tag>
        raze_always_inline void operator()(Tag, tail_mask_type auto const& ignore) noexcept {
            _count += vx::count_set[ignore](_predicate(_proj(vx::load<Tag>[ignore](std::to_address(_iterator)))));
        }

        raze_nodiscard static constexpr raze_always_inline decltype(auto) static_size() noexcept
            requires(constexpr_sized_source<source_type>)
        {
            return source_type::static_size();
        }

        raze_nodiscard constexpr raze_always_inline auto size() const {
            return _source.size();
        }

        constexpr raze_always_inline difference_type result() const {
            return _count;
        }
    };

    template <std::input_iterator It, std::sentinel_for<It> Sent,
        class Predicate, class Projection = std::identity>
    raze_nodiscard constexpr raze_always_inline std::iter_difference_t<It>
    operator()(It first, Sent sent, Predicate pred, Projection proj = {}) const
        requires(std::indirect_unary_predicate<Predicate, std::projected<It, Projection>>)
    {
        return this->dispatch(get_source(std::move(first), std::move(sent)), traits::fwd_fn(pred), traits::fwd_fn(proj));
    }

    template <std::ranges::input_range R, class Predicate, class Projection = std::identity>
    raze_nodiscard constexpr raze_always_inline std::ranges::range_difference_t<R>
    operator()(R&& r, Predicate pred, Projection proj = {}) const
        requires(std::indirect_unary_predicate<Predicate, std::projected<std::ranges::iterator_t<R>, Projection>>)
    {
        return this->dispatch(get_source(std::forward<R>(r)), traits::fwd_fn(pred), traits::fwd_fn(proj));
    }
};

constexpr inline auto count_if = options::function_with_traits<count_if_t>[options::unroll<4>];

__RAZE_ALGORITHM_NAMESPACE_END
