#include <tests/rts/rts.h>
#include <raze/algorithm/partition/IsPartitioned.h>

#include <algorithm>
#include <array>
#include <forward_list>
#include <list>
#include <ranges>
#include <vector>
#include <random>

RTTS_CASE_TPL("raze::algorithm::is_partitioned", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    using namespace rtts::algorithm;
    namespace opt = raze::options;
    auto cfg = config::thorough();
    auto pred = [](T x) { return x < T(50); };

    run<range, T>(cfg,
        [pred](auto c) { return raze::algorithm::is_partitioned(c.first(), c.last(), pred); },
        [pred](auto c) { return std::ranges::is_partitioned(c.first(), c.last(), pred); });

    run<range, T>(cfg,
        [pred](auto c) { return raze::algorithm::is_partitioned[opt::fscalar](c.range(), pred); },
        [pred](auto c) { return std::ranges::is_partitioned(c.range(), pred); });

    for (size_t sz : { size_t(0), size_t(1), size_t(2), size_t(15), size_t(64) }) {
        for (size_t cut : { size_t(0), sz / 2, sz }) {
            std::vector<T> v(sz);
            for (size_t i = 0; i < cut; ++i) v[i] = T(10);
            for (size_t i = cut; i < sz; ++i) v[i] = T(80);
            RTTS_EXPECT(raze::algorithm::is_partitioned(v, pred));
            if (cut > 0 && cut < sz) {
                std::swap(v[cut - 1], v[cut]);
                RTTS_EXPECT(!raze::algorithm::is_partitioned(v, pred));
            }
        }
    }

    static_assert([]() constexpr {
        std::array<T, 6> a{ T(2), T(4), T(6), T(1), T(3), T(5) };
        std::array<T, 6> b{ T(2), T(1), T(4), T(3), T(6), T(5) };
        std::array<T, 0> e{};
        auto is_even = [](T x) constexpr { return (static_cast<int>(x) % 2) == 0; };
        return raze::algorithm::is_partitioned(a, is_even)
            && !raze::algorithm::is_partitioned(b, is_even)
            && raze::algorithm::is_partitioned(e, is_even);
    }());
};

RTTS_CASE("raze::algorithm::is_partitioned projections, single-pass input, and non-common") {
    const std::vector<rtts::test_item> items{ { 1, 99 }, { 2, 88 }, { 10, 1 }, { 20, 2 } };
    RTTS_EXPECT(raze::algorithm::is_partitioned(items, [](int k) { return k < 5; }, &rtts::test_item::key));

    const std::vector<int> src{ 2, 4, 6, 7, 9 };
    rtts::single_pass_input_range<int> sp(src);
    RTTS_EXPECT(raze::algorithm::is_partitioned(sp, [](int x) { return x % 2 == 0; }));
    RTTS_EXPECT(raze::algorithm::is_partitioned(rtts::make_non_common(src), [](int x) { return x % 2 == 0; }));
};

namespace {
    inline constexpr unsigned fuzz_seeds[] = { 42u, 0x9E3779B9u, 0xDEADBEEFu };

    template <class T>
    void is_partitioned_fuzz(unsigned seed, size_t iters) {
        namespace opt = raze::options;
        std::mt19937 rng(seed);

        for (size_t it = 0; it < iters; ++it) {
            const size_t n = static_cast<size_t>(rng() % 257);
            const T threshold = static_cast<T>(rng() % 80u + 10u);
            auto pred = [threshold](T x) { return x < threshold; };

            std::vector<T> v(n);
            const unsigned mode = static_cast<unsigned>(it % 3);
            if (mode == 0u || mode == 1u) {
                const size_t cut = n ? static_cast<size_t>(rng() % (n + 1)) : 0;
                for (size_t i = 0; i < cut; ++i)
                    v[i] = static_cast<T>(rng() % static_cast<unsigned>(threshold));
                for (size_t i = cut; i < n; ++i)
                    v[i] = static_cast<T>(static_cast<unsigned>(threshold) + (rng() % 30u));
                if (mode == 1u && n >= 2) {
                    const size_t p1 = static_cast<size_t>(rng() % n);
                    const size_t p2 = static_cast<size_t>(rng() % n);
                    std::swap(v[p1], v[p2]);
                }
            }
            else {
                v = rtts::random::vector<T>(n, rng());
            }

            const size_t lo = n ? static_cast<size_t>(rng() % (n + 1)) : 0;
            const size_t hi = n ? (lo + static_cast<size_t>(rng() % (n - lo + 1))) : 0;
            auto sub = std::ranges::subrange(v.begin() + static_cast<std::ptrdiff_t>(lo),
                                             v.begin() + static_cast<std::ptrdiff_t>(hi));

            const bool exp = std::ranges::is_partitioned(sub, pred);
            RTTS_EQUAL(raze::algorithm::is_partitioned(sub, pred), exp);
            RTTS_EQUAL(raze::algorithm::is_partitioned(sub.begin(), sub.end(), pred), exp);
            RTTS_EQUAL(raze::algorithm::is_partitioned[opt::fscalar](sub, pred), exp);
            RTTS_EQUAL(raze::algorithm::is_partitioned[opt::fstatic](sub, pred), exp);

            if (it % 8 == 0 && (hi - lo) <= 64) {
                std::forward_list<T> fl(sub.begin(), sub.end());
                RTTS_EQUAL(raze::algorithm::is_partitioned(fl, pred), exp);
            }
        }
    }
}

RTTS_CASE_TPL("raze::algorithm::is_partitioned fuzz", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    for (unsigned seed : fuzz_seeds)
        is_partitioned_fuzz<T>(seed, 40);
};
