#include <tests/rts/rts.h>
#include <raze/algorithm/heap/IsHeap.h>

#include <algorithm>
#include <array>
#include <deque>
#include <ranges>
#include <vector>
#include <random>

RTTS_CASE_TPL("raze::algorithm::is_heap", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    using namespace rtts::algorithm;
    namespace opt = raze::options;
    auto cfg = config::thorough();

    run<range, T, random_access_iterator>(cfg,
        [](auto c) { return raze::algorithm::is_heap(c.first(), c.last()); },
        [](auto c) { return std::ranges::is_heap(c.first(), c.last()); });

    run<range, T, random_access_iterator>(cfg,
        [](auto c) { return raze::algorithm::is_heap[opt::fscalar](c.range()); },
        [](auto c) { return std::ranges::is_heap(c.range()); });

    for (size_t sz : { size_t(0), size_t(1), size_t(2), size_t(15), size_t(128) }) {
        auto v = rtts::random::vector<T>(sz, 410u + static_cast<unsigned>(sz));
        std::ranges::make_heap(v);
        RTTS_EXPECT(raze::algorithm::is_heap(v));
        RTTS_EXPECT(raze::algorithm::is_heap[opt::fstatic](v));
    }

    static_assert([]() constexpr {
        std::array<T, 5> h{ T(9), T(5), T(4), T(1), T(2) };
        std::array<T, 5> bad{ T(1), T(5), T(4), T(9), T(2) };
        std::array<T, 0> e{};
        return raze::algorithm::is_heap(h)
            && !raze::algorithm::is_heap(bad)
            && raze::algorithm::is_heap(e);
    }());
};

RTTS_CASE("raze::algorithm::is_heap comparator, projections, and non-common") {
    std::deque<int> min_heap{ 1, 4, 2, 9, 5 };
    RTTS_EXPECT(raze::algorithm::is_heap(min_heap, std::greater<>{}));
    RTTS_EXPECT(raze::algorithm::is_heap(rtts::make_non_common(min_heap), std::greater<>{}));

    const std::vector<rtts::test_item> items{ { 10, 1 }, { 7, 2 }, { 8, 3 }, { 2, 4 } };
    RTTS_EXPECT(raze::algorithm::is_heap(items, std::less<>{}, &rtts::test_item::key));
};

namespace {
    inline constexpr unsigned fuzz_seeds[] = { 42u, 0x9E3779B9u, 0xDEADBEEFu };

    template <class T>
    void is_heap_fuzz(unsigned seed, size_t iters) {
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
                        const size_t pos = lo + static_cast<size_t>(rng() % (hi - lo));
                        v[pos] = val_gen();
                    }
                }
                const bool exp = std::ranges::is_heap(sub, comp);
                RTTS_EQUAL(raze::algorithm::is_heap(sub, comp), exp);
                RTTS_EQUAL(raze::algorithm::is_heap(sub.begin(), sub.end(), comp), exp);
                RTTS_EQUAL(raze::algorithm::is_heap[opt::fscalar](sub, comp), exp);
                RTTS_EQUAL(raze::algorithm::is_heap[opt::fstatic](sub, comp), exp);

                if (it % 8 == 0 && (hi - lo) <= 64) {
                    std::deque<T> dq(sub.begin(), sub.end());
                    RTTS_EQUAL(raze::algorithm::is_heap(dq, comp), exp);
                }
            };

            if (min_heap) run_check(std::greater<>{});
            else          run_check(std::less<>{});
        }
    }
}

RTTS_CASE_TPL("raze::algorithm::is_heap fuzz", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    for (unsigned seed : fuzz_seeds)
        is_heap_fuzz<T>(seed, 40);
};
