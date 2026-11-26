#include <tests/rts/rts.h>
#include <raze/algorithm/set/InplaceMerge.h>

#include <algorithm>
#include <array>
#include <list>
#include <memory>
#include <ranges>
#include <vector>

RTTS_CASE_TPL("raze::algorithm::inplace_merge", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    namespace opt = raze::options;

    for (size_t sz : { size_t(0), size_t(1), size_t(2), size_t(7), size_t(31), size_t(128), size_t(300) }) {
        for (size_t mid_idx : { size_t(0), sz / 3, sz / 2, (2 * sz) / 3, sz }) {
            auto base = rtts::random::vector<T>(sz, 610u + static_cast<unsigned>(sz + mid_idx));
            std::ranges::sort(base.begin(), base.begin() + static_cast<std::ptrdiff_t>(mid_idx));
            std::ranges::sort(base.begin() + static_cast<std::ptrdiff_t>(mid_idx), base.end());

            auto exp = base;
            std::ranges::inplace_merge(exp, exp.begin() + static_cast<std::ptrdiff_t>(mid_idx));

            auto check = [&](auto fn) {
                auto v = base;
                auto end = fn(v, v.begin() + static_cast<std::ptrdiff_t>(mid_idx));
                RTTS_EXPECT(end == v.end());
                RTTS_ALL_EQUAL(v, exp);
            };

            check([](auto& v, auto m) { return raze::algorithm::inplace_merge(v, m); });
            check([](auto& v, auto m) { return raze::algorithm::inplace_merge(v.begin(), m, v.end()); });
            check([](auto& v, auto m) { return raze::algorithm::inplace_merge[opt::fscalar](v, m); });
            check([](auto& v, auto m) { return raze::algorithm::inplace_merge[opt::fstatic](v, m); });
        }
    }

    // C++26 P2562: constexpr inplace_merge
    static_assert([]() constexpr {
        std::array<T, 8> a{ T(1), T(3), T(5), T(7), T(2), T(3), T(6), T(8) };
        auto end = raze::algorithm::inplace_merge(a, a.begin() + 4);
        return end == a.end()
            && a == std::array<T, 8>{ T(1), T(2), T(3), T(3), T(5), T(6), T(7), T(8) };
    }());
};

RTTS_CASE("raze::algorithm::inplace_merge stability, list, move-only, constexpr stability, and dangling") {
    // Stability with projections (runtime buffer path)
    {
        std::vector<rtts::test_item> v;
        for (int i = 0; i < 150; ++i) v.push_back({ i / 3, i });
        for (int i = 0; i < 150; ++i) v.push_back({ i / 3, 1000 + i });
        auto exp = v;

        raze::algorithm::inplace_merge(v, v.begin() + 150, std::less<>{}, &rtts::test_item::key);
        std::ranges::inplace_merge(exp, exp.begin() + 150, std::less<>{}, &rtts::test_item::key);
        RTTS_ALL_EQUAL(v, exp);
    }

    // Bidirectional std::list & move-only unique_ptr
    {
        std::list<int> lst{ 1, 4, 7, 2, 3, 8 };
        auto mid = std::ranges::next(lst.begin(), 3);
        auto end = raze::algorithm::inplace_merge(lst, mid);
        RTTS_EXPECT(end == lst.end() && (lst == std::list<int>{ 1, 2, 3, 4, 7, 8 }));

        std::vector<std::unique_ptr<int>> mp;
        for (int x : { 1, 5, 9, 2, 6, 8 }) mp.push_back(std::make_unique<int>(x));
        raze::algorithm::inplace_merge(mp, mp.begin() + 3, [](const auto& a, const auto& b) { return *a < *b; });
        RTTS_EXPECT(*mp[0] == 1 && *mp[1] == 2 && *mp[2] == 5 && *mp[5] == 9);

        std::vector<int> tmp{ 1, 3, 2, 4 };
        auto m = tmp.begin() + 2;
        auto dr = raze::algorithm::inplace_merge(std::move(tmp), m);
        static_assert(std::same_as<decltype(dr), std::ranges::dangling>);
    }

    // Constexpr stability (C++26 P2562)
    static_assert([]() constexpr {
        std::array<rtts::test_item, 6> a{ {
            { 1, 1 }, { 2, 2 }, { 3, 3 }, { 2, 4 }, { 2, 5 }, { 4, 6 }
        } };
        raze::algorithm::inplace_merge(a, a.begin() + 3, std::less<>{}, &rtts::test_item::key);
        return a[1].payload == 2 && a[2].payload == 4 && a[3].payload == 5;
    }());
};

