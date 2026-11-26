#include <tests/rts/rts.h>
#include <raze/algorithm/heap/PopHeap.h>

#include <algorithm>
#include <array>
#include <deque>
#include <memory>
#include <ranges>
#include <vector>

RTTS_CASE_TPL("raze::algorithm::pop_heap", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    namespace opt = raze::options;

    for (size_t sz : { size_t(1), size_t(2), size_t(7), size_t(31), size_t(128) }) {
        auto v = rtts::random::vector<T>(sz, 430u + static_cast<unsigned>(sz));
        std::ranges::make_heap(v);
        auto expected = v;
        std::ranges::sort(expected);

        for (size_t len = sz; len > 0; --len) {
            auto end = (len & 1)
                ? raze::algorithm::pop_heap(v.begin(), v.begin() + static_cast<std::ptrdiff_t>(len))
                : raze::algorithm::pop_heap[opt::fscalar](std::ranges::subrange(v.begin(), v.begin() + static_cast<std::ptrdiff_t>(len)));
            RTTS_EXPECT(end == v.begin() + static_cast<std::ptrdiff_t>(len));
            RTTS_EXPECT(std::ranges::is_heap(v.begin(), v.begin() + static_cast<std::ptrdiff_t>(len - 1)));
        }
        RTTS_ALL_EQUAL(v, expected);
    }

    static_assert([]() constexpr {
        std::array<T, 5> a{ T(9), T(5), T(4), T(1), T(2) };
        auto end = raze::algorithm::pop_heap(a);
        return end == a.end() && a.back() == T(9)
            && std::ranges::is_heap(a.begin(), a.end() - 1);
    }());
};

RTTS_CASE("raze::algorithm::pop_heap comparator, projections, move-only, and dangling") {
    std::vector<rtts::test_item> v{ { 1, 10 }, { 3, 20 }, { 2, 30 }, { 8, 40 }, { 5, 50 } };
    auto end = raze::algorithm::pop_heap(v, std::greater<>{}, &rtts::test_item::key);
    RTTS_EXPECT(end == v.end() && v.back().key == 1 && v.back().payload == 10);
    RTTS_EXPECT(std::ranges::is_heap(v.begin(), v.end() - 1, std::greater<>{}, &rtts::test_item::key));

    std::vector<std::unique_ptr<int>> mp;
    for (int x : { 20, 10, 15, 3, 7 }) mp.push_back(std::make_unique<int>(x));
    raze::algorithm::pop_heap(mp, [](const auto& a, const auto& b) { return *a < *b; });
    RTTS_EQUAL(*mp.back(), 20);

    std::deque<int> dq{ 9, 5, 6, 1 };
    auto ni = raze::algorithm::pop_heap(rtts::make_non_common(dq));
    RTTS_EXPECT(ni == dq.end() && dq.back() == 9);

    auto dr = raze::algorithm::pop_heap(std::vector<int>{ 5, 2 });
    static_assert(std::same_as<decltype(dr), std::ranges::dangling>);
};

