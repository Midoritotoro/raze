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

    {
        rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
            [](auto c) { return raze::algorithm::swap_ranges[raze::options::fscalar](c.first(), c.last(), c.first2(), c.last2()); },
            [](auto c) { return std::ranges::swap_ranges(c.first(), c.last(), c.first2(), c.last2()); });

        rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
            [](auto c) { return raze::algorithm::swap_ranges[raze::options::fscalar](c.range(), c.range2()); },
            [](auto c) { return std::ranges::swap_ranges(c.range(), c.range2()); });

        rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
            [](auto c) { return raze::algorithm::swap_ranges[raze::options::unroll<1>](c.range(), c.range2()); },
            [](auto c) { return std::ranges::swap_ranges(c.range(), c.range2()); });

        rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
            [](auto c) { return raze::algorithm::swap_ranges[raze::options::unroll<2>](c.range(), c.range2()); },
            [](auto c) { return std::ranges::swap_ranges(c.range(), c.range2()); });

        rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
            [](auto c) { return raze::algorithm::swap_ranges[raze::options::unroll<4>](c.range(), c.range2()); },
            [](auto c) { return std::ranges::swap_ranges(c.range(), c.range2()); });

        rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
            [](auto c) { return raze::algorithm::swap_ranges[raze::options::unroll<8>](c.range(), c.range2()); },
            [](auto c) { return std::ranges::swap_ranges(c.range(), c.range2()); });

        rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
            [](auto c) { return raze::algorithm::swap_ranges[raze::options::fstatic](c.range(), c.range2()); },
            [](auto c) { return std::ranges::swap_ranges(c.range(), c.range2()); });

        rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
            [](auto c) { return raze::algorithm::swap_ranges[raze::options::fscalar][raze::options::unroll<2>](c.range(), c.range2()); },
            [](auto c) { return std::ranges::swap_ranges(c.range(), c.range2()); });

        rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
            [](auto c) { return raze::algorithm::swap_ranges[raze::options::fstatic][raze::options::unroll<2>](c.range(), c.range2()); },
            [](auto c) { return std::ranges::swap_ranges(c.range(), c.range2()); });
    }

    static_assert(rtts::algorithm::constexpr_run<T, rtts::algorithm::range, 8>(
        [](auto c) constexpr { return raze::algorithm::swap_ranges(c.first(), c.last(), c.first2(), c.last2()); },
        [](auto c) constexpr { return std::ranges::swap_ranges(c.first(), c.last(), c.first2(), c.last2()); }));

    static_assert(rtts::algorithm::constexpr_run<T, rtts::algorithm::range, 8>(
        [](auto c) constexpr { return raze::algorithm::swap_ranges(c.range(), c.range2()); },
        [](auto c) constexpr { return std::ranges::swap_ranges(c.range(), c.range2()); }));

    static_assert(rtts::algorithm::constexpr_run<T, rtts::algorithm::range, 8>(
        [](auto c) constexpr { return raze::algorithm::swap_ranges[raze::options::fscalar](c.range(), c.range2()); },
        [](auto c) constexpr { return std::ranges::swap_ranges(c.range(), c.range2()); }));

    static_assert(rtts::algorithm::constexpr_run<T, rtts::algorithm::range, 8>(
        [](auto c) constexpr { return raze::algorithm::swap_ranges[raze::options::unroll<2>](c.range(), c.range2()); },
        [](auto c) constexpr { return std::ranges::swap_ranges(c.range(), c.range2()); }));
};