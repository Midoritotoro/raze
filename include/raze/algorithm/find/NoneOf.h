#pragma once

#include <raze/vx/Algorithm.h>
#include <src/raze/algorithm/RangesSize.h>
#include <src/raze/algorithm/UncheckedAlgorithms.h>
#include <src/raze/algorithm/DataSource.h>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <class Traits>
struct none_of_t : Traits, dispatchable<none_of_t<Traits>> {
    template <source Source, class Predicate, class Projection>
    struct kernel {
        using source_type = std::remove_cvref_t<Source>;
        using iterator_type = typename source_type::iterator_type;

        using unchecked_iterator_type = typename source_type::unchecked_iterator_type;
        using unchecked_sentinel_type = typename source_type::unchecked_sentinel_type;

        using vector_value_type = std::iter_value_t<unchecked_iterator_type>;

        source_type _source;
        unchecked_iterator_type _iterator;
        unchecked_sentinel_type _sentinel;
        raze_no_unique_address Predicate _predicate;
        raze_no_unique_address Projection _proj;
        bool _result = true;

        static consteval bool vectorizable() noexcept {
            return contiguous_source<source_type>
                && !traits::is_iterator_volatile_v<iterator_type>
                && vectorizable_unary_predicate<Predicate, unchecked_iterator_type>
                && vectorizable_projection<Projection, unchecked_iterator_type>;
        }

        constexpr explicit kernel(Source&& source, Predicate pred, Projection proj)
            : _source(std::forward<Source>(source)),
              _iterator(_source.ubegin()),
              _sentinel(_source.uend()),
              _predicate(std::move(pred)),
              _proj(std::move(proj))
        {}

        raze_always_inline constexpr void operator()() {
            raze_disable_unrolling
            for (; _iterator != _sentinel; ++_iterator) {
                if (_predicate(_proj(*_iterator))) {
                    _result = false;
                    return;
                }
            }
        }

        template <vectorizable_tag Tag>
        raze_always_inline bool operator()(Tag, sizetype aligned_size) {
            auto* ptr = std::to_address(_iterator);
            const auto aligned_end = bytes_pointer_offset(ptr, aligned_size);

            raze_disable_unrolling do {
                if (vx::any_of(_predicate(_proj(vx::load<Tag>(ptr))))) {
                    _result = false;
                    return false;
                }

                advance_bytes(ptr, sizeof(Tag));
            } while (ptr != aligned_end);

            source_type::from_ptr(_iterator, ptr);
            return true;
        }

        template <vectorizable_tag Tag>
        raze_always_inline void operator()(Tag, tail_mask_type auto const& ignore) {
            const auto mask = _predicate(_proj(vx::load<Tag>[ignore](std::to_address(_iterator))));
            _result = vx::none_of[ignore](mask);
        }

        raze_nodiscard static constexpr raze_always_inline decltype(auto) static_size()
            requires(constexpr_sized_source<source_type>)
        {
            return source_type::static_size();
        }

        raze_nodiscard constexpr raze_always_inline auto size() const {
            return _source.size();
        }

        raze_nodiscard constexpr raze_always_inline bool result() const noexcept {
            return _result;
        }
    };

    template <std::input_iterator It, std::sentinel_for<It> Sent, class Predicate, class Projection = std::identity>
    raze_nodiscard constexpr raze_always_inline bool operator()(It first, Sent last, Predicate pred, Projection proj = {}) const
        requires(std::indirect_unary_predicate<Predicate, std::projected<It, Projection>>)
    {
        return this->dispatch(get_source(std::move(first), std::move(last)), traits::fwd_fn(pred), traits::fwd_fn(proj));
    }

    template <std::ranges::input_range R, class Predicate, class Projection = std::identity>
    raze_nodiscard constexpr raze_always_inline bool operator()(R&& r, Predicate pred, Projection proj = {}) const
        requires(std::indirect_unary_predicate<Predicate, std::projected<std::ranges::iterator_t<R>, Projection>>)
    {
        return this->dispatch(get_source(std::forward<R>(r)), traits::fwd_fn(pred), traits::fwd_fn(proj));
    }
};

constexpr inline auto none_of = options::function_with_traits<none_of_t>;

__RAZE_ALGORITHM_NAMESPACE_END