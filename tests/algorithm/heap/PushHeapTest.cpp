#include <tests/rts/rts.h>
#include <raze/algorithm/heap/PushHeap.h>

#include <algorithm>
#include <array>
#include <deque>
#include <memory>
#include <ranges>
#include <vector>

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
    // Min-heap with projection
    {
        std::vector<rtts::test_item> v{ { 2, 1 }, { 5, 2 }, { 7, 3 }, { 1, 4 } };
        auto end = raze::algorithm::push_heap(v, std::greater<>{}, &rtts::test_item::key);
        RTTS_EXPECT(end == v.end() && v[0].key == 1 && v[0].payload == 4);
        RTTS_EXPECT(std::ranges::is_heap(v, std::greater<>{}, &rtts::test_item::key));
    }

    // Move-only unique_ptr & non-common & dangling
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

