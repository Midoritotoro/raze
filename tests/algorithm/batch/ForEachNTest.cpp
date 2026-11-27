#include <tests/rts/rts.h>
#include <raze/algorithm/batch/ForEachN.h>

template <class It, class Size, class Fn, class Proj = std::identity>
constexpr auto std_for_each_n(It first, Size count, Fn fn, Proj proj = {}) {
#if defined(raze_cpp_msvc_only)
    using Diff = std::iter_difference_t<It>;
    auto n = static_cast<Diff>(count);
    for (; n > 0; --n, (void)++first) {
        std::invoke(fn, std::invoke(proj, *first));
    }
    return std::ranges::for_each_n_result<It, Fn>{std::move(first), std::move(fn)};
#else
    return std::ranges::for_each_n(std::move(first), static_cast<std::iter_difference_t<It>>(count), std::move(fn), std::move(proj));
#endif
}

RTTS_CASE_TPL("raze::algorithm::for_each_n", rtts::algorithm::all_types)
<class T> (rtts::type<T>) {
    namespace opt = raze::options;
    auto cfg = rtts::algorithm::config::thorough();

    const T add_val = rtts::random::generator<T>(42)();
    auto op_scalar = [add_val](T& x) { x += add_val; };
    auto op_vector = [add_val](auto& x) { x += add_val; };
    auto op_branching = [add_val](auto& x) { if (x > add_val) x += add_val; };

    auto check = [&](auto algo, auto op) {
        rtts::algorithm::run<rtts::algorithm::n, T>(cfg,
            [algo, op](auto c) { return algo(c.first(), c.count(), op); },
            [op](auto c) { return std_for_each_n(c.first(), c.count(), op); });
    };

    check(raze::algorithm::for_each_n, op_scalar);
    check(raze::algorithm::for_each_n, op_vector);
    check(raze::algorithm::for_each_n[opt::fscalar], op_vector);
    check(raze::algorithm::for_each_n[opt::unroll<1>], op_vector);
    check(raze::algorithm::for_each_n[opt::fstatic], op_vector);
    check(raze::algorithm::for_each_n[opt::fscalar][opt::unroll<2>], op_vector);
    check(raze::algorithm::for_each_n[opt::fstatic][opt::unroll<2>], op_vector);
    check(raze::algorithm::for_each_n[opt::fautovec], op_vector);
    check(raze::algorithm::for_each_n[opt::fautovec], op_branching);
    check(raze::algorithm::for_each_n[opt::fautovec][opt::fstatic], op_branching);

    {
        auto acfg = rtts::algorithm::config::compact();

        auto check_aligned = [&]<size_t Align>(auto algo, auto op) {
            rtts::algorithm::run_aligned<Align, rtts::algorithm::n, T>(acfg,
                [algo, op](auto c) { return algo(c.first(), c.count(), op); },
                [op](auto c) { return std_for_each_n(c.first(), c.count(), op); });
        };

        check_aligned.template operator()<16>(raze::algorithm::for_each_n[opt::aligned<16>], op_vector);
        check_aligned.template operator()<32>(raze::algorithm::for_each_n[opt::aligned<32>], op_vector);
        check_aligned.template operator()<32>(raze::algorithm::for_each_n[opt::aligned<32>][opt::unroll<2>], op_vector);
        check_aligned.template operator()<64>(raze::algorithm::for_each_n[opt::aligned<64>], op_vector);
        check_aligned.template operator()<64>(raze::algorithm::for_each_n[opt::aligned<64>][opt::fautovec], op_branching);
        check_aligned.template operator()<64>(raze::algorithm::for_each_n[opt::aligned<64>][opt::fscalar], op_vector);
        check_aligned.template operator()<1>(raze::algorithm::for_each_n[opt::aligned<opt::unaligned>], op_vector);
        check_aligned.template operator()<1>(raze::algorithm::for_each_n[opt::aligned<64>][opt::aligned<opt::unaligned>], op_vector);
    }

    constexpr auto check_ce = [](auto algo, auto op) constexpr {
        return rtts::algorithm::constexpr_run<T, rtts::algorithm::n, 8, 5>(
            [algo, op](auto c) constexpr { return algo(c.first(), c.count(), op); },
            [op](auto c) constexpr { return std_for_each_n(c.first(), c.count(), op); });
    };

    static_assert(check_ce(raze::algorithm::for_each_n, [](T& x) constexpr { x += T(1); }));
    static_assert(check_ce(raze::algorithm::for_each_n[opt::fscalar], [](auto& x) constexpr { x += T(1); }));
    static_assert(check_ce(raze::algorithm::for_each_n[opt::unroll<2>], [](auto& x) constexpr { x += T(1); }));
    static_assert(check_ce(raze::algorithm::for_each_n[opt::fautovec], [](auto& x) constexpr { if (x > T(2)) x += T(1); }));
    static_assert(check_ce(raze::algorithm::for_each_n[opt::aligned<64>], [](auto& x) constexpr { x += T(1); }));
};

RTTS_CASE_TPL("raze::algorithm::for_each_n.projection", rtts::algorithm::all_types)
<class T> (rtts::type<T>) {
    namespace opt = raze::options;
    auto cfg = rtts::algorithm::config::thorough();
    auto acfg = rtts::algorithm::config::compact();

    const T add_val = rtts::random::generator<T>(100000)();
    auto op = [add_val](auto& x) { x += add_val; };
    auto op_branching = [add_val](auto& x) { if (x > add_val) x += add_val; };
    auto proj = [](auto& x) -> auto& { return x; };

    auto check = [&](auto algo, auto fn) {
        rtts::algorithm::run<rtts::algorithm::n, T>(cfg,
            [algo, fn, proj](auto c) { return algo(c.first(), c.count(), fn, proj); },
            [fn, proj](auto c) { return std_for_each_n(c.first(), c.count(), fn, proj); });
    };

    check(raze::algorithm::for_each_n, op);
    check(raze::algorithm::for_each_n[opt::fscalar], op);
    check(raze::algorithm::for_each_n[opt::unroll<2>], op);
    check(raze::algorithm::for_each_n[opt::fautovec], op_branching);

    auto check_aligned = [&]<size_t Align>(auto algo) {
        rtts::algorithm::run_aligned<Align, rtts::algorithm::n, T>(acfg,
            [algo, op, proj](auto c) { return algo(c.first(), c.count(), op, proj); },
            [op, proj](auto c) { return std_for_each_n(c.first(), c.count(), op, proj); });
    };
    check_aligned.template operator()<64>(raze::algorithm::for_each_n[opt::aligned<64>]);
    check_aligned.template operator()<1>(raze::algorithm::for_each_n[opt::aligned<opt::unaligned>]);

    constexpr auto check_ce = [](auto algo, auto fn) constexpr {
        return rtts::algorithm::constexpr_run<T, rtts::algorithm::n, 8, 5>(
            [algo, fn](auto c) constexpr { return algo(c.first(), c.count(), fn, [](T& x) constexpr -> T& { return x; }); },
            [fn](auto c) constexpr { return std_for_each_n(c.first(), c.count(), fn, [](T& x) constexpr -> T& { return x; }); });
    };

    static_assert(check_ce(raze::algorithm::for_each_n, [](T& x) constexpr { x += T(10); }));
    static_assert(check_ce(raze::algorithm::for_each_n[opt::fscalar], [](T& x) constexpr { x += T(10); }));
    static_assert(check_ce(raze::algorithm::for_each_n[opt::fautovec][opt::aligned<64>], [](auto& x) constexpr { if (x > T(2)) x += T(10); }));
};

RTTS_CASE("raze::algorithm::for_each_n views and edge cases") {
    namespace opt = raze::options;
    namespace views = std::views;

    {
        std::vector<int> v1(32);
        std::iota(v1.begin(), v1.end(), 1);
        auto v2 = v1;

        auto tv1 = v1 | views::drop(4) | views::take(20);
        auto tv2 = v2 | views::drop(4) | views::take(20);
        raze::algorithm::for_each_n[opt::fautovec][opt::fstatic](tv1.begin(), 15, [](auto& x) { if (x > 8) x *= 2; });
        std::ranges::for_each_n(tv2.begin(), 15, [](auto& x) { if (x > 8) x *= 2; });
        RTTS_EXPECT(v1 == v2);

        auto rv1 = v1 | views::reverse;
        auto rv2 = v2 | views::reverse;
        raze::algorithm::for_each_n(rv1.begin(), 10, [](int& x) { x += 3; });
        std::ranges::for_each_n(rv2.begin(), 10, [](int& x) { x += 3; });
        RTTS_EXPECT(v1 == v2);

        auto fv1 = v1 | views::filter([](int x) { return (x % 2) == 0; });
        auto fv2 = v2 | views::filter([](int x) { return (x % 2) == 0; });
        raze::algorithm::for_each_n[opt::aligned<64>](fv1.begin(), 5, [](int& x) { x -= 1; });
        std::ranges::for_each_n(fv2.begin(), 5, [](int& x) { x -= 1; });
        RTTS_EXPECT(v1 == v2);

        struct acc_fn {
            int sum = 0;
            void operator()(int x) { sum += x; }
        };
        auto iv = views::iota(1, 50);
        auto r_iota = raze::algorithm::for_each_n(iv.begin(), 10, acc_fn{});
        auto s_iota = std::ranges::for_each_n(iv.begin(), 10, acc_fn{});
        RTTS_EXPECT(r_iota.fun.sum == s_iota.fun.sum);

        auto trv = v1 | views::transform([](int x) { return x + 1; });
        auto r_tr = raze::algorithm::for_each_n[opt::fscalar](trv.begin(), 10, acc_fn{});
        auto s_tr = std::ranges::for_each_n(trv.begin(), 10, acc_fn{});
        RTTS_EXPECT(r_tr.fun.sum == s_tr.fun.sum);
    }

    {
        std::vector<int> v = { 10, 20, 30 };
        auto r0 = raze::algorithm::for_each_n(v.begin(), 0, [](auto& x) { x = 999; });
        RTTS_EXPECT(r0.in == v.begin() && v[0] == 10);

        auto r1 = raze::algorithm::for_each_n(v.begin(), 1, [](auto& x) { x += 5; });
        RTTS_EXPECT(r1.in == v.begin() + 1 && v[0] == 15 && v[1] == 20);

        const std::vector<int> data = { 2, 4, 6, 8, 10 };
        rtts::single_pass_input_range<int> spr(data);
        int sp_sum = 0;
        raze::algorithm::for_each_n(spr.begin(), 3, [&sp_sum](int x) { sp_sum += x; });
        RTTS_EXPECT(sp_sum == 12 && spr.reads() == 3);

        volatile int vbuf[16];
        for (int i = 0; i < 16; ++i) vbuf[i] = i;
        raze::algorithm::for_each_n(vbuf, 12, [](volatile int& x) { x = x + 10; });
        for (int i = 0; i < 12; ++i) RTTS_EXPECT(vbuf[i] == i + 10);
        for (int i = 12; i < 16; ++i) RTTS_EXPECT(vbuf[i] == i);

        std::vector<rtts::test_item> items = { { 1, 100 }, { 2, 200 }, { 3, 300 } };
        raze::algorithm::for_each_n(items.begin(), 2, [](int& k) { k += 7; }, &rtts::test_item::key);
        RTTS_EXPECT(items[0].key == 8 && items[1].key == 9 && items[2].key == 3);
    }
};
