#include <tests/rts/rts.h>
#include <raze/algorithm/fill/Fill.h>

RTTS_CASE_TPL("raze::algorithm::fill", rtts::algorithm::all_types)
<class T> (rtts::type<T>) {
    auto cfg = rtts::algorithm::config::thorough();

    {
        const T fill_val = rtts::random::generator<T>(42)();

        rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
            [fill_val](auto c) { return raze::algorithm::fill(c.first(), c.last(), fill_val); },
            [fill_val](auto c) { return std::ranges::fill(c.first(), c.last(), fill_val); });

        rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
            [fill_val](auto c) { return raze::algorithm::fill(c.range(), fill_val); },
            [fill_val](auto c) { return std::ranges::fill(c.range(), fill_val); });
    }

    {
        const T zero_val = T(0);

        rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
            [zero_val](auto c) { return raze::algorithm::fill(c.first(), c.last(), zero_val); },
            [zero_val](auto c) { return std::ranges::fill(c.first(), c.last(), zero_val); });

        rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
            [zero_val](auto c) { return raze::algorithm::fill(c.range(), zero_val); },
            [zero_val](auto c) { return std::ranges::fill(c.range(), zero_val); });
    }

    {
        const T special_val = T(42);

        rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
            [special_val](auto c) { return raze::algorithm::fill(c.first(), c.last(), special_val); },
            [special_val](auto c) { return std::ranges::fill(c.first(), c.last(), special_val); });

        rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
            [special_val](auto c) { return raze::algorithm::fill(c.range(), special_val); },
            [special_val](auto c) { return std::ranges::fill(c.range(), special_val); });
    }

    static_assert(rtts::algorithm::constexpr_run<T, rtts::algorithm::range, 8>(
        [](auto c) constexpr { return raze::algorithm::fill(c.first(), c.last(), T(99)); },
        [](auto c) constexpr { return std::ranges::fill(c.first(), c.last(), T(99)); }));

    static_assert(rtts::algorithm::constexpr_run<T, rtts::algorithm::range, 8>(
        [](auto c) constexpr { return raze::algorithm::fill(c.range(), T(99)); },
        [](auto c) constexpr { return std::ranges::fill(c.range(), T(99)); }));
};