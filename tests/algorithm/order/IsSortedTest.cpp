#include <tests/rts/rts.h>
#include <raze/algorithm/order/IsSorted.h> 

#include <algorithm>
#include <array>
#include <cstdint>
#include <functional>
#include <random>
#include <ranges>
#include <vector>

namespace {
    inline constexpr unsigned fuzz_seeds[] = { 42u, 0x9E3779B9u, 0xDEADBEEFu };

    struct is_sorted_item {
        int32_t key = 0;
        int32_t value = 0;
        bool operator==(const is_sorted_item&) const = default;
    };

    template <class T>
    void is_sorted_boundary_sweep() {
        constexpr size_t MaxN = 140;
        for (size_t n = 0; n <= MaxN; ++n) {
            std::vector<T> sorted(n);
            for (size_t i = 0; i < n; ++i)
                sorted[i] = static_cast<T>(i / 2 + 1);  

            const bool exp = std::ranges::is_sorted(sorted);
            RTTS_EXPECT(raze::algorithm::is_sorted(sorted) == exp);
            RTTS_EXPECT(raze::algorithm::is_sorted[raze::options::fscalar](sorted) == exp);

            for (size_t p = 1; p < n; ++p) {
                auto bad = sorted;
                bad[p] = static_cast<T>(bad[p - 1] - 1);
                const bool e = std::ranges::is_sorted(bad);
                RTTS_EXPECT(raze::algorithm::is_sorted(bad) == e);
                RTTS_EXPECT(raze::algorithm::is_sorted[raze::options::fscalar](bad) == e);
            }
        }
    }

    template <class T>
    void is_sorted_fuzz(unsigned seed, size_t iters) {
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
            RTTS_EQUAL(raze::algorithm::is_sorted(v), std::ranges::is_sorted(v));
            RTTS_EQUAL(raze::algorithm::is_sorted(v, std::greater<>{}), std::ranges::is_sorted(v, std::greater<>{}));
            RTTS_EQUAL(raze::algorithm::is_sorted[raze::options::fscalar](v), std::ranges::is_sorted(v));
        }
    }

    template <class T>
    void is_sorted_struct_fuzz(unsigned seed, size_t iters) {
        std::mt19937 rng(seed);
        std::uniform_int_distribution<size_t> sz(0, 400);
        for (size_t i = 0; i < iters; ++i) {
            const size_t n = sz(rng);
            auto v = rtts::random::vector<T>(n, seed + static_cast<unsigned>(i));
            if (i % 2 == 0) std::ranges::sort(v, std::less<>{}, &T::key);
            RTTS_EQUAL(raze::algorithm::is_sorted(v, std::less<>{}, &T::key),
                       std::ranges::is_sorted(v, std::less<>{}, &T::key));
        }
    }
}

namespace {
    template <class T, std::size_t N>
    constexpr bool is_sorted_constexpr(std::array<T, N> a) {
        return raze::algorithm::is_sorted(a);
    }
}

RTTS_CASE_TPL("raze::algorithm::is_sorted", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    using namespace rtts::algorithm;
    namespace opt = raze::options;
    auto cfg = config::thorough();

    auto check_iter = [&](auto algo) {
        run<range, T>(cfg,
            [algo](auto c) { return algo(c.first(), c.last()); },
            [](auto c) { return std::ranges::is_sorted(c.first(), c.last()); });
    };

    auto check_range = [&](auto algo) {
        run<range, T>(cfg,
            [algo](auto c) { return algo(c.range()); },
            [](auto c) { return std::ranges::is_sorted(c.range()); });
    };

    auto check_cmp = [&](auto algo, auto cmp) {
        run<range, T>(cfg,
            [algo, cmp](auto c) { return algo(c.range(), cmp); },
            [cmp](auto c) { return std::ranges::is_sorted(c.range(), cmp); });
    };

    check_iter(raze::algorithm::is_sorted);
    check_range(raze::algorithm::is_sorted);
    check_iter(raze::algorithm::is_sorted[opt::fscalar]);
    check_range(raze::algorithm::is_sorted[opt::unroll<2>]);
    check_range(raze::algorithm::is_sorted[opt::fstatic]);
    check_cmp(raze::algorithm::is_sorted, std::greater<>{});

    auto cmp_branching = [](auto a, auto b) { if (a < b) return true; return false; };
    check_iter(raze::algorithm::is_sorted[opt::fautovec]);
    check_cmp(raze::algorithm::is_sorted[opt::fautovec], cmp_branching);
    check_cmp(raze::algorithm::is_sorted[opt::fautovec][opt::fstatic], cmp_branching);

    {
        auto acfg = config::compact();

        auto check_aligned = [&]<size_t Align>(auto algo) {
            run_aligned<Align, range, T>(acfg,
                [algo](auto c) { return algo(c.first(), c.last()); },
                [](auto c) { return std::ranges::is_sorted(c.first(), c.last()); });

            run_aligned<Align, range, T>(acfg,
                [algo](auto c) { return algo(c.range()); },
                [](auto c) { return std::ranges::is_sorted(c.range()); });
        };

        check_aligned.template operator()<16>(raze::algorithm::is_sorted[opt::aligned<16>]);
        check_aligned.template operator()<32>(raze::algorithm::is_sorted[opt::aligned<32>]);
        check_aligned.template operator()<32>(raze::algorithm::is_sorted[opt::aligned<32>][opt::unroll<2>]);
        check_aligned.template operator()<32>(raze::algorithm::is_sorted[opt::aligned<32>][opt::fstatic]);
        check_aligned.template operator()<64>(raze::algorithm::is_sorted[opt::aligned<64>]);
        check_aligned.template operator()<64>(raze::algorithm::is_sorted[opt::aligned<64>][opt::fautovec]);
        check_aligned.template operator()<64>(raze::algorithm::is_sorted[opt::aligned<64>][opt::fscalar]);
        check_aligned.template operator()<1>(raze::algorithm::is_sorted[opt::aligned<opt::unaligned>]);
        check_aligned.template operator()<1>(raze::algorithm::is_sorted[opt::aligned<64>][opt::aligned<opt::unaligned>]);
    }

    constexpr auto check_ce = [](auto algo) constexpr {
        return constexpr_run<T, range, 8>(
            [algo](auto c) constexpr { return algo(c.range()); },
            [](auto c) constexpr { return std::ranges::is_sorted(c.range()); });
    };

    static_assert(constexpr_run<T, range, 8>(
        [](auto c) constexpr { return raze::algorithm::is_sorted(c.first(), c.last()); },
        [](auto c) constexpr { return std::ranges::is_sorted(c.first(), c.last()); }));
    static_assert(check_ce(raze::algorithm::is_sorted[opt::fscalar]));
    static_assert(check_ce(raze::algorithm::is_sorted[opt::fautovec]));
    static_assert(check_ce(raze::algorithm::is_sorted[opt::aligned<64>]));

    static_assert(is_sorted_constexpr(std::array<T, 1>{T(1)}));
    static_assert(is_sorted_constexpr(std::array<T, 3>{T(1), T(2), T(3)}));
    static_assert(is_sorted_constexpr(std::array<T, 3>{T(2), T(2), T(2)}));
    static_assert(!is_sorted_constexpr(std::array<T, 3>{T(3), T(1), T(2)}));

    static_assert([]() constexpr {
        std::array<T, 1> a{ T(1) };
        return raze::algorithm::is_sorted(std::ranges::subrange(a.begin(), a.begin()));
    }());
};

RTTS_CASE_TPL("raze::algorithm::is_sorted.projection", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    using namespace rtts::algorithm;
    namespace opt = raze::options;
    auto cfg = config::thorough();
    auto proj = [](auto x) { return x; };

    auto check_range = [&](auto algo) {
        run<range, T>(cfg,
            [algo, proj](auto c) { return algo(c.range(), std::less<>{}, proj); },
            [proj](auto c) { return std::ranges::is_sorted(c.range(), std::less<>{}, proj); });
    };

    check_range(raze::algorithm::is_sorted);
    run<range, T>(cfg,
        [proj](auto c) { return raze::algorithm::is_sorted[opt::fscalar](c.first(), c.last(), std::less<>{}, proj); },
        [proj](auto c) { return std::ranges::is_sorted(c.first(), c.last(), std::less<>{}, proj); });
    check_range(raze::algorithm::is_sorted[opt::fautovec]);

    auto check_aligned = [&]<size_t Align>(auto algo) {
        run_aligned<Align, range, T>(config::compact(),
            [algo, proj](auto c) { return algo(c.range(), std::less<>{}, proj); },
            [proj](auto c) { return std::ranges::is_sorted(c.range(), std::less<>{}, proj); });
    };
    check_aligned.template operator()<64>(raze::algorithm::is_sorted[opt::aligned<64>]);
    check_aligned.template operator()<1>(raze::algorithm::is_sorted[opt::aligned<opt::unaligned>]);

    static_assert(constexpr_run<T, range, 8>(
        [](auto c) constexpr { return raze::algorithm::is_sorted(c.range(), std::less<>{}, [](auto x) constexpr { return x; }); },
        [](auto c) constexpr { return std::ranges::is_sorted(c.range(), std::less<>{}, [](auto x) constexpr { return x; }); }));
};

RTTS_CASE_TPL("raze::algorithm::is_sorted boundaries", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    is_sorted_boundary_sweep<T>();
};

RTTS_CASE_TPL("raze::algorithm::is_sorted fuzz", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    for (unsigned seed : fuzz_seeds)
        is_sorted_fuzz<T>(seed, 200);
};

RTTS_CASE("raze::algorithm::is_sorted structs") {
    using namespace rtts::algorithm;
    auto cfg = config::thorough();

    run<range, is_sorted_item>(cfg,
        [](auto c) { return raze::algorithm::is_sorted(c.range(), std::less<>{}, &is_sorted_item::key); },
        [](auto c) { return std::ranges::is_sorted(c.range(), std::less<>{}, &is_sorted_item::key); });

    run<range, is_sorted_item>(cfg,
        [](auto c) { return raze::algorithm::is_sorted[raze::options::fscalar](c.first(), c.last(), std::less<>{}, &is_sorted_item::key); },
        [](auto c) { return std::ranges::is_sorted(c.first(), c.last(), std::less<>{}, &is_sorted_item::key); });

    static_assert(constexpr_run<is_sorted_item, range, 8>(
        [](auto c) constexpr { return raze::algorithm::is_sorted(c.range(), std::less<>{}, &is_sorted_item::key); },
        [](auto c) constexpr { return std::ranges::is_sorted(c.range(), std::less<>{}, &is_sorted_item::key); }));
};

RTTS_CASE("raze::algorithm::is_sorted structs fuzz") {
    for (unsigned seed : fuzz_seeds)
        is_sorted_struct_fuzz<is_sorted_item>(seed, 200);
};

RTTS_CASE("raze::algorithm::is_sorted views and edge cases") {
    namespace opt = raze::options;
    namespace views = std::views;

    {
        RTTS_EXPECT(raze::algorithm::is_sorted(views::iota(1, 50)));
        RTTS_EXPECT(!raze::algorithm::is_sorted(views::iota(1, 50) | views::reverse));
        RTTS_EXPECT(raze::algorithm::is_sorted(views::iota(1, 50) | views::reverse, std::greater<>{}));

        const std::vector<int> src = { 10, 1, 20, 2, 30, 3, 40, 4, 50, 5 };
        auto evens_gt5 = src | views::filter([](int x) { return x >= 10; });
        RTTS_EXPECT(raze::algorithm::is_sorted(evens_gt5));
        RTTS_EXPECT(raze::algorithm::is_sorted[opt::aligned<64>](src | views::filter([](int x) { return x < 10; })));
        RTTS_EXPECT(!raze::algorithm::is_sorted(src | views::drop(1) | views::take(6)));

        auto squared = views::iota(-5, 6) | views::transform([](int x) { return x * x; });
        RTTS_EXPECT(!raze::algorithm::is_sorted(squared));
        RTTS_EXPECT(raze::algorithm::is_sorted(views::iota(0, 20) | views::transform([](int x) { return x * x; })));

        const std::vector<int> sorted = { 1, 2, 2, 3, 5, 8, 13, 21 };
        RTTS_EXPECT(raze::algorithm::is_sorted[opt::fstatic](rtts::make_non_common(sorted)));
        RTTS_EXPECT(raze::algorithm::is_sorted[opt::fautovec][opt::fstatic](
            sorted, [](auto a, auto b) { if (a < b) return true; return false; }));
    }

    {
        std::vector<int> empty_v;
        RTTS_EXPECT(raze::algorithm::is_sorted(empty_v));
        RTTS_EXPECT(raze::algorithm::is_sorted(std::vector<int>{ 42 }));
        RTTS_EXPECT(raze::algorithm::is_sorted(std::vector<int>{ 1, 2 }));
        RTTS_EXPECT(raze::algorithm::is_sorted(std::vector<int>{ 2, 2 }));
        RTTS_EXPECT(!raze::algorithm::is_sorted(std::vector<int>{ 2, 1 }));
        RTTS_EXPECT(raze::algorithm::is_sorted(std::vector<int>(64, 7)));

        volatile int vbuf[16];
        for (int i = 0; i < 16; ++i) vbuf[i] = i;
        RTTS_EXPECT(raze::algorithm::is_sorted(vbuf, vbuf + 16));
        vbuf[15] = 0;
        RTTS_EXPECT(!raze::algorithm::is_sorted(vbuf, vbuf + 16));

        const std::vector<rtts::test_item> items = { { 1, 90 }, { 2, 10 }, { 2, 50 }, { 5, 0 } };
        RTTS_EXPECT(raze::algorithm::is_sorted(items, std::less<>{}, &rtts::test_item::key));
        RTTS_EXPECT(!raze::algorithm::is_sorted(items, std::less<>{}, &rtts::test_item::payload));
    }
};