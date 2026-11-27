#include <tests/rts/rts.h>
#include <raze/algorithm/partition/Partition.h>

#include <algorithm>
#include <array>
#include <forward_list>
#include <list>
#include <ranges>
#include <vector>
#include <random>

RTTS_CASE_TPL("raze::algorithm::partition", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    namespace opt = raze::options;
    auto pred = [](T x) { return static_cast<int>(x) % 2 == 0; };

    for (size_t sz : { size_t(0), size_t(1), size_t(2), size_t(7), size_t(31), size_t(128) }) {
        const auto base = rtts::random::vector<T>(sz, 300u + static_cast<unsigned>(sz));

        auto check = [&](auto fn) {
            auto v = base;
            auto r = fn(v);
            RTTS_EXPECT(r.end() == v.end());
            RTTS_EXPECT(std::ranges::is_partitioned(v, pred));
            RTTS_EXPECT(r.begin() == std::ranges::partition_point(v, pred));
            RTTS_EXPECT(std::ranges::is_permutation(v, base));
        };

        check([&](auto& v) { return raze::algorithm::partition(v, pred); });
        check([&](auto& v) { return raze::algorithm::partition(v.begin(), v.end(), pred); });
        check([&](auto& v) { return raze::algorithm::partition[opt::fscalar](v, pred); });
        check([&](auto& v) { return raze::algorithm::partition[opt::fstatic](v, pred); });
    }

    static_assert([]() constexpr {
        std::array<T, 6> a{ T(1), T(2), T(3), T(4), T(5), T(6) };
        auto is_even = [](T x) constexpr { return (static_cast<int>(x) % 2) == 0; };
        auto r = raze::algorithm::partition(a, is_even);
        return r.begin() == a.begin() + 3 && r.end() == a.end()
            && std::ranges::is_partitioned(a, is_even);
    }());
};

RTTS_CASE("raze::algorithm::partition forward_list, list, projections, non-common, and dangling") {
    auto is_even = [](int x) { return x % 2 == 0; };

    {
        std::forward_list<int> fl{ 1, 2, 3, 4, 5, 6, 7, 8 };
        auto orig = fl;
        auto r = raze::algorithm::partition(fl, is_even);
        RTTS_EXPECT(r.end() == fl.end());
        RTTS_EXPECT(std::ranges::is_partitioned(fl, is_even));
        RTTS_EXPECT(r.begin() == std::ranges::partition_point(fl, is_even));
        RTTS_EXPECT(std::ranges::is_permutation(fl, orig));
    }

    {
        std::list<int> lst{ 7, 2, 9, 4, 1, 6 };
        auto orig = lst;
        auto r = raze::algorithm::partition(lst, is_even);
        RTTS_EXPECT(r.end() == lst.end());
        RTTS_EXPECT(std::ranges::is_partitioned(lst, is_even));
        RTTS_EXPECT(std::ranges::is_permutation(lst, orig));
    }

    {
        std::vector<rtts::test_item> items{ { 1, 10 }, { 2, 20 }, { 3, 30 }, { 4, 40 } };
        auto r = raze::algorithm::partition(items, is_even, &rtts::test_item::key);
        RTTS_EXPECT(std::ranges::is_partitioned(items, is_even, &rtts::test_item::key));
        RTTS_EXPECT(r.begin() == items.begin() + 2);

        std::vector<int> v{ 1, 2, 3, 4, 5 };
        auto nr = raze::algorithm::partition(rtts::make_non_common(v), is_even);
        RTTS_EXPECT(nr.end() == v.end() && std::ranges::is_partitioned(v, is_even));

        auto dr = raze::algorithm::partition(std::vector<int>{ 1, 2, 3 }, is_even);
        static_assert(std::same_as<decltype(dr), std::ranges::dangling>);
    }
};

namespace {
    inline constexpr unsigned fuzz_seeds[] = { 42u, 0x9E3779B9u, 0xDEADBEEFu };

    template <class T>
    void partition_fuzz(unsigned seed, size_t iters) {
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

            std::vector<T> exp_sorted(base.begin() + static_cast<std::ptrdiff_t>(lo),
                                      base.begin() + static_cast<std::ptrdiff_t>(hi));
            std::ranges::sort(exp_sorted);

            auto verify = [&](auto fn) {
                auto v = base;
                auto sub = std::ranges::subrange(v.begin() + static_cast<std::ptrdiff_t>(lo),
                                                 v.begin() + static_cast<std::ptrdiff_t>(hi));
                auto r = fn(sub);
                RTTS_EXPECT(r.end() == sub.end());
                RTTS_EXPECT(std::ranges::is_partitioned(sub, pred));
                RTTS_EXPECT(r.begin() == std::ranges::partition_point(sub, pred));
                RTTS_EXPECT(std::equal(v.begin(), v.begin() + static_cast<std::ptrdiff_t>(lo), base.begin()));
                RTTS_EXPECT(std::equal(v.begin() + static_cast<std::ptrdiff_t>(hi), v.end(),
                                       base.begin() + static_cast<std::ptrdiff_t>(hi)));
                std::vector<T> got_sorted(sub.begin(), sub.end());
                std::ranges::sort(got_sorted);
                RTTS_ALL_EQUAL(got_sorted, exp_sorted);
            };

            verify([&](auto sub) { return raze::algorithm::partition(sub, pred); });
            verify([&](auto sub) { return raze::algorithm::partition(sub.begin(), sub.end(), pred); });
            verify([&](auto sub) { return raze::algorithm::partition[opt::fscalar](sub, pred); });
            verify([&](auto sub) { return raze::algorithm::partition[opt::fstatic](sub, pred); });

            if (it % 8 == 0 && (hi - lo) <= 64) {
                std::forward_list<T> fl(base.begin() + static_cast<std::ptrdiff_t>(lo),
                                        base.begin() + static_cast<std::ptrdiff_t>(hi));
                auto rf = raze::algorithm::partition(fl, pred);
                RTTS_EXPECT(rf.end() == fl.end() && std::ranges::is_partitioned(fl, pred));
                RTTS_EXPECT(rf.begin() == std::ranges::partition_point(fl, pred));

                std::list<T> lst(base.begin() + static_cast<std::ptrdiff_t>(lo),
                                 base.begin() + static_cast<std::ptrdiff_t>(hi));
                auto rl = raze::algorithm::partition(lst, pred);
                RTTS_EXPECT(rl.end() == lst.end() && std::ranges::is_partitioned(lst, pred));
                RTTS_EXPECT(rl.begin() == std::ranges::partition_point(lst, pred));
            }
        }
    }
}

RTTS_CASE_TPL("raze::algorithm::partition fuzz", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    for (unsigned seed : fuzz_seeds)
        partition_fuzz<T>(seed, 35);
};
