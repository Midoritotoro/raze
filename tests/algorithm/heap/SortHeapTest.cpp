#include <tests/rts/rts.h>
#include <raze/algorithm/heap/MakeHeap.h>
#include <raze/algorithm/heap/SortHeap.h>

#include <algorithm>
#include <array>
#include <deque>
#include <ranges>
#include <vector>
#include <random>

RTTS_CASE_TPL("raze::algorithm::sort_heap", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    namespace opt = raze::options;

    for (size_t sz : { size_t(0), size_t(1), size_t(2), size_t(7), size_t(31), size_t(128) }) {
        auto base = rtts::random::vector<T>(sz, 450u + static_cast<unsigned>(sz));
        std::ranges::make_heap(base);
        auto exp = base;
        std::ranges::sort_heap(exp);

        auto check = [&](auto fn) {
            auto v = base;
            auto end = fn(v);
            RTTS_EXPECT(end == v.end());
            RTTS_ALL_EQUAL(v, exp);
        };

        check([](auto& v) { return raze::algorithm::sort_heap(v); });
        check([](auto& v) { return raze::algorithm::sort_heap(v.begin(), v.end()); });
        check([](auto& v) { return raze::algorithm::sort_heap[opt::fscalar](v); });
        check([](auto& v) { return raze::algorithm::sort_heap[opt::fstatic](v); });
    }

    static_assert([]() constexpr {
        std::array<T, 6> a{ T(9), T(5), T(7), T(1), T(3), T(2) };
        raze::algorithm::make_heap(a);
        auto end = raze::algorithm::sort_heap(a);
        return end == a.end() && std::ranges::is_sorted(a);
    }());
};

RTTS_CASE("raze::algorithm::sort_heap comparator, projections, non-common, and dangling") {
    std::deque<int> dq{ 1, 3, 2, 9, 5, 4 };
    std::ranges::make_heap(dq, std::greater<>{});
    auto end = raze::algorithm::sort_heap(dq, std::greater<>{});
    RTTS_EXPECT(end == dq.end() && std::ranges::is_sorted(dq, std::greater<>{}));

    std::vector<rtts::test_item> items{ { 9, 1 }, { 5, 2 }, { 7, 3 }, { 2, 4 } };
    std::ranges::make_heap(items, std::less<>{}, &rtts::test_item::key);
    raze::algorithm::sort_heap(items, std::less<>{}, &rtts::test_item::key);
    RTTS_EXPECT(std::ranges::is_sorted(items, std::less<>{}, &rtts::test_item::key));

    std::vector<int> v{ 9, 4, 7, 1, 3 };
    auto ni = raze::algorithm::sort_heap(rtts::make_non_common(v));
    RTTS_EXPECT(ni == v.end() && std::ranges::is_sorted(v));

    auto dr = raze::algorithm::sort_heap(std::vector<int>{ 3, 1, 2 });
    static_assert(std::same_as<decltype(dr), std::ranges::dangling>);
};

namespace {
    inline constexpr unsigned fuzz_seeds[] = { 42u, 0x9E3779B9u, 0xDEADBEEFu };

    template <class T>
    void sort_heap_fuzz(unsigned seed, size_t iters) {
        namespace opt = raze::options;
        std::mt19937 rng(seed);

        for (size_t it = 0; it < iters; ++it) {
            const size_t n = static_cast<size_t>(rng() % 257);
            std::vector<T> base(n);
            if ((it & 1u) == 0u) {
                base = rtts::random::vector<T>(n, rng());
            }
            else {
                for (size_t i = 0; i < n; ++i)
                    base[i] = static_cast<T>(rng() % 12u);
            }

            const size_t lo = n ? static_cast<size_t>(rng() % (n + 1)) : 0;
            const size_t hi = n ? (lo + static_cast<size_t>(rng() % (n - lo + 1))) : 0;
            const bool min_heap = (it % 3 == 2);

            auto run_check = [&](auto comp) {
                auto heap_base = base;
                std::ranges::make_heap(heap_base.begin() + static_cast<std::ptrdiff_t>(lo),
                                       heap_base.begin() + static_cast<std::ptrdiff_t>(hi), comp);
                auto exp = heap_base;
                std::ranges::sort_heap(exp.begin() + static_cast<std::ptrdiff_t>(lo),
                                       exp.begin() + static_cast<std::ptrdiff_t>(hi), comp);

                auto verify = [&](auto fn) {
                    auto v = heap_base;
                    auto sub = std::ranges::subrange(v.begin() + static_cast<std::ptrdiff_t>(lo),
                                                     v.begin() + static_cast<std::ptrdiff_t>(hi));
                    auto end = fn(sub);
                    RTTS_EXPECT(end == sub.end());
                    RTTS_ALL_EQUAL(v, exp);
                };

                verify([&](auto sub) { return raze::algorithm::sort_heap(sub, comp); });
                verify([&](auto sub) { return raze::algorithm::sort_heap(sub.begin(), sub.end(), comp); });
                verify([&](auto sub) { return raze::algorithm::sort_heap[opt::fscalar](sub, comp); });
                verify([&](auto sub) { return raze::algorithm::sort_heap[opt::fstatic](sub, comp); });

                if (it % 8 == 0 && (hi - lo) <= 64) {
                    std::deque<T> dq(heap_base.begin() + static_cast<std::ptrdiff_t>(lo),
                                     heap_base.begin() + static_cast<std::ptrdiff_t>(hi));
                    auto end = raze::algorithm::sort_heap(dq, comp);
                    RTTS_EXPECT(end == dq.end());
                    RTTS_EXPECT(std::ranges::equal(dq, std::ranges::subrange(
                        exp.begin() + static_cast<std::ptrdiff_t>(lo),
                        exp.begin() + static_cast<std::ptrdiff_t>(hi))));
                }
            };

            if (min_heap) run_check(std::greater<>{});
            else          run_check(std::less<>{});
        }
    }
}

RTTS_CASE_TPL("raze::algorithm::sort_heap fuzz", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    for (unsigned seed : fuzz_seeds)
        sort_heap_fuzz<T>(seed, 35);
};
