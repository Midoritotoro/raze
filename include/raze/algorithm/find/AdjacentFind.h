#pragma once

#include <raze/vx/Algorithm.h>
#include <src/raze/algorithm/RangesSize.h>
#include <src/raze/algorithm/UncheckedAlgorithms.h>
#include <src/raze/algorithm/DataSource.h>
#include <raze/math/Math.h>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <class Traits>
struct adjacent_find_t : Traits, dispatchable<adjacent_find_t<Traits>> {
    template <source Source, class Predicate, class Projection>
    struct kernel {
        using source_type = std::remove_cvref_t<Source>;
        using iterator_type = typename source_type::iterator_type;

        using unchecked_iterator_type = typename source_type::unchecked_iterator_type;
        using unchecked_sentinel_type = typename source_type::unchecked_sentinel_type;

        using value_type = std::iter_value_t<unchecked_iterator_type>;
        using vector_value_type = value_type;

        source_type _source;
        unchecked_iterator_type _iterator;
        unchecked_sentinel_type _sentinel;
        raze_no_unique_address Predicate _predicate;
        raze_no_unique_address Projection _proj;

        static consteval bool vectorizable() noexcept {
            return contiguous_source<source_type>
                && !traits::is_iterator_volatile_v<iterator_type>
                && vectorizable_binary_predicate<Predicate, unchecked_iterator_type>
                && vectorizable_projection<Projection, unchecked_iterator_type>;
        }

        constexpr explicit kernel(Source&& source, Predicate pred, Projection proj)
            : _source(std::forward<Source>(source)),
              _iterator(_source.ubegin()),
              _sentinel(_source.uend()),
              _predicate(std::move(pred)),
              _proj(std::move(proj))
        {}

        raze_nodiscard constexpr raze_always_inline bool exit() const {
            return _iterator == _sentinel;
        }

        constexpr raze_always_inline iterator_type default_result() const {
            return _source.wrap(_iterator);
        }

        raze_always_inline constexpr void operator()() {
            auto next = _iterator;

            raze_disable_unrolling
            for (; ++next != _sentinel; _iterator = next) {
                if (_predicate(_proj(*_iterator), _proj(*next)))
                    return;
            }

            _iterator = next;
        }

        template <vectorizable_tag Tag>
        raze_always_inline bool operator()(Tag, sizetype aligned_size) {
            auto* ptr = std::to_address(_iterator);
            const auto aligned_end = bytes_pointer_offset(ptr, aligned_size);
            auto* next_ptr = ptr + 1;

            raze_disable_unrolling do {
                const auto current = _proj(vx::load<Tag>(ptr));
                const auto next = _proj(vx::load<Tag>(next_ptr));

                const auto mask = _predicate(current, next);

                if (vx::any_of(mask)) {
                    source_type::from_ptr(_iterator, ptr + vx::find_first_set[vx::not_null](mask));
                    return false;
                }

                advance_bytes(ptr, next_ptr, sizeof(Tag));
            } while (ptr != aligned_end);

            source_type::from_ptr(_iterator, ptr);
            return true;
        }

        raze_nodiscard constexpr raze_always_inline auto size() const {
            const auto total = _source.size();
            return total > sizeof(value_type) ? total - sizeof(value_type) : sizetype(0);
        }

        raze_nodiscard static constexpr raze_always_inline decltype(auto) static_size()
            requires(constexpr_sized_source<source_type>)
        {
            constexpr sizetype total = source_type::static_size();
            return std::integral_constant<sizetype, (total > sizeof(value_type) ? total - sizeof(value_type) : 0)>{};
        }

        raze_nodiscard constexpr raze_always_inline iterator_type result() const {
            return _source.wrap(_iterator);
        }
    };

    template <std::forward_iterator It, std::sentinel_for<It> Sent, class Predicate = std::equal_to<>, class Projection = std::identity>
    raze_nodiscard constexpr raze_always_inline It operator()(It first, Sent last, Predicate pred = {}, Projection proj = {}) const
        requires(std::indirect_binary_predicate<Predicate, std::projected<It, Projection>, std::projected<It, Projection>>)
    {
        return this->dispatch(get_source(std::move(first), std::move(last)), traits::fwd_fn(pred), traits::fwd_fn(proj));
    }

    template <std::ranges::forward_range R, class Predicate = std::equal_to<>, class Projection = std::identity>
    raze_nodiscard constexpr raze_always_inline std::ranges::borrowed_iterator_t<R>
    operator()(R&& r, Predicate pred = {}, Projection proj = {}) const
        requires(std::indirect_binary_predicate<Predicate,
            std::projected<std::ranges::iterator_t<R>, Projection>,
            std::projected<std::ranges::iterator_t<R>, Projection>>)
    {
        return this->dispatch(get_source(std::forward<R>(r)), traits::fwd_fn(pred), traits::fwd_fn(proj));
    }
};

constexpr inline auto adjacent_find = options::function_with_traits<adjacent_find_t>;

__RAZE_ALGORITHM_NAMESPACE_END