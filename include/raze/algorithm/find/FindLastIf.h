#pragma once

#include <raze/vx/Algorithm.h>
#include <src/raze/algorithm/RangesSize.h>
#include <src/raze/algorithm/UncheckedAlgorithms.h>
#include <src/raze/algorithm/DataSource.h>
#include <raze/math/Math.h>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <class Traits>
struct find_last_if_t : Traits, dispatchable<find_last_if_t<Traits>> {
    template <source Source, class Predicate, class Projection>
    struct kernel {
        using source_type = std::remove_cvref_t<Source>;
        using iterator_type = typename source_type::iterator_type;

        using unchecked_iterator_type = typename source_type::unchecked_iterator_type;
        using unchecked_sentinel_type = typename source_type::unchecked_sentinel_type;

        using vector_value_type = std::iter_value_t<unchecked_iterator_type>;

        static constexpr unchecked_iterator_type init_end(
            const unchecked_iterator_type& it, const unchecked_sentinel_type& sent)
        {
            if constexpr (std::random_access_iterator<unchecked_iterator_type> && std::sized_sentinel_for<
                unchecked_sentinel_type, unchecked_iterator_type>) return std::ranges::next(it, sent);
            else return it;
        }

        static consteval bool vectorizable() noexcept {
            return contiguous_source<source_type> && !traits::is_iterator_volatile_v<iterator_type>
                && vectorizable_unary_predicate<Predicate, unchecked_iterator_type>
                && vectorizable_projection<Projection, unchecked_iterator_type>;
        }

        source_type _source;
        unchecked_iterator_type _iterator;
        unchecked_sentinel_type _sentinel;
        raze_no_unique_address Predicate _predicate;
        raze_no_unique_address Projection _proj;
        unchecked_iterator_type _end;
        unchecked_iterator_type _back;

        constexpr explicit kernel(Source&& source, Predicate pred, Projection proj)
            : _source(std::forward<Source>(source)),
              _iterator(_source.ubegin()), _sentinel(_source.uend()),
              _predicate(std::move(pred)), _proj(std::move(proj)),
              _end(init_end(_iterator, _sentinel)), _back(_end)
        {}

        raze_always_inline constexpr void operator()() {
            if constexpr (std::bidirectional_iterator<unchecked_iterator_type> && 
                std::random_access_iterator<unchecked_iterator_type> &&
                std::sized_sentinel_for<unchecked_sentinel_type, unchecked_iterator_type>) 
            {
                raze_disable_unrolling
                for (auto it = _back; it != _iterator;) {
                    if (std::invoke(_predicate, std::invoke(_proj, *--it))) {
                        _back = it;
                        return;
                    }
                }

                _back = _end;
            }
            else {
                auto it = _iterator;
                auto found = it;
                bool has_match = false;

                raze_disable_unrolling
                for (; it != _sentinel; ++it) {
                    if (std::invoke(_predicate, std::invoke(_proj, *it))) {
                        found = it;
                        has_match = true;
                    }
                }

                _end = it;
                _back = has_match ? found : it;
            }
        }

        template <vectorizable_tag Tag>
        raze_always_inline bool operator()(Tag, sizetype aligned_size) {
            auto* ptr = std::to_address(_back);
            const auto* const stop_at = bytes_pointer_offset(ptr, -static_cast<std::ptrdiff_t>(aligned_size));

            raze_disable_unrolling do {
                rewind_bytes(ptr, sizeof(Tag));
                const auto mask = _predicate(_proj(vx::load<Tag>(ptr)));

                if (vx::any_of(mask)) {
                    source_type::from_ptr(_back, ptr + (Tag::size() - 1 - vx::find_last_set[vx::not_null](mask)));
                    return false;
                }
            } while (ptr != stop_at);

            source_type::from_ptr(_back, ptr);
            return true;
        }

        template <vectorizable_tag Tag>
        raze_always_inline void operator()(Tag, tail_mask_type auto const& ignore) {
            auto* ptr = std::to_address(_iterator);
            const auto mask = _predicate(_proj(vx::load<Tag>[ignore](ptr)));
            const auto pos = vx::find_last_set[ignore](mask);

            const auto target = static_cast<sizetype>(pos) < static_cast<sizetype>(Tag::size())
                ? ptr + (Tag::size() - 1 - pos) : std::to_address(_end);

            source_type::from_ptr(_back, target);
        }

        constexpr raze_always_inline std::ranges::subrange<iterator_type> result() const {
            return { _source.wrap(_back), _source.wrap(_end) };
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
        class Predicate, class Projection = std::identity>
    raze_nodiscard constexpr raze_always_inline std::ranges::subrange<It>
        operator()(It first, Sent sent, Predicate pred, Projection proj = {}) const
            requires(std::indirect_unary_predicate<Predicate, std::projected<It, Projection>>)
    {
        return this->dispatch(get_source(std::move(first), std::move(sent)),
            traits::fwd_fn(pred), traits::fwd_fn(proj));
    }

    template <std::ranges::forward_range R, class Predicate, class Projection = std::identity>
    raze_nodiscard constexpr raze_always_inline std::ranges::borrowed_subrange_t<R>
        operator()(R&& r, Predicate pred, Projection proj = {}) const
            requires(std::indirect_unary_predicate<Predicate, std::projected<std::ranges::iterator_t<R>, Projection>>)
    {
        return this->dispatch(get_source(std::forward<R>(r)), traits::fwd_fn(pred), traits::fwd_fn(proj));
    }
};

constexpr inline auto find_last_if = options::function_with_traits<find_last_if_t>;

__RAZE_ALGORITHM_NAMESPACE_END