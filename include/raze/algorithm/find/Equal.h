#pragma once

#include <raze/vx/Algorithm.h>
#include <src/raze/algorithm/RangesSize.h>
#include <src/raze/algorithm/UncheckedAlgorithms.h>
#include <src/raze/algorithm/DataSource.h>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <class Traits>
struct equal_t : Traits, dispatchable<equal_t<Traits>> {
    template <source Source1, source Source2,
        class Predicate, class Projection1, class Projection2>
    struct kernel {
        using source1_type = std::remove_cvref_t<Source1>;
        using source2_type = std::remove_cvref_t<Source2>;

        using iterator1_type = typename source1_type::iterator_type;
        using iterator2_type = typename source2_type::iterator_type;

        using unchecked_iterator1_type = typename source1_type::unchecked_iterator_type;
        using unchecked_iterator2_type = typename source2_type::unchecked_iterator_type;

        using unchecked_sentinel1_type = typename source1_type::unchecked_sentinel_type;
        using unchecked_sentinel2_type = typename source2_type::unchecked_sentinel_type;

        using vector_value_type = std::iter_value_t<unchecked_iterator1_type>;

        source1_type _source1;
        source2_type _source2;

        unchecked_iterator1_type _iterator1;
        unchecked_iterator2_type _iterator2;
        unchecked_sentinel1_type _sentinel1;
        unchecked_sentinel2_type _sentinel2;

        raze_no_unique_address Predicate _predicate;
        raze_no_unique_address Projection1 _proj1;
        raze_no_unique_address Projection2 _proj2;

        bool _result = true;

        static consteval bool vectorizable() noexcept {
            return contiguous_source<source1_type> && contiguous_source<source2_type>
                && !traits::is_iterator_volatile_v<iterator1_type> && !traits::is_iterator_volatile_v<iterator2_type>
                && std::same_as<std::iter_value_t<unchecked_iterator1_type>, std::iter_value_t<unchecked_iterator2_type>>
                && vectorizable_binary_predicate<Predicate, unchecked_iterator1_type, unchecked_iterator2_type>
                && vectorizable_projection<Projection1, unchecked_iterator1_type>
                && vectorizable_projection<Projection2, unchecked_iterator2_type>;
        }

        constexpr explicit kernel(Source1&& src1, Source2&& src2,
            Predicate pred, Projection1 proj1, Projection2 proj2)
            : _source1(std::forward<Source1>(src1)),
              _source2(std::forward<Source2>(src2)),
              _iterator1(_source1.ubegin()),
              _iterator2(_source2.ubegin()),
              _sentinel1(_source1.uend()),
              _sentinel2(_source2.uend()),
              _predicate(std::move(pred)),
              _proj1(std::move(proj1)),
              _proj2(std::move(proj2))
        {}

        constexpr raze_always_inline void operator()() {
            if constexpr (std::sized_sentinel_for<unchecked_sentinel1_type, unchecked_iterator1_type>
                && std::sized_sentinel_for<unchecked_sentinel2_type, unchecked_iterator2_type>)
            {
                raze_disable_unrolling
                for (; _iterator1 != _sentinel1; ++_iterator1, ++_iterator2) {
                    if (!_predicate(_proj1(*_iterator1), _proj2(*_iterator2))) {
                        _result = false;
                        return;
                    }
                }
            }
            else {
                raze_disable_unrolling
                for (; _iterator1 != _sentinel1 && _iterator2 != _sentinel2; ++_iterator1, ++_iterator2) {
                    if (!_predicate(_proj1(*_iterator1), _proj2(*_iterator2))) {
                        _result = false;
                        return;
                    }
                }

                _result = (_iterator1 == _sentinel1 && _iterator2 == _sentinel2);
            }
        }

        template <vectorizable_tag Tag>
        raze_always_inline bool operator()(Tag, sizetype aligned_size) {
            auto* ptr1 = std::to_address(_iterator1);
            auto* ptr2 = std::to_address(_iterator2);

            const auto aligned_end = bytes_pointer_offset(ptr1, aligned_size);

            raze_disable_unrolling do {
                const auto mask = _predicate(_proj1(vx::load<Tag>(ptr1)), _proj2(vx::load<Tag>(ptr2)));

                if (!vx::all_of(mask)) {
                    _result = false;
                    return false;
                }

                advance_bytes(ptr1, ptr2, sizeof(Tag));
            } while (ptr1 != aligned_end);

            source1_type::from_ptr(_iterator1, ptr1);
            source2_type::from_ptr(_iterator2, ptr2);

            return true;
        }

        template <vectorizable_tag Tag>
        raze_always_inline void operator()(Tag, tail_mask_type auto const& ignore) {
            auto* ptr1 = std::to_address(_iterator1);
            auto* ptr2 = std::to_address(_iterator2);

            _result = vx::all_of[ignore](_predicate(_proj1(vx::load<Tag>[ignore](ptr1)),
                _proj2(vx::load<Tag>[ignore](ptr2))));

            advance_bytes(ptr1, ptr2, ignore.tail_bytes());
            source1_type::from_ptr(_iterator1, ptr1);
            source2_type::from_ptr(_iterator2, ptr2);
        }

        raze_nodiscard static constexpr raze_always_inline decltype(auto) static_size()
            requires(constexpr_sized_source<source1_type> || constexpr_sized_source<source2_type>)
        {
            if constexpr (constexpr_sized_source<source1_type>) return source1_type::static_size();
            else return source2_type::static_size();
        }

        raze_nodiscard constexpr raze_always_inline auto size() const {
            return _source1.size();
        }

        raze_nodiscard constexpr raze_always_inline bool exit() const requires(std::sized_sentinel_for<unchecked_sentinel1_type, unchecked_iterator1_type>
            && std::sized_sentinel_for<unchecked_sentinel2_type, unchecked_iterator2_type>)
        {
            return (_sentinel1 - _iterator1) != (_sentinel2 - _iterator2);
        }

        raze_nodiscard constexpr raze_always_inline bool result() const noexcept {
            return _result;
        }

        raze_nodiscard constexpr raze_always_inline bool default_result() const noexcept {
            return false;
        }
    };

    template <std::input_iterator It1, std::sentinel_for<It1> Sent1, std::input_iterator It2,
        std::sentinel_for<It2> Sent2, class Predicate = std::equal_to<>, class Projection1 = std::identity,
        class Projection2 = std::identity>
    raze_nodiscard constexpr raze_always_inline bool operator()(It1 first1, Sent1 sent1, It2 first2, Sent2 sent2,
        Predicate pred = {}, Projection1 proj1 = {}, Projection2 proj2 = {}) const
            requires(std::indirectly_comparable<It1, It2, Predicate, Projection1, Projection2>)
    {
        return this->dispatch(get_source(std::move(first1), std::move(sent1)),
            get_source(std::move(first2), std::move(sent2)),
            traits::fwd_fn(pred), traits::fwd_fn(proj1), traits::fwd_fn(proj2));
    }

    template <std::ranges::input_range R1, std::ranges::input_range R2,
        class Predicate = std::equal_to<>, class Projection1 = std::identity,
        class Projection2 = std::identity>
    raze_nodiscard constexpr raze_always_inline bool operator()(R1&& r1, R2&& r2,
        Predicate pred = {}, Projection1 proj1 = {}, Projection2 proj2 = {}) const
        requires(std::indirectly_comparable<std::ranges::iterator_t<R1>, 
            std::ranges::iterator_t<R2>, Predicate, Projection1, Projection2>)
    {
        return this->dispatch(get_source(std::forward<R1>(r1)), get_source(std::forward<R2>(r2)),
            traits::fwd_fn(pred), traits::fwd_fn(proj1), traits::fwd_fn(proj2));
    }
};

constexpr inline auto equal = options::function_with_traits<equal_t>;

__RAZE_ALGORITHM_NAMESPACE_END