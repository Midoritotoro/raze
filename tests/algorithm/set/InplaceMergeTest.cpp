#include <tests/rts/rts.h>
#include <raze/algorithm/set/InplaceMerge.h>

#include <algorithm>
#include <array>
#include <list>
#include <memory>
#include <ranges>
#include <vector>
#include <random>

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

    static_assert([]() constexpr {
        std::array<T, 8> a{ T(1), T(3), T(5), T(7), T(2), T(3), T(6), T(8) };
        auto end = raze::algorithm::inplace_merge(a, a.begin() + 4);
        return end == a.end()
            && a == std::array<T, 8>{ T(1), T(2), T(3), T(3), T(5), T(6), T(7), T(8) };
    }());
};

RTTS_CASE("raze::algorithm::inplace_merge stability, list, move-only, constexpr stability, and dangling") {
    {
        std::vector<rtts::test_item> v;
        for (int i = 0; i < 150; ++i) v.push_back({ i / 3, i });
        for (int i = 0; i < 150; ++i) v.push_back({ i / 3, 1000 + i });
        auto exp = v;

        raze::algorithm::inplace_merge(v, v.begin() + 150, std::less<>{}, &rtts::test_item::key);
        std::ranges::inplace_merge(exp, exp.begin() + 150, std::less<>{}, &rtts::test_item::key);
        RTTS_ALL_EQUAL(v, exp);
    }

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

    static_assert([]() constexpr {
        std::array<rtts::test_item, 6> a{ {
            { 1, 1 }, { 2, 2 }, { 3, 3 }, { 2, 4 }, { 2, 5 }, { 4, 6 }
        } };
        raze::algorithm::inplace_merge(a, a.begin() + 3, std::less<>{}, &rtts::test_item::key);
        return a[1].payload == 2 && a[2].payload == 4 && a[3].payload == 5;
    }());
};

namespace {
    inline constexpr unsigned fuzz_seeds[] = { 42u, 0x9E3779B9u, 0xDEADBEEFu };

    template <class T>
    void inplace_merge_fuzz(unsigned seed, size_t iters) {
        namespace opt = raze::options;
        std::mt19937 rng(seed);

        for (size_t it = 0; it < iters; ++it) {
            const size_t n = static_cast<size_t>(rng() % 321);
            std::vector<T> base(n);
            if ((it & 1u) == 0u) {
                base = rtts::random::vector<T>(n, rng());
            }
            else {
                const unsigned alpha = (it % 4 == 1) ? 6u : 16u;
                for (size_t i = 0; i < n; ++i)
                    base[i] = static_cast<T>(rng() % alpha);
            }

            const size_t lo = n ? static_cast<size_t>(rng() % (n + 1)) : 0;
            const size_t hi = n ? (lo + static_cast<size_t>(rng() % (n - lo + 1))) : 0;
            const size_t mid = lo + (hi > lo ? static_cast<size_t>(rng() % (hi - lo + 1)) : 0);
            const bool desc = (it % 3 == 2);

            auto run_check = [&](auto comp) {
                std::ranges::sort(base.begin() + static_cast<std::ptrdiff_t>(lo),
                                  base.begin() + static_cast<std::ptrdiff_t>(mid), comp);
                std::ranges::sort(base.begin() + static_cast<std::ptrdiff_t>(mid),
                                  base.begin() + static_cast<std::ptrdiff_t>(hi), comp);

                auto exp = base;
                std::ranges::inplace_merge(exp.begin() + static_cast<std::ptrdiff_t>(lo),
                                           exp.begin() + static_cast<std::ptrdiff_t>(mid),
                                           exp.begin() + static_cast<std::ptrdiff_t>(hi), comp);

                auto verify = [&](auto fn) {
                    auto v = base;
                    auto sub = std::ranges::subrange(v.begin() + static_cast<std::ptrdiff_t>(lo),
                                                     v.begin() + static_cast<std::ptrdiff_t>(hi));
                    auto end = fn(sub, v.begin() + static_cast<std::ptrdiff_t>(mid));
                    RTTS_EXPECT(end == sub.end());
                    RTTS_ALL_EQUAL(v, exp);
                };

                verify([&](auto sub, auto m) { return raze::algorithm::inplace_merge(sub, m, comp); });
                verify([&](auto sub, auto m) { return raze::algorithm::inplace_merge(sub.begin(), m, sub.end(), comp); });
                verify([&](auto sub, auto m) { return raze::algorithm::inplace_merge[opt::fscalar](sub, m, comp); });
                verify([&](auto sub, auto m) { return raze::algorithm::inplace_merge[opt::fstatic](sub, m, comp); });

                if (it % 8 == 0 && (hi - lo) <= 64) {
                    std::list<T> lst(base.begin() + static_cast<std::ptrdiff_t>(lo),
                                     base.begin() + static_cast<std::ptrdiff_t>(hi));
                    auto lmid = std::ranges::next(lst.begin(), static_cast<std::ptrdiff_t>(mid - lo));
                    auto lend = raze::algorithm::inplace_merge(lst, lmid, comp);
                    RTTS_EXPECT(lend == lst.end());
                    RTTS_EXPECT(std::ranges::equal(lst, std::ranges::subrange(
                        exp.begin() + static_cast<std::ptrdiff_t>(lo),
                        exp.begin() + static_cast<std::ptrdiff_t>(hi))));
                }
            };

            if (desc) run_check(std::greater<>{});
            else      run_check(std::less<>{});
        }
    }
}

RTTS_CASE_TPL("raze::algorithm::inplace_merge fuzz", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    for (unsigned seed : fuzz_seeds)
        inplace_merge_fuzz<T>(seed, 30);
};

RTTS_CASE("raze::algorithm::inplace_merge stability fuzz") {
    for (unsigned seed : fuzz_seeds) {
        std::mt19937 rng(seed);
        for (size_t it = 0; it < 40; ++it) {
            const size_t n = static_cast<size_t>(rng() % 321);
            const size_t mid = n ? static_cast<size_t>(rng() % (n + 1)) : 0;
            std::vector<rtts::test_item> v(n);
            for (size_t i = 0; i < n; ++i)
                v[i] = { static_cast<int>(rng() % 12u), static_cast<int>(i) };
            std::ranges::stable_sort(v.begin(), v.begin() + static_cast<std::ptrdiff_t>(mid),
                                     std::less<>{}, &rtts::test_item::key);
            std::ranges::stable_sort(v.begin() + static_cast<std::ptrdiff_t>(mid), v.end(),
                                     std::less<>{}, &rtts::test_item::key);
            auto exp = v;

            auto end = raze::algorithm::inplace_merge(v, v.begin() + static_cast<std::ptrdiff_t>(mid),
                                                      std::less<>{}, &rtts::test_item::key);
            std::ranges::inplace_merge(exp, exp.begin() + static_cast<std::ptrdiff_t>(mid),
                                       std::less<>{}, &rtts::test_item::key);
            RTTS_EXPECT(end == v.end());
            RTTS_ALL_EQUAL(v, exp);
        }
    }
};
