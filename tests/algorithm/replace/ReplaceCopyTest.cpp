#include <tests/rts/rts.h>
#include <raze/algorithm/replace/ReplaceCopy.h>

RTTS_CASE_TPL("raze::algorithm::replace_copy", rtts::algorithm::all_types)
<class T> (rtts::type<T>) {
    using raze::algorithm::replace_copy;
    namespace opt = raze::options;

    auto cfg = rtts::algorithm::config::thorough();

    auto check = [&](auto algo) {
        auto check_vals = [&](T old_val, T new_val) {
            rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
                [algo, old_val, new_val](auto c) { return algo(c.first(), c.last(), c.out(), old_val, new_val); },
                [old_val, new_val](auto c) { return std::ranges::replace_copy(c.first(), c.last(), c.out(), old_val, new_val); });

            rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
                [algo, old_val, new_val](auto c) { return algo(c.range(), c.out(), old_val, new_val); },
                [old_val, new_val](auto c) { return std::ranges::replace_copy(c.range(), c.out(), old_val, new_val); });
        };

        check_vals(rtts::random::generator<T>(42)(), rtts::random::generator<T>(1000)());
        const T same_val = rtts::random::generator<T>(777)();
        check_vals(same_val, same_val);
    };

    check(replace_copy);
    check(replace_copy[opt::fstatic]);
    check(replace_copy[opt::fscalar]);
    check(replace_copy[opt::fautovec]);
    check(replace_copy[opt::unroll<1>]);
    check(replace_copy[opt::unroll<2>]);

    {
        auto acfg = rtts::algorithm::config::compact();
        const T old_val = rtts::random::generator<T>(42)();
        const T new_val = rtts::random::generator<T>(1000)();

        auto check_aligned = [&]<size_t A1, size_t AOut>(auto algo) {
            rtts::algorithm::run_aligned<A1, AOut, rtts::algorithm::range, T>(acfg,
                [algo, old_val, new_val](auto c) { return algo(c.first(), c.last(), c.out(), old_val, new_val); },
                [old_val, new_val](auto c) { return std::ranges::replace_copy(c.first(), c.last(), c.out(), old_val, new_val); });

            rtts::algorithm::run_aligned<A1, AOut, rtts::algorithm::range, T>(acfg,
                [algo, old_val, new_val](auto c) { return algo(c.range(), c.out(), old_val, new_val); },
                [old_val, new_val](auto c) { return std::ranges::replace_copy(c.range(), c.out(), old_val, new_val); });
        };

        check_aligned.template operator()<16, 16>(replace_copy[opt::aligned<16>]);
        check_aligned.template operator()<32, 32>(replace_copy[opt::aligned<32>]);
        check_aligned.template operator()<32, 32>(replace_copy[opt::aligned<32>][opt::fstatic]);
        check_aligned.template operator()<32, 32>(replace_copy[opt::aligned<32>][opt::unroll<2>]);
        check_aligned.template operator()<64, 64>(replace_copy[opt::aligned<64>]);
        check_aligned.template operator()<64, 64>(replace_copy[opt::aligned<64>][opt::fautovec]);
        check_aligned.template operator()<64, 64>(replace_copy[opt::aligned<64>][opt::fscalar]);
        check_aligned.template operator()<1, 1>(replace_copy[opt::aligned<opt::unaligned>]);
        check_aligned.template operator()<1, 1>(replace_copy[opt::aligned<64>][opt::aligned<opt::unaligned>]);
        check_aligned.template operator()<64, 1>(replace_copy[opt::aligned<64, opt::unaligned>]);
        check_aligned.template operator()<1, 64>(replace_copy[opt::aligned<opt::unaligned, 64>]);
        check_aligned.template operator()<32, 64>(replace_copy[opt::aligned<32, 64>]);
        check_aligned.template operator()<32, 64>(replace_copy[opt::aligned<32, 64>][opt::fautovec]);
        check_aligned.template operator()<64, 16>(replace_copy[opt::aligned<64, 16>]);
    }

    constexpr auto check_ce_iter = [](auto algo) constexpr {
        return rtts::algorithm::constexpr_run<T, rtts::algorithm::range, 8>(
            [algo](auto c) constexpr { return algo(c.first(), c.last(), c.out(), T(2), T(99)); },
            [](auto c) constexpr { return std::ranges::replace_copy(c.first(), c.last(), c.out(), T(2), T(99)); });
    };
    constexpr auto check_ce_range = [](auto algo) constexpr {
        return rtts::algorithm::constexpr_run<T, rtts::algorithm::range, 8>(
            [algo](auto c) constexpr { return algo(c.range(), c.out(), T(2), T(99)); },
            [](auto c) constexpr { return std::ranges::replace_copy(c.range(), c.out(), T(2), T(99)); });
    };

    static_assert(check_ce_iter(replace_copy));
    static_assert(check_ce_iter(replace_copy[opt::fstatic]));
    static_assert(check_ce_range(replace_copy[opt::fscalar]));
    static_assert(check_ce_range(replace_copy[opt::fautovec]));
    static_assert(check_ce_range(replace_copy[opt::aligned<64, opt::unaligned>]));
};

RTTS_CASE_TPL("raze::algorithm::replace_copy.projection", rtts::algorithm::all_types)
<class T> (rtts::type<T>) {
    using raze::algorithm::replace_copy;
    namespace opt = raze::options;

    auto cfg = rtts::algorithm::config::thorough();
    auto acfg = rtts::algorithm::config::compact();

    const T old_val = rtts::random::generator<T>(12345)();
    const T new_val = rtts::random::generator<T>(67890)();
    auto proj = [](const T& x) -> const T& { return x; };

    auto check = [&](auto algo) {
        rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
            [algo, old_val, new_val, proj](auto c) { return algo(c.first(), c.last(), c.out(), old_val, new_val, proj); },
            [old_val, new_val, proj](auto c) { return std::ranges::replace_copy(c.first(), c.last(), c.out(), old_val, new_val, proj); });

        rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
            [algo, old_val, new_val, proj](auto c) { return algo(c.range(), c.out(), old_val, new_val, proj); },
            [old_val, new_val, proj](auto c) { return std::ranges::replace_copy(c.range(), c.out(), old_val, new_val, proj); });
    };

    check(replace_copy);
    check(replace_copy[opt::fstatic]);
    check(replace_copy[opt::fscalar]);
    check(replace_copy[opt::fautovec]);

    rtts::algorithm::run_aligned<64, 1, rtts::algorithm::range, T>(acfg,
        [old_val, new_val, proj](auto c) { return replace_copy[opt::aligned<64, opt::unaligned>](c.range(), c.out(), old_val, new_val, proj); },
        [old_val, new_val, proj](auto c) { return std::ranges::replace_copy(c.range(), c.out(), old_val, new_val, proj); });

    rtts::algorithm::run_aligned<1, 64, rtts::algorithm::range, T>(acfg,
        [old_val, new_val, proj](auto c) { return replace_copy[opt::aligned<opt::unaligned, 64>](c.range(), c.out(), old_val, new_val, proj); },
        [old_val, new_val, proj](auto c) { return std::ranges::replace_copy(c.range(), c.out(), old_val, new_val, proj); });

    constexpr auto check_ce_range = [](auto algo) constexpr {
        return rtts::algorithm::constexpr_run<T, rtts::algorithm::range, 8>(
            [algo](auto c) constexpr {
                return algo(c.range(), c.out(), T(2), T(99), [](const T& x) constexpr -> const T& { return x; });
            },
            [](auto c) constexpr {
                return std::ranges::replace_copy(c.range(), c.out(), T(2), T(99), [](const T& x) constexpr -> const T& { return x; });
            });
    };

    static_assert(rtts::algorithm::constexpr_run<T, rtts::algorithm::range, 8>(
        [](auto c) constexpr {
            return replace_copy(c.first(), c.last(), c.out(), T(2), T(99), [](const T& x) constexpr -> const T& { return x; });
        },
        [](auto c) constexpr {
            return std::ranges::replace_copy(c.first(), c.last(), c.out(), T(2), T(99), [](const T& x) constexpr -> const T& { return x; });
        }));
    static_assert(check_ce_range(replace_copy[opt::fscalar]));
    static_assert(check_ce_range(replace_copy[opt::fautovec][opt::aligned<64, opt::unaligned>]));
};

RTTS_CASE("raze::algorithm::replace_copy views and edge cases") {
    namespace opt = raze::options;
    namespace views = std::views;

    {
        const std::vector<int> src = { 1, 2, 3, 2, 4, 2, 5, 6, 2, 7, 8, 2, 9, 10 };

        std::vector<int> r1, s1;
        raze::algorithm::replace_copy(views::iota(1, 20), std::back_inserter(r1), 5, 500);
        std::ranges::replace_copy(views::iota(1, 20), std::back_inserter(s1), 5, 500);
        RTTS_EXPECT(r1 == s1);

        auto chain = src | views::reverse | views::filter([](int x) { return x < 9; }) | views::transform([](int x) { return x * 2; });
        std::vector<int> r2, s2;
        raze::algorithm::replace_copy[opt::fautovec][opt::fstatic](chain, std::back_inserter(r2), 4, 99);
        std::ranges::replace_copy(chain, std::back_inserter(s2), 4, 99);
        RTTS_EXPECT(r2 == s2);

        std::vector<int> nc1(src.size()), nc2(src.size());
        raze::algorithm::replace_copy[opt::fstatic](rtts::make_non_common(src), nc1.begin(), 2, 42);
        std::ranges::replace_copy(rtts::make_non_common(src), nc2.begin(), 2, 42);
        RTTS_EXPECT(nc1 == nc2);
    }

    {
        const std::vector<int> src = { 1, 2, 3, 2, 4, 2, 5 };
        std::vector<int> expected = { 1, 9, 3, 9, 4, 9, 5 };

        std::vector<int> b1, b2, b3, b4;
        raze::algorithm::replace_copy(src, std::back_inserter(b1), 2, 9);
        raze::algorithm::replace_copy[opt::fscalar](src, std::back_inserter(b2), 2, 9);
        raze::algorithm::replace_copy[opt::fautovec][opt::fstatic](src, std::back_inserter(b3), 2, 9);
        alignas(64) const std::array<int, 7> src_aligned = { 1, 2, 3, 2, 4, 2, 5 };
        raze::algorithm::replace_copy[opt::aligned<64, opt::unaligned>](src_aligned, std::back_inserter(b4), 2, 9);
        RTTS_EXPECT(b1 == expected && b2 == expected && b3 == expected && b4 == expected);

        std::deque<int> f1, f2;
        raze::algorithm::replace_copy(src, std::front_inserter(f1), 2, 9);
        std::ranges::replace_copy(src, std::front_inserter(f2), 2, 9);
        RTTS_EXPECT(f1 == f2);

        std::vector<int> i1 = { 0, 0 }, i2 = { 0, 0 };
        raze::algorithm::replace_copy(src, std::inserter(i1, i1.begin() + 1), 2, 9);
        std::ranges::replace_copy(src, std::inserter(i2, i2.begin() + 1), 2, 9);
        RTTS_EXPECT(i1 == i2);

        rtts::single_pass_input_range<int> spr(src);
        std::vector<int> sp_out;
        raze::algorithm::replace_copy(spr, std::back_inserter(sp_out), 2, 9);
        RTTS_EXPECT(sp_out == expected && spr.reads() == src.size());
    }

    {
        std::vector<int> inplace = { 1, 2, 3, 2, 4, 2, 5, 2, 6, 2, 7, 2, 8, 2, 9, 2, 10 };
        auto exp = inplace;
        raze::algorithm::replace_copy(inplace, inplace.begin(), 2, 88);
        std::ranges::replace(exp, 2, 88);
        RTTS_EXPECT(inplace == exp);

        volatile int vsrc[16];
        volatile int vdst[16];
        for (int i = 0; i < 16; ++i) vsrc[i] = (i % 4 == 0) ? 99 : i;
        raze::algorithm::replace_copy(vsrc, vsrc + 16, vdst, 99, 77);
        for (int i = 0; i < 16; ++i) RTTS_EXPECT(vdst[i] == ((i % 4 == 0) ? 77 : i));

        std::vector<int> rdst;
        auto rres = raze::algorithm::replace_copy(std::vector<int>{ 1, 2, 1 }, std::back_inserter(rdst), 1, 9);
        static_assert(std::same_as<decltype(rres.in), std::ranges::dangling>);
        RTTS_EXPECT((rdst == std::vector<int>{ 9, 2, 9 }));

        const std::vector<rtts::test_item> items = { { 1, 10 }, { 2, 20 }, { 1, 30 } };
        std::vector<rtts::test_item> out_items;
        raze::algorithm::replace_copy(items, std::back_inserter(out_items), 1, rtts::test_item{ 9, 90 }, &rtts::test_item::key);
        RTTS_EXPECT(out_items.size() == 3 && out_items[0].key == 9 && out_items[1].key == 2 && out_items[2].key == 9);
    }
};
