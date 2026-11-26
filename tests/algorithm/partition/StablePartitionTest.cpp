#include <tests/rts/rts.h>
#include <raze/algorithm/partition/StablePartition.h>

#include <algorithm>
#include <array>
#include <list>
#include <memory>
#include <ranges>
#include <vector>

RTTS_CASE_TPL("raze::algorithm::stable_partition", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    using namespace rtts::algorithm;
    namespace opt = raze::options;
    auto cfg = config::compact();
    auto pred = [](T x) { return static_cast<int>(x) % 2 == 0; };

    auto std_sp = [pred](auto c) {
        return std::ranges::stable_partition(c.data.value.begin(), c.data.value.end(), pred);
    };

    run<range, T, bidirectional_iterator>(cfg,
        [pred](auto c) { return raze::algorithm::stable_partition(c.first(), c.last(), pred); },
        std_sp);

    run<range, T, bidirectional_iterator>(cfg,
        [pred](auto c) { return raze::algorithm::stable_partition(c.range(), pred); },
        std_sp);

    run<range, T, bidirectional_iterator>(cfg,
        [pred](auto c) { return raze::algorithm::stable_partition[opt::fscalar](c.range(), pred); },
        std_sp);

    // C++26 P2562: constexpr stable_partition
    static_assert([]() constexpr {
        std::array<T, 8> a{ T(1), T(2), T(3), T(4), T(5), T(6), T(7), T(8) };
        auto r = raze::algorithm::stable_partition(a, [](T x) constexpr { return (static_cast<int>(x) % 2) == 0; });
        return r.begin() == a.begin() + 4 && r.end() == a.end()
            && a == std::array<T, 8>{ T(2), T(4), T(6), T(8), T(1), T(3), T(5), T(7) };
    }());
};

RTTS_CASE("raze::algorithm::stable_partition stability, large runtime buffer, constexpr stability, and move-only") {
    // Runtime stability (including large input > stack/small thresholds)
    {
        std::vector<rtts::test_item> v;
        for (int i = 0; i < 300; ++i)
            v.push_back({ i % 3, i });
        auto exp = v;

        auto pred = [](int k) { return k == 0; };
        auto r = raze::algorithm::stable_partition(v, pred, &rtts::test_item::key);
        auto er = std::ranges::stable_partition(exp, pred, &rtts::test_item::key);

        RTTS_EXPECT(r.begin() - v.begin() == er.begin() - exp.begin());
        RTTS_EXPECT(r.end() == v.end());
        RTTS_ALL_EQUAL(v, exp);
    }

    // Move-only unique_ptr
    {
        std::vector<std::unique_ptr<int>> v;
        for (int i = 1; i <= 10; ++i)
            v.push_back(std::make_unique<int>(i));
        auto r = raze::algorithm::stable_partition(v, [](const auto& p) { return (*p % 2) == 0; });
        RTTS_EXPECT(r.begin() == v.begin() + 5);
        RTTS_EXPECT(*v[0] == 2 && *v[4] == 10 && *v[5] == 1 && *v[9] == 9);
    }

    // Non-common sentinel & rvalue dangling
    {
        std::vector<int> v{ 1, 2, 3, 4, 5, 6 };
        auto nr = raze::algorithm::stable_partition(rtts::make_non_common(v), [](int x) { return x > 3; });
        RTTS_EXPECT(nr.end() == v.end() && (v == std::vector<int>{ 4, 5, 6, 1, 2, 3 }));

        auto dr = raze::algorithm::stable_partition(std::vector<int>{ 1, 2, 3 }, [](int x) { return x == 2; });
        static_assert(std::same_as<decltype(dr), std::ranges::dangling>);
    }

    // Constexpr stability with payloads (C++26 P2562)
    static_assert([]() constexpr {
        std::array<rtts::test_item, 6> a{ {
            { 1, 10 }, { 2, 20 }, { 1, 30 }, { 2, 40 }, { 1, 50 }, { 2, 60 }
        } };
        auto r = raze::algorithm::stable_partition(a, [](int k) constexpr { return k == 2; }, &rtts::test_item::key);
        return r.begin() == a.begin() + 3
            && a[0].payload == 20 && a[1].payload == 40 && a[2].payload == 60
            && a[3].payload == 10 && a[4].payload == 30 && a[5].payload == 50;
    }());
};

