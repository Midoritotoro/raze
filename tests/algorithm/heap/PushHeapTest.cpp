#include <tests/rts/rts.h>
#include <raze/algorithm/heap/PushHeap.h>

#include <algorithm>
#include <array>
#include <deque>
#include <memory>
#include <ranges>
#include <vector>
#include <random>

RTTS_CASE_TPL("raze::algorithm::push_heap", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    namespace opt = raze::options;

    for (size_t sz : { size_t(1), size_t(2), size_t(7), size_t(31), size_t(128) }) {
        const auto raw = rtts::random::vector<T>(sz, 420u + static_cast<unsigned>(sz));

        std::vector<T> h1, h2;
        for (T x : raw) {
            h1.push_back(x);
            auto e1 = raze::algorithm::push_heap(h1);
            RTTS_EXPECT(e1 == h1.end());
            RTTS_EXPECT(std::ranges::is_heap(h1));

            h2.push_back(x);
            auto e2 = raze::algorithm::push_heap[opt::fscalar](h2.begin(), h2.end());
            RTTS_EXPECT(e2 == h2.end());
            RTTS_EXPECT(std::ranges::is_heap(h2));
        }
        RTTS_EXPECT(std::ranges::is_permutation(h1, raw));
        RTTS_EQUAL(h1.front(), std::ranges::max(raw));
    }

    static_assert([]() constexpr {
        std::array<T, 6> a{ T(9), T(5), T(4), T(1), T(2), T(10) };
        auto end = raze::algorithm::push_heap(a);
        return end == a.end() && a[0] == T(10) && std::ranges::is_heap(a);
    }());
};

RTTS_CASE("raze::algorithm::push_heap comparator, projections, move-only, and dangling") {
    {
        std::vector<rtts::test_item> v{ { 2, 1 }, { 5, 2 }, { 7, 3 }, { 1, 4 } };
        auto end = raze::algorithm::push_heap(v, std::greater<>{}, &rtts::test_item::key);
        RTTS_EXPECT(end == v.end() && v[0].key == 1 && v[0].payload == 4);
        RTTS_EXPECT(std::ranges::is_heap(v, std::greater<>{}, &rtts::test_item::key));
    }

    {
        std::vector<std::unique_ptr<int>> v;
        for (int x : { 10, 5, 8, 2, 20 }) {
            v.push_back(std::make_unique<int>(x));
            raze::algorithm::push_heap(v, [](const auto& a, const auto& b) { return *a < *b; });
        }
        RTTS_EQUAL(*v.front(), 20);

        std::deque<int> dq{ 8, 5, 6, 9 };
        auto ni = raze::algorithm::push_heap(rtts::make_non_common(dq));
        RTTS_EXPECT(ni == dq.end() && dq.front() == 9);

        auto dr = raze::algorithm::push_heap(std::vector<int>{ 5, 9 });
        static_assert(std::same_as<decltype(dr), std::ranges::dangling>);
    }
};

namespace {
    inline constexpr unsigned fuzz_seeds[] = { 42u, 0x9E3779B9u, 0xDEADBEEFu };

    template <class T>
    void push_heap_fuzz(unsigned seed, size_t iters) {
        namespace opt = raze::options;
        std::mt19937 rng(seed);

        for (size_t it = 0; it < iters; ++it) {
            const size_t n = static_cast<size_t>(rng() % 129);
            std::vector<T> raw(n);
            if ((it & 1u) == 0u) {
                raw = rtts::random::vector<T>(n, rng());
            }
            else {
                for (size_t i = 0; i < n; ++i)
                    raw[i] = static_cast<T>(rng() % 12u);
            }

            const bool min_heap = (it % 3 == 2);
            auto run_check = [&](auto comp) {
                std::vector<T> h;
                h.reserve(n);
                for (size_t i = 0; i < n; ++i) {
                    h.push_back(raw[i]);
                    const unsigned mode = static_cast<unsigned>((it + i) % 4);
                    auto end = (mode == 0u) ? raze::algorithm::push_heap(h, comp)
                             : (mode == 1u) ? raze::algorithm::push_heap(h.begin(), h.end(), comp)
                             : (mode == 2u) ? raze::algorithm::push_heap[opt::fscalar](h, comp)
                                            : raze::algorithm::push_heap[opt::fstatic](h, comp);
                    RTTS_EXPECT(end == h.end());
                    RTTS_EXPECT(std::ranges::is_heap(h, comp));
                }
                auto s_got = h;
                auto s_exp = raw;
                std::ranges::sort(s_got);
                std::ranges::sort(s_exp);
                RTTS_ALL_EQUAL(s_got, s_exp);

                if (it % 8 == 0 && n <= 64) {
                    std::deque<T> dq;
                    for (T x : raw) {
                        dq.push_back(x);
                        auto end = raze::algorithm::push_heap(dq, comp);
                        RTTS_EXPECT(end == dq.end() && std::ranges::is_heap(dq, comp));
                    }
                }
            };

            if (min_heap) run_check(std::greater<>{});
            else          run_check(std::less<>{});
        }
    }
}

RTTS_CASE_TPL("raze::algorithm::push_heap fuzz", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    for (unsigned seed : fuzz_seeds)
        push_heap_fuzz<T>(seed, 30);
};
