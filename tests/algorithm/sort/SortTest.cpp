#include <tests/rts/rts.h>
#include <raze/algorithm/sort/Sort.h>

#include <algorithm>
#include <array>
#include <deque>
#include <memory>
#include <ranges>
#include <vector>

RTTS_CASE_TPL("raze::algorithm::sort", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    namespace opt = raze::options;

    for (size_t sz : { size_t(0), size_t(1), size_t(2), size_t(3), size_t(15), size_t(16), size_t(17), size_t(64), size_t(300) }) {
        const auto base = rtts::random::vector<T>(sz, 800u + static_cast<unsigned>(sz));
        auto exp = base;
        std::ranges::sort(exp);

        auto check = [&](auto fn) {
            auto v = base;
            auto end = fn(v);
            RTTS_EXPECT(end == v.end());
            RTTS_ALL_EQUAL(v, exp);
        };

        check([](auto& v) { return raze::algorithm::sort(v); });
        check([](auto& v) { return raze::algorithm::sort(v.begin(), v.end()); });
        check([](auto& v) { return raze::algorithm::sort[opt::fscalar](v); });
        check([](auto& v) { return raze::algorithm::sort[opt::fstatic](v); });
    }

    // Pathological inputs: already sorted, reverse sorted, all equal, organ-pipe
    for (size_t sz : { size_t(64), size_t(257) }) {
        std::vector<T> rev(sz), same(sz, T(7)), pipe(sz);
        for (size_t i = 0; i < sz; ++i) {
            rev[i] = static_cast<T>((sz - i) % 100);
            pipe[i] = static_cast<T>((i < sz / 2 ? i : sz - i) % 100);
        }
        for (const auto& src : { rev, same, pipe }) {
            auto a = src, b = src;
            raze::algorithm::sort(a);
            std::ranges::sort(b);
            RTTS_ALL_EQUAL(a, b);
        }
    }

    static_assert([]() constexpr {
        std::array<T, 10> a{ T(9), T(1), T(5), T(3), T(8), T(2), T(7), T(4), T(6), T(0) };
        auto end = raze::algorithm::sort(a);
        return end == a.end() && std::ranges::is_sorted(a);
    }());
};

RTTS_CASE("raze::algorithm::sort comparator, projections, deque, move-only, and dangling") {
    std::deque<int> dq{ 5, 1, 9, 2, 8, 3, 7, 4, 6 };
    raze::algorithm::sort(dq, std::greater<>{});
    RTTS_EXPECT(std::ranges::is_sorted(dq, std::greater<>{}));

    std::vector<rtts::test_item> items{ { 4, 1 }, { 1, 2 }, { 3, 3 }, { 2, 4 } };
    raze::algorithm::sort(items, std::less<>{}, &rtts::test_item::key);
    RTTS_EXPECT(std::ranges::is_sorted(items, std::less<>{}, &rtts::test_item::key));

    std::vector<std::unique_ptr<int>> mp;
    for (int x : { 5, 2, 8, 1, 9, 3 }) mp.push_back(std::make_unique<int>(x));
    raze::algorithm::sort(mp, [](const auto& a, const auto& b) { return *a < *b; });
    for (size_t i = 1; i < mp.size(); ++i) RTTS_EXPECT(*mp[i - 1] <= *mp[i]);

    auto ni = raze::algorithm::sort(rtts::make_non_common(dq));
    RTTS_EXPECT(ni == dq.end() && std::ranges::is_sorted(dq));

    auto dr = raze::algorithm::sort(std::vector<int>{ 3, 1, 2 });
    static_assert(std::same_as<decltype(dr), std::ranges::dangling>);
};

