#include <tests/rts/rts.h>
#include <raze/algorithm/remove/Remove.h>

RTTS_CASE_TPL("raze::algorithm::remove", rtts::algorithm::all_types)
< class T > (rtts::type<T>) {
    auto cfg = rtts::algorithm::config::thorough();

    const T value = rtts::random::generator<T>(1000)();

    {
        rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
            [value](auto c) { return raze::algorithm::remove(c.first(), c.last(), value); },
            [value](auto c) { return std::ranges::remove(c.first(), c.last(), value); },
            rtts::algorithm::prefix_result);

        rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
            [value](auto c) { return raze::algorithm::remove(c.range(), value); },
            [value](auto c) { return std::ranges::remove(c.range(), value); },
            rtts::algorithm::prefix_result);
    }

    {
        rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
            [value](auto c) { return raze::algorithm::remove[raze::options::fscalar](c.first(), c.last(), value); },
            [value](auto c) { return std::ranges::remove(c.first(), c.last(), value); },
            rtts::algorithm::prefix_result);

        rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
            [value](auto c) { return raze::algorithm::remove[raze::options::fscalar](c.range(), value); },
            [value](auto c) { return std::ranges::remove(c.range(), value); },
            rtts::algorithm::prefix_result);

        rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
            [value](auto c) { return raze::algorithm::remove[raze::options::unroll<1>](c.range(), value); },
            [value](auto c) { return std::ranges::remove(c.range(), value); },
            rtts::algorithm::prefix_result);

        rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
            [value](auto c) { return raze::algorithm::remove[raze::options::fstatic](c.range(), value); },
            [value](auto c) { return std::ranges::remove(c.range(), value); },
            rtts::algorithm::prefix_result);

        rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
            [value](auto c) { return raze::algorithm::remove[raze::options::fscalar][raze::options::unroll<2>](c.range(), value); },
            [value](auto c) { return std::ranges::remove(c.range(), value); },
            rtts::algorithm::prefix_result);

        rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
            [value](auto c) { return raze::algorithm::remove[raze::options::fstatic][raze::options::unroll<2>](c.range(), value); },
            [value](auto c) { return std::ranges::remove(c.range(), value); },
            rtts::algorithm::prefix_result);

        rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
            [value](auto c) { return raze::algorithm::remove[raze::options::fautovec](c.first(), c.last(), value); },
            [value](auto c) { return std::ranges::remove(c.first(), c.last(), value); },
            rtts::algorithm::prefix_result);

        rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
            [value](auto c) { return raze::algorithm::remove[raze::options::fautovec][raze::options::fstatic](c.range(), value); },
            [value](auto c) { return std::ranges::remove(c.range(), value); },
            rtts::algorithm::prefix_result);
    }

    {
        namespace opt = raze::options;
        auto acfg = rtts::algorithm::config::compact();

        auto check_aligned = [&]<size_t Align>(auto algo) {
            rtts::algorithm::run_aligned<Align, rtts::algorithm::range, T>(acfg,
                [algo, value](auto c) { return algo(c.first(), c.last(), value); },
                [value](auto c) { return std::ranges::remove(c.first(), c.last(), value); },
                rtts::algorithm::prefix_result);

            rtts::algorithm::run_aligned<Align, rtts::algorithm::range, T>(acfg,
                [algo, value](auto c) { return algo(c.range(), value); },
                [value](auto c) { return std::ranges::remove(c.range(), value); },
                rtts::algorithm::prefix_result);
        };

        check_aligned.template operator()<16>(raze::algorithm::remove[opt::aligned<16>]);
        check_aligned.template operator()<32>(raze::algorithm::remove[opt::aligned<32>]);
        check_aligned.template operator()<32>(raze::algorithm::remove[opt::aligned<32>][opt::unroll<2>]);
        check_aligned.template operator()<32>(raze::algorithm::remove[opt::aligned<32>][opt::fstatic]);
        check_aligned.template operator()<64>(raze::algorithm::remove[opt::aligned<64>]);
        check_aligned.template operator()<64>(raze::algorithm::remove[opt::aligned<64>][opt::fautovec]);
        check_aligned.template operator()<64>(raze::algorithm::remove[opt::aligned<64>][opt::fscalar]);
        check_aligned.template operator()<1>(raze::algorithm::remove[opt::aligned<opt::unaligned>]);
        check_aligned.template operator()<1>(raze::algorithm::remove[opt::aligned<64>][opt::aligned<opt::unaligned>]);
    }

    static_assert(rtts::algorithm::constexpr_run<T, rtts::algorithm::range, 8>(
        [](auto c) constexpr { return raze::algorithm::remove(c.first(), c.last(), T(0)); },
        [](auto c) constexpr { return std::ranges::remove(c.first(), c.last(), T(0)); },
        rtts::algorithm::prefix_result));

    static_assert(rtts::algorithm::constexpr_run<T, rtts::algorithm::range, 8>(
        [](auto c) constexpr { return raze::algorithm::remove(c.range(), T(0)); },
        [](auto c) constexpr { return std::ranges::remove(c.range(), T(0)); },
        rtts::algorithm::prefix_result));

    static_assert(rtts::algorithm::constexpr_run<T, rtts::algorithm::range, 8>(
        [](auto c) constexpr { return raze::algorithm::remove[raze::options::fscalar](c.range(), T(0)); },
        [](auto c) constexpr { return std::ranges::remove(c.range(), T(0)); },
        rtts::algorithm::prefix_result));

    static_assert(rtts::algorithm::constexpr_run<T, rtts::algorithm::range, 8>(
        [](auto c) constexpr { return raze::algorithm::remove[raze::options::unroll<2>](c.range(), T(0)); },
        [](auto c) constexpr { return std::ranges::remove(c.range(), T(0)); },
        rtts::algorithm::prefix_result));

    static_assert(rtts::algorithm::constexpr_run<T, rtts::algorithm::range, 8>(
        [](auto c) constexpr { return raze::algorithm::remove[raze::options::fautovec](c.range(), T(0)); },
        [](auto c) constexpr { return std::ranges::remove(c.range(), T(0)); },
        rtts::algorithm::prefix_result));

    static_assert(rtts::algorithm::constexpr_run<T, rtts::algorithm::range, 8>(
        [](auto c) constexpr { return raze::algorithm::remove[raze::options::aligned<64>](c.range(), T(0)); },
        [](auto c) constexpr { return std::ranges::remove(c.range(), T(0)); },
        rtts::algorithm::prefix_result));
};

RTTS_CASE_TPL("raze::algorithm::remove.projection", rtts::algorithm::all_types)
< class T > (rtts::type<T>) {
    auto cfg = rtts::algorithm::config::thorough();
    auto acfg = rtts::algorithm::config::compact();

    const T value = rtts::random::generator<T>(12345)();
    auto proj = [](const T& x) -> const T& { return x; };

    rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
        [value, proj](auto c) { return raze::algorithm::remove(c.first(), c.last(), value, proj); },
        [value, proj](auto c) { return std::ranges::remove(c.first(), c.last(), value, proj); },
        rtts::algorithm::prefix_result);

    rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
        [value, proj](auto c) { return raze::algorithm::remove(c.range(), value, proj); },
        [value, proj](auto c) { return std::ranges::remove(c.range(), value, proj); },
        rtts::algorithm::prefix_result);

    rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
        [value, proj](auto c) { return raze::algorithm::remove[raze::options::fscalar](c.range(), value, proj); },
        [value, proj](auto c) { return std::ranges::remove(c.range(), value, proj); },
        rtts::algorithm::prefix_result);

    rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
        [value, proj](auto c) { return raze::algorithm::remove[raze::options::unroll<2>](c.range(), value, proj); },
        [value, proj](auto c) { return std::ranges::remove(c.range(), value, proj); },
        rtts::algorithm::prefix_result);

    rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
        [value, proj](auto c) { return raze::algorithm::remove[raze::options::fautovec](c.range(), value, proj); },
        [value, proj](auto c) { return std::ranges::remove(c.range(), value, proj); },
        rtts::algorithm::prefix_result);

    rtts::algorithm::run_aligned<64, rtts::algorithm::range, T>(acfg,
        [value, proj](auto c) { return raze::algorithm::remove[raze::options::aligned<64>](c.range(), value, proj); },
        [value, proj](auto c) { return std::ranges::remove(c.range(), value, proj); },
        rtts::algorithm::prefix_result);

    rtts::algorithm::run_aligned<1, rtts::algorithm::range, T>(acfg,
        [value, proj](auto c) { return raze::algorithm::remove[raze::options::aligned<raze::options::unaligned>](c.range(), value, proj); },
        [value, proj](auto c) { return std::ranges::remove(c.range(), value, proj); },
        rtts::algorithm::prefix_result);

    static_assert(rtts::algorithm::constexpr_run<T, rtts::algorithm::range, 8>(
        [](auto c) constexpr {
            return raze::algorithm::remove(
                c.first(), c.last(), T(0),
                [](const T& x) constexpr -> const T& { return x; });
        },
        [](auto c) constexpr {
            return std::ranges::remove(
                c.first(), c.last(), T(0),
                [](const T& x) constexpr -> const T& { return x; });
        },
        rtts::algorithm::prefix_result));

    static_assert(rtts::algorithm::constexpr_run<T, rtts::algorithm::range, 8>(
        [](auto c) constexpr {
            return raze::algorithm::remove[raze::options::fscalar](
                c.range(), T(0),
                [](const T& x) constexpr -> const T& { return x; });
        },
        [](auto c) constexpr {
            return std::ranges::remove(
                c.range(), T(0),
                [](const T& x) constexpr -> const T& { return x; });
        },
        rtts::algorithm::prefix_result));
};


RTTS_CASE("raze::algorithm::remove views and edge cases") {
    namespace opt = raze::options;
    namespace views = std::views;

    // Writable views: take, drop, reverse, filter, non-common
    {
        std::vector<int> v1 = { 1, 2, 3, 2, 4, 2, 5, 2, 6, 2, 7, 2, 8, 2, 9, 2 };
        auto v2 = v1;

        auto r1 = raze::algorithm::remove(v1 | views::drop(2) | views::take(10), 2);
        auto s1 = std::ranges::remove(v2 | views::drop(2) | views::take(10), 2);
        RTTS_EXPECT(std::ranges::equal(v1.begin() + 2, r1.begin(), v2.begin() + 2, s1.begin()));

        std::vector<int> vr1 = { 1, 2, 3, 2, 4, 2, 5 }, vr2 = vr1;
        auto rr1 = raze::algorithm::remove[opt::fstatic](vr1 | views::reverse, 2);
        auto sr1 = std::ranges::remove(vr2 | views::reverse, 2);
        RTTS_EXPECT(std::ranges::distance((vr1 | views::reverse).begin(), rr1.begin()) ==
                    std::ranges::distance((vr2 | views::reverse).begin(), sr1.begin()));

        std::vector<int> vf1 = { 1, 2, 3, 4, 2, 6, 7, 2, 8 }, vf2 = vf1;
        auto is_even = [](int x) { return (x % 2) == 0; };
        auto fv1 = vf1 | views::filter(is_even);
        auto fv2 = vf2 | views::filter(is_even);
        auto rf1 = raze::algorithm::remove[opt::aligned<64>](fv1, 2);
        auto sf1 = std::ranges::remove(fv2, 2);
        RTTS_EXPECT(std::ranges::distance(fv1.begin(), rf1.begin()) ==
                    std::ranges::distance(fv2.begin(), sf1.begin()));

        std::vector<int> vn1 = { 2, 1, 2, 3, 2 }, vn2 = vn1;
        auto rn1 = raze::algorithm::remove(rtts::make_non_common(vn1), 2);
        auto sn1 = std::ranges::remove(rtts::make_non_common(vn2), 2);
        RTTS_EXPECT(std::ranges::equal(vn1.begin(), rn1.begin(), vn2.begin(), sn1.begin()));
    }

    // Empty, 1-elem, all-removed, none-removed, volatile, rvalue dangling, member projection
    {
        std::vector<int> empty_v;
        auto er = raze::algorithm::remove(empty_v, 1);
        RTTS_EXPECT(er.begin() == empty_v.end() && er.end() == empty_v.end());

        std::vector<int> one_m = { 5 };
        RTTS_EXPECT(raze::algorithm::remove(one_m, 5).begin() == one_m.begin());

        std::vector<int> one_k = { 5 };
        RTTS_EXPECT(raze::algorithm::remove(one_k, 9).begin() == one_k.end());

        std::vector<int> all_rm(25, 3);
        RTTS_EXPECT(raze::algorithm::remove(all_rm, 3).begin() == all_rm.begin());

        volatile int vbuf[16];
        for (int i = 0; i < 16; ++i) vbuf[i] = (i % 2 == 0) ? 99 : i;
        auto vres = raze::algorithm::remove(vbuf, vbuf + 16, 99);
        RTTS_EXPECT(vres.begin() == vbuf + 8);
        for (int i = 0; i < 8; ++i) RTTS_EXPECT(vbuf[i] == i * 2 + 1);

        auto rval = raze::algorithm::remove(std::vector<int>{ 1, 2, 1, 3 }, 1);
        static_assert(std::same_as<decltype(rval), std::ranges::dangling>);

        std::vector<rtts::test_item> items = { { 1, 10 }, { 2, 20 }, { 1, 30 }, { 3, 40 } };
        auto ir = raze::algorithm::remove(items, 1, &rtts::test_item::key);
        items.erase(ir.begin(), ir.end());
        RTTS_EXPECT(items.size() == 2 && items[0].key == 2 && items[1].key == 3);
    }
};