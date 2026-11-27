#include <tests/rts/rts.h>
#include <raze/algorithm/order/IsSortedUntil.h>

#include <algorithm>
#include <array>
#include <cstdint>
#include <functional>
#include <random>
#include <ranges>
#include <vector>

namespace {
    inline constexpr unsigned fuzz_seeds[] = { 42u, 0x9E3779B9u, 0xDEADBEEFu };

    struct is_sorted_until_item {
        int32_t key = 0;
        int32_t value = 0;
        bool operator==(const is_sorted_until_item&) const = default;
    };

    template <class T>
    void is_sorted_until_boundary_sweep() {
        constexpr size_t MaxN = 140;
        for (size_t n = 0; n <= MaxN; ++n) {
            std::vector<T> sorted(n);
            for (size_t i = 0; i < n; ++i)
                sorted[i] = static_cast<T>(i / 2 + 1);

            auto check = [](auto& v) {
                const auto exp = std::ranges::is_sorted_until(v);
                RTTS_EXPECT(raze::algorithm::is_sorted_until(v) == exp);
                RTTS_EXPECT(raze::algorithm::is_sorted_until(v.begin(), v.end()) == exp);
                RTTS_EXPECT(raze::algorithm::is_sorted_until[raze::options::fscalar](v) == exp);
                RTTS_EXPECT(raze::algorithm::is_sorted_until[raze::options::unroll<2>](v) == exp);
                RTTS_EXPECT(raze::algorithm::is_sorted_until[raze::options::fstatic](v) == exp);
            };

            check(sorted);

            for (size_t p = 1; p < n; ++p) {
                auto bad = sorted;
                bad[p] = static_cast<T>(bad[p - 1] - 1);
                RTTS_EXPECT(std::ranges::is_sorted_until(bad) == bad.begin() + p);
                check(bad);
            }
        }
    }

    template <class T>
    void is_sorted_until_fuzz(unsigned seed, size_t iters) {
        std::mt19937 rng(seed);
        std::uniform_int_distribution<size_t> sz(0, 512);
        for (size_t i = 0; i < iters; ++i) {
            const size_t n = sz(rng);
            auto v = rtts::random::vector<T>(n, seed + static_cast<unsigned>(i));
            switch (i % 4) {
                case 1: std::ranges::sort(v); break;
                case 2: std::ranges::sort(v, std::greater<>{}); break;
                case 3: if (n > 1) { std::ranges::sort(v); std::swap(v[rng() % n], v[rng() % n]); } break;
                default: break;
            }
            RTTS_EXPECT(raze::algorithm::is_sorted_until(v) == std::ranges::is_sorted_until(v));
            RTTS_EXPECT(raze::algorithm::is_sorted_until(v, std::greater<>{}) == std::ranges::is_sorted_until(v, std::greater<>{}));
            RTTS_EXPECT(raze::algorithm::is_sorted_until[raze::options::fscalar](v) == std::ranges::is_sorted_until(v));
        }
    }

    template <class T>
    void is_sorted_until_struct_fuzz(unsigned seed, size_t iters) {
        std::mt19937 rng(seed);
        std::uniform_int_distribution<size_t> sz(0, 400);
        for (size_t i = 0; i < iters; ++i) {
            const size_t n = sz(rng);
            auto v = rtts::random::vector<T>(n, seed + static_cast<unsigned>(i));
            if (i % 2 == 0) std::ranges::sort(v, std::less<>{}, &T::key);
            RTTS_EXPECT(raze::algorithm::is_sorted_until(v, std::less<>{}, &T::key) ==
                        std::ranges::is_sorted_until(v, std::less<>{}, &T::key));
            RTTS_EXPECT(raze::algorithm::is_sorted_until[raze::options::fscalar](v, std::greater<>{}, &T::key) ==
                        std::ranges::is_sorted_until(v, std::greater<>{}, &T::key));
        }
    }
}

RTTS_CASE_TPL("raze::algorithm::is_sorted_until", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    using namespace rtts::algorithm;
    namespace opt = raze::options;
    auto cfg = config::thorough();

    auto check_iter = [&](auto algo) {
        run<range, T>(cfg,
            [algo](auto c) { return std::ranges::distance(c.first(), algo(c.first(), c.last())); },
            [](auto c) { return std::ranges::distance(c.first(), std::ranges::is_sorted_until(c.first(), c.last())); });
    };

    auto check_range = [&](auto algo) {
        run<range, T>(cfg,
            [algo](auto c) { return std::ranges::distance(c.first(), algo(c.range())); },
            [](auto c) { return std::ranges::distance(c.first(), std::ranges::is_sorted_until(c.range())); });
    };

    auto check_cmp = [&](auto algo, auto cmp) {
        run<range, T>(cfg,
            [algo, cmp](auto c) { return std::ranges::distance(c.first(), algo(c.range(), cmp)); },
            [cmp](auto c) { return std::ranges::distance(c.first(), std::ranges::is_sorted_until(c.range(), cmp)); });
    };

    check_iter(raze::algorithm::is_sorted_until);
    check_range(raze::algorithm::is_sorted_until);
    check_iter(raze::algorithm::is_sorted_until[opt::fscalar]);
    check_range(raze::algorithm::is_sorted_until[opt::unroll<2>]);
    check_range(raze::algorithm::is_sorted_until[opt::fstatic]);
    check_cmp(raze::algorithm::is_sorted_until, std::greater<>{});

    auto cmp_branching = [](auto a, auto b) { if (a < b) return true; return false; };
    check_iter(raze::algorithm::is_sorted_until[opt::fautovec]);
    check_cmp(raze::algorithm::is_sorted_until[opt::fautovec], cmp_branching);
    check_cmp(raze::algorithm::is_sorted_until[opt::fautovec][opt::fstatic], cmp_branching);

    {
        auto acfg = config::compact();

        auto check_aligned = [&]<size_t Align>(auto algo) {
            run_aligned<Align, range, T>(acfg,
                [algo](auto c) { return std::ranges::distance(c.first(), algo(c.first(), c.last())); },
                [](auto c) { return std::ranges::distance(c.first(), std::ranges::is_sorted_until(c.first(), c.last())); });

            run_aligned<Align, range, T>(acfg,
                [algo](auto c) { return std::ranges::distance(c.first(), algo(c.range())); },
                [](auto c) { return std::ranges::distance(c.first(), std::ranges::is_sorted_until(c.range())); });
        };

        check_aligned.template operator()<16>(raze::algorithm::is_sorted_until[opt::aligned<16>]);
        check_aligned.template operator()<32>(raze::algorithm::is_sorted_until[opt::aligned<32>]);
        check_aligned.template operator()<32>(raze::algorithm::is_sorted_until[opt::aligned<32>][opt::unroll<2>]);
        check_aligned.template operator()<32>(raze::algorithm::is_sorted_until[opt::aligned<32>][opt::fstatic]);
        check_aligned.template operator()<64>(raze::algorithm::is_sorted_until[opt::aligned<64>]);
        check_aligned.template operator()<64>(raze::algorithm::is_sorted_until[opt::aligned<64>][opt::fautovec]);
        check_aligned.template operator()<64>(raze::algorithm::is_sorted_until[opt::aligned<64>][opt::fscalar]);
        check_aligned.template operator()<1>(raze::algorithm::is_sorted_until[opt::aligned<opt::unaligned>]);
        check_aligned.template operator()<1>(raze::algorithm::is_sorted_until[opt::aligned<64>][opt::aligned<opt::unaligned>]);
    }

    constexpr auto check_ce = [](auto algo) constexpr {
        return constexpr_run<T, range, 8>(
            [algo](auto c) constexpr { return std::ranges::distance(c.first(), algo(c.range())); },
            [](auto c) constexpr { return std::ranges::distance(c.first(), std::ranges::is_sorted_until(c.range())); });
    };

    static_assert(constexpr_run<T, range, 8>(
        [](auto c) constexpr { return std::ranges::distance(c.first(), raze::algorithm::is_sorted_until(c.first(), c.last())); },
        [](auto c) constexpr { return std::ranges::distance(c.first(), std::ranges::is_sorted_until(c.first(), c.last())); }));
    static_assert(check_ce(raze::algorithm::is_sorted_until[opt::fscalar]));
    static_assert(check_ce(raze::algorithm::is_sorted_until[opt::fautovec]));
    static_assert(check_ce(raze::algorithm::is_sorted_until[opt::aligned<64>]));

    static_assert([]() constexpr {
        std::array<T, 0> a0{};
        std::array<T, 1> a1{ T(1) };
        std::array<T, 3> a_asc{ T(1), T(2), T(3) };
        std::array<T, 3> a_eq{ T(2), T(2), T(2) };
        std::array<T, 3> a_b1{ T(3), T(1), T(2) };
        std::array<T, 3> a_b2{ T(1), T(3), T(2) };
        return raze::algorithm::is_sorted_until(a0) == a0.end()
            && raze::algorithm::is_sorted_until(a1) == a1.end()
            && raze::algorithm::is_sorted_until(a_asc) == a_asc.end()
            && raze::algorithm::is_sorted_until(a_eq) == a_eq.end()
            && raze::algorithm::is_sorted_until(a_b1) == a_b1.begin() + 1
            && raze::algorithm::is_sorted_until(a_b2) == a_b2.begin() + 2;
    }());
};

RTTS_CASE_TPL("raze::algorithm::is_sorted_until.projection", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    using namespace rtts::algorithm;
    namespace opt = raze::options;
    auto cfg = config::thorough();
    auto proj = [](auto x) { return x; };

    auto check_range = [&](auto algo) {
        run<range, T>(cfg,
            [algo, proj](auto c) { return std::ranges::distance(c.first(), algo(c.range(), std::less<>{}, proj)); },
            [proj](auto c) { return std::ranges::distance(c.first(), std::ranges::is_sorted_until(c.range(), std::less<>{}, proj)); });
    };

    check_range(raze::algorithm::is_sorted_until);
    run<range, T>(cfg,
        [proj](auto c) { return std::ranges::distance(c.first(), raze::algorithm::is_sorted_until[opt::fscalar](c.first(), c.last(), std::less<>{}, proj)); },
        [proj](auto c) { return std::ranges::distance(c.first(), std::ranges::is_sorted_until(c.first(), c.last(), std::less<>{}, proj)); });
    check_range(raze::algorithm::is_sorted_until[opt::fautovec]);

    auto check_aligned = [&]<size_t Align>(auto algo) {
        run_aligned<Align, range, T>(config::compact(),
            [algo, proj](auto c) { return std::ranges::distance(c.first(), algo(c.range(), std::less<>{}, proj)); },
            [proj](auto c) { return std::ranges::distance(c.first(), std::ranges::is_sorted_until(c.range(), std::less<>{}, proj)); });
    };
    check_aligned.template operator()<64>(raze::algorithm::is_sorted_until[opt::aligned<64>]);
    check_aligned.template operator()<1>(raze::algorithm::is_sorted_until[opt::aligned<opt::unaligned>]);

    static_assert(constexpr_run<T, range, 8>(
        [](auto c) constexpr { return std::ranges::distance(c.first(), raze::algorithm::is_sorted_until(c.range(), std::less<>{}, [](auto x) constexpr { return x; })); },
        [](auto c) constexpr { return std::ranges::distance(c.first(), std::ranges::is_sorted_until(c.range(), std::less<>{}, [](auto x) constexpr { return x; })); }));
};

RTTS_CASE_TPL("raze::algorithm::is_sorted_until boundaries", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    is_sorted_until_boundary_sweep<T>();
};

RTTS_CASE_TPL("raze::algorithm::is_sorted_until fuzz", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    for (unsigned seed : fuzz_seeds)
        is_sorted_until_fuzz<T>(seed, 200);
};

RTTS_CASE("raze::algorithm::is_sorted_until structs") {
    using namespace rtts::algorithm;
    auto cfg = config::thorough();

    run<range, is_sorted_until_item>(cfg,
        [](auto c) { return std::ranges::distance(c.first(), raze::algorithm::is_sorted_until(c.range(), std::less<>{}, &is_sorted_until_item::key)); },
        [](auto c) { return std::ranges::distance(c.first(), std::ranges::is_sorted_until(c.range(), std::less<>{}, &is_sorted_until_item::key)); });

    run<range, is_sorted_until_item>(cfg,
        [](auto c) { return std::ranges::distance(c.first(), raze::algorithm::is_sorted_until[raze::options::fscalar](c.first(), c.last(), std::less<>{}, &is_sorted_until_item::key)); },
        [](auto c) { return std::ranges::distance(c.first(), std::ranges::is_sorted_until(c.first(), c.last(), std::less<>{}, &is_sorted_until_item::key)); });

    static_assert(constexpr_run<is_sorted_until_item, range, 8>(
        [](auto c) constexpr { return std::ranges::distance(c.first(), raze::algorithm::is_sorted_until(c.range(), std::less<>{}, &is_sorted_until_item::key)); },
        [](auto c) constexpr { return std::ranges::distance(c.first(), std::ranges::is_sorted_until(c.range(), std::less<>{}, &is_sorted_until_item::key)); }));
};

RTTS_CASE("raze::algorithm::is_sorted_until structs fuzz") {
    for (unsigned seed : fuzz_seeds)
        is_sorted_until_struct_fuzz<is_sorted_until_item>(seed, 200);
};

RTTS_CASE("raze::algorithm::is_sorted_until views and edge cases") {
    namespace opt = raze::options;
    namespace views = std::views;

    {
        auto iv = views::iota(1, 30);
        RTTS_EXPECT(raze::algorithm::is_sorted_until(iv) == iv.end());

        auto rv = views::iota(1, 30) | views::reverse;
        RTTS_EXPECT(raze::algorithm::is_sorted_until(rv) == std::ranges::is_sorted_until(rv));

        const std::vector<int> src = { 1, 2, 3, 10, 5, 6, 7 };
        auto dv = src | views::drop(1) | views::take(5);
        RTTS_EXPECT(raze::algorithm::is_sorted_until(dv) == std::ranges::is_sorted_until(dv));

        auto fv = src | views::filter([](int x) { return x != 10; });
        RTTS_EXPECT(raze::algorithm::is_sorted_until[opt::aligned<64>](fv) == std::ranges::is_sorted_until(fv));

        auto tv = src | views::transform([](int x) { return x * 2; });
        RTTS_EXPECT(raze::algorithm::is_sorted_until(tv) == std::ranges::is_sorted_until(tv));

        auto nc = rtts::make_non_common(src);
        RTTS_EXPECT(raze::algorithm::is_sorted_until[opt::fstatic](nc) == std::ranges::is_sorted_until(nc));
    }

    {
        std::vector<int> empty_v;
        RTTS_EXPECT(raze::algorithm::is_sorted_until(empty_v) == empty_v.end());

        std::vector<int> one_v = { 5 };
        RTTS_EXPECT(raze::algorithm::is_sorted_until(one_v) == one_v.end());

        volatile int vbuf[16];
        for (int i = 0; i < 16; ++i) vbuf[i] = i;
        vbuf[9] = 1;
        RTTS_EXPECT(raze::algorithm::is_sorted_until(vbuf, vbuf + 16) == vbuf + 9);

        auto rv = raze::algorithm::is_sorted_until(std::vector<int>{ 1, 2, 0 });
        static_assert(std::same_as<decltype(rv), std::ranges::dangling>);

        const std::vector<rtts::test_item> items = { { 1, 9 }, { 3, 8 }, { 2, 7 } };
        auto it = raze::algorithm::is_sorted_until(items, std::less<>{}, &rtts::test_item::key);
        RTTS_EXPECT(it == items.begin() + 2);
    }
};
