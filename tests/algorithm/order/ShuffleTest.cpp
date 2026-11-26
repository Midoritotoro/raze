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
    // Non-trivial permutation check on large input
    {
        std::vector<int> v(100);
        std::iota(v.begin(), v.end(), 0);
        auto orig = v;
        std::mt19937 rng(42);
        raze::algorithm::shuffle(v, rng);
        RTTS_EXPECT(std::ranges::is_permutation(v, orig));
        RTTS_EXPECT(v != orig);
    }

    // Deque, non-common sentinel, rvalue dangling
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

