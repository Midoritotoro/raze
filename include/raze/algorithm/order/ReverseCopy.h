#pragma once

#include <src/raze/algorithm/RangesSize.h>
#include <src/raze/algorithm/UncheckedAlgorithms.h>
#include <src/raze/algorithm/DataSource.h>
#include <src/raze/algorithm/Destination.h>
#include <algorithm>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <class Traits>
struct reverse_copy_t : Traits, dispatchable<reverse_copy_t<Traits>> {
    template <source Source, destination Destination>
    struct kernel {
        using source_type = std::remove_cvref_t<Source>;
        using destination_type = std::remove_cvref_t<Destination>;

        using iterator_type = typename source_type::iterator_type;
        using out_iterator_type = typename destination_type::iterator_type;

        using unchecked_iterator_type = typename source_type::unchecked_iterator_type;
        using unchecked_sentinel_type = typename source_type::unchecked_sentinel_type;
        using unchecked_out_iterator_type = typename destination_type::unchecked_iterator_type;

        template <class It>
        static consteval auto deduce_value_type() noexcept {
            if constexpr (std::indirectly_readable<It>) return std::type_identity<std::iter_value_t<It>>{};
            else return std::type_identity<void>{};
        }

        template <class T>
        static consteval auto deduce_vector_value_type() noexcept {
            if constexpr (!std::is_void_v<T>) {
                if constexpr (sizeof(T) <= 8 && ((sizeof(T) & (sizeof(T) - 1)) == 0)) {
                    if constexpr (vx::arithmetic_type<T>) return T{};
                    else return typename IntegerForSizeof<T>::Unsigned{};
                }
            }
        }

        using in_t = std::iter_value_t<unchecked_iterator_type>;
        using out_t = typename decltype(deduce_value_type<unchecked_out_iterator_type>())::type;

        using vtype_in = decltype(deduce_vector_value_type<in_t>());
        using vtype_out = decltype(deduce_vector_value_type<out_t>());
        using vector_value_type = vtype_in;

        static consteval bool vectorizable() noexcept {
            return contiguous_source<Source> && contiguous_destination<Destination> 
                && !traits::is_iterator_volatile_v<iterator_type>
                && !traits::is_iterator_volatile_v<out_iterator_type>
                && !std::same_as<vtype_in, void> && !std::same_as<vtype_out, void>
                && ((vx::arithmetic_type<in_t> && vx::arithmetic_type<out_t>)
                    || (std::same_as<in_t, out_t> && std::is_trivially_copyable_v<in_t>
                        && std::is_trivially_copy_assignable_v<in_t>));
        }

        source_type _source;
        destination_type _dest;
        unchecked_iterator_type _front;
        unchecked_iterator_type _back;
        unchecked_iterator_type _end;
        unchecked_out_iterator_type _out;

        constexpr explicit kernel(Source&& source, Destination&& dest)
            : _source(std::forward<Source>(source)),
              _dest(std::forward<Destination>(dest)),
              _front(_source.ubegin()), _back(std::ranges::next(_front, _source.uend())),
              _end(_back), _out(_dest.ubegin())
        {}

        raze_always_inline constexpr void operator()() {
            raze_disable_unrolling
            while (_back != _front) {
                --_back;
                *_out = *_back;
                ++_out;
            }
        }

        template <vectorizable_tag Tag>
        raze_always_inline void operator()(Tag, sizetype aligned_size)
            requires(vx::supports_fast_conversion<Tag, vtype_out>)
        {
            auto* back = std::to_address(_back);
            auto* out = std::to_address(_out);

            auto* aligned_begin = back;
            rewind_bytes(aligned_begin, aligned_size);

            raze_disable_unrolling do {
                rewind_bytes(back, sizeof(Tag));
                const auto vb = vx::reverse(vx::load<Tag>(back));
                vx::store(out, vx::convert<vtype_out>(vb));
                advance_bytes(out, Tag::size() * sizeof(vtype_out));
            } while (back != aligned_begin);

            source_type::from_ptr(_back, back);
            destination_type::from_ptr(_out, out);
        }

        constexpr raze_always_inline std::ranges::in_out_result<iterator_type, out_iterator_type> result() const {
            return { _source.wrap(_end), _dest.wrap(_out) };
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
        return this->dispatch(get_source(std::move(first), std::move(last)), get_destination(std::move(out)));
    }

    template <std::ranges::bidirectional_range R, std::weakly_incrementable Out>
    constexpr raze_always_inline std::ranges::in_out_result<std::ranges::borrowed_iterator_t<R>, Out>
    operator()(R&& r, Out out) const requires(std::indirectly_copyable<std::ranges::iterator_t<R>, Out>)
    {
        return this->dispatch(get_source(std::forward<R>(r)), get_destination(std::move(out)));
    }
};

constexpr inline auto reverse_copy = options::function_with_traits<reverse_copy_t>;

__RAZE_ALGORITHM_NAMESPACE_END