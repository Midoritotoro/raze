#include <tests/rts/rts.h>
#include <raze/algorithm/heap/MakeHeap.h>

#include <algorithm>
#include <array>
#include <deque>
#include <memory>
#include <ranges>
#include <vector>

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

