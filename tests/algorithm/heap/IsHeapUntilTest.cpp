#include <tests/rts/rts.h>
#include <raze/algorithm/heap/IsHeapUntil.h>

#include <algorithm>
#include <array>
#include <deque>
#include <ranges>
#include <vector>
#include <random>

RTTS_CASE_TPL("raze::algorithm::is_heap_until", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    using namespace rtts::algorithm;
    namespace opt = raze::options;
    auto cfg = config::thorough();

    run<range, T, random_access_iterator>(cfg,
        [](auto c) { return raze::algorithm::is_heap_until(c.first(), c.last()); },
        [](auto c) { return std::ranges::is_heap_until(c.first(), c.last()); });

    run<range, T, random_access_iterator>(cfg,
        [](auto c) { return raze::algorithm::is_heap_until[opt::fscalar](c.range()); },
        [](auto c) { return std::ranges::is_heap_until(c.range()); });

    for (size_t sz : { size_t(0), size_t(1), size_t(2), size_t(7), size_t(31), size_t(128) }) {
        auto v = rtts::random::vector<T>(sz, 400u + static_cast<unsigned>(sz));
        std::ranges::make_heap(v);
        RTTS_EXPECT(raze::algorithm::is_heap_until(v) == v.end());
        if (sz >= 4) {
            v.push_back(static_cast<T>(v[0] + T(1)));
            RTTS_EXPECT(raze::algorithm::is_heap_until(v) == std::ranges::is_heap_until(v));
        }
    }

    static_assert([]() constexpr {
        std::array<T, 6> a{ T(9), T(5), T(4), T(1), T(1), T(10) };
        return raze::algorithm::is_heap_until(a) == a.begin() + 5;
    }());
};

RTTS_CASE("raze::algorithm::is_heap_until comparator, projections, non-common, and dangling") {
    std::vector<int> min_heap{ 1, 3, 2, 7, 6, 5, 4 };
    RTTS_EXPECT(raze::algorithm::is_heap_until(min_heap, std::greater<>{}) == min_heap.end());

    std::vector<rtts::test_item> items{ { 9, 1 }, { 5, 2 }, { 6, 3 }, { 2, 4 }, { 8, 5 } };
    auto it = raze::algorithm::is_heap_until(items, std::less<>{}, &rtts::test_item::key);
    RTTS_EXPECT(it == items.begin() + 4);

    auto ni = raze::algorithm::is_heap_until(rtts::make_non_common(min_heap), std::greater<>{});
    RTTS_EXPECT(ni == min_heap.end());

    auto dr = raze::algorithm::is_heap_until(std::vector<int>{ 3, 2, 1 });
    static_assert(std::same_as<decltype(dr), std::ranges::dangling>);
};

namespace {
    inline constexpr unsigned fuzz_seeds[] = { 42u, 0x9E3779B9u, 0xDEADBEEFu };

    template <class T>
    void is_heap_until_fuzz(unsigned seed, size_t iters) {
        namespace opt = raze::options;
        std::mt19937 rng(seed);
        rtts::random::generator<T> val_gen(seed ^ 0x5A5A5A5Au);

        for (size_t it = 0; it < iters; ++it) {
            const size_t n = static_cast<size_t>(rng() % 257);
            std::vector<T> v(n);
            if ((it & 1u) == 0u) {
                v = rtts::random::vector<T>(n, rng());
            }
            else {
                for (size_t i = 0; i < n; ++i)
                    v[i] = static_cast<T>(rng() % 16u);
            }

            const bool min_heap = (it % 4 == 3);
            const size_t lo = n ? static_cast<size_t>(rng() % (n + 1)) : 0;
            const size_t hi = n ? (lo + static_cast<size_t>(rng() % (n - lo + 1))) : 0;
            auto sub = std::ranges::subrange(v.begin() + static_cast<std::ptrdiff_t>(lo),
                                             v.begin() + static_cast<std::ptrdiff_t>(hi));

            auto run_check = [&](auto comp) {
                const unsigned mode = static_cast<unsigned>(it % 3);
                if (mode == 0u || mode == 1u) {
                    std::ranges::make_heap(sub, comp);
                    if (mode == 1u && (hi - lo) >= 2) {
                        const size_t pos = lo + 1 + static_cast<size_t>(rng() % (hi - lo - 1));
                        v[pos] = val_gen();
                    }
                }
                const auto exp = std::ranges::is_heap_until(sub, comp);
                RTTS_EXPECT(raze::algorithm::is_heap_until(sub, comp) == exp);
                RTTS_EXPECT(raze::algorithm::is_heap_until(sub.begin(), sub.end(), comp) == exp);
                RTTS_EXPECT(raze::algorithm::is_heap_until[opt::fscalar](sub, comp) == exp);
                RTTS_EXPECT(raze::algorithm::is_heap_until[opt::fstatic](sub, comp) == exp);

                if (it % 8 == 0 && (hi - lo) <= 64) {
                    std::deque<T> dq(sub.begin(), sub.end());
                    RTTS_EXPECT(raze::algorithm::is_heap_until(dq, comp) == std::ranges::is_heap_until(dq, comp));
                }
            };

            if (min_heap) run_check(std::greater<>{});
            else          run_check(std::less<>{});
        }
    }
}

RTTS_CASE_TPL("raze::algorithm::is_heap_until fuzz", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    for (unsigned seed : fuzz_seeds)
        is_heap_until_fuzz<T>(seed, 40);
};
