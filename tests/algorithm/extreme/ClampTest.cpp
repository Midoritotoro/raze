#include <tests/rts/rts.h>
#include <raze/algorithm/extreme/Clamp.h>

#include <algorithm>
#include <functional>

RTTS_CASE_TPL("raze::algorithm::clamp", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    namespace opt = raze::options;

    const T lo = T(10);
    const T hi = T(50);

    for (int raw : { 0, 9, 10, 11, 30, 49, 50, 51, 100 }) {
        const T v = static_cast<T>(raw);
        RTTS_EQUAL(raze::algorithm::clamp(v, lo, hi), std::ranges::clamp(v, lo, hi));
        RTTS_EQUAL(raze::algorithm::clamp[opt::fscalar](v, lo, hi), std::ranges::clamp(v, lo, hi));
        RTTS_EQUAL(raze::algorithm::clamp(v, hi, lo, std::greater<>{}), std::ranges::clamp(v, hi, lo, std::greater<>{}));
    }

    static_assert(raze::algorithm::clamp(T(5), T(10), T(20)) == T(10));
    static_assert(raze::algorithm::clamp(T(15), T(10), T(20)) == T(15));
    static_assert(raze::algorithm::clamp(T(25), T(10), T(20)) == T(20));
    static_assert(raze::algorithm::clamp(T(10), T(10), T(20)) == T(10));
    static_assert(raze::algorithm::clamp(T(20), T(10), T(20)) == T(20));
};

RTTS_CASE("raze::algorithm::clamp reference identity and projections") {
    const rtts::test_item v{ 10, 1 };
    const rtts::test_item lo{ 10, 2 };
    const rtts::test_item hi{ 20, 3 };
    const rtts::test_item low_v{ 5, 4 };
    const rtts::test_item high_v{ 25, 5 };

    // Equivalent to lo -> returns v (same as std::ranges::clamp)
    const auto& r_eq = raze::algorithm::clamp(v, lo, hi, std::less<>{}, &rtts::test_item::key);
    RTTS_EXPECT(&r_eq == &v);

    const auto& r_lo = raze::algorithm::clamp(low_v, lo, hi, std::less<>{}, &rtts::test_item::key);
    RTTS_EXPECT(&r_lo == &lo);

    const auto& r_hi = raze::algorithm::clamp(high_v, lo, hi, std::less<>{}, &rtts::test_item::key);
    RTTS_EXPECT(&r_hi == &hi);
};

