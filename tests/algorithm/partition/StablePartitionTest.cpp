#include <tests/rts/rts.h>
#include <raze/algorithm/partition/StablePartition.h>

#include <algorithm>
#include <array>
#include <list>
#include <memory>
#include <ranges>
#include <vector>
#include <random>

RTTS_CASE_TPL("raze::algorithm::stable_partition", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    using namespace rtts::algorithm;
    namespace opt = raze::options;
    auto cfg = config::thorough();
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

    static_assert([]() constexpr {
        std::array<T, 8> a{ T(1), T(2), T(3), T(4), T(5), T(6), T(7), T(8) };
        auto r = raze::algorithm::stable_partition(a, [](T x) constexpr { return (static_cast<int>(x) % 2) == 0; });
        return r.begin() == a.begin() + 4 && r.end() == a.end()
            && a == std::array<T, 8>{ T(2), T(4), T(6), T(8), T(1), T(3), T(5), T(7) };
    }());
};

RTTS_CASE("raze::algorithm::stable_partition stability, large runtime buffer, constexpr stability, and move-only") {
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

    {
        std::vector<std::unique_ptr<int>> v;
        for (int i = 1; i <= 10; ++i)
            v.push_back(std::make_unique<int>(i));
        auto r = raze::algorithm::stable_partition(v, [](const auto& p) { return (*p % 2) == 0; });
        RTTS_EXPECT(r.begin() == v.begin() + 5);
        RTTS_EXPECT(*v[0] == 2 && *v[4] == 10 && *v[5] == 1 && *v[9] == 9);
    }

    {
        std::vector<int> v{ 1, 2, 3, 4, 5, 6 };
        auto nr = raze::algorithm::stable_partition(rtts::make_non_common(v), [](int x) { return x > 3; });
        RTTS_EXPECT(nr.end() == v.end() && (v == std::vector<int>{ 4, 5, 6, 1, 2, 3 }));

        auto dr = raze::algorithm::stable_partition(std::vector<int>{ 1, 2, 3 }, [](int x) { return x == 2; });
        static_assert(std::same_as<decltype(dr), std::ranges::dangling>);
    }

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

namespace {
    inline constexpr unsigned fuzz_seeds[] = { 42u, 0x9E3779B9u, 0xDEADBEEFu };

    template <class T>
    void stable_partition_fuzz(unsigned seed, size_t iters) {
        namespace opt = raze::options;
        std::mt19937 rng(seed);
        rtts::random::generator<T> val_gen(seed ^ 0x5A5A5A5Au);

        for (size_t it = 0; it < iters; ++it) {
            const size_t n = static_cast<size_t>(rng() % 257);
            const auto base = rtts::random::vector<T>(n, rng());
            const size_t lo = n ? static_cast<size_t>(rng() % (n + 1)) : 0;
            const size_t hi = n ? (lo + static_cast<size_t>(rng() % (n - lo + 1))) : 0;
            const T threshold = val_gen();
            auto pred = [threshold](T x) { return x < threshold; };

            auto exp = base;
            auto er = std::ranges::stable_partition(exp.begin() + static_cast<std::ptrdiff_t>(lo),
                                                    exp.begin() + static_cast<std::ptrdiff_t>(hi), pred);
            const auto exp_off = er.begin() - exp.begin();

            auto verify = [&](auto fn) {
                auto v = base;
                auto sub = std::ranges::subrange(v.begin() + static_cast<std::ptrdiff_t>(lo),
                                                 v.begin() + static_cast<std::ptrdiff_t>(hi));
                auto r = fn(sub);
                RTTS_EXPECT(r.begin() - v.begin() == exp_off && r.end() == sub.end());
                RTTS_ALL_EQUAL(v, exp);
            };

            verify([&](auto sub) { return raze::algorithm::stable_partition(sub, pred); });
            verify([&](auto sub) { return raze::algorithm::stable_partition(sub.begin(), sub.end(), pred); });
            verify([&](auto sub) { return raze::algorithm::stable_partition[opt::fscalar](sub, pred); });
            verify([&](auto sub) { return raze::algorithm::stable_partition[opt::fstatic](sub, pred); });

            if (it % 8 == 0 && (hi - lo) <= 64) {
                std::list<T> lst(base.begin() + static_cast<std::ptrdiff_t>(lo),
                                 base.begin() + static_cast<std::ptrdiff_t>(hi));
                std::list<T> lst_exp = lst;
                auto rl = raze::algorithm::stable_partition(lst, pred);
                std::ranges::stable_partition(lst_exp, pred);
                RTTS_EXPECT(rl.end() == lst.end());
                RTTS_ALL_EQUAL(lst, lst_exp);
            }
        }
    }
}

RTTS_CASE_TPL("raze::algorithm::stable_partition fuzz", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    for (unsigned seed : fuzz_seeds)
        stable_partition_fuzz<T>(seed, 35);
};

RTTS_CASE("raze::algorithm::stable_partition stability fuzz") {
    for (unsigned seed : fuzz_seeds) {
        std::mt19937 rng(seed);
        for (size_t it = 0; it < 40; ++it) {
            const size_t n = static_cast<size_t>(rng() % 321);
            std::vector<rtts::test_item> v(n);
            for (size_t i = 0; i < n; ++i)
                v[i] = { static_cast<int>(rng() % 5u), static_cast<int>(i) };
            auto exp = v;
            const int target = static_cast<int>(rng() % 5u);
            auto pred = [target](int k) { return k <= target; };

            auto r = raze::algorithm::stable_partition(v, pred, &rtts::test_item::key);
            auto er = std::ranges::stable_partition(exp, pred, &rtts::test_item::key);
            RTTS_EXPECT(r.begin() - v.begin() == er.begin() - exp.begin() && r.end() == v.end());
            RTTS_ALL_EQUAL(v, exp);
        }
    }
};
