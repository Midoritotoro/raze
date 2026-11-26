#pragma once

#include <raze/vx/Algorithm.h>
#include <raze/algorithm/find/Equal.h>
#include <src/raze/algorithm/RangesSize.h>
#include <src/raze/algorithm/UncheckedAlgorithms.h>
#include <src/raze/algorithm/DataSource.h>
#include <src/raze/algorithm/VectorizablePredicate.h>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <bool IsFstatic, class T, class Predicate, class Projection1, class Projection2>
raze_never_inline bool __search_verify_tail_long_stateless(
    const T* cand_mid, const T* needle_mid, sizetype mid_len)
{
    if constexpr (IsFstatic) {
        return algorithm::equal[options::fstatic](
            cand_mid, cand_mid + mid_len, needle_mid, needle_mid + mid_len,
            Predicate{}, Projection1{}, Projection2{});
    }
    else {
        return algorithm::equal(
            cand_mid, cand_mid + mid_len, needle_mid, needle_mid + mid_len,
            Predicate{}, Projection1{}, Projection2{});
    }
}

template <bool IsFstatic, class T, class Predicate, class Projection1, class Projection2>
raze_never_inline bool __search_verify_tail_long(
    const T* cand_mid, const T* needle_mid, sizetype mid_len,
    Predicate& pred, Projection1& proj1, Projection2& proj2)
{
    if constexpr (IsFstatic) {
        return algorithm::equal[options::fstatic](
            cand_mid, cand_mid + mid_len, needle_mid, needle_mid + mid_len,
            pred, proj1, proj2);
    }
    else {
        return algorithm::equal(
            cand_mid, cand_mid + mid_len, needle_mid, needle_mid + mid_len,
            pred, proj1, proj2);
    }
}

template <class Traits>
struct search_t : Traits, dispatchable<search_t<Traits>> {
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

        using value_type = std::iter_value_t<unchecked_iterator1_type>;
        using vector_value_type = value_type;
        using traits_type = options::unwrap_traits_t<Traits>;
        using align = options::alignments<Traits, 2>;

        static consteval bool vectorizable() noexcept {
            return contiguous_source<source1_type> && contiguous_source<source2_type> &&
                !traits::is_iterator_volatile_v<iterator1_type> &&
                !traits::is_iterator_volatile_v<iterator2_type> &&
                std::same_as<value_type, std::iter_value_t<unchecked_iterator2_type>> &&
                vectorizable_binary_predicate<Predicate, unchecked_iterator1_type, unchecked_iterator2_type> &&
                vectorizable_projection<Projection1, unchecked_iterator1_type> &&
                vectorizable_projection<Projection2, unchecked_iterator2_type>;
        }

        raze_no_unique_address source1_type _source1;
        raze_no_unique_address source2_type _source2;

        unchecked_iterator1_type _iterator1;
        unchecked_iterator2_type _iterator2;
        raze_no_unique_address unchecked_sentinel1_type _sentinel1;
        raze_no_unique_address unchecked_sentinel2_type _sentinel2;

        raze_no_unique_address Predicate _predicate;
        raze_no_unique_address Projection1 _proj1;
        raze_no_unique_address Projection2 _proj2;

        unchecked_iterator1_type _match_begin;
        unchecked_iterator1_type _match_end;

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
              _proj2(std::move(proj2)),
              _match_begin(_iterator1),
              _match_end(_iterator1)
        {}

        raze_nodiscard constexpr raze_always_inline bool exit() const {
            if (_iterator2 == _sentinel2) return true;

            if constexpr (std::sized_sentinel_for<unchecked_sentinel1_type, unchecked_iterator1_type> &&
                          std::sized_sentinel_for<unchecked_sentinel2_type, unchecked_iterator2_type>)
            {
                return (_sentinel1 - _iterator1) < (_sentinel2 - _iterator2);
            }

            return false;
        }

        raze_nodiscard constexpr raze_always_inline std::ranges::subrange<iterator1_type> default_result() const {
            if (_iterator2 == _sentinel2) return { _source1.wrap(_iterator1), _source1.wrap(_iterator1) };

            auto end1 = std::ranges::next(_iterator1, _sentinel1);
            return { _source1.wrap(end1), _source1.wrap(end1) };
        }

        template <vectorizable_tag Tag>
        raze_always_inline bool verify_middle(
            const value_type* cand_mid, const value_type* needle_mid, sizetype mid_len)
        {
            if (mid_len == 0) return true;

            using SingleTag = vx::simd<value_type,
                vx::resize_abi_t<vx::abi_t<Tag>, Tag::size() / Tag::__chunks_count()>>;
            constexpr auto step = static_cast<sizetype>(SingleTag::size());

            while (mid_len >= step) {
                const auto mask = _predicate(
                    _proj1(vx::load<SingleTag>(cand_mid)),
                    _proj2(vx::load<SingleTag>(needle_mid)));

                if (!vx::all_of(mask)) return false;

                cand_mid += step;
                needle_mid += step;
                mid_len -= step;
            }

            raze_novec_loop((sizetype i = 0; i < mid_len; ++i), {
                if (!std::invoke(_predicate, std::invoke(_proj1, cand_mid[i]), std::invoke(_proj2, needle_mid[i])))
                    return false;
            })

            return true;
        }

        raze_always_inline bool verify_tail_middle(
            const value_type* cand_mid, const value_type* needle_mid, sizetype mid_len)
        {
            if (mid_len == 0) return true;

            if (mid_len * sizeof(value_type) < 32) {
                raze_novec_loop((sizetype i = 0; i < mid_len; ++i), {
                    if (!std::invoke(_predicate, std::invoke(_proj1, cand_mid[i]), std::invoke(_proj2, needle_mid[i])))
                        return false;
                })
                return true;
            }

            if constexpr (std::is_empty_v<Predicate> && std::is_default_constructible_v<Predicate> &&
                          std::is_empty_v<Projection1> && std::is_default_constructible_v<Projection1> &&
                          std::is_empty_v<Projection2> && std::is_default_constructible_v<Projection2>)
            {
                return __search_verify_tail_long_stateless<
                    options::is_fstatic<traits_type>(), value_type, Predicate, Projection1, Projection2>(
                    cand_mid, needle_mid, mid_len);
            }
            else {
                return __search_verify_tail_long<options::is_fstatic<traits_type>()>(
                    cand_mid, needle_mid, mid_len, _predicate, _proj1, _proj2);
            }
        }

        raze_always_inline constexpr void operator()() {
            if constexpr (contiguous_source<source1_type> && contiguous_source<source2_type>) {
                if !consteval {
                    if constexpr (!options::is_fscalar<traits_type>() && !options::is_fautovec<traits_type>()) {
                        if constexpr (vectorizable()) {
                            auto* ptr = std::to_address(_iterator1);
                            const auto end_ptr = ptr + (_sentinel1 - _iterator1);
                            const auto* needle = std::to_address(_iterator2);
                            const auto needle_len = static_cast<sizetype>(_sentinel2 - _iterator2);
                            const auto last_idx = needle_len - 1;
                            const auto stop_ptr = end_ptr - last_idx;
                            const auto needle_first = needle[0];
                            const auto needle_last = needle[last_idx];
                            const auto mid_len = needle_len >= 2 ? (needle_len - 2) : sizetype(0);

                            raze_novec_loop((; ptr != stop_ptr; ++ptr), {
                                if (std::invoke(_predicate, std::invoke(_proj1, ptr[0]), std::invoke(_proj2, needle_first)) &&
                                    (needle_len == 1 ||
                                     (std::invoke(_predicate, std::invoke(_proj1, ptr[last_idx]), std::invoke(_proj2, needle_last)) &&
                                      verify_tail_middle(ptr + 1, needle + 1, mid_len))))
                                {
                                    source1_type::from_ptr(_match_begin, ptr);
                                    source1_type::from_ptr(_match_end, ptr + needle_len);
                                    return;
                                }
                            })

                            source1_type::from_ptr(_match_begin, end_ptr);
                            source1_type::from_ptr(_match_end, end_ptr);
                            return;
                        }
                    }
                }
            }

            if constexpr (std::random_access_iterator<unchecked_iterator1_type> &&
                          std::sized_sentinel_for<unchecked_sentinel1_type, unchecked_iterator1_type> &&
                          std::sized_sentinel_for<unchecked_sentinel2_type, unchecked_iterator2_type>)
            {
                const auto needle_len = _sentinel2 - _iterator2;
                const auto end1 = std::ranges::next(_iterator1, _sentinel1);
                const auto stop1 = end1 - (needle_len - 1);

                for (auto cand = _iterator1; cand != stop1; ++cand) {
                    auto it1 = cand;
                    auto it2 = _iterator2;
                    bool matched = true;

                    raze_novec_loop((; it2 != _sentinel2; ++it1, ++it2), {
                        if (!std::invoke(_predicate, std::invoke(_proj1, *it1), std::invoke(_proj2, *it2))) {
                            matched = false;
                            break;
                        }
                    })

                    if (matched) {
                        _match_begin = cand;
                        _match_end = it1;
                        return;
                    }
                }

                _match_begin = end1;
                _match_end = end1;
            }
            else {
                for (auto cand = _iterator1;; ++cand) {
                    auto it1 = cand;
                    auto it2 = _iterator2;

                    for (;; ++it1, ++it2) {
                        if (it2 == _sentinel2) {
                            _match_begin = cand;
                            _match_end = it1;
                            return;
                        }

                        if (it1 == _sentinel1) {
                            _match_begin = it1;
                            _match_end = it1;
                            return;
                        }

                        if (!std::invoke(_predicate, std::invoke(_proj1, *it1), std::invoke(_proj2, *it2)))
                            break;
                    }
                }
            }
        }

        template <vectorizable_tag Tag>
        raze_always_inline bool operator()(Tag, sizetype aligned_size) {
            constexpr auto h_policy = align::template policy_for<0, Tag>;
            constexpr auto isa = vx::abi_t<Tag>::isa;

            auto* ptr = std::to_address(_iterator1);
            const auto aligned_end = bytes_pointer_offset(ptr, aligned_size);
            const auto* needle = std::to_address(_iterator2);
            const auto needle_len = static_cast<sizetype>(_sentinel2 - _iterator2);

            if (needle_len == 1) {
                const auto first_vec = _proj2(Tag(needle[0]));

                raze_disable_unrolling do {
                    const auto equal_first = _predicate(_proj1(vx::load<Tag>[h_policy](ptr)), first_vec);

                    if (vx::any_of(equal_first)) {
                        auto* cand = ptr + vx::find_first_set[vx::not_null](equal_first);
                        source1_type::from_ptr(_match_begin, cand);
                        source1_type::from_ptr(_match_end, cand + 1);
                        return false;
                    }

                    advance_bytes(ptr, sizeof(Tag));
                } while (ptr != aligned_end);

                source1_type::from_ptr(_iterator1, ptr);
                return true;
            }

            const auto last_offset = (needle_len - 1) * sizeof(value_type);
            const auto mid_len = needle_len - 2;
            const auto* needle_mid = needle + 1;
            const auto first_vec = _proj2(Tag(needle[0]));
            const auto last_vec = _proj2(Tag(needle[needle_len - 1]));

            raze_disable_unrolling do {
                const auto equal_first = _predicate(_proj1(vx::load<Tag>[h_policy](ptr)), first_vec);

                if (vx::none_of(equal_first)) {
                    advance_bytes(ptr, sizeof(Tag));
                    continue;
                }

                const auto equal_last = _predicate(
                    _proj1(vx::load<Tag>(bytes_pointer_offset(ptr, last_offset))), last_vec);
                const auto combined = equal_first & equal_last;

                if (vx::any_of(combined)) {
                    sizetype chunk_base = 0;

                    const bool found = combined.__for_each_chunk_any_of(
                        [&]<class Chunk>(const Chunk& chunk) raze_always_inline_lambda -> bool {
                            auto bits = vx::to_mask_<isa, value_type>(vx::ustorage(chunk));

                            while (bits != 0) {
                                const auto bit = static_cast<sizetype>(
                                    math::ctz_n_bits<isa, Chunk::size, true>(bits));
                                auto* cand = ptr + chunk_base + bit;

                                if (verify_middle<Tag>(cand + 1, needle_mid, mid_len)) {
                                    source1_type::from_ptr(_match_begin, cand);
                                    source1_type::from_ptr(_match_end, cand + needle_len);
                                    return true;
                                }

                                bits = static_cast<decltype(bits)>(bits & (bits - 1));
                            }

                            chunk_base += Chunk::size;
                            return false;
                        });

                    if (found) return false;
                }

                advance_bytes(ptr, sizeof(Tag));
            } while (ptr != aligned_end);

            source1_type::from_ptr(_iterator1, ptr);
            return true;
        }

        raze_nodiscard constexpr raze_always_inline std::ranges::subrange<iterator1_type> result() const {
            return { _source1.wrap(_match_begin), _source1.wrap(_match_end) };
        }

        constexpr void expects() const {
            align::verify(_source1, _source2);
        }

        raze_nodiscard static constexpr raze_always_inline decltype(auto) static_size()
            requires(constexpr_sized_source<source1_type> && constexpr_sized_source<source2_type>)
        {
            constexpr sizetype h = source1_type::static_size();
            constexpr sizetype n = source2_type::static_size();
            constexpr sizetype s = (n > 0 && h >= n) ? (h - n + sizeof(vector_value_type)) : 0;
            return std::integral_constant<sizetype, s>{};
        }

        raze_nodiscard constexpr raze_always_inline auto size() const {
            if constexpr (constexpr_sized_source<source1_type> && constexpr_sized_source<source2_type>) return static_size();
            else return (_source1.size() - _source2.size()) + sizeof(vector_value_type);
        }
    };

    template <std::forward_iterator It1, std::sentinel_for<It1> Sent1,
        std::forward_iterator It2, std::sentinel_for<It2> Sent2,
        class Predicate = std::equal_to<>, class Projection1 = std::identity,
        class Projection2 = std::identity>
    raze_nodiscard constexpr raze_always_inline std::ranges::subrange<It1>
    operator()(It1 first1, Sent1 last1, It2 first2, Sent2 last2,
        Predicate pred = {}, Projection1 proj1 = {}, Projection2 proj2 = {}) const
        requires(std::indirectly_comparable<It1, It2, Predicate, Projection1, Projection2>)
    {
        return this->dispatch(get_source(std::move(first1), std::move(last1)),
            get_source(std::move(first2), std::move(last2)),
            traits::fwd_fn(pred), traits::fwd_fn(proj1), traits::fwd_fn(proj2));
    }

    template <std::ranges::forward_range R1, std::ranges::forward_range R2,
        class Predicate = std::equal_to<>, class Projection1 = std::identity,
        class Projection2 = std::identity>
    raze_nodiscard constexpr raze_always_inline std::ranges::borrowed_subrange_t<R1>
    operator()(R1&& r1, R2&& r2,
        Predicate pred = {}, Projection1 proj1 = {}, Projection2 proj2 = {}) const
        requires(std::indirectly_comparable<std::ranges::iterator_t<R1>,
            std::ranges::iterator_t<R2>, Predicate, Projection1, Projection2>)
    {
        return this->dispatch(get_source(std::forward<R1>(r1)), get_source(std::forward<R2>(r2)),
            traits::fwd_fn(pred), traits::fwd_fn(proj1), traits::fwd_fn(proj2));
    }
};

/**
 * @ingroup raze_algo_subrange
 * @memberof raze::algorithm
 * @hideinitializer
 * @brief Searches for the first occurrence of a subsequence in a range.
 *
 * 1) Searches `[first1, last1)` for the first contiguous subsequence matching `[first2, last2)`
 *    element-wise under `pred`, `proj1`, and `proj2`.
 *
 * 2) Same as (1), but uses `r1` and `r2` as the input ranges, as if by:
 *    `search(std::ranges::begin(r1), std::ranges::end(r1), std::ranges::begin(r2), std::ranges::end(r2), pred, proj1, proj2)`.
 *
 * ### Declarations
 * ```cpp
 * // (1) Iterator-sentinel overload
 * template< std::forward_iterator It1, std::sentinel_for<It1> Sent1,
 *           std::forward_iterator It2, std::sentinel_for<It2> Sent2,
 *           class Predicate = std::equal_to<>,
 *           class Projection1 = std::identity, class Projection2 = std::identity >
 *   requires(std::indirectly_comparable<It1, It2, Predicate, Projection1, Projection2>)
 * constexpr std::ranges::subrange<It1>
 * search( It1 first1, Sent1 last1, It2 first2, Sent2 last2,
 *         Predicate pred = {}, Projection1 proj1 = {}, Projection2 proj2 = {} );
 *
 * // (2) Range overload
 * template< std::ranges::forward_range R1, std::ranges::forward_range R2,
 *           class Predicate = std::equal_to<>,
 *           class Projection1 = std::identity, class Projection2 = std::identity >
 *   requires(std::indirectly_comparable<std::ranges::iterator_t<R1>, std::ranges::iterator_t<R2>,
 *                                       Predicate, Projection1, Projection2>)
 * constexpr std::ranges::borrowed_subrange_t<R1>
 * search( R1&& r1, R2&& r2,
 *         Predicate pred = {}, Projection1 proj1 = {}, Projection2 proj2 = {} );
 * ```
 *
 * ### Parameters
 * - `first1`, `last1` - the range of elements to search in (haystack)
 * - `first2`, `last2` - the subsequence to search for (needle)
 * - `r1`              - the range of elements to search in (haystack)
 * - `r2`              - the subsequence to search for (needle)
 * - `pred`            - binary predicate applied to projected elements (defaults to `std::equal_to<>`)
 * - `proj1`, `proj2`  - projections to apply to elements of the first and second ranges (default to `std::identity`)
 *
 * ### Return value
 * A `std::ranges::subrange` spanning `[found_begin, found_end)` for the first matching subsequence,
 * `{first1, first1}` (or `{std::ranges::begin(r1), std::ranges::begin(r1)}`) if the pattern is empty,
 * or `{end1, end1}` if no match is found.
 *
 * ### Complexity
 * At most `N * M` applications of `pred`, `proj1`, and `proj2` in the worst case
 * (where `N = std::ranges::distance(first1, last1)` and `M = std::ranges::distance(first2, last2)`),
 * typically linear in `N` via SIMD two-anchor filtering.
 *
 * ### Options
 * Accepts standard modifiers from @ref raze_options (see @ref dynamic_dispatch and @ref vectorization_semantics).
 * - **`raze::options::aligned<...>`:** Supports broadcast `aligned<N>` and positional
 *   `aligned<Haystack, Needle>` (2 ranges). Aligned SIMD loads are applied to the haystack's first-anchor
 *   scan; offset anchor and middle-verification loads remain unaligned.
 *
 * ### Notes
 * - Explicit `raze::vx` SIMD accelerates matching-type contiguous ranges via two-anchor filtering
 *   (first and last pattern elements with bitmask candidate enumeration and SIMD middle verification),
 *   plus a dedicated single-element fast path.
 *
 * ### Example
 * ```cpp
 * #include <iostream>
 * #include <vector>
 * #include <raze/algorithm/find/Search.h>
 *
 * int main() {
 *     const std::vector<int> haystack{1, 2, 3, 4, 2, 3, 5};
 *     const std::vector<int> needle{2, 3};
 *
 *     auto found = raze::algorithm::search(haystack, needle);
 *     if (!found.empty())
 *         std::cout << "First match at index " << (found.begin() - haystack.begin()) << '\n';
 * }
 * ```
 *
 * Possible output:
 * ```text
 * First match at index 1
 * ```
 *
 * ### See also
 * - @ref raze_options
 * - @ref dynamic_dispatch
 * - @ref vectorization_semantics
 * - `raze::algorithm::find_end`
 * - `raze::algorithm::contains_subrange`
 * - `raze::algorithm::search_n`
 * - `raze::algorithm::find`
*/
constexpr inline auto search = options::function_with_traits<search_t>;

__RAZE_ALGORITHM_NAMESPACE_END
