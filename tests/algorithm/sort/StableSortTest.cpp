#include <tests/rts/rts.h>
#include <raze/algorithm/sort/StableSort.h>

#include <algorithm>
#include <array>
#include <deque>
#include <memory>
#include <ranges>
#include <vector>
#include <random>

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

    static_assert([]() constexpr {
        std::array<T, 10> a{ T(5), T(2), T(8), T(1), T(9), T(2), T(7), T(3), T(5), T(0) };
        auto end = raze::algorithm::stable_sort(a);
        return end == a.end() && std::ranges::is_sorted(a);
    }());
};

RTTS_CASE("raze::algorithm::stable_sort stability, large runtime buffer, constexpr stability, and move-only") {
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

namespace {
    inline constexpr unsigned fuzz_seeds[] = { 42u, 0x9E3779B9u, 0xDEADBEEFu };

    template <class T>
    void stable_sort_fuzz(unsigned seed, size_t iters) {
        namespace opt = raze::options;
        std::mt19937 rng(seed);

        for (size_t it = 0; it < iters; ++it) {
            const size_t n = static_cast<size_t>(rng() % 351);
            std::vector<T> base(n);
            const unsigned shape = static_cast<unsigned>(it % 4);
            if (shape == 0u) {
                base = rtts::random::vector<T>(n, rng());
            }
            else if (shape == 1u) {
                for (size_t i = 0; i < n; ++i) base[i] = static_cast<T>(rng() % 6u);
            }
            else if (shape == 2u) {
                base = rtts::random::vector<T>(n, rng());
                std::ranges::sort(base);
                for (size_t k = 0; k < 4 && n >= 2; ++k)
                    std::swap(base[static_cast<size_t>(rng() % n)], base[static_cast<size_t>(rng() % n)]);
            }
            else {
                base = rtts::random::vector<T>(n, rng());
                std::ranges::sort(base, std::greater<>{});
            }

            const size_t lo = n ? static_cast<size_t>(rng() % (n + 1)) : 0;
            const size_t hi = n ? (lo + static_cast<size_t>(rng() % (n - lo + 1))) : 0;
            const bool desc = (it % 3 == 2);

            auto run_check = [&](auto comp) {
                auto exp = base;
                std::ranges::stable_sort(exp.begin() + static_cast<std::ptrdiff_t>(lo),
                                         exp.begin() + static_cast<std::ptrdiff_t>(hi), comp);

                auto verify = [&](auto fn) {
                    auto v = base;
                    auto sub = std::ranges::subrange(v.begin() + static_cast<std::ptrdiff_t>(lo),
                                                     v.begin() + static_cast<std::ptrdiff_t>(hi));
                    auto end = fn(sub);
                    RTTS_EXPECT(end == sub.end());
                    RTTS_ALL_EQUAL(v, exp);
                };

                verify([&](auto sub) { return raze::algorithm::stable_sort(sub, comp); });
                verify([&](auto sub) { return raze::algorithm::stable_sort(sub.begin(), sub.end(), comp); });
                verify([&](auto sub) { return raze::algorithm::stable_sort[opt::fscalar](sub, comp); });
                verify([&](auto sub) { return raze::algorithm::stable_sort[opt::fstatic](sub, comp); });

                if (it % 8 == 0 && (hi - lo) <= 64) {
                    std::deque<T> dq(base.begin() + static_cast<std::ptrdiff_t>(lo),
                                     base.begin() + static_cast<std::ptrdiff_t>(hi));
                    auto end = raze::algorithm::stable_sort(dq, comp);
                    RTTS_EXPECT(end == dq.end());
                    RTTS_EXPECT(std::ranges::equal(dq, std::ranges::subrange(
                        exp.begin() + static_cast<std::ptrdiff_t>(lo),
                        exp.begin() + static_cast<std::ptrdiff_t>(hi))));
                }
            };

            if (desc) run_check(std::greater<>{});
            else      run_check(std::less<>{});
        }
    }
}

RTTS_CASE_TPL("raze::algorithm::stable_sort fuzz", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    for (unsigned seed : fuzz_seeds)
        stable_sort_fuzz<T>(seed, 30);
};

RTTS_CASE("raze::algorithm::stable_sort stability fuzz") {
    for (unsigned seed : fuzz_seeds) {
        std::mt19937 rng(seed);
        for (size_t it = 0; it < 40; ++it) {
            const size_t n = static_cast<size_t>(rng() % 385);
            const unsigned mod = (it & 1u) ? 8u : 32u;
            std::vector<rtts::test_item> v(n);
            for (size_t i = 0; i < n; ++i)
                v[i] = { static_cast<int>(rng() % mod), static_cast<int>(i) };
            auto exp = v;

            if (it % 3 == 2) {
                auto end = raze::algorithm::stable_sort(v, std::greater<>{}, &rtts::test_item::key);
                std::ranges::stable_sort(exp, std::greater<>{}, &rtts::test_item::key);
                RTTS_EXPECT(end == v.end());
            }
            else {
                auto end = raze::algorithm::stable_sort(v, std::less<>{}, &rtts::test_item::key);
                std::ranges::stable_sort(exp, std::less<>{}, &rtts::test_item::key);
                RTTS_EXPECT(end == v.end());
            }
            RTTS_ALL_EQUAL(v, exp);
        }
    }
};
