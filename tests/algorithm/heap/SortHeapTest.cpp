#include <tests/rts/rts.h>
#include <raze/algorithm/heap/MakeHeap.h>
#include <raze/algorithm/heap/SortHeap.h>

#include <algorithm>
#include <array>
#include <deque>
#include <ranges>
#include <vector>

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

