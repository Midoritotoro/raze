#include <tests/rts/rts.h>
#include <raze/algorithm/extreme/MaxElement.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <functional>
#include <random>
#include <ranges>
#include <type_traits>
#include <vector>

RTTS_CASE_TPL("raze::algorithm::max_element", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    using namespace rtts::algorithm;
    namespace opt = raze::options;
    auto cfg = config::thorough();

    auto check_iter = [&](auto algo) {
        run<range, T>(cfg,
            [algo](auto c) { return algo(c.first(), c.last()); },
            [](auto c) { return std::ranges::max_element(c.first(), c.last()); });
    };

    auto check_range = [&](auto algo) {
        run<range, T>(cfg,
            [algo](auto c) { return algo(c.range()); },
            [](auto c) { return std::ranges::max_element(c.range()); });
    };

    auto check_cmp = [&](auto algo, auto cmp) {
        run<range, T>(cfg,
            [algo, cmp](auto c) { return algo(c.range(), cmp); },
            [cmp](auto c) { return std::ranges::max_element(c.range(), cmp); });
    };

    check_iter(raze::algorithm::max_element);
    check_range(raze::algorithm::max_element);
    check_iter(raze::algorithm::max_element[opt::fscalar]);
    check_range(raze::algorithm::max_element[opt::unroll<2>]);
    check_range(raze::algorithm::max_element[opt::fstatic]);
    check_cmp(raze::algorithm::max_element, std::greater<>{});
    check_cmp(raze::algorithm::max_element, std::ranges::greater{});
    check_cmp(raze::algorithm::max_element, std::less<T>{});

    auto cmp_branch = [](auto a, auto b) { if (a < b) return true; return false; };
    check_cmp(raze::algorithm::max_element[opt::fautovec], cmp_branch);

    {
        auto acfg = config::compact();
        auto check_aligned = [&]<size_t Align>(auto algo) {
            run_aligned<Align, range, T>(acfg,
                [algo](auto c) { return algo(c.first(), c.last()); },
                [](auto c) { return std::ranges::max_element(c.first(), c.last()); });
            run_aligned<Align, range, T>(acfg,
                [algo](auto c) { return algo(c.range()); },
                [](auto c) { return std::ranges::max_element(c.range()); });
        };

        check_aligned.template operator()<16>(raze::algorithm::max_element[opt::aligned<16>]);
        check_aligned.template operator()<32>(raze::algorithm::max_element[opt::aligned<32>]);
        check_aligned.template operator()<32>(raze::algorithm::max_element[opt::aligned<32>][opt::unroll<2>]);
        check_aligned.template operator()<32>(raze::algorithm::max_element[opt::aligned<32>][opt::fstatic]);
        check_aligned.template operator()<64>(raze::algorithm::max_element[opt::aligned<64>]);
        check_aligned.template operator()<64>(raze::algorithm::max_element[opt::aligned<64>][opt::fscalar]);
        check_aligned.template operator()<1>(raze::algorithm::max_element[opt::aligned<opt::unaligned>]);
        check_aligned.template operator()<1>(raze::algorithm::max_element[opt::aligned<64>][opt::aligned<opt::unaligned>]);
    }

    {
        for (size_t len : { size_t(1), size_t(7), size_t(16), size_t(31), size_t(32), size_t(64), size_t(65), size_t(130) }) {
            std::vector<T> eq(len, T(5));
            RTTS_EXPECT(raze::algorithm::max_element(eq) == eq.begin());

            for (size_t first_pos : { size_t(0), len / 3, len / 2, len - 1 }) {
                std::vector<T> v(len, T(1));
                v[first_pos] = T(100);
                if (first_pos + 1 < len) v[len - 1] = T(100);
                auto got = raze::algorithm::max_element(v);
                auto exp = std::ranges::max_element(v);
                RTTS_EQUAL(got - v.begin(), exp - v.begin());
            }
        }
    }

    {
        auto custom_vec_cmp = [](auto a, auto b) { return a > b; };
        auto v = rtts::random::vector<T>(300, 22222u);
        RTTS_EQUAL(raze::algorithm::max_element(v, custom_vec_cmp) - v.begin(),
                   std::ranges::max_element(v, custom_vec_cmp) - v.begin());
    }

    constexpr auto check_ce_range = [](auto algo) constexpr {
        return constexpr_run<T, range, 8>(
            [algo](auto c) constexpr { return algo(c.range()); },
            [](auto c) constexpr { return std::ranges::max_element(c.range()); });
    };

    static_assert(constexpr_run<T, range, 8>(
        [](auto c) constexpr { return raze::algorithm::max_element(c.first(), c.last()); },
        [](auto c) constexpr { return std::ranges::max_element(c.first(), c.last()); }));
    static_assert(check_ce_range(raze::algorithm::max_element[opt::fscalar]));
    static_assert(check_ce_range(raze::algorithm::max_element[opt::aligned<64>]));
};

RTTS_CASE("raze::algorithm::max_element chunking, signed zero, views, dangling, and edge cases") {
    namespace opt = raze::options;
    namespace views = std::views;

    // Multi-portion chunking for u8 / int8_t (> 255 * 64 elements!)
    {
        const size_t N = 20000;
        for (size_t pos : { size_t(0), size_t(254 * 16), size_t(255 * 16), size_t(255 * 32),
                            size_t(255 * 64 - 1), size_t(255 * 64), size_t(18765), N - 1 })
        {
            std::vector<uint8_t> v(N, 10);
            v[pos] = 250;
            if (pos + 100 < N) v[pos + 100] = 250; // duplicate after true first max!
            auto got = raze::algorithm::max_element(v);
            RTTS_EQUAL(static_cast<size_t>(got - v.begin()), pos);

            std::vector<int8_t> sv(N, -50);
            sv[pos] = 120;
            if (pos + 100 < N) sv[pos + 100] = 120;
            auto sgot = raze::algorithm::max_element(sv);
            RTTS_EQUAL(static_cast<size_t>(sgot - sv.begin()), pos);
        }
    }

    // -0.0f / +0.0f tie-breaking: MUST return iterator to FIRST zero!
    {
        std::vector<float> vf(64, -1.0f);
        vf[13] = -0.0f;
        vf[45] = +0.0f;
        RTTS_EQUAL(raze::algorithm::max_element(vf) - vf.begin(), 13);

        vf[13] = +0.0f;
        vf[45] = -0.0f;
        RTTS_EQUAL(raze::algorithm::max_element(vf) - vf.begin(), 13);
    }

    // Projections, views, dangling, volatile, empty
    {
        const std::vector<rtts::test_item> items = { { 5, 1 }, { 9, 10 }, { 9, 20 }, { 2, 3 } };
        auto it = raze::algorithm::max_element(items, std::less<>{}, &rtts::test_item::key);
        RTTS_EQUAL(it - items.begin(), 1);

        const std::vector<int> src = { 2, 9, 7, 1, 8, 9, 6, 4 };
        auto fview = src | views::filter([](int x) { return x < 9; });
        RTTS_EQUAL(*raze::algorithm::max_element(fview), 8);

        auto rview = src | views::reverse;
        RTTS_EQUAL(*raze::algorithm::max_element(rview), 9);
        RTTS_EQUAL(raze::algorithm::max_element(rview) - rview.begin(), 2); // index 5 in src == index 2 in reverse!

        auto nc = rtts::make_non_common(src);
        RTTS_EQUAL(raze::algorithm::max_element[opt::fstatic](nc) - src.begin(), 1);

        auto d = raze::algorithm::max_element(std::vector<int>{ 3, 1, 2 });
        static_assert(std::same_as<decltype(d), std::ranges::dangling>);
        (void)d;

        std::vector<int> empty_v;
        RTTS_EXPECT(raze::algorithm::max_element(empty_v) == empty_v.end());

        volatile int vbuf[16] = { 10, 9, 99, 7, 6, 99, 4, 3, 2, 1, 0, 5, 11, 12, 13, 14 };
        RTTS_EQUAL(raze::algorithm::max_element(vbuf, vbuf + 16) - vbuf, 2);
    }
};

