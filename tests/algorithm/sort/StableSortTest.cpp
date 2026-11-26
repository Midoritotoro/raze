#include <tests/rts/rts.h>
#include <raze/algorithm/sort/StableSort.h>

#include <algorithm>
#include <array>
#include <deque>
#include <memory>
#include <ranges>
#include <vector>

RTTS_CASE_TPL("raze::algorithm::stable_sort", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    namespace opt = raze::options;

    for (size_t sz : { size_t(0), size_t(1), size_t(2), size_t(7), size_t(16), size_t(17), size_t(64), size_t(300) }) {
        const auto base = rtts::random::vector<T>(sz, 810u + static_cast<unsigned>(sz));
        auto exp = base;
        std::ranges::stable_sort(exp);

        auto check = [&](auto fn) {
            auto v = base;
            auto end = fn(v);
            RTTS_EXPECT(end == v.end());
            RTTS_ALL_EQUAL(v, exp);
        };

        check([](auto& v) { return raze::algorithm::stable_sort(v); });
        check([](auto& v) { return raze::algorithm::stable_sort(v.begin(), v.end()); });
        check([](auto& v) { return raze::algorithm::stable_sort[opt::fscalar](v); });
        check([](auto& v) { return raze::algorithm::stable_sort[opt::fstatic](v); });
    }

    // C++26 P2562: constexpr stable_sort
    static_assert([]() constexpr {
        std::array<T, 10> a{ T(5), T(2), T(8), T(1), T(9), T(2), T(7), T(3), T(5), T(0) };
        auto end = raze::algorithm::stable_sort(a);
        return end == a.end() && std::ranges::is_sorted(a);
    }());
};

RTTS_CASE("raze::algorithm::stable_sort stability, large runtime buffer, constexpr stability, and move-only") {
    // Runtime stability on large dataset with duplicate keys
    {
        std::vector<rtts::test_item> v;
        for (int i = 0; i < 400; ++i)
            v.push_back({ (i * 37) % 11, i });
        auto exp = v;

        auto end = raze::algorithm::stable_sort(v, std::less<>{}, &rtts::test_item::key);
        std::ranges::stable_sort(exp, std::less<>{}, &rtts::test_item::key);
        RTTS_EXPECT(end == v.end());
        RTTS_ALL_EQUAL(v, exp);
    }

    // Move-only unique_ptr & non-common & dangling
    {
        std::vector<std::unique_ptr<int>> mp;
        for (int x : { 4, 1, 3, 2, 5 }) mp.push_back(std::make_unique<int>(x));
        raze::algorithm::stable_sort(mp, [](const auto& a, const auto& b) { return *a < *b; });
        for (int i = 0; i < 5; ++i) RTTS_EQUAL(*mp[i], i + 1);

        std::deque<int> dq{ 4, 2, 5, 1, 3 };
        auto ni = raze::algorithm::stable_sort(rtts::make_non_common(dq));
        RTTS_EXPECT(ni == dq.end() && std::ranges::is_sorted(dq));

        auto dr = raze::algorithm::stable_sort(std::vector<int>{ 3, 1, 2 });
        static_assert(std::same_as<decltype(dr), std::ranges::dangling>);
    }

    // Constexpr stability (C++26 P2562)
    static_assert([]() constexpr {
        std::array<rtts::test_item, 8> a{ {
            { 3, 0 }, { 1, 1 }, { 2, 2 }, { 1, 3 }, { 3, 4 }, { 2, 5 }, { 1, 6 }, { 2, 7 }
        } };
        raze::algorithm::stable_sort(a, std::less<>{}, &rtts::test_item::key);
        return a[0].payload == 1 && a[1].payload == 3 && a[2].payload == 6
            && a[3].payload == 2 && a[4].payload == 5 && a[5].payload == 7
            && a[6].payload == 0 && a[7].payload == 4;
    }());
};

