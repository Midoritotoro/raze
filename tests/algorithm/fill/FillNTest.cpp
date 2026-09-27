#include <tests/rts/rts.h>
#include <raze/algorithm/fill/FillN.h>

template <class It, class Size, class T>
constexpr auto std_fill_n(It first, Size count, const T& value) {
    return std::ranges::fill_n(std::move(first), static_cast<std::iter_difference_t<It>>(count), value);
}

RTTS_CASE_TPL("raze::algorithm::fill_n", rtts::algorithm::all_types)
<class T> (rtts::type<T>) {
    auto cfg = rtts::algorithm::config::thorough();

    {
        const T fill_val = rtts::random::generator<T>(42)();

        rtts::algorithm::run<rtts::algorithm::n, T>(cfg,
            [fill_val](auto c) { return raze::algorithm::fill_n(c.first(), c.count(), fill_val); },
            [fill_val](auto c) { return std_fill_n(c.first(), c.count(), fill_val); });
    }

    {
        const T zero_val = T(0);

        rtts::algorithm::run<rtts::algorithm::n, T>(cfg,
            [zero_val](auto c) { return raze::algorithm::fill_n(c.first(), c.count(), zero_val); },
            [zero_val](auto c) { return std_fill_n(c.first(), c.count(), zero_val); });
    }

    {
        const T special_val = T(99);

        rtts::algorithm::run<rtts::algorithm::n, T>(cfg,
            [special_val](auto c) { return raze::algorithm::fill_n(c.first(), c.count(), special_val); },
            [special_val](auto c) { return std_fill_n(c.first(), c.count(), special_val); });
    }

    static_assert(rtts::algorithm::constexpr_run<T, rtts::algorithm::n, 8, 5>(
        [](auto c) constexpr { return raze::algorithm::fill_n(c.first(), c.count(), T(77)); },
        [](auto c) constexpr { return std_fill_n(c.first(), c.count(), T(77)); }));

    static_assert(rtts::algorithm::constexpr_run<T, rtts::algorithm::n, 8, 0>(
        [](auto c) constexpr { return raze::algorithm::fill_n(c.first(), c.count(), T(77)); },
        [](auto c) constexpr { return std_fill_n(c.first(), c.count(), T(77)); }));
};