#include <tests/rts/rts.h>
#include <raze/algorithm/order/Shuffle.h>

#include <algorithm>
#include <array>
#include <deque>
#include <random>
#include <ranges>
#include <vector>

RTTS_CASE_TPL("raze::algorithm::shuffle", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    namespace opt = raze::options;

    for (size_t sz : { size_t(0), size_t(1), size_t(2), size_t(5), size_t(32), size_t(128) }) {
        std::vector<T> orig(sz);
        for (size_t i = 0; i < sz; ++i)
            orig[i] = static_cast<T>((i % 97) + 1);

        auto v1 = orig;
        auto v2 = orig;
        std::mt19937 g1(12345), g2(12345);

        auto e1 = raze::algorithm::shuffle(v1, g1);
        auto e2 = raze::algorithm::shuffle[opt::fscalar](v2.begin(), v2.end(), g2);

        RTTS_EXPECT(e1 == v1.end() && e2 == v2.end());
        RTTS_ALL_EQUAL(v1, v2);
        RTTS_EXPECT(std::ranges::is_permutation(v1, orig));
    }
};

RTTS_CASE("raze::algorithm::shuffle deque, non-common sentinel, distribution, and dangling") {
    {
        std::vector<int> v(100);
        std::iota(v.begin(), v.end(), 0);
        auto orig = v;
        std::mt19937 rng(42);
        raze::algorithm::shuffle(v, rng);
        RTTS_EXPECT(std::ranges::is_permutation(v, orig));
        RTTS_EXPECT(v != orig);
    }

    {
        std::deque<int> dq{ 1, 2, 3, 4, 5, 6, 7, 8 };
        auto orig = dq;
        std::mt19937 rng(7);
        auto it = raze::algorithm::shuffle(rtts::make_non_common(dq), rng);
        RTTS_EXPECT(it == dq.end());
        RTTS_EXPECT(std::ranges::is_permutation(dq, orig));

        auto dr = raze::algorithm::shuffle(std::vector<int>{ 1, 2, 3 }, rng);
        static_assert(std::same_as<decltype(dr), std::ranges::dangling>);
    }
};

namespace {
    inline constexpr unsigned fuzz_seeds[] = { 42u, 0x9E3779B9u, 0xDEADBEEFu };

    template <class T>
    void shuffle_fuzz(unsigned seed, size_t iters) {
        namespace opt = raze::options;
        std::mt19937 rng(seed);

        for (size_t it = 0; it < iters; ++it) {
            const size_t n = static_cast<size_t>(rng() % 257);
            const auto base = rtts::random::vector<T>(n, rng());
            const size_t lo = n ? static_cast<size_t>(rng() % (n + 1)) : 0;
            const size_t hi = n ? (lo + static_cast<size_t>(rng() % (n - lo + 1))) : 0;
            const unsigned gseed = rng();

            auto v1 = base, v2 = base, v3 = base, v4 = base;
            std::mt19937 g1(gseed), g2(gseed), g3(gseed), g4(gseed);

            auto s1 = std::ranges::subrange(v1.begin() + static_cast<std::ptrdiff_t>(lo),
                                            v1.begin() + static_cast<std::ptrdiff_t>(hi));
            auto s2 = std::ranges::subrange(v2.begin() + static_cast<std::ptrdiff_t>(lo),
                                            v2.begin() + static_cast<std::ptrdiff_t>(hi));
            auto s3 = std::ranges::subrange(v3.begin() + static_cast<std::ptrdiff_t>(lo),
                                            v3.begin() + static_cast<std::ptrdiff_t>(hi));
            auto s4 = std::ranges::subrange(v4.begin() + static_cast<std::ptrdiff_t>(lo),
                                            v4.begin() + static_cast<std::ptrdiff_t>(hi));

            auto e1 = raze::algorithm::shuffle(s1, g1);
            auto e2 = raze::algorithm::shuffle(s2.begin(), s2.end(), g2);
            auto e3 = raze::algorithm::shuffle[opt::fscalar](s3, g3);
            auto e4 = raze::algorithm::shuffle[opt::fstatic](s4, g4);

            RTTS_EXPECT(e1 == s1.end() && e2 == s2.end() && e3 == s3.end() && e4 == s4.end());
            RTTS_ALL_EQUAL(v1, v2);
            RTTS_ALL_EQUAL(v1, v3);
            RTTS_ALL_EQUAL(v1, v4);
            const auto exp_next = g1();
            RTTS_EQUAL(g2(), exp_next);
            RTTS_EQUAL(g3(), exp_next);
            RTTS_EQUAL(g4(), exp_next);

            RTTS_EXPECT(std::equal(v1.begin(), v1.begin() + static_cast<std::ptrdiff_t>(lo), base.begin()));
            RTTS_EXPECT(std::equal(v1.begin() + static_cast<std::ptrdiff_t>(hi), v1.end(),
                                   base.begin() + static_cast<std::ptrdiff_t>(hi)));
            std::vector<T> got_sub(s1.begin(), s1.end());
            std::vector<T> exp_sub(base.begin() + static_cast<std::ptrdiff_t>(lo),
                                   base.begin() + static_cast<std::ptrdiff_t>(hi));
            std::ranges::sort(got_sub);
            std::ranges::sort(exp_sub);
            RTTS_ALL_EQUAL(got_sub, exp_sub);

            if (it % 8 == 0 && (hi - lo) <= 64) {
                std::deque<T> dq(base.begin() + static_cast<std::ptrdiff_t>(lo),
                                 base.begin() + static_cast<std::ptrdiff_t>(hi));
                std::mt19937 gd(gseed);
                auto ed = raze::algorithm::shuffle(dq, gd);
                RTTS_EXPECT(ed == dq.end());
                RTTS_EXPECT(std::ranges::equal(dq, s1));
            }
        }
    }
}

RTTS_CASE_TPL("raze::algorithm::shuffle fuzz", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    for (unsigned seed : fuzz_seeds)
        shuffle_fuzz<T>(seed, 40);
};
