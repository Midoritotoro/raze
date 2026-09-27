#include <tests/rts/rts.h>
#include <raze/algorithm/swap/Swap.h>

RTTS_CASE_TPL("raze::algorithm::swap_ranges", rtts::algorithm::all_types)
<class T> (rtts::type<T>) {
    auto cfg = rtts::algorithm::config::thorough();

    {
        rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
            [](auto c) { return raze::algorithm::swap_ranges(c.first(), c.last(), c.first2(), c.last2()); },
            [](auto c) { return std::ranges::swap_ranges(c.first(), c.last(), c.first2(), c.last2()); });

        rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
            [](auto c) { return raze::algorithm::swap_ranges(c.range(), c.range2()); },
            [](auto c) { return std::ranges::swap_ranges(c.range(), c.range2()); });
    }

    static_assert(rtts::algorithm::constexpr_run<T, rtts::algorithm::range, 8>(
        [](auto c) constexpr { return raze::algorithm::swap_ranges(c.first(), c.last(), c.first2(), c.last2()); },
        [](auto c) constexpr { return std::ranges::swap_ranges(c.first(), c.last(), c.first2(), c.last2()); }));

    static_assert(rtts::algorithm::constexpr_run<T, rtts::algorithm::range, 8>(
        [](auto c) constexpr { return raze::algorithm::swap_ranges(c.range(), c.range2()); },
        [](auto c) constexpr { return std::ranges::swap_ranges(c.range(), c.range2()); }));
};