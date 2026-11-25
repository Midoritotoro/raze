#include <tests/rts/rts.h>
#include <raze/algorithm/extreme/MinElement.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <functional>
#include <random>
#include <ranges>
#include <type_traits>
#include <vector>

RTTS_CASE_TPL("raze::algorithm::min_element", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    using namespace rtts::algorithm;
    namespace opt = raze::options;
    auto cfg = config::thorough();

    run<range, T>(cfg,
        [](auto c) { return raze::algorithm::min_element(c.first(), c.last()); },
        [](auto c) { return std::ranges::min_element(c.first(), c.last()); });

    run<range, T>(cfg,
        [](auto c) { return raze::algorithm::min_element(c.range()); },
        [](auto c) { return std::ranges::min_element(c.range()); });

    run<range, T>(cfg,
        [](auto c) { return raze::algorithm::min_element[opt::fscalar](c.first(), c.last()); },
        [](auto c) { return std::ranges::min_element(c.first(), c.last()); });

    run<range, T>(cfg,
        [](auto c) { return raze::algorithm::min_element[opt::unroll<2>](c.range()); },
        [](auto c) { return std::ranges::min_element(c.range()); });

    run<range, T>(cfg,
        [](auto c) { return raze::algorithm::min_element[opt::fstatic](c.range()); },
        [](auto c) { return std::ranges::min_element(c.range()); });

    run<range, T>(cfg,
        [](auto c) { return raze::algorithm::min_element(c.range(), std::greater<>{}); },
        [](auto c) { return std::ranges::min_element(c.range(), std::greater<>{}); });

    run<range, T>(cfg,
        [](auto c) { return raze::algorithm::min_element(c.range(), std::ranges::less{}); },
        [](auto c) { return std::ranges::min_element(c.range(), std::ranges::less{}); });

    run<range, T>(cfg,
        [](auto c) { return raze::algorithm::min_element(c.range(), std::less<T>{}); },
        [](auto c) { return std::ranges::min_element(c.range(), std::less<T>{}); });

    auto cmp_branch = [](auto a, auto b) { if (a < b) return true; return false; };
    run<range, T>(cfg,
        [cmp_branch](auto c) { return raze::algorithm::min_element[opt::fautovec](c.range(), cmp_branch); },
        [cmp_branch](auto c) { return std::ranges::min_element(c.range(), cmp_branch); });

    {
        auto acfg = config::compact();
        auto check_aligned = [&]<size_t Align>(auto algo) {
            run_aligned<Align, range, T>(acfg,
                [algo](auto c) { return algo(c.first(), c.last()); },
                [](auto c) { return std::ranges::min_element(c.first(), c.last()); });
            run_aligned<Align, range, T>(acfg,
                [algo](auto c) { return algo(c.range()); },
                [](auto c) { return std::ranges::min_element(c.range()); });
        };

        check_aligned.template operator()<16>(raze::algorithm::min_element[opt::aligned<16>]);
        check_aligned.template operator()<32>(raze::algorithm::min_element[opt::aligned<32>]);
        check_aligned.template operator()<32>(raze::algorithm::min_element[opt::aligned<32>][opt::unroll<2>]);
        check_aligned.template operator()<32>(raze::algorithm::min_element[opt::aligned<32>][opt::fstatic]);
        check_aligned.template operator()<64>(raze::algorithm::min_element[opt::aligned<64>]);
        check_aligned.template operator()<64>(raze::algorithm::min_element[opt::aligned<64>][opt::fscalar]);
        check_aligned.template operator()<1>(raze::algorithm::min_element[opt::aligned<opt::unaligned>]);
        check_aligned.template operator()<1>(raze::algorithm::min_element[opt::aligned<64>][opt::aligned<opt::unaligned>]);
    }

    // Exact tie-breaking sweep: all-equal, ties in lane 0 / mid / last / tail
    {
        for (size_t len : { size_t(1), size_t(7), size_t(16), size_t(31), size_t(32), size_t(64), size_t(65), size_t(130) }) {
            std::vector<T> eq(len, T(5));
            RTTS_EXPECT(raze::algorithm::min_element(eq) == eq.begin());

            for (size_t first_pos : { size_t(0), len / 3, len / 2, len - 1 }) {
                std::vector<T> v(len, T(50));
                v[first_pos] = T(1);
                if (first_pos + 1 < len) v[len - 1] = T(1);
                auto got = raze::algorithm::min_element(v);
                auto exp = std::ranges::min_element(v);
                RTTS_EQUAL(got - v.begin(), exp - v.begin());
            }
        }
    }

    // Custom vectorizable comparator
    {
        auto custom_vec_cmp = [](auto a, auto b) { return a > b; };
        auto v = rtts::random::vector<T>(300, 11111u);
        RTTS_EQUAL(raze::algorithm::min_element(v, custom_vec_cmp) - v.begin(),
                   std::ranges::min_element(v, custom_vec_cmp) - v.begin());
    }

    static_assert(constexpr_run<T, range, 8>(
        [](auto c) constexpr { return raze::algorithm::min_element(c.first(), c.last()); },
        [](auto c) constexpr { return std::ranges::min_element(c.first(), c.last()); }));

    static_assert(constexpr_run<T, range, 8>(
        [](auto c) constexpr { return raze::algorithm::min_element[opt::fscalar](c.range()); },
        [](auto c) constexpr { return std::ranges::min_element(c.range()); }));

    static_assert(constexpr_run<T, range, 8>(
        [](auto c) constexpr { return raze::algorithm::min_element[opt::aligned<64>](c.range()); },
        [](auto c) constexpr { return std::ranges::min_element(c.range()); }));
};

RTTS_CASE("raze::algorithm::min_element chunking, signed zero, views, dangling, and edge cases") {
    namespace opt = raze::options;
    namespace views = std::views;

    // Multi-portion chunking for u8 / int8_t (> 255 * 64 elements!)
    {
        const size_t N = 20000;
        for (size_t pos : { size_t(0), size_t(254 * 16), size_t(255 * 16), size_t(255 * 32),
                            size_t(255 * 64 - 1), size_t(255 * 64), size_t(18765), N - 1 })
        {
            std::vector<uint8_t> v(N, 200);
            v[pos] = 3;
            if (pos + 100 < N) v[pos + 100] = 3; // duplicate after true first min!
            auto got = raze::algorithm::min_element(v);
            RTTS_EQUAL(static_cast<size_t>(got - v.begin()), pos);

            std::vector<int8_t> sv(N, 100);
            sv[pos] = -120;
            if (pos + 100 < N) sv[pos + 100] = -120;
            auto sgot = raze::algorithm::min_element(sv);
            RTTS_EQUAL(static_cast<size_t>(sgot - sv.begin()), pos);
        }
    }

    // -0.0f / +0.0f tie-breaking: MUST return iterator to FIRST zero!
    {
        std::vector<float> vf(64, 1.0f);
        vf[13] = +0.0f;
        vf[45] = -0.0f;
        RTTS_EQUAL(raze::algorithm::min_element(vf) - vf.begin(), 13);

        vf[13] = -0.0f;
        vf[45] = +0.0f;
        RTTS_EQUAL(raze::algorithm::min_element(vf) - vf.begin(), 13);

        std::vector<double> vd(64, 1.0);
        vd[5] = -0.0;
        vd[50] = +0.0;
        RTTS_EQUAL(raze::algorithm::min_element(vd) - vd.begin(), 5);
    }

    // Projections, views, dangling, volatile, empty
    {
        const std::vector<rtts::test_item> items = { { 5, 1 }, { 2, 10 }, { 2, 20 }, { 9, 3 } };
        auto it = raze::algorithm::min_element(items, std::less<>{}, &rtts::test_item::key);
        RTTS_EQUAL(it - items.begin(), 1);

        const std::vector<int> src = { 9, 2, 7, 1, 8, 1, 6, 4 };
        auto fview = src | views::filter([](int x) { return x > 2; });
        RTTS_EQUAL(*raze::algorithm::min_element(fview), 4);

        auto rview = src | views::reverse;
        RTTS_EQUAL(*raze::algorithm::min_element(rview), 1);
        RTTS_EQUAL(raze::algorithm::min_element(rview) - rview.begin(), 2); // index 5 in src == index 2 in reverse!

        auto nc = rtts::make_non_common(src);
        RTTS_EQUAL(raze::algorithm::min_element[opt::fstatic](nc) - src.begin(), 3);

        // Rvalue non-borrowed range returns std::ranges::dangling
        auto d = raze::algorithm::min_element(std::vector<int>{ 3, 1, 2 });
        static_assert(std::same_as<decltype(d), std::ranges::dangling>);
        (void)d;

        std::vector<int> empty_v;
        RTTS_EXPECT(raze::algorithm::min_element(empty_v) == empty_v.end());

        volatile int vbuf[16] = { 10, 9, -5, 7, 6, -5, 4, 3, 2, 1, 0, 5, 11, 12, 13, 14 };
        RTTS_EQUAL(raze::algorithm::min_element(vbuf, vbuf + 16) - vbuf, 2);
    }
};

