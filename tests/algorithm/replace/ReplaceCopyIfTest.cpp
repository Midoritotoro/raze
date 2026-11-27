#include <tests/rts/rts.h>
#include <raze/algorithm/replace/ReplaceCopyIf.h>

RTTS_CASE_TPL("raze::algorithm::replace_copy_if", rtts::algorithm::all_types)
<class T> (rtts::type<T>) {
    using raze::algorithm::replace_copy_if;
    namespace opt = raze::options;

    auto cfg = rtts::algorithm::config::thorough();

    auto check = [&](auto algo) {
        auto check_pred = [&](auto pred, T new_val) {
            rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
                [algo, pred, new_val](auto c) { return algo(c.first(), c.last(), c.out(), pred, new_val); },
                [pred, new_val](auto c) { return std::ranges::replace_copy_if(c.first(), c.last(), c.out(), pred, new_val); });

            rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
                [algo, pred, new_val](auto c) { return algo(c.range(), c.out(), pred, new_val); },
                [pred, new_val](auto c) { return std::ranges::replace_copy_if(c.range(), c.out(), pred, new_val); });
        };

        auto check_pair = [&](auto pred_scalar, auto pred_vector, T new_val) {
            check_pred(pred_scalar, new_val);
            check_pred(pred_vector, new_val);
        };

        const T threshold = rtts::random::generator<T>(1000)();
        check_pair(
            [threshold](const T& x) { return x > threshold; },
            [threshold](auto x) { return x > threshold; },
            rtts::random::generator<T>(2000)());

        const T target = rtts::random::generator<T>(4000)();
        check_pair(
            [target](const T& x) { return x == target; },
            [target](auto x) { return x == target; },
            rtts::random::generator<T>(5000)());

        check_pair(
            [](const T&) { return false; },
            [](auto) { return false; },
            rtts::random::generator<T>(6000)());

        check_pair(
            [](const T&) { return true; },
            [](auto) { return true; },
            rtts::random::generator<T>(7000)());
    };

    check(replace_copy_if);
    check(replace_copy_if[opt::fstatic]);
    check(replace_copy_if[opt::fscalar]);
    check(replace_copy_if[opt::fautovec]);
    check(replace_copy_if[opt::unroll<1>]);
    check(replace_copy_if[opt::unroll<2>]);

    {
        auto acfg = rtts::algorithm::config::compact();
        const T threshold = rtts::random::generator<T>(1000)();
        const T new_val = rtts::random::generator<T>(2000)();
        auto pred_scalar = [threshold](const T& x) { return x > threshold; };
        auto pred_vector = [threshold](auto x) { return x > threshold; };
        auto pred_branching = [threshold](auto x) { if (x > threshold) return true; return false; };

        rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
            [pred_branching, new_val](auto c) { return replace_copy_if[opt::fautovec](c.range(), c.out(), pred_branching, new_val); },
            [pred_branching, new_val](auto c) { return std::ranges::replace_copy_if(c.range(), c.out(), pred_branching, new_val); });

        rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
            [pred_branching, new_val](auto c) { return replace_copy_if[opt::fautovec][opt::fstatic](c.range(), c.out(), pred_branching, new_val); },
            [pred_branching, new_val](auto c) { return std::ranges::replace_copy_if(c.range(), c.out(), pred_branching, new_val); });

        auto check_aligned = [&]<size_t A1, size_t AOut>(auto algo, auto pred) {
            rtts::algorithm::run_aligned<A1, AOut, rtts::algorithm::range, T>(acfg,
                [algo, pred, new_val](auto c) { return algo(c.first(), c.last(), c.out(), pred, new_val); },
                [pred, new_val](auto c) { return std::ranges::replace_copy_if(c.first(), c.last(), c.out(), pred, new_val); });

            rtts::algorithm::run_aligned<A1, AOut, rtts::algorithm::range, T>(acfg,
                [algo, pred, new_val](auto c) { return algo(c.range(), c.out(), pred, new_val); },
                [pred, new_val](auto c) { return std::ranges::replace_copy_if(c.range(), c.out(), pred, new_val); });
        };

        check_aligned.template operator()<16, 16>(replace_copy_if[opt::aligned<16>], pred_vector);
        check_aligned.template operator()<32, 32>(replace_copy_if[opt::aligned<32>], pred_vector);
        check_aligned.template operator()<32, 32>(replace_copy_if[opt::aligned<32>][opt::fstatic], pred_vector);
        check_aligned.template operator()<32, 32>(replace_copy_if[opt::aligned<32>][opt::unroll<2>], pred_vector);
        check_aligned.template operator()<64, 64>(replace_copy_if[opt::aligned<64>], pred_vector);
        check_aligned.template operator()<64, 64>(replace_copy_if[opt::aligned<64>], pred_scalar);
        check_aligned.template operator()<64, 64>(replace_copy_if[opt::aligned<64>][opt::fautovec], pred_branching);
        check_aligned.template operator()<64, 64>(replace_copy_if[opt::aligned<64>][opt::fscalar], pred_vector);
        check_aligned.template operator()<1, 1>(replace_copy_if[opt::aligned<opt::unaligned>], pred_vector);
        check_aligned.template operator()<1, 1>(replace_copy_if[opt::aligned<64>][opt::aligned<opt::unaligned>], pred_vector);
        check_aligned.template operator()<64, 1>(replace_copy_if[opt::aligned<64, opt::unaligned>], pred_vector);
        check_aligned.template operator()<1, 64>(replace_copy_if[opt::aligned<opt::unaligned, 64>], pred_vector);
        check_aligned.template operator()<32, 64>(replace_copy_if[opt::aligned<32, 64>], pred_vector);
        check_aligned.template operator()<32, 64>(replace_copy_if[opt::aligned<32, 64>][opt::fautovec], pred_branching);
        check_aligned.template operator()<64, 16>(replace_copy_if[opt::aligned<64, 16>], pred_vector);
        check_aligned.template operator()<1, 32>(replace_copy_if[opt::aligned<64>][opt::aligned<opt::unaligned, 32>], pred_vector);
    }

    constexpr auto check_ce_iter = [](auto algo, auto pred) constexpr {
        return rtts::algorithm::constexpr_run<T, rtts::algorithm::range, 8>(
            [algo, pred](auto c) constexpr { return algo(c.first(), c.last(), c.out(), pred, T(99)); },
            [pred](auto c) constexpr { return std::ranges::replace_copy_if(c.first(), c.last(), c.out(), pred, T(99)); });
    };
    constexpr auto check_ce_range = [](auto algo, auto pred) constexpr {
        return rtts::algorithm::constexpr_run<T, rtts::algorithm::range, 8>(
            [algo, pred](auto c) constexpr { return algo(c.range(), c.out(), pred, T(99)); },
            [pred](auto c) constexpr { return std::ranges::replace_copy_if(c.range(), c.out(), pred, T(99)); });
    };

    static_assert(check_ce_iter(replace_copy_if, [](const T& x) constexpr { return x > T(0); }));
    static_assert(check_ce_iter(replace_copy_if[opt::fstatic], [](const T& x) constexpr { return x > T(0); }));
    static_assert(check_ce_range(replace_copy_if[opt::fscalar], [](auto x) constexpr { return x > T(0); }));
    static_assert(check_ce_range(replace_copy_if[opt::fautovec], [](auto x) constexpr { if (x > T(2)) return true; return false; }));
    static_assert(check_ce_range(replace_copy_if[opt::aligned<64, opt::unaligned>], [](auto x) constexpr { return x > T(0); }));
};

RTTS_CASE_TPL("raze::algorithm::replace_copy_if.projection", rtts::algorithm::all_types)
<class T> (rtts::type<T>) {
    using raze::algorithm::replace_copy_if;
    namespace opt = raze::options;

    auto cfg = rtts::algorithm::config::thorough();
    auto acfg = rtts::algorithm::config::compact();

    const T threshold = rtts::random::generator<T>(12345)();
    const T new_val = rtts::random::generator<T>(67890)();

    auto pred_scalar = [threshold](const T& x) { return x > threshold; };
    auto pred_vector = [threshold](auto x) { return x > threshold; };
    auto pred_branching = [threshold](auto x) { if (x > threshold) return true; return false; };
    auto proj = [](const T& x) -> const T& { return x; };

    auto check = [&](auto algo) {
        auto check_pred = [&](auto pred) {
            rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
                [algo, pred, new_val, proj](auto c) { return algo(c.first(), c.last(), c.out(), pred, new_val, proj); },
                [pred, new_val, proj](auto c) { return std::ranges::replace_copy_if(c.first(), c.last(), c.out(), pred, new_val, proj); });

            rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
                [algo, pred, new_val, proj](auto c) { return algo(c.range(), c.out(), pred, new_val, proj); },
                [pred, new_val, proj](auto c) { return std::ranges::replace_copy_if(c.range(), c.out(), pred, new_val, proj); });
        };

        check_pred(pred_scalar);
        check_pred(pred_vector);
    };

    check(replace_copy_if);
    check(replace_copy_if[opt::fstatic]);
    check(replace_copy_if[opt::fscalar]);
    check(replace_copy_if[opt::fautovec]);

    rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
        [pred_branching, new_val, proj](auto c) { return replace_copy_if[opt::fautovec](c.range(), c.out(), pred_branching, new_val, proj); },
        [pred_branching, new_val, proj](auto c) { return std::ranges::replace_copy_if(c.range(), c.out(), pred_branching, new_val, proj); });

    rtts::algorithm::run_aligned<64, 1, rtts::algorithm::range, T>(acfg,
        [pred_vector, new_val, proj](auto c) { return replace_copy_if[opt::aligned<64, opt::unaligned>](c.range(), c.out(), pred_vector, new_val, proj); },
        [pred_vector, new_val, proj](auto c) { return std::ranges::replace_copy_if(c.range(), c.out(), pred_vector, new_val, proj); });

    rtts::algorithm::run_aligned<1, 64, rtts::algorithm::range, T>(acfg,
        [pred_vector, new_val, proj](auto c) { return replace_copy_if[opt::aligned<opt::unaligned, 64>](c.range(), c.out(), pred_vector, new_val, proj); },
        [pred_vector, new_val, proj](auto c) { return std::ranges::replace_copy_if(c.range(), c.out(), pred_vector, new_val, proj); });

    constexpr auto check_ce_range = [](auto algo, auto p) constexpr {
        return rtts::algorithm::constexpr_run<T, rtts::algorithm::range, 8>(
            [algo, p](auto c) constexpr {
                return algo(c.range(), c.out(), p, T(99), [](const T& x) constexpr -> const T& { return x; });
            },
            [p](auto c) constexpr {
                return std::ranges::replace_copy_if(c.range(), c.out(), p, T(99), [](const T& x) constexpr -> const T& { return x; });
            });
    };

    static_assert(rtts::algorithm::constexpr_run<T, rtts::algorithm::range, 8>(
        [](auto c) constexpr {
            return replace_copy_if(c.first(), c.last(), c.out(),
                [](const T& x) constexpr { return x > T(0); }, T(99),
                [](const T& x) constexpr -> const T& { return x; });
        },
        [](auto c) constexpr {
            return std::ranges::replace_copy_if(c.first(), c.last(), c.out(),
                [](const T& x) constexpr { return x > T(0); }, T(99),
                [](const T& x) constexpr -> const T& { return x; });
        }));
    static_assert(check_ce_range(replace_copy_if[opt::fscalar], [](auto x) constexpr { return x > T(0); }));
    static_assert(check_ce_range(replace_copy_if[opt::fautovec][opt::aligned<64, opt::unaligned>], [](auto x) constexpr { if (x > T(2)) return true; return false; }));
};

RTTS_CASE("raze::algorithm::replace_copy_if views and edge cases") {
    namespace opt = raze::options;
    namespace views = std::views;

    {
        const std::vector<int> src = { -4, -3, -2, -1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
        auto neg_branch = [](auto x) { if (x < 0) return true; return false; };

        std::vector<int> r1, s1;
        raze::algorithm::replace_copy_if[opt::fautovec][opt::fstatic](views::iota(-10, 10), std::back_inserter(r1), neg_branch, 0);
        std::ranges::replace_copy_if(views::iota(-10, 10), std::back_inserter(s1), neg_branch, 0);
        RTTS_EXPECT(r1 == s1);

        auto chain = src | views::reverse | views::filter([](int x) { return (x % 2) == 0; }) | views::transform([](int x) { return x - 1; });
        std::vector<int> r2, s2;
        raze::algorithm::replace_copy_if[opt::aligned<64, opt::unaligned>](chain, std::back_inserter(r2), [](int x) { return x < 0; }, 99);
        std::ranges::replace_copy_if(chain, std::back_inserter(s2), [](int x) { return x < 0; }, 99);
        RTTS_EXPECT(r2 == s2);

        std::vector<int> nc1(src.size()), nc2(src.size());
        raze::algorithm::replace_copy_if(rtts::make_non_common(src), nc1.begin(), [](auto x) { return x > 5; }, 5);
        std::ranges::replace_copy_if(rtts::make_non_common(src), nc2.begin(), [](auto x) { return x > 5; }, 5);
        RTTS_EXPECT(nc1 == nc2);
    }

    {
        const std::vector<int> src = { -2, -1, 0, 1, 2, 3 };
        auto is_neg = [](auto x) { return x < 0; };
        const std::vector<int> expected = { 0, 0, 0, 1, 2, 3 };

        std::vector<int> b1, b2, b3, b4;
        raze::algorithm::replace_copy_if(src, std::back_inserter(b1), is_neg, 0);
        raze::algorithm::replace_copy_if[opt::fscalar](src, std::back_inserter(b2), is_neg, 0);
        raze::algorithm::replace_copy_if[opt::fautovec][opt::fstatic](src, std::back_inserter(b3), [](auto x) { if (x < 0) return true; return false; }, 0);
        alignas(64) const std::array<int, 6> src_aligned = { -2, -1, 0, 1, 2, 3 };
        raze::algorithm::replace_copy_if[opt::aligned<64, opt::unaligned>](src_aligned, std::back_inserter(b4), is_neg, 0);
        RTTS_EXPECT(b1 == expected && b2 == expected && b3 == expected && b4 == expected);

        std::deque<int> f1, f2;
        raze::algorithm::replace_copy_if(src, std::front_inserter(f1), is_neg, 0);
        std::ranges::replace_copy_if(src, std::front_inserter(f2), is_neg, 0);
        RTTS_EXPECT(f1 == f2);

        std::vector<int> i1 = { 9, 9 }, i2 = { 9, 9 };
        raze::algorithm::replace_copy_if(src, std::inserter(i1, i1.begin() + 1), is_neg, 0);
        std::ranges::replace_copy_if(src, std::inserter(i2, i2.begin() + 1), is_neg, 0);
        RTTS_EXPECT(i1 == i2);

        rtts::single_pass_input_range<int> spr(src);
        std::vector<int> sp_out;
        raze::algorithm::replace_copy_if(spr, std::back_inserter(sp_out), is_neg, 0);
        RTTS_EXPECT(sp_out == expected && spr.reads() == src.size());
    }

    {
        std::vector<int> inplace = { -5, -1, 0, 2, -3, 4, -7, 8, -9, 10, -11, 12, -13, 14, -15, 16, -17 };
        auto exp = inplace;
        raze::algorithm::replace_copy_if(inplace, inplace.begin(), [](auto x) { return x < 0; }, 0);
        std::ranges::replace_if(exp, [](int x) { return x < 0; }, 0);
        RTTS_EXPECT(inplace == exp);

        volatile int vsrc[16];
        volatile int vdst[16];
        for (int i = 0; i < 16; ++i) vsrc[i] = i - 5;
        raze::algorithm::replace_copy_if(vsrc, vsrc + 16, vdst, [](int x) { return x < 0; }, 99);
        for (int i = 0; i < 16; ++i) RTTS_EXPECT(vdst[i] == (i < 5 ? 99 : i - 5));

        std::vector<int> rdst;
        auto rres = raze::algorithm::replace_copy_if(std::vector<int>{ -1, 2, -3 }, std::back_inserter(rdst), [](auto x) { return x < 0; }, 0);
        static_assert(std::same_as<decltype(rres.in), std::ranges::dangling>);
        RTTS_EXPECT((rdst == std::vector<int>{ 0, 2, 0 }));

        const std::vector<rtts::test_item> items = { { 1, 10 }, { 6, 60 }, { 2, 20 } };
        std::vector<rtts::test_item> out_items;
        raze::algorithm::replace_copy_if(items, std::back_inserter(out_items), [](int k) { return k > 5; }, rtts::test_item{ 0, 0 }, &rtts::test_item::key);
        RTTS_EXPECT(out_items.size() == 3 && out_items[1].key == 0 && out_items[0].key == 1);
    }
};
