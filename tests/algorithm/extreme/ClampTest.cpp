#include <tests/rts/rts.h>
#include <raze/algorithm/extreme/Clamp.h>

#include <algorithm>
#include <functional>
#include <random>

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

    const auto& r_eq = raze::algorithm::clamp(v, lo, hi, std::less<>{}, &rtts::test_item::key);
    RTTS_EXPECT(&r_eq == &v);

    const auto& r_lo = raze::algorithm::clamp(low_v, lo, hi, std::less<>{}, &rtts::test_item::key);
    RTTS_EXPECT(&r_lo == &lo);

    const auto& r_hi = raze::algorithm::clamp(high_v, lo, hi, std::less<>{}, &rtts::test_item::key);
    RTTS_EXPECT(&r_hi == &hi);
};

namespace {
    inline constexpr unsigned fuzz_seeds[] = { 42u, 0x9E3779B9u, 0xDEADBEEFu };

    template <class T>
    void clamp_fuzz(unsigned seed, size_t iters) {
        namespace opt = raze::options;
        std::mt19937 rng(seed);
        rtts::random::generator<T> val_gen(seed ^ 0x5A5A5A5Au);

        for (size_t it = 0; it < iters; ++it) {
            T lo = val_gen();
            T hi = (it % 7 == 0) ? lo : val_gen();
            if (hi < lo) std::swap(lo, hi);

            T v = (it % 11 == 0) ? lo : (it % 11 == 1) ? hi : val_gen();

            const T exp_lt = std::ranges::clamp(v, lo, hi);
            RTTS_EQUAL(raze::algorithm::clamp(v, lo, hi), exp_lt);
            RTTS_EQUAL(raze::algorithm::clamp[opt::fscalar](v, lo, hi), exp_lt);
            RTTS_EQUAL(raze::algorithm::clamp[opt::fstatic](v, lo, hi), exp_lt);

            const T exp_gt = std::ranges::clamp(v, hi, lo, std::greater<>{});
            RTTS_EQUAL(raze::algorithm::clamp(v, hi, lo, std::greater<>{}), exp_gt);
            RTTS_EQUAL(raze::algorithm::clamp[opt::fscalar](v, hi, lo, std::greater<>{}), exp_gt);
        }
    }
}

RTTS_CASE_TPL("raze::algorithm::clamp fuzz", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    for (unsigned seed : fuzz_seeds)
        clamp_fuzz<T>(seed, 200);
};

RTTS_CASE("raze::algorithm::clamp reference identity fuzz") {
    for (unsigned seed : fuzz_seeds) {
        std::mt19937 rng(seed);
        for (size_t it = 0; it < 200; ++it) {
            int k1 = static_cast<int>(rng() % 20u);
            int k2 = static_cast<int>(rng() % 20u);
            if (k2 < k1) std::swap(k1, k2);
            int kv = static_cast<int>(rng() % 20u);

            const rtts::test_item v{ kv, 1 };
            const rtts::test_item lo{ k1, 2 };
            const rtts::test_item hi{ k2, 3 };

            const auto& got = raze::algorithm::clamp(v, lo, hi, std::less<>{}, &rtts::test_item::key);
            const auto& exp = std::ranges::clamp(v, lo, hi, std::less<>{}, &rtts::test_item::key);
            RTTS_EXPECT(&got == &exp);
        }
    }
};
