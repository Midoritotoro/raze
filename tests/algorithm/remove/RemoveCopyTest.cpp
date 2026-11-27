#include <tests/rts/rts.h>
#include <raze/algorithm/remove/RemoveCopy.h>

RTTS_CASE_TPL("raze::algorithm::remove_copy", rtts::algorithm::all_types)
< class T > (rtts::type<T>) {
    namespace opt = raze::options;
    auto cfg = rtts::algorithm::config::thorough();

    const T value = rtts::random::generator<T>(1000)();

    auto check_iter = [&](auto algo) {
        rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
            [algo, value](auto c) { return algo(c.first(), c.last(), c.out(), value); },
            [value](auto c) { return std::ranges::remove_copy(c.first(), c.last(), c.out(), value); });
    };

    auto check_range = [&](auto algo) {
        rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
            [algo, value](auto c) { return algo(c.range(), c.out(), value); },
            [value](auto c) { return std::ranges::remove_copy(c.range(), c.out(), value); });
    };

    check_iter(raze::algorithm::remove_copy);
    check_range(raze::algorithm::remove_copy);
    check_iter(raze::algorithm::remove_copy[opt::fscalar]);
    check_range(raze::algorithm::remove_copy[opt::fscalar]);
    check_range(raze::algorithm::remove_copy[opt::unroll<1>]);
    check_range(raze::algorithm::remove_copy[opt::fstatic]);
    check_range(raze::algorithm::remove_copy[opt::fscalar][opt::unroll<2>]);
    check_range(raze::algorithm::remove_copy[opt::fstatic][opt::unroll<2>]);
    check_iter(raze::algorithm::remove_copy[opt::fautovec]);
    check_range(raze::algorithm::remove_copy[opt::fautovec][opt::fstatic]);

    {
        auto acfg = rtts::algorithm::config::compact();

        auto check_aligned = [&]<size_t A1, size_t AOut>(auto algo) {
            rtts::algorithm::run_aligned<A1, AOut, rtts::algorithm::range, T>(acfg,
                [algo, value](auto c) { return algo(c.first(), c.last(), c.out(), value); },
                [value](auto c) { return std::ranges::remove_copy(c.first(), c.last(), c.out(), value); });

            rtts::algorithm::run_aligned<A1, AOut, rtts::algorithm::range, T>(acfg,
                [algo, value](auto c) { return algo(c.range(), c.out(), value); },
                [value](auto c) { return std::ranges::remove_copy(c.range(), c.out(), value); });
        };

        check_aligned.template operator()<16, 16>(raze::algorithm::remove_copy[opt::aligned<16>]);
        check_aligned.template operator()<32, 32>(raze::algorithm::remove_copy[opt::aligned<32>]);
        check_aligned.template operator()<32, 32>(raze::algorithm::remove_copy[opt::aligned<32>][opt::unroll<2>]);
        check_aligned.template operator()<64, 64>(raze::algorithm::remove_copy[opt::aligned<64>]);
        check_aligned.template operator()<64, 64>(raze::algorithm::remove_copy[opt::aligned<64>][opt::fautovec]);
        check_aligned.template operator()<64, 64>(raze::algorithm::remove_copy[opt::aligned<64>][opt::fscalar]);
        check_aligned.template operator()<1, 1>(raze::algorithm::remove_copy[opt::aligned<opt::unaligned>]);
        check_aligned.template operator()<1, 1>(raze::algorithm::remove_copy[opt::aligned<64>][opt::aligned<opt::unaligned>]);
        check_aligned.template operator()<64, 1>(raze::algorithm::remove_copy[opt::aligned<64, opt::unaligned>]);
        check_aligned.template operator()<1, 64>(raze::algorithm::remove_copy[opt::aligned<opt::unaligned, 64>]);
        check_aligned.template operator()<32, 64>(raze::algorithm::remove_copy[opt::aligned<32, 64>][opt::fstatic]);
        check_aligned.template operator()<64, 16>(raze::algorithm::remove_copy[opt::aligned<64, 16>]);
    }

    constexpr auto check_ce_range = [](auto algo) constexpr {
        return rtts::algorithm::constexpr_run<T, rtts::algorithm::range, 8>(
            [algo](auto c) constexpr { return algo(c.range(), c.out(), T(0)); },
            [](auto c) constexpr { return std::ranges::remove_copy(c.range(), c.out(), T(0)); });
    };

    static_assert(rtts::algorithm::constexpr_run<T, rtts::algorithm::range, 8>(
        [](auto c) constexpr { return raze::algorithm::remove_copy(c.first(), c.last(), c.out(), T(0)); },
        [](auto c) constexpr { return std::ranges::remove_copy(c.first(), c.last(), c.out(), T(0)); }));
    static_assert(check_ce_range(raze::algorithm::remove_copy));
    static_assert(check_ce_range(raze::algorithm::remove_copy[opt::fscalar]));
    static_assert(check_ce_range(raze::algorithm::remove_copy[opt::unroll<2>]));
    static_assert(check_ce_range(raze::algorithm::remove_copy[opt::fautovec]));
    static_assert(check_ce_range(raze::algorithm::remove_copy[opt::aligned<64, opt::unaligned>]));
};

RTTS_CASE_TPL("raze::algorithm::remove_copy.projection", rtts::algorithm::all_types)
< class T > (rtts::type<T>) {
    namespace opt = raze::options;
    auto cfg = rtts::algorithm::config::thorough();
    auto acfg = rtts::algorithm::config::compact();

    const T value = rtts::random::generator<T>(12345)();
    auto proj = [](const T& x) -> const T& { return x; };

    auto check_range = [&](auto algo) {
        rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
            [algo, value, proj](auto c) { return algo(c.range(), c.out(), value, proj); },
            [value, proj](auto c) { return std::ranges::remove_copy(c.range(), c.out(), value, proj); });
    };

    rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
        [value, proj](auto c) { return raze::algorithm::remove_copy(c.first(), c.last(), c.out(), value, proj); },
        [value, proj](auto c) { return std::ranges::remove_copy(c.first(), c.last(), c.out(), value, proj); });

    check_range(raze::algorithm::remove_copy);
    check_range(raze::algorithm::remove_copy[opt::fscalar]);
    check_range(raze::algorithm::remove_copy[opt::unroll<2>]);
    check_range(raze::algorithm::remove_copy[opt::fautovec]);

    auto check_aligned = [&]<size_t A1, size_t AOut>(auto algo) {
        rtts::algorithm::run_aligned<A1, AOut, rtts::algorithm::range, T>(acfg,
            [algo, value, proj](auto c) { return algo(c.range(), c.out(), value, proj); },
            [value, proj](auto c) { return std::ranges::remove_copy(c.range(), c.out(), value, proj); });
    };
    check_aligned.template operator()<64, 1>(raze::algorithm::remove_copy[opt::aligned<64, opt::unaligned>]);
    check_aligned.template operator()<1, 64>(raze::algorithm::remove_copy[opt::aligned<opt::unaligned, 64>]);

    static_assert(rtts::algorithm::constexpr_run<T, rtts::algorithm::range, 8>(
        [](auto c) constexpr {
            return raze::algorithm::remove_copy(c.first(), c.last(), c.out(), T(0), [](const T& x) constexpr -> const T& { return x; });
        },
        [](auto c) constexpr {
            return std::ranges::remove_copy(c.first(), c.last(), c.out(), T(0), [](const T& x) constexpr -> const T& { return x; });
        }));

    static_assert(rtts::algorithm::constexpr_run<T, rtts::algorithm::range, 8>(
        [](auto c) constexpr {
            return raze::algorithm::remove_copy[opt::fscalar](c.range(), c.out(), T(0), [](const T& x) constexpr -> const T& { return x; });
        },
        [](auto c) constexpr {
            return std::ranges::remove_copy(c.range(), c.out(), T(0), [](const T& x) constexpr -> const T& { return x; });
        }));
};

RTTS_CASE("raze::algorithm::remove_copy.mixed") {
    namespace opt = raze::options;
    auto cfg = rtts::algorithm::config::thorough();

    auto check_iter = [&]<class T1, class T2>(auto algo) {
        rtts::algorithm::run<rtts::algorithm::range, T1, T2>(cfg,
            [algo](auto c) { return algo(c.first(), c.last(), c.out2(), T1(0)); },
            [](auto c) { return std::ranges::remove_copy(c.first(), c.last(), c.out2(), T1(0)); });
    };

    auto check_range = [&]<class T1, class T2>(auto algo) {
        rtts::algorithm::run<rtts::algorithm::range, T1, T2>(cfg,
            [algo](auto c) { return algo(c.range(), c.out2(), T1(0)); },
            [](auto c) { return std::ranges::remove_copy(c.range(), c.out2(), T1(0)); });
    };

    check_iter.template operator()<int32_t, float>(raze::algorithm::remove_copy);
    check_iter.template operator()<uint32_t, float>(raze::algorithm::remove_copy);
    check_range.template operator()<int64_t, double>(raze::algorithm::remove_copy);
    check_range.template operator()<int8_t, int16_t>(raze::algorithm::remove_copy);
    check_range.template operator()<int32_t, int64_t>(raze::algorithm::remove_copy);
    check_range.template operator()<int16_t, int8_t>(raze::algorithm::remove_copy);
    check_iter.template operator()<int32_t, float>(raze::algorithm::remove_copy[opt::fscalar]);
    check_iter.template operator()<int32_t, float>(raze::algorithm::remove_copy[opt::fautovec]);

    rtts::algorithm::run_aligned_mixed<64, 1, rtts::algorithm::range, int32_t, float>(rtts::algorithm::config::compact(),
        [](auto c) { return raze::algorithm::remove_copy[opt::aligned<64, opt::unaligned>](c.range(), c.out2(), int32_t(0)); },
        [](auto c) { return std::ranges::remove_copy(c.range(), c.out2(), int32_t(0)); });

    constexpr auto check_ce = []<class T1, class T2>() constexpr {
        return rtts::algorithm::constexpr_run_mixed<T1, T2, rtts::algorithm::range, 8>(
            [](auto c) constexpr { return raze::algorithm::remove_copy(c.range(), c.out2(), T1(0)); },
            [](auto c) constexpr { return std::ranges::remove_copy(c.range(), c.out2(), T1(0)); });
    };

    static_assert(rtts::algorithm::constexpr_run_mixed<int32_t, float, rtts::algorithm::range, 8>(
        [](auto c) constexpr { return raze::algorithm::remove_copy(c.first(), c.last(), c.out2(), int32_t(0)); },
        [](auto c) constexpr { return std::ranges::remove_copy(c.first(), c.last(), c.out2(), int32_t(0)); }));
    static_assert(check_ce.template operator()<int64_t, double>());
    static_assert(check_ce.template operator()<int8_t, int16_t>());
};

RTTS_CASE("raze::algorithm::remove_copy views and edge cases") {
    namespace opt = raze::options;
    namespace views = std::views;

    {
        const std::vector<int> src = { 1, 2, 3, 2, 4, 2, 5, 6, 2, 7, 8, 2, 9, 10 };

        std::vector<int> r1, s1;
        raze::algorithm::remove_copy(views::iota(1, 15), std::back_inserter(r1), 7);
        std::ranges::remove_copy(views::iota(1, 15), std::back_inserter(s1), 7);
        RTTS_EXPECT(r1 == s1);

        auto chain = src | views::reverse | views::filter([](int x) { return x < 9; }) | views::transform([](int x) { return x * 2; });
        std::vector<int> r2, s2;
        raze::algorithm::remove_copy[opt::aligned<64, opt::unaligned>](chain, std::back_inserter(r2), 4);
        std::ranges::remove_copy(chain, std::back_inserter(s2), 4);
        RTTS_EXPECT(r2 == s2);

        std::vector<int> r3, s3;
        raze::algorithm::remove_copy(rtts::make_non_common(src), std::back_inserter(r3), 2);
        std::ranges::remove_copy(rtts::make_non_common(src), std::back_inserter(s3), 2);
        RTTS_EXPECT(r3 == s3);
    }

    {
        const std::vector<int> src = { 1, 2, 3, 2, 4, 2, 5 };
        const std::vector<int> expected = { 1, 3, 4, 5 };

        std::vector<int> b1, b2, b3, b4;
        raze::algorithm::remove_copy(src, std::back_inserter(b1), 2);
        raze::algorithm::remove_copy[opt::fscalar](src, std::back_inserter(b2), 2);
        raze::algorithm::remove_copy[opt::fautovec][opt::fstatic](src, std::back_inserter(b3), 2);
        alignas(64) const std::array<int, 7> src_aligned = { 1, 2, 3, 2, 4, 2, 5 };
        raze::algorithm::remove_copy[opt::aligned<64, opt::unaligned>](src_aligned, std::back_inserter(b4), 2);
        RTTS_EXPECT(b1 == expected && b2 == expected && b3 == expected && b4 == expected);

        std::deque<int> f1, f2;
        raze::algorithm::remove_copy(src, std::front_inserter(f1), 2);
        std::ranges::remove_copy(src, std::front_inserter(f2), 2);
        RTTS_EXPECT(f1 == f2);

        std::vector<int> i1 = { 0, 0 }, i2 = { 0, 0 };
        raze::algorithm::remove_copy(src, std::inserter(i1, i1.begin() + 1), 2);
        std::ranges::remove_copy(src, std::inserter(i2, i2.begin() + 1), 2);
        RTTS_EXPECT(i1 == i2);

        rtts::single_pass_input_range<int> spr(src);
        std::vector<int> sp_out;
        raze::algorithm::remove_copy(spr, std::back_inserter(sp_out), 2);
        RTTS_EXPECT(sp_out == expected && spr.reads() == src.size());

        std::vector<std::string> words = { "keep", "drop", "also_keep", "drop" };
        std::vector<std::string> out_w;
        raze::algorithm::remove_copy(words, std::back_inserter(out_w), std::string("drop"));
        RTTS_EXPECT((out_w == std::vector<std::string>{ "keep", "also_keep" }));
        RTTS_EXPECT((words == std::vector<std::string>{ "keep", "drop", "also_keep", "drop" }));

        volatile int vsrc[16];
        volatile int vdst[16];
        for (int i = 0; i < 16; ++i) vsrc[i] = (i % 2 == 0) ? 99 : i;
        auto vr = raze::algorithm::remove_copy(vsrc, vsrc + 16, vdst, 99);
        RTTS_EXPECT(vr.out == vdst + 8);
        for (int i = 0; i < 8; ++i) RTTS_EXPECT(vdst[i] == i * 2 + 1);

        std::vector<int> rdst;
        auto rres = raze::algorithm::remove_copy(std::vector<int>{ 1, 2, 1, 3 }, std::back_inserter(rdst), 1);
        static_assert(std::same_as<decltype(rres.in), std::ranges::dangling>);
        RTTS_EXPECT((rdst == std::vector<int>{ 2, 3 }));

        const std::vector<rtts::test_item> items = { { 1, 10 }, { 2, 20 }, { 1, 30 }, { 3, 40 } };
        std::vector<rtts::test_item> out_items;
        raze::algorithm::remove_copy(items, std::back_inserter(out_items), 1, &rtts::test_item::key);
        RTTS_EXPECT(out_items.size() == 2 && out_items[0].key == 2 && out_items[1].key == 3);
    }
};