#include <tests/rts/rts.h>
#include <raze/algorithm/heap/IsHeap.h>

#include <algorithm>
#include <array>
#include <deque>
#include <ranges>
#include <vector>

RTTS_CASE_TPL("raze::algorithm::is_heap", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    using namespace rtts::algorithm;
    namespace opt = raze::options;
    auto cfg = config::compact();

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

