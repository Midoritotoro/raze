#include <tests/rts/rts.h>
#include <raze/algorithm/heap/MakeHeap.h>

#include <algorithm>
#include <array>
#include <deque>
#include <memory>
#include <ranges>
#include <vector>
#include <random>

RTTS_CASE_TPL("raze::algorithm::make_heap", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    namespace opt = raze::options;

    for (size_t sz : { size_t(0), size_t(1), size_t(2), size_t(3), size_t(7), size_t(31), size_t(128) }) {
        const auto base = rtts::random::vector<T>(sz, 440u + static_cast<unsigned>(sz));

        auto check = [&](auto fn) {
            auto v = base;
            auto end = fn(v);
            RTTS_EXPECT(end == v.end());
            RTTS_EXPECT(std::ranges::is_heap(v));
            RTTS_EXPECT(std::ranges::is_permutation(v, base));
            if (!v.empty())
                RTTS_EQUAL(v.front(), std::ranges::max(base));
        };

        check([](auto& v) { return raze::algorithm::make_heap(v); });
        check([](auto& v) { return raze::algorithm::make_heap(v.begin(), v.end()); });
        check([](auto& v) { return raze::algorithm::make_heap[opt::fscalar](v); });
        check([](auto& v) { return raze::algorithm::make_heap[opt::fstatic](v); });
    }

    static_assert([]() constexpr {
        std::array<T, 7> a{ T(3), T(1), T(4), T(1), T(5), T(9), T(2) };
        auto end = raze::algorithm::make_heap(a);
        return end == a.end() && a[0] == T(9) && std::ranges::is_heap(a);
    }());
};

RTTS_CASE("raze::algorithm::make_heap comparator, projections, deque, and dangling") {
    std::deque<int> dq{ 5, 2, 9, 1, 7, 3 };
    auto end = raze::algorithm::make_heap(dq, std::greater<>{});
    RTTS_EXPECT(end == dq.end() && dq.front() == 1);
    RTTS_EXPECT(std::ranges::is_heap(dq, std::greater<>{}));

    std::vector<rtts::test_item> items{ { 3, 1 }, { 9, 2 }, { 1, 3 }, { 7, 4 } };
    raze::algorithm::make_heap(items, std::less<>{}, &rtts::test_item::key);
    RTTS_EQUAL(items.front().key, 9);

    auto ni = raze::algorithm::make_heap(rtts::make_non_common(dq));
    RTTS_EXPECT(ni == dq.end() && std::ranges::is_heap(dq));

    auto dr = raze::algorithm::make_heap(std::vector<int>{ 3, 1, 4 });
    static_assert(std::same_as<decltype(dr), std::ranges::dangling>);
};

namespace {
    inline constexpr unsigned fuzz_seeds[] = { 42u, 0x9E3779B9u, 0xDEADBEEFu };

    template <class T>
    void make_heap_fuzz(unsigned seed, size_t iters) {
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
                auto verify = [&](auto fn) {
                    auto v = base;
                    auto sub = std::ranges::subrange(v.begin() + static_cast<std::ptrdiff_t>(lo),
                                                     v.begin() + static_cast<std::ptrdiff_t>(hi));
                    auto end = fn(sub);
                    RTTS_EXPECT(end == sub.end());
                    RTTS_EXPECT(std::ranges::is_heap(sub, comp));
                    RTTS_EXPECT(std::equal(v.begin(), v.begin() + static_cast<std::ptrdiff_t>(lo), base.begin()));
                    RTTS_EXPECT(std::equal(v.begin() + static_cast<std::ptrdiff_t>(hi), v.end(),
                                           base.begin() + static_cast<std::ptrdiff_t>(hi)));
                    std::vector<T> s_got(sub.begin(), sub.end());
                    std::vector<T> s_exp(base.begin() + static_cast<std::ptrdiff_t>(lo),
                                         base.begin() + static_cast<std::ptrdiff_t>(hi));
                    std::ranges::sort(s_got);
                    std::ranges::sort(s_exp);
                    RTTS_ALL_EQUAL(s_got, s_exp);
                };

                verify([&](auto sub) { return raze::algorithm::make_heap(sub, comp); });
                verify([&](auto sub) { return raze::algorithm::make_heap(sub.begin(), sub.end(), comp); });
                verify([&](auto sub) { return raze::algorithm::make_heap[opt::fscalar](sub, comp); });
                verify([&](auto sub) { return raze::algorithm::make_heap[opt::fstatic](sub, comp); });

                if (it % 8 == 0 && (hi - lo) <= 64) {
                    std::deque<T> dq(base.begin() + static_cast<std::ptrdiff_t>(lo),
                                     base.begin() + static_cast<std::ptrdiff_t>(hi));
                    auto end = raze::algorithm::make_heap(dq, comp);
                    RTTS_EXPECT(end == dq.end() && std::ranges::is_heap(dq, comp));
                }
            };

            if (min_heap) run_check(std::greater<>{});
            else          run_check(std::less<>{});
        }
    }
}

RTTS_CASE_TPL("raze::algorithm::make_heap fuzz", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    for (unsigned seed : fuzz_seeds)
        make_heap_fuzz<T>(seed, 35);
};
