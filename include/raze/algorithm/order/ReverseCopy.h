#pragma once

#include <src/raze/algorithm/RangesSize.h>
#include <src/raze/algorithm/UncheckedAlgorithms.h>
#include <src/raze/algorithm/DataSource.h>

#include <algorithm>
#include <iterator>
#include <ranges>
#include <type_traits>
#include <utility>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <class Traits>
struct reverse_copy_t : Traits, dispatchable<reverse_copy_t<Traits>> {
    template <source Source, class Out>
    struct kernel {
        using source_type = std::remove_cvref_t<Source>;
        using iterator_type = typename source_type::iterator_type;
        using unchecked_iterator_type = typename source_type::unchecked_iterator_type;
        using vector_value_type = std::iter_value_t<unchecked_iterator_type>;

        static consteval bool vectorizable() noexcept {
            if constexpr (!contiguous_source<Source> || !std::contiguous_iterator<Out>) return false;
            else return contiguous_source<Source> && std::is_arithmetic_v<vector_value_type> &&
                !std::same_as<vector_value_type, bool> &&
                std::same_as<vector_value_type, std::iter_value_t<Out>> &&
                !std::is_volatile_v<std::remove_reference_t<std::iter_reference_t<iterator_type>>> &&
                !std::is_volatile_v<std::remove_reference_t<std::iter_reference_t<Out>>> &&
                sizeof(vector_value_type) <= 8 &&
                (sizeof(vector_value_type) & (sizeof(vector_value_type) - 1)) == 0;
        }

        source_type _source;
        unchecked_iterator_type _begin;
        unchecked_iterator_type _current;
        unchecked_iterator_type _end;
        Out _out;

        constexpr explicit kernel(Source&& source, Out out)
            : _source(std::forward<Source>(source)),
              _begin(_source.ubegin()),
              _current(std::ranges::next(_begin, _source.uend())),
              _end(_current),
              _out(std::move(out))
        {}

        raze_always_inline constexpr void operator()() {
            while (_current != _begin) {
                --_current;
                *_out = *_current;
                ++_out;
            }
        }

        template <vectorizable_tag Tag>
        raze_always_inline void operator()(Tag, sizetype aligned_size) {
            if (aligned_size == 0) return;

            auto* in = std::to_address(_current);
            auto* out = std::to_address(_out);
            const auto count = aligned_size / sizeof(Tag);

            raze_disable_unrolling
            for (sizetype i = 0; i < count; ++i) {
                rewind_bytes(in, sizeof(Tag));
                const auto v = vx::load<Tag>(in);
                vx::store(out, vx::reverse(v));
                advance_bytes(out, sizeof(Tag));
            }

            source_type::from_ptr(_current, in);
            _out += aligned_size / sizeof(vector_value_type);
        }

        constexpr raze_always_inline std::ranges::in_out_result<iterator_type, Out> result() {
            return { _source.wrap(_end), std::move(_out) };
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

    template <std::bidirectional_iterator It, std::sentinel_for<It> Sent, std::weakly_incrementable Out>
    constexpr raze_always_inline std::ranges::in_out_result<It, Out> operator()(It first, Sent last, Out out) const
        requires(std::indirectly_copyable<It, Out>)
    {
        return this->dispatch(get_source(std::move(first), std::move(last)), std::move(out));
    }

    template <std::ranges::bidirectional_range R, std::weakly_incrementable Out>
    constexpr raze_always_inline std::ranges::in_out_result<std::ranges::borrowed_iterator_t<R>, Out>
    operator()(R&& r, Out out) const requires(std::indirectly_copyable<std::ranges::iterator_t<R>, Out>) {
        return this->dispatch(get_source(std::forward<R>(r)), std::move(out));
    }
};

constexpr inline auto reverse_copy = options::function_with_traits<reverse_copy_t>;

__RAZE_ALGORITHM_NAMESPACE_END