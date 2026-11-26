#include <tests/rts/rts.h>
#include <raze/algorithm/heap/IsHeapUntil.h>

#include <algorithm>
#include <array>
#include <deque>
#include <ranges>
#include <vector>

RTTS_CASE_TPL("raze::algorithm::is_heap_until", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    using namespace rtts::algorithm;
    namespace opt = raze::options;
    auto cfg = config::compact();

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

