#include <tests/rts/rts.h>
#include <raze/algorithm/remove/RemoveIf.h>

RTTS_CASE_TPL("raze::algorithm::remove_if", rtts::algorithm::all_types)
< class T > (rtts::type<T>) {
    auto cfg = rtts::algorithm::config::thorough();

    const T threshold = rtts::random::generator<T>(1000)();
    auto pred_scalar = [threshold](const T& x) { return x > threshold; };
    auto pred_vector = [threshold](auto x) { return x > threshold; };
    auto pred_branching = [threshold](auto x) { if (x > threshold) return true; return false; };

    {
        rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
            [pred_scalar](auto c) { return raze::algorithm::remove_if(c.first(), c.last(), pred_scalar); },
            [pred_scalar](auto c) { return std::ranges::remove_if(c.first(), c.last(), pred_scalar); },
            rtts::algorithm::prefix_result);

        rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
            [pred_vector](auto c) { return raze::algorithm::remove_if(c.first(), c.last(), pred_vector); },
            [pred_vector](auto c) { return std::ranges::remove_if(c.first(), c.last(), pred_vector); },
            rtts::algorithm::prefix_result);

        rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
            [pred_scalar](auto c) { return raze::algorithm::remove_if(c.range(), pred_scalar); },
            [pred_scalar](auto c) { return std::ranges::remove_if(c.range(), pred_scalar); },
            rtts::algorithm::prefix_result);

        rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
            [pred_vector](auto c) { return raze::algorithm::remove_if(c.range(), pred_vector); },
            [pred_vector](auto c) { return std::ranges::remove_if(c.range(), pred_vector); },
            rtts::algorithm::prefix_result);
    }

    {
        rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
            [pred_vector](auto c) { return raze::algorithm::remove_if[raze::options::fscalar](c.first(), c.last(), pred_vector); },
            [pred_vector](auto c) { return std::ranges::remove_if(c.first(), c.last(), pred_vector); },
            rtts::algorithm::prefix_result);

        rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
            [pred_vector](auto c) { return raze::algorithm::remove_if[raze::options::fscalar](c.range(), pred_vector); },
            [pred_vector](auto c) { return std::ranges::remove_if(c.range(), pred_vector); },
            rtts::algorithm::prefix_result);

        rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
            [pred_vector](auto c) { return raze::algorithm::remove_if[raze::options::unroll<1>](c.range(), pred_vector); },
            [pred_vector](auto c) { return std::ranges::remove_if(c.range(), pred_vector); },
            rtts::algorithm::prefix_result);

        rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
            [pred_vector](auto c) { return raze::algorithm::remove_if[raze::options::fstatic](c.range(), pred_vector); },
            [pred_vector](auto c) { return std::ranges::remove_if(c.range(), pred_vector); },
            rtts::algorithm::prefix_result);

        rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
            [pred_vector](auto c) { return raze::algorithm::remove_if[raze::options::fscalar][raze::options::unroll<2>](c.range(), pred_vector); },
            [pred_vector](auto c) { return std::ranges::remove_if(c.range(), pred_vector); },
            rtts::algorithm::prefix_result);

        rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
            [pred_vector](auto c) { return raze::algorithm::remove_if[raze::options::fstatic][raze::options::unroll<2>](c.range(), pred_vector); },
            [pred_vector](auto c) { return std::ranges::remove_if(c.range(), pred_vector); },
            rtts::algorithm::prefix_result);

        rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
            [pred_vector](auto c) { return raze::algorithm::remove_if[raze::options::fautovec](c.first(), c.last(), pred_vector); },
            [pred_vector](auto c) { return std::ranges::remove_if(c.first(), c.last(), pred_vector); },
            rtts::algorithm::prefix_result);

        rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
            [pred_branching](auto c) { return raze::algorithm::remove_if[raze::options::fautovec](c.range(), pred_branching); },
            [pred_branching](auto c) { return std::ranges::remove_if(c.range(), pred_branching); },
            rtts::algorithm::prefix_result);

        rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
            [pred_branching](auto c) { return raze::algorithm::remove_if[raze::options::fautovec][raze::options::fstatic](c.range(), pred_branching); },
            [pred_branching](auto c) { return std::ranges::remove_if(c.range(), pred_branching); },
            rtts::algorithm::prefix_result);
    }

    {
        namespace opt = raze::options;
        auto acfg = rtts::algorithm::config::compact();

        auto check_aligned = [&]<size_t Align>(auto algo, auto pred) {
            rtts::algorithm::run_aligned<Align, rtts::algorithm::range, T>(acfg,
                [algo, pred](auto c) { return algo(c.first(), c.last(), pred); },
                [pred](auto c) { return std::ranges::remove_if(c.first(), c.last(), pred); },
                rtts::algorithm::prefix_result);

            rtts::algorithm::run_aligned<Align, rtts::algorithm::range, T>(acfg,
                [algo, pred](auto c) { return algo(c.range(), pred); },
                [pred](auto c) { return std::ranges::remove_if(c.range(), pred); },
                rtts::algorithm::prefix_result);
        };

        check_aligned.template operator()<16>(raze::algorithm::remove_if[opt::aligned<16>], pred_vector);
        check_aligned.template operator()<32>(raze::algorithm::remove_if[opt::aligned<32>], pred_vector);
        check_aligned.template operator()<32>(raze::algorithm::remove_if[opt::aligned<32>][opt::unroll<2>], pred_vector);
        check_aligned.template operator()<32>(raze::algorithm::remove_if[opt::aligned<32>][opt::fstatic], pred_vector);
        check_aligned.template operator()<64>(raze::algorithm::remove_if[opt::aligned<64>], pred_vector);
        check_aligned.template operator()<64>(raze::algorithm::remove_if[opt::aligned<64>][opt::fautovec], pred_branching);
        check_aligned.template operator()<64>(raze::algorithm::remove_if[opt::aligned<64>][opt::fscalar], pred_vector);
        check_aligned.template operator()<1>(raze::algorithm::remove_if[opt::aligned<opt::unaligned>], pred_vector);
        check_aligned.template operator()<1>(raze::algorithm::remove_if[opt::aligned<64>][opt::aligned<opt::unaligned>], pred_vector);
    }

    static_assert(rtts::algorithm::constexpr_run<T, rtts::algorithm::range, 8>(
        [](auto c) constexpr { return raze::algorithm::remove_if(c.first(), c.last(), [](const T& x) constexpr { return x > T(0); }); },
        [](auto c) constexpr { return std::ranges::remove_if(c.first(), c.last(), [](const T& x) constexpr { return x > T(0); }); },
        rtts::algorithm::prefix_result));

    static_assert(rtts::algorithm::constexpr_run<T, rtts::algorithm::range, 8>(
        [](auto c) constexpr { return raze::algorithm::remove_if(c.range(), [](auto x) constexpr { return x > T(0); }); },
        [](auto c) constexpr { return std::ranges::remove_if(c.range(), [](auto x) constexpr { return x > T(0); }); },
        rtts::algorithm::prefix_result));

    static_assert(rtts::algorithm::constexpr_run<T, rtts::algorithm::range, 8>(
        [](auto c) constexpr { return raze::algorithm::remove_if[raze::options::fscalar](c.range(), [](auto x) constexpr { return x > T(0); }); },
        [](auto c) constexpr { return std::ranges::remove_if(c.range(), [](auto x) constexpr { return x > T(0); }); },
        rtts::algorithm::prefix_result));

    static_assert(rtts::algorithm::constexpr_run<T, rtts::algorithm::range, 8>(
        [](auto c) constexpr { return raze::algorithm::remove_if[raze::options::unroll<2>](c.range(), [](auto x) constexpr { return x > T(0); }); },
        [](auto c) constexpr { return std::ranges::remove_if(c.range(), [](auto x) constexpr { return x > T(0); }); },
        rtts::algorithm::prefix_result));

    static_assert(rtts::algorithm::constexpr_run<T, rtts::algorithm::range, 8>(
        [](auto c) constexpr { return raze::algorithm::remove_if[raze::options::fautovec](c.range(), [](auto x) constexpr { if (x > T(2)) return true; return false; }); },
        [](auto c) constexpr { return std::ranges::remove_if(c.range(), [](auto x) constexpr { if (x > T(2)) return true; return false; }); },
        rtts::algorithm::prefix_result));

    static_assert(rtts::algorithm::constexpr_run<T, rtts::algorithm::range, 8>(
        [](auto c) constexpr { return raze::algorithm::remove_if[raze::options::aligned<64>](c.range(), [](auto x) constexpr { return x > T(0); }); },
        [](auto c) constexpr { return std::ranges::remove_if(c.range(), [](auto x) constexpr { return x > T(0); }); },
        rtts::algorithm::prefix_result));
};

RTTS_CASE_TPL("raze::algorithm::remove_if.projection", rtts::algorithm::all_types)
< class T > (rtts::type<T>) {
    auto cfg = rtts::algorithm::config::thorough();
    auto acfg = rtts::algorithm::config::compact();

    const T threshold = rtts::random::generator<T>(12345)();
    auto pred = [threshold](auto x) { return x > threshold; };
    auto pred_branching = [threshold](auto x) { if (x > threshold) return true; return false; };
    auto proj = [](const T& x) -> const T& { return x; };

    rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
        [pred, proj](auto c) { return raze::algorithm::remove_if(c.first(), c.last(), pred, proj); },
        [pred, proj](auto c) { return std::ranges::remove_if(c.first(), c.last(), pred, proj); },
        rtts::algorithm::prefix_result);

    rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
        [pred, proj](auto c) { return raze::algorithm::remove_if(c.range(), pred, proj); },
        [pred, proj](auto c) { return std::ranges::remove_if(c.range(), pred, proj); },
        rtts::algorithm::prefix_result);

    rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
        [pred, proj](auto c) { return raze::algorithm::remove_if[raze::options::fscalar](c.range(), pred, proj); },
        [pred, proj](auto c) { return std::ranges::remove_if(c.range(), pred, proj); },
        rtts::algorithm::prefix_result);

    rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
        [pred, proj](auto c) { return raze::algorithm::remove_if[raze::options::unroll<2>](c.range(), pred, proj); },
        [pred, proj](auto c) { return std::ranges::remove_if(c.range(), pred, proj); },
        rtts::algorithm::prefix_result);

    rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
        [pred_branching, proj](auto c) { return raze::algorithm::remove_if[raze::options::fautovec](c.range(), pred_branching, proj); },
        [pred_branching, proj](auto c) { return std::ranges::remove_if(c.range(), pred_branching, proj); },
        rtts::algorithm::prefix_result);

    rtts::algorithm::run_aligned<64, rtts::algorithm::range, T>(acfg,
        [pred, proj](auto c) { return raze::algorithm::remove_if[raze::options::aligned<64>](c.range(), pred, proj); },
        [pred, proj](auto c) { return std::ranges::remove_if(c.range(), pred, proj); },
        rtts::algorithm::prefix_result);

    rtts::algorithm::run_aligned<1, rtts::algorithm::range, T>(acfg,
        [pred, proj](auto c) { return raze::algorithm::remove_if[raze::options::aligned<raze::options::unaligned>](c.range(), pred, proj); },
        [pred, proj](auto c) { return std::ranges::remove_if(c.range(), pred, proj); },
        rtts::algorithm::prefix_result);

    static_assert(rtts::algorithm::constexpr_run<T, rtts::algorithm::range, 8>(
        [](auto c) constexpr {
            return raze::algorithm::remove_if(
                c.first(), c.last(),
                [](const T& x) constexpr { return x > T(0); },
                [](const T& x) constexpr -> const T& { return x; });
        },
        [](auto c) constexpr {
            return std::ranges::remove_if(
                c.first(), c.last(),
                [](const T& x) constexpr { return x > T(0); },
                [](const T& x) constexpr -> const T& { return x; });
        },
        rtts::algorithm::prefix_result));

    static_assert(rtts::algorithm::constexpr_run<T, rtts::algorithm::range, 8>(
        [](auto c) constexpr {
            return raze::algorithm::remove_if[raze::options::fscalar](
                c.range(),
                [](auto x) constexpr { return x > T(0); },
                [](const T& x) constexpr -> const T& { return x; });
        },
        [](auto c) constexpr {
            return std::ranges::remove_if(
                c.range(),
                [](auto x) constexpr { return x > T(0); },
                [](const T& x) constexpr -> const T& { return x; });
        },
        rtts::algorithm::prefix_result));
};


RTTS_CASE("raze::algorithm::remove_if views and edge cases") {
    namespace opt = raze::options;
    namespace views = std::views;

    // Writable views: take, drop, reverse, filter, non-common
    {
        std::vector<int> v1(30);
        std::iota(v1.begin(), v1.end(), 1);
        auto v2 = v1;

        auto pred_branch = [](auto x) { if ((x % 3) == 0) return true; return false; };
        auto r1 = raze::algorithm::remove_if[opt::fautovec][opt::fstatic](v1 | views::drop(3) | views::take(20), pred_branch);
        auto s1 = std::ranges::remove_if(v2 | views::drop(3) | views::take(20), pred_branch);
        RTTS_EXPECT(std::ranges::equal(v1.begin() + 3, r1.begin(), v2.begin() + 3, s1.begin()));

        std::vector<int> vr1 = { 1, 2, 3, 4, 5, 6, 7 }, vr2 = vr1;
        auto rr1 = raze::algorithm::remove_if(vr1 | views::reverse, [](auto x) { return (x & 1) == 0; });
        auto sr1 = std::ranges::remove_if(vr2 | views::reverse, [](int x) { return (x & 1) == 0; });
        RTTS_EXPECT(std::ranges::distance((vr1 | views::reverse).begin(), rr1.begin()) ==
                    std::ranges::distance((vr2 | views::reverse).begin(), sr1.begin()));

        std::vector<int> vn1 = { 1, 2, 3, 4, 5, 6 }, vn2 = vn1;
        auto rn1 = raze::algorithm::remove_if[opt::aligned<64>](rtts::make_non_common(vn1), [](auto x) { return x > 3; });
        auto sn1 = std::ranges::remove_if(rtts::make_non_common(vn2), [](int x) { return x > 3; });
        RTTS_EXPECT(std::ranges::equal(vn1.begin(), rn1.begin(), vn2.begin(), sn1.begin()));
    }

    // Empty, 1-elem, volatile, rvalue dangling, member projection
    {
        std::vector<int> empty_v;
        auto er = raze::algorithm::remove_if(empty_v, [](auto x) { return x > 0; });
        RTTS_EXPECT(er.begin() == empty_v.end());

        volatile int vbuf[16];
        for (int i = 0; i < 16; ++i) vbuf[i] = i;
        auto vres = raze::algorithm::remove_if(vbuf, vbuf + 16, [](int x) { return (x & 1) == 0; });
        RTTS_EXPECT(vres.begin() == vbuf + 8);
        for (int i = 0; i < 8; ++i) RTTS_EXPECT(vbuf[i] == i * 2 + 1);

        auto rval = raze::algorithm::remove_if(std::vector<int>{ 1, 2, 3, 4 }, [](auto x) { return (x & 1) == 0; });
        static_assert(std::same_as<decltype(rval), std::ranges::dangling>);

        std::vector<rtts::test_item> items = { { 1, 10 }, { 5, 50 }, { 2, 20 }, { 6, 60 } };
        auto ir = raze::algorithm::remove_if(items, [](int k) { return k >= 5; }, &rtts::test_item::key);
        items.erase(ir.begin(), ir.end());
        RTTS_EXPECT(items.size() == 2 && items[0].key == 1 && items[1].key == 2);
    }
};