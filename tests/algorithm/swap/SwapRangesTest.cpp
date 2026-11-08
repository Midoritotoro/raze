#include <tests/rts/rts.h>
#include <raze/algorithm/swap/Swap.h>
#include <bit>
#include <cstdint>

RTTS_CASE_TPL("raze::algorithm::swap_ranges", rtts::algorithm::all_types)
< class T > (rtts::type<T>) {
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
            [](auto c) { return raze::algorithm::swap_ranges[raze::options::unroll<2>](c.range(), c.range2()); },
            [](auto c) { return std::ranges::swap_ranges(c.range(), c.range2()); });

        rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
            [](auto c) { return raze::algorithm::swap_ranges[raze::options::fstatic](c.range(), c.range2()); },
            [](auto c) { return std::ranges::swap_ranges(c.range(), c.range2()); });
    }

    static_assert(rtts::algorithm::constexpr_run<T, rtts::algorithm::range, 8>(
        [](auto c) constexpr { return raze::algorithm::swap_ranges(c.first(), c.last(), c.first2(), c.last2()); },
        [](auto c) constexpr { return std::ranges::swap_ranges(c.first(), c.last(), c.first2(), c.last2()); }));

    static_assert(rtts::algorithm::constexpr_run<T, rtts::algorithm::range, 8>(
        [](auto c) constexpr { return raze::algorithm::swap_ranges(c.range(), c.range2()); },
        [](auto c) constexpr { return std::ranges::swap_ranges(c.range(), c.range2()); }));
};

RTTS_CASE("raze::algorithm::swap_ranges mixed arithmetic types") {
    auto cfg = rtts::algorithm::config::thorough();

    {
        rtts::algorithm::run<rtts::algorithm::range, int32_t, float>(cfg,
            [](auto c) { return raze::algorithm::swap_ranges(c.first(), c.last(), c.first2(), c.last2()); },
            [](auto c) { return std::ranges::swap_ranges(c.first(), c.last(), c.first2(), c.last2()); });

        rtts::algorithm::run<rtts::algorithm::range, float, int32_t>(cfg,
            [](auto c) { return raze::algorithm::swap_ranges(c.first(), c.last(), c.first2(), c.last2()); },
            [](auto c) { return std::ranges::swap_ranges(c.first(), c.last(), c.first2(), c.last2()); });
    }

    {
        rtts::algorithm::run<rtts::algorithm::range, int64_t, double>(cfg,
            [](auto c) { return raze::algorithm::swap_ranges(c.first(), c.last(), c.first2(), c.last2()); },
            [](auto c) { return std::ranges::swap_ranges(c.first(), c.last(), c.first2(), c.last2()); });

        rtts::algorithm::run<rtts::algorithm::range, double, int64_t>(cfg,
            [](auto c) { return raze::algorithm::swap_ranges(c.first(), c.last(), c.first2(), c.last2()); },
            [](auto c) { return std::ranges::swap_ranges(c.first(), c.last(), c.first2(), c.last2()); });
    }

    {
        rtts::algorithm::run<rtts::algorithm::range, int8_t, int16_t>(cfg,
            [](auto c) { return raze::algorithm::swap_ranges(c.first(), c.last(), c.first2(), c.last2()); },
            [](auto c) { return std::ranges::swap_ranges(c.first(), c.last(), c.first2(), c.last2()); });
    }

    {
        rtts::algorithm::run<rtts::algorithm::range, uint32_t, float>(cfg,
            [](auto c) { return raze::algorithm::swap_ranges(c.first(), c.last(), c.first2(), c.last2()); },
            [](auto c) { return std::ranges::swap_ranges(c.first(), c.last(), c.first2(), c.last2()); });
    }

    {
        rtts::algorithm::run<rtts::algorithm::range, int32_t, float>(cfg,
            [](auto c) { return raze::algorithm::swap_ranges[raze::options::fscalar](c.first(), c.last(), c.first2(), c.last2()); },
            [](auto c) { return std::ranges::swap_ranges(c.first(), c.last(), c.first2(), c.last2()); });
    }

    static_assert(rtts::algorithm::constexpr_run_mixed<int32_t, float, rtts::algorithm::range, 8>(
        [](auto c) constexpr { return raze::algorithm::swap_ranges(c.first(), c.last(), c.first2(), c.last2()); },
        [](auto c) constexpr { return std::ranges::swap_ranges(c.first(), c.last(), c.first2(), c.last2()); }));

    static_assert(rtts::algorithm::constexpr_run_mixed<int64_t, double, rtts::algorithm::range, 8>(
        [](auto c) constexpr { return raze::algorithm::swap_ranges(c.first(), c.last(), c.first2(), c.last2()); },
        [](auto c) constexpr { return std::ranges::swap_ranges(c.first(), c.last(), c.first2(), c.last2()); }));
};

struct tc_struct_4 {
    int32_t x;
    bool operator==(const tc_struct_4&) const = default;
};
struct tc_struct_8 {
    int64_t x;
    bool operator==(const tc_struct_8&) const = default;
};

RTTS_CASE("raze::algorithm::swap_ranges structs") {
    auto cfg = rtts::algorithm::config::thorough();

    {
        rtts::algorithm::run<rtts::algorithm::range, tc_struct_4>(cfg,
            [](auto c) { return raze::algorithm::swap_ranges(c.first(), c.last(), c.first2(), c.last2()); },
            [](auto c) { return std::ranges::swap_ranges(c.first(), c.last(), c.first2(), c.last2()); });

        rtts::algorithm::run<rtts::algorithm::range, tc_struct_8>(cfg,
            [](auto c) { return raze::algorithm::swap_ranges(c.first(), c.last(), c.first2(), c.last2()); },
            [](auto c) { return std::ranges::swap_ranges(c.first(), c.last(), c.first2(), c.last2()); });
    }

    {
        rtts::algorithm::run<rtts::algorithm::range, tc_struct_4>(cfg,
            [](auto c) { return raze::algorithm::swap_ranges[raze::options::fscalar](c.first(), c.last(), c.first2(), c.last2()); },
            [](auto c) { return std::ranges::swap_ranges(c.first(), c.last(), c.first2(), c.last2()); });

        rtts::algorithm::run<rtts::algorithm::range, tc_struct_4>(cfg,
            [](auto c) { return raze::algorithm::swap_ranges[raze::options::unroll<2>](c.first(), c.last(), c.first2(), c.last2()); },
            [](auto c) { return std::ranges::swap_ranges(c.first(), c.last(), c.first2(), c.last2()); });
    }

    static_assert(rtts::algorithm::constexpr_run<tc_struct_4, rtts::algorithm::range, 8>(
        [](auto c) constexpr { return raze::algorithm::swap_ranges(c.first(), c.last(), c.first2(), c.last2()); },
        [](auto c) constexpr { return std::ranges::swap_ranges(c.first(), c.last(), c.first2(), c.last2()); }));

    static_assert(rtts::algorithm::constexpr_run<tc_struct_8, rtts::algorithm::range, 8>(
        [](auto c) constexpr { return raze::algorithm::swap_ranges(c.first(), c.last(), c.first2(), c.last2()); },
        [](auto c) constexpr { return std::ranges::swap_ranges(c.first(), c.last(), c.first2(), c.last2()); }));
};