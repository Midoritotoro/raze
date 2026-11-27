#include <tests/rts/rts.h>
#include <raze/algorithm/remove/RemoveCopyIf.h>

RTTS_CASE_TPL("raze::algorithm::remove_copy_if", rtts::algorithm::all_types)
< class T > (rtts::type<T>) {
    namespace opt = raze::options;
    auto cfg = rtts::algorithm::config::thorough();

    const T threshold = rtts::random::generator<T>(1000)();
    auto pred_scalar = [threshold](const T& x) { return x > threshold; };
    auto pred_vector = [threshold](auto x) { return x > threshold; };
    auto pred_branching = [threshold](auto x) { if (x > threshold) return true; return false; };

    auto check_iter = [&](auto algo, auto pred) {
        rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
            [algo, pred](auto c) { return algo(c.first(), c.last(), c.out(), pred); },
            [pred](auto c) { return std::ranges::remove_copy_if(c.first(), c.last(), c.out(), pred); });
    };

    auto check_range = [&](auto algo, auto pred) {
        rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
            [algo, pred](auto c) { return algo(c.range(), c.out(), pred); },
            [pred](auto c) { return std::ranges::remove_copy_if(c.range(), c.out(), pred); });
    };

    check_iter(raze::algorithm::remove_copy_if, pred_scalar);
    check_iter(raze::algorithm::remove_copy_if, pred_vector);
    check_range(raze::algorithm::remove_copy_if, pred_scalar);
    check_range(raze::algorithm::remove_copy_if, pred_vector);

    check_iter(raze::algorithm::remove_copy_if[opt::fscalar], pred_vector);
    check_range(raze::algorithm::remove_copy_if[opt::fscalar], pred_vector);
    check_range(raze::algorithm::remove_copy_if[opt::unroll<1>], pred_vector);
    check_range(raze::algorithm::remove_copy_if[opt::fstatic], pred_vector);
    check_range(raze::algorithm::remove_copy_if[opt::fscalar][opt::unroll<2>], pred_vector);
    check_range(raze::algorithm::remove_copy_if[opt::fstatic][opt::unroll<2>], pred_vector);
    check_iter(raze::algorithm::remove_copy_if[opt::fautovec], pred_vector);
    check_range(raze::algorithm::remove_copy_if[opt::fautovec], pred_branching);
    check_range(raze::algorithm::remove_copy_if[opt::fautovec][opt::fstatic], pred_branching);

    {
        auto acfg = rtts::algorithm::config::compact();

        auto check_aligned = [&]<size_t A1, size_t AOut>(auto algo, auto pred) {
            rtts::algorithm::run_aligned<A1, AOut, rtts::algorithm::range, T>(acfg,
                [algo, pred](auto c) { return algo(c.first(), c.last(), c.out(), pred); },
                [pred](auto c) { return std::ranges::remove_copy_if(c.first(), c.last(), c.out(), pred); });

            rtts::algorithm::run_aligned<A1, AOut, rtts::algorithm::range, T>(acfg,
                [algo, pred](auto c) { return algo(c.range(), c.out(), pred); },
                [pred](auto c) { return std::ranges::remove_copy_if(c.range(), c.out(), pred); });
        };

        check_aligned.template operator()<16, 16>(raze::algorithm::remove_copy_if[opt::aligned<16>], pred_vector);
        check_aligned.template operator()<32, 32>(raze::algorithm::remove_copy_if[opt::aligned<32>], pred_vector);
        check_aligned.template operator()<32, 32>(raze::algorithm::remove_copy_if[opt::aligned<32>][opt::unroll<2>], pred_vector);
        check_aligned.template operator()<64, 64>(raze::algorithm::remove_copy_if[opt::aligned<64>], pred_vector);
        check_aligned.template operator()<64, 64>(raze::algorithm::remove_copy_if[opt::aligned<64>][opt::fautovec], pred_branching);
        check_aligned.template operator()<64, 64>(raze::algorithm::remove_copy_if[opt::aligned<64>][opt::fscalar], pred_vector);
        check_aligned.template operator()<1, 1>(raze::algorithm::remove_copy_if[opt::aligned<opt::unaligned>], pred_vector);
        check_aligned.template operator()<1, 1>(raze::algorithm::remove_copy_if[opt::aligned<64>][opt::aligned<opt::unaligned>], pred_vector);
        check_aligned.template operator()<64, 1>(raze::algorithm::remove_copy_if[opt::aligned<64, opt::unaligned>], pred_vector);
        check_aligned.template operator()<1, 64>(raze::algorithm::remove_copy_if[opt::aligned<opt::unaligned, 64>], pred_vector);
        check_aligned.template operator()<32, 64>(raze::algorithm::remove_copy_if[opt::aligned<32, 64>][opt::fstatic], pred_vector);
        check_aligned.template operator()<64, 16>(raze::algorithm::remove_copy_if[opt::aligned<64, 16>], pred_vector);
    }

    constexpr auto check_ce_range = [](auto algo, auto pred) constexpr {
        return rtts::algorithm::constexpr_run<T, rtts::algorithm::range, 8>(
            [algo, pred](auto c) constexpr { return algo(c.range(), c.out(), pred); },
            [pred](auto c) constexpr { return std::ranges::remove_copy_if(c.range(), c.out(), pred); });
    };

    static_assert(rtts::algorithm::constexpr_run<T, rtts::algorithm::range, 8>(
        [](auto c) constexpr { return raze::algorithm::remove_copy_if(c.first(), c.last(), c.out(), [](const T& x) constexpr { return x > T(0); }); },
        [](auto c) constexpr { return std::ranges::remove_copy_if(c.first(), c.last(), c.out(), [](const T& x) constexpr { return x > T(0); }); }));
    static_assert(check_ce_range(raze::algorithm::remove_copy_if, [](auto x) constexpr { return x > T(0); }));
    static_assert(check_ce_range(raze::algorithm::remove_copy_if[opt::fscalar], [](auto x) constexpr { return x > T(0); }));
    static_assert(check_ce_range(raze::algorithm::remove_copy_if[opt::unroll<2>], [](auto x) constexpr { return x > T(0); }));
    static_assert(check_ce_range(raze::algorithm::remove_copy_if[opt::fautovec], [](auto x) constexpr { if (x > T(2)) return true; return false; }));
    static_assert(check_ce_range(raze::algorithm::remove_copy_if[opt::aligned<64, opt::unaligned>], [](auto x) constexpr { return x > T(0); }));
};

RTTS_CASE_TPL("raze::algorithm::remove_copy_if.projection", rtts::algorithm::all_types)
< class T > (rtts::type<T>) {
    namespace opt = raze::options;
    auto cfg = rtts::algorithm::config::thorough();
    auto acfg = rtts::algorithm::config::compact();

    const T threshold = rtts::random::generator<T>(12345)();
    auto pred = [threshold](auto x) { return x > threshold; };
    auto pred_branching = [threshold](auto x) { if (x > threshold) return true; return false; };
    auto proj = [](const T& x) -> const T& { return x; };

    auto check_range = [&](auto algo, auto p) {
        rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
            [algo, p, proj](auto c) { return algo(c.range(), c.out(), p, proj); },
            [p, proj](auto c) { return std::ranges::remove_copy_if(c.range(), c.out(), p, proj); });
    };

    rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
        [pred, proj](auto c) { return raze::algorithm::remove_copy_if(c.first(), c.last(), c.out(), pred, proj); },
        [pred, proj](auto c) { return std::ranges::remove_copy_if(c.first(), c.last(), c.out(), pred, proj); });

    check_range(raze::algorithm::remove_copy_if, pred);
    check_range(raze::algorithm::remove_copy_if[opt::fscalar], pred);
    check_range(raze::algorithm::remove_copy_if[opt::unroll<2>], pred);
    check_range(raze::algorithm::remove_copy_if[opt::fautovec], pred_branching);

    auto check_aligned = [&]<size_t A1, size_t AOut>(auto algo) {
        rtts::algorithm::run_aligned<A1, AOut, rtts::algorithm::range, T>(acfg,
            [algo, pred, proj](auto c) { return algo(c.range(), c.out(), pred, proj); },
            [pred, proj](auto c) { return std::ranges::remove_copy_if(c.range(), c.out(), pred, proj); });
    };
    check_aligned.template operator()<64, 1>(raze::algorithm::remove_copy_if[opt::aligned<64, opt::unaligned>]);
    check_aligned.template operator()<1, 64>(raze::algorithm::remove_copy_if[opt::aligned<opt::unaligned, 64>]);

    static_assert(rtts::algorithm::constexpr_run<T, rtts::algorithm::range, 8>(
        [](auto c) constexpr {
            return raze::algorithm::remove_copy_if(c.first(), c.last(), c.out(),
                [](const T& x) constexpr { return x > T(0); },
                [](const T& x) constexpr -> const T& { return x; });
        },
        [](auto c) constexpr {
            return std::ranges::remove_copy_if(c.first(), c.last(), c.out(),
                [](const T& x) constexpr { return x > T(0); },
                [](const T& x) constexpr -> const T& { return x; });
        }));

    static_assert(rtts::algorithm::constexpr_run<T, rtts::algorithm::range, 8>(
        [](auto c) constexpr {
            return raze::algorithm::remove_copy_if[opt::fscalar](c.range(), c.out(),
                [](auto x) constexpr { return x > T(0); },
                [](const T& x) constexpr -> const T& { return x; });
        },
        [](auto c) constexpr {
            return std::ranges::remove_copy_if(c.range(), c.out(),
                [](auto x) constexpr { return x > T(0); },
                [](const T& x) constexpr -> const T& { return x; });
        }));
};

RTTS_CASE("raze::algorithm::remove_copy_if.mixed") {
    namespace opt = raze::options;
    auto cfg = rtts::algorithm::config::thorough();

    auto pos = [](auto x) { return x > decltype(x)(0); };
    auto even = [](auto x) { return (x & 1) == 0; };
    auto pos_branching = [](auto x) { if (x > decltype(x)(0)) return true; return false; };

    auto check_iter = [&]<class T1, class T2>(auto algo, auto pred) {
        rtts::algorithm::run<rtts::algorithm::range, T1, T2>(cfg,
            [algo, pred](auto c) { return algo(c.first(), c.last(), c.out2(), pred); },
            [pred](auto c) { return std::ranges::remove_copy_if(c.first(), c.last(), c.out2(), pred); });
    };

    auto check_range = [&]<class T1, class T2>(auto algo, auto pred) {
        rtts::algorithm::run<rtts::algorithm::range, T1, T2>(cfg,
            [algo, pred](auto c) { return algo(c.range(), c.out2(), pred); },
            [pred](auto c) { return std::ranges::remove_copy_if(c.range(), c.out2(), pred); });
    };

    check_iter.template operator()<int32_t, float>(raze::algorithm::remove_copy_if, pos);
    check_iter.template operator()<uint32_t, float>(raze::algorithm::remove_copy_if, even);
    check_range.template operator()<int64_t, double>(raze::algorithm::remove_copy_if, pos);
    check_range.template operator()<int8_t, int16_t>(raze::algorithm::remove_copy_if, even);
    check_range.template operator()<int32_t, int64_t>(raze::algorithm::remove_copy_if, pos);
    check_range.template operator()<int16_t, int8_t>(raze::algorithm::remove_copy_if, even);
    check_iter.template operator()<int32_t, float>(raze::algorithm::remove_copy_if[opt::fscalar], pos);
    check_iter.template operator()<int32_t, float>(raze::algorithm::remove_copy_if[opt::fautovec], pos_branching);

    rtts::algorithm::run_aligned_mixed<64, 1, rtts::algorithm::range, int32_t, float>(rtts::algorithm::config::compact(),
        [pos](auto c) { return raze::algorithm::remove_copy_if[opt::aligned<64, opt::unaligned>](c.range(), c.out2(), pos); },
        [pos](auto c) { return std::ranges::remove_copy_if(c.range(), c.out2(), pos); });

    constexpr auto check_ce_even = []<class T1, class T2>() constexpr {
        return rtts::algorithm::constexpr_run_mixed<T1, T2, rtts::algorithm::range, 8>(
            [](auto c) constexpr { return raze::algorithm::remove_copy_if(c.range(), c.out2(), [](auto x) constexpr { return (x & 1) == 0; }); },
            [](auto c) constexpr { return std::ranges::remove_copy_if(c.range(), c.out2(), [](auto x) constexpr { return (x & 1) == 0; }); });
    };

    static_assert(rtts::algorithm::constexpr_run_mixed<int32_t, float, rtts::algorithm::range, 8>(
        [](auto c) constexpr { return raze::algorithm::remove_copy_if(c.first(), c.last(), c.out2(), [](auto x) constexpr { return x > int32_t(0); }); },
        [](auto c) constexpr { return std::ranges::remove_copy_if(c.first(), c.last(), c.out2(), [](auto x) constexpr { return x > int32_t(0); }); }));
    static_assert(check_ce_even.template operator()<int64_t, double>());
    static_assert(check_ce_even.template operator()<int8_t, int16_t>());
};


RTTS_CASE("raze::algorithm::remove_copy_if views and edge cases") {
    namespace opt = raze::options;
    namespace views = std::views;

    // Views: iota, take, drop, reverse, filter, transform, chained, non-common
    {
        const std::vector<int> src = { -3, -2, -1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
        auto pred_branch = [](auto x) { if ((x & 1) == 0) return true; return false; };

        std::vector<int> r1, s1;
        raze::algorithm::remove_copy_if[opt::fautovec][opt::fstatic](views::iota(1, 20), std::back_inserter(r1), pred_branch);
        std::ranges::remove_copy_if(views::iota(1, 20), std::back_inserter(s1), pred_branch);
        RTTS_EXPECT(r1 == s1);

        auto chain = src | views::reverse | views::filter([](int x) { return x > 0; }) | views::transform([](int x) { return x * 3; });
        std::vector<int> r2, s2;
        raze::algorithm::remove_copy_if[opt::aligned<64, opt::unaligned>](chain, std::back_inserter(r2), [](int x) { return x > 20; });
        std::ranges::remove_copy_if(chain, std::back_inserter(s2), [](int x) { return x > 20; });
        RTTS_EXPECT(r2 == s2);

        std::vector<int> r3, s3;
        raze::algorithm::remove_copy_if(rtts::make_non_common(src), std::back_inserter(r3), [](auto x) { return x < 0; });
        std::ranges::remove_copy_if(rtts::make_non_common(src), std::back_inserter(s3), [](int x) { return x < 0; });
        RTTS_EXPECT(r3 == s3);
    }

    // Output iterators, single-pass, non-destructive strings, volatile, dangling, member projection
    {
        const std::vector<int> src = { 1, 2, 3, 4, 5, 6 };
        auto is_even = [](auto x) { return (x & 1) == 0; };
        const std::vector<int> expected = { 1, 3, 5 };

        std::vector<int> b1, b2;
        raze::algorithm::remove_copy_if(src, std::back_inserter(b1), is_even);
        raze::algorithm::remove_copy_if[opt::fscalar](src, std::back_inserter(b2), is_even);
        RTTS_EXPECT(b1 == expected && b2 == expected);

        std::deque<int> f1, f2;
        raze::algorithm::remove_copy_if(src, std::front_inserter(f1), is_even);
        std::ranges::remove_copy_if(src, std::front_inserter(f2), is_even);
        RTTS_EXPECT(f1 == f2);

        std::vector<int> i1 = { 0, 0 }, i2 = { 0, 0 };
        raze::algorithm::remove_copy_if(src, std::inserter(i1, i1.begin() + 1), is_even);
        std::ranges::remove_copy_if(src, std::inserter(i2, i2.begin() + 1), is_even);
        RTTS_EXPECT(i1 == i2);

        rtts::single_pass_input_range<int> spr(src);
        std::vector<int> sp_out;
        raze::algorithm::remove_copy_if(spr, std::back_inserter(sp_out), is_even);
        RTTS_EXPECT(sp_out == expected && spr.reads() == src.size());

        std::vector<std::string> words = { "a", "hello", "b", "world" };
        std::vector<std::string> kept;
        raze::algorithm::remove_copy_if(words, std::back_inserter(kept), [](const std::string& s) { return s.size() < 2; });
        RTTS_EXPECT((kept == std::vector<std::string>{ "hello", "world" }));
        RTTS_EXPECT((words == std::vector<std::string>{ "a", "hello", "b", "world" }));

        volatile int vsrc[16];
        volatile int vdst[16];
        for (int i = 0; i < 16; ++i) vsrc[i] = i;
        auto vr = raze::algorithm::remove_copy_if(vsrc, vsrc + 16, vdst, [](int x) { return (x & 1) == 0; });
        RTTS_EXPECT(vr.out == vdst + 8);
        for (int i = 0; i < 8; ++i) RTTS_EXPECT(vdst[i] == i * 2 + 1);

        std::vector<int> rdst;
        auto rres = raze::algorithm::remove_copy_if(std::vector<int>{ 1, 2, 3, 4 }, std::back_inserter(rdst), is_even);
        static_assert(std::same_as<decltype(rres.in), std::ranges::dangling>);
        RTTS_EXPECT((rdst == std::vector<int>{ 1, 3 }));

        const std::vector<rtts::test_item> items = { { 1, 10 }, { 5, 50 }, { 2, 20 }, { 6, 60 } };
        std::vector<rtts::test_item> out_items;
        raze::algorithm::remove_copy_if(items, std::back_inserter(out_items), [](int k) { return k >= 5; }, &rtts::test_item::key);
        RTTS_EXPECT(out_items.size() == 2 && out_items[0].key == 1 && out_items[1].key == 2);
    }
};