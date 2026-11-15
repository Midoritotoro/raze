#pragma once

#include <src/raze/algorithm/RangesSize.h>
#include <src/raze/algorithm/UncheckedAlgorithms.h>
#include <src/raze/algorithm/DataSource.h>
#include <algorithm>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <class Traits>
struct reverse_t : Traits, dispatchable<reverse_t<Traits>> {
    template <source Source>
    struct kernel {
        using source_type = std::remove_cvref_t<Source>;

        using iterator_type = typename source_type::iterator_type;
        using unchecked_iterator_type = typename source_type::unchecked_iterator_type;
        using unchecked_sentinel_type = typename source_type::unchecked_sentinel_type;

        using vector_value_type = std::iter_value_t<unchecked_iterator_type>;

        static consteval bool vectorizable() noexcept {
            return contiguous_source<Source> &&
                std::is_arithmetic_v<vector_value_type> &&
                (sizeof(vector_value_type) & (sizeof(vector_value_type) - 1)) == 0;
        }

        source_type _source;
        unchecked_iterator_type _front;
        unchecked_iterator_type _back;
        unchecked_iterator_type _end;

        constexpr explicit kernel(Source&& source)
            : _source(std::forward<Source>(source)),
              _front(_source.ubegin()),
              _back(std::ranges::next(_front, _source.uend())),
              _end(_back)
        {}

        raze_always_inline constexpr void operator()() {
            raze_disable_unrolling
            while (_front != _back) {
                --_back;
                if (_front == _back) break;
                std::ranges::iter_swap(_front, _back);
                ++_front;
            }
        }

        template <vectorizable_tag Tag>
        raze_always_inline void operator()(Tag, sizetype aligned_size) {
            const auto pairs = aligned_size / sizeof(Tag) / 2;
            if (pairs == 0) return;

            auto* front = std::to_address(_front);
            auto* back = std::to_address(_back);

            raze_disable_unrolling
            for (sizetype i = 0; i < pairs; ++i) {
                rewind_bytes(back, sizeof(Tag));

                const auto vf = vx::load<Tag>(front);
                const auto vb = vx::load<Tag>(back);

                vx::store(front, vx::reverse(vb));
                vx::store(back, vx::reverse(vf));

                advance_bytes(front, sizeof(Tag));
            }

            source_type::from_ptr(_front, front);
            source_type::from_ptr(_back, back);
        }

        constexpr raze_always_inline iterator_type result() const {
            return _source.wrap(_end);
        }

        raze_nodiscard static constexpr raze_always_inline decltype(auto) static_size()
            requires(constexpr_sized_source<Source>)
        {
            return source_type::static_size();
        }

        raze_nodiscard constexpr raze_always_inline auto size() const {
            return _source.size();
        }
    };

    template <std::bidirectional_iterator It, std::sentinel_for<It> Sent>
    constexpr raze_always_inline It operator()(It first, Sent last) const
        requires(std::permutable<It>)
    {
        return this->dispatch(get_source(std::move(first), std::move(last)));
    }

    template <std::ranges::bidirectional_range R>
    constexpr raze_always_inline std::ranges::borrowed_iterator_t<R> operator()(R&& r) const
        requires(std::permutable<std::ranges::iterator_t<R>>)
    {
        return this->dispatch(get_source(std::forward<R>(r)));
    }
};

constexpr inline auto reverse = options::function_with_traits<reverse_t>;

__RAZE_ALGORITHM_NAMESPACE_END