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
    auto cfg = rtts::algorithm::config::thorough();

    const T add_val = rtts::random::generator<T>(42)();
    auto op_scalar = [add_val](T& x) { x += add_val; };
    auto op_vector = [add_val](auto& x) { x += add_val; };
    auto op_branching = [add_val](auto& x) { if (x > add_val) x += add_val; };

    {
        rtts::algorithm::run<rtts::algorithm::n, T>(cfg,
            [op_scalar](auto c) { return raze::algorithm::for_each_n(c.first(), c.count(), op_scalar); },
            [op_scalar](auto c) { return std_for_each_n(c.first(), c.count(), op_scalar); });

        rtts::algorithm::run<rtts::algorithm::n, T>(cfg,
            [op_vector](auto c) { return raze::algorithm::for_each_n(c.first(), c.count(), op_vector); },
            [op_vector](auto c) { return std_for_each_n(c.first(), c.count(), op_vector); });
    }

    {
        rtts::algorithm::run<rtts::algorithm::n, T>(cfg,
            [op_vector](auto c) { return raze::algorithm::for_each_n[raze::options::fscalar](c.first(), c.count(), op_vector); },
            [op_vector](auto c) { return std_for_each_n(c.first(), c.count(), op_vector); });

        rtts::algorithm::run<rtts::algorithm::n, T>(cfg,
            [op_vector](auto c) { return raze::algorithm::for_each_n[raze::options::unroll<1>](c.first(), c.count(), op_vector); },
            [op_vector](auto c) { return std_for_each_n(c.first(), c.count(), op_vector); });

        rtts::algorithm::run<rtts::algorithm::n, T>(cfg,
            [op_vector](auto c) { return raze::algorithm::for_each_n[raze::options::fstatic](c.first(), c.count(), op_vector); },
            [op_vector](auto c) { return std_for_each_n(c.first(), c.count(), op_vector); });

        rtts::algorithm::run<rtts::algorithm::n, T>(cfg,
            [op_vector](auto c) { return raze::algorithm::for_each_n[raze::options::fscalar][raze::options::unroll<2>](c.first(), c.count(), op_vector); },
            [op_vector](auto c) { return std_for_each_n(c.first(), c.count(), op_vector); });

        rtts::algorithm::run<rtts::algorithm::n, T>(cfg,
            [op_vector](auto c) { return raze::algorithm::for_each_n[raze::options::fstatic][raze::options::unroll<2>](c.first(), c.count(), op_vector); },
            [op_vector](auto c) { return std_for_each_n(c.first(), c.count(), op_vector); });

        rtts::algorithm::run<rtts::algorithm::n, T>(cfg,
            [op_vector](auto c) { return raze::algorithm::for_each_n[raze::options::fautovec](c.first(), c.count(), op_vector); },
            [op_vector](auto c) { return std_for_each_n(c.first(), c.count(), op_vector); });

        rtts::algorithm::run<rtts::algorithm::n, T>(cfg,
            [op_branching](auto c) { return raze::algorithm::for_each_n[raze::options::fautovec](c.first(), c.count(), op_branching); },
            [op_branching](auto c) { return std_for_each_n(c.first(), c.count(), op_branching); });

        rtts::algorithm::run<rtts::algorithm::n, T>(cfg,
            [op_branching](auto c) { return raze::algorithm::for_each_n[raze::options::fautovec][raze::options::fstatic](c.first(), c.count(), op_branching); },
            [op_branching](auto c) { return std_for_each_n(c.first(), c.count(), op_branching); });
    }

    {
        auto acfg = rtts::algorithm::config::compact();

        rtts::algorithm::run_aligned<16, rtts::algorithm::n, T>(acfg,
            [op_vector](auto c) { return raze::algorithm::for_each_n[raze::options::aligned<16>](c.first(), c.count(), op_vector); },
            [op_vector](auto c) { return std_for_each_n(c.first(), c.count(), op_vector); });

        rtts::algorithm::run_aligned<32, rtts::algorithm::n, T>(acfg,
            [op_vector](auto c) { return raze::algorithm::for_each_n[raze::options::aligned<32>](c.first(), c.count(), op_vector); },
            [op_vector](auto c) { return std_for_each_n(c.first(), c.count(), op_vector); });

        rtts::algorithm::run_aligned<32, rtts::algorithm::n, T>(acfg,
            [op_vector](auto c) { return raze::algorithm::for_each_n[raze::options::aligned<32>][raze::options::unroll<2>](c.first(), c.count(), op_vector); },
            [op_vector](auto c) { return std_for_each_n(c.first(), c.count(), op_vector); });

        rtts::algorithm::run_aligned<64, rtts::algorithm::n, T>(acfg,
            [op_vector](auto c) { return raze::algorithm::for_each_n[raze::options::aligned<64>](c.first(), c.count(), op_vector); },
            [op_vector](auto c) { return std_for_each_n(c.first(), c.count(), op_vector); });

        rtts::algorithm::run_aligned<64, rtts::algorithm::n, T>(acfg,
            [op_branching](auto c) { return raze::algorithm::for_each_n[raze::options::aligned<64>][raze::options::fautovec](c.first(), c.count(), op_branching); },
            [op_branching](auto c) { return std_for_each_n(c.first(), c.count(), op_branching); });

        rtts::algorithm::run_aligned<64, rtts::algorithm::n, T>(acfg,
            [op_vector](auto c) { return raze::algorithm::for_each_n[raze::options::aligned<64>][raze::options::fscalar](c.first(), c.count(), op_vector); },
            [op_vector](auto c) { return std_for_each_n(c.first(), c.count(), op_vector); });

        rtts::algorithm::run_aligned<1, rtts::algorithm::n, T>(acfg,
            [op_vector](auto c) { return raze::algorithm::for_each_n[raze::options::aligned<raze::options::unaligned>](c.first(), c.count(), op_vector); },
            [op_vector](auto c) { return std_for_each_n(c.first(), c.count(), op_vector); });

        rtts::algorithm::run_aligned<1, rtts::algorithm::n, T>(acfg,
            [op_vector](auto c) { return raze::algorithm::for_each_n[raze::options::aligned<64>][raze::options::aligned<raze::options::unaligned>](c.first(), c.count(), op_vector); },
            [op_vector](auto c) { return std_for_each_n(c.first(), c.count(), op_vector); });
    }

    static_assert(rtts::algorithm::constexpr_run<T, rtts::algorithm::n, 8, 5>(
        [](auto c) constexpr { return raze::algorithm::for_each_n(c.first(), c.count(), [](T& x) constexpr { x += T(1); }); },
        [](auto c) constexpr { return std_for_each_n(c.first(), c.count(), [](T& x) constexpr { x += T(1); }); }));

    static_assert(rtts::algorithm::constexpr_run<T, rtts::algorithm::n, 8, 5>(
        [](auto c) constexpr { return raze::algorithm::for_each_n[raze::options::fscalar](c.first(), c.count(), [](auto& x) constexpr { x += T(1); }); },
        [](auto c) constexpr { return std_for_each_n(c.first(), c.count(), [](auto& x) constexpr { x += T(1); }); }));

    static_assert(rtts::algorithm::constexpr_run<T, rtts::algorithm::n, 8, 5>(
        [](auto c) constexpr { return raze::algorithm::for_each_n[raze::options::unroll<2>](c.first(), c.count(), [](auto& x) constexpr { x += T(1); }); },
        [](auto c) constexpr { return std_for_each_n(c.first(), c.count(), [](auto& x) constexpr { x += T(1); }); }));

    static_assert(rtts::algorithm::constexpr_run<T, rtts::algorithm::n, 8, 5>(
        [](auto c) constexpr { return raze::algorithm::for_each_n[raze::options::fautovec](c.first(), c.count(), [](auto& x) constexpr { if (x > T(2)) x += T(1); }); },
        [](auto c) constexpr { return std_for_each_n(c.first(), c.count(), [](auto& x) constexpr { if (x > T(2)) x += T(1); }); }));

    static_assert(rtts::algorithm::constexpr_run<T, rtts::algorithm::n, 8, 5>(
        [](auto c) constexpr { return raze::algorithm::for_each_n[raze::options::aligned<64>](c.first(), c.count(), [](auto& x) constexpr { x += T(1); }); },
        [](auto c) constexpr { return std_for_each_n(c.first(), c.count(), [](auto& x) constexpr { x += T(1); }); }));
};

RTTS_CASE_TPL("raze::algorithm::for_each_n.projection", rtts::algorithm::all_types)
<class T> (rtts::type<T>) {
    auto cfg = rtts::algorithm::config::thorough();
    auto acfg = rtts::algorithm::config::compact();

    const T add_val = rtts::random::generator<T>(100000)();
    auto op = [add_val](auto& x) { x += add_val; };
    auto op_branching = [add_val](auto& x) { if (x > add_val) x += add_val; };
    auto proj = [](auto& x) -> auto& { return x; };

    rtts::algorithm::run<rtts::algorithm::n, T>(cfg,
        [op, proj](auto c) { return raze::algorithm::for_each_n(c.first(), c.count(), op, proj); },
        [op, proj](auto c) { return std_for_each_n(c.first(), c.count(), op, proj); });

    rtts::algorithm::run<rtts::algorithm::n, T>(cfg,
        [op, proj](auto c) { return raze::algorithm::for_each_n[raze::options::fscalar](c.first(), c.count(), op, proj); },
        [op, proj](auto c) { return std_for_each_n(c.first(), c.count(), op, proj); });

    rtts::algorithm::run<rtts::algorithm::n, T>(cfg,
        [op, proj](auto c) { return raze::algorithm::for_each_n[raze::options::unroll<2>](c.first(), c.count(), op, proj); },
        [op, proj](auto c) { return std_for_each_n(c.first(), c.count(), op, proj); });

    rtts::algorithm::run<rtts::algorithm::n, T>(cfg,
        [op_branching, proj](auto c) { return raze::algorithm::for_each_n[raze::options::fautovec](c.first(), c.count(), op_branching, proj); },
        [op_branching, proj](auto c) { return std_for_each_n(c.first(), c.count(), op_branching, proj); });

    rtts::algorithm::run_aligned<64, rtts::algorithm::n, T>(acfg,
        [op, proj](auto c) { return raze::algorithm::for_each_n[raze::options::aligned<64>](c.first(), c.count(), op, proj); },
        [op, proj](auto c) { return std_for_each_n(c.first(), c.count(), op, proj); });

    rtts::algorithm::run_aligned<1, rtts::algorithm::n, T>(acfg,
        [op, proj](auto c) { return raze::algorithm::for_each_n[raze::options::aligned<raze::options::unaligned>](c.first(), c.count(), op, proj); },
        [op, proj](auto c) { return std_for_each_n(c.first(), c.count(), op, proj); });

    static_assert(rtts::algorithm::constexpr_run<T, rtts::algorithm::n, 8, 5>(
        [](auto c) constexpr { return raze::algorithm::for_each_n(c.first(), c.count(), 
            [](T& x) constexpr { x += T(10); }, [](T& x) constexpr -> T& { return x; }); },
        [](auto c) constexpr { return std_for_each_n(c.first(), c.count(),
            [](T& x) constexpr { x += T(10); }, [](T& x) constexpr -> T& { return x; }); }));

    static_assert(rtts::algorithm::constexpr_run<T, rtts::algorithm::n, 8, 5>(
        [](auto c) constexpr { return raze::algorithm::for_each_n[raze::options::fscalar](c.first(), c.count(), 
            [](T& x) constexpr { x += T(10); }, [](T& x) constexpr -> T& { return x; }); },
        [](auto c) constexpr { return std_for_each_n(c.first(), c.count(),
            [](T& x) constexpr { x += T(10); }, [](T& x) constexpr -> T& { return x; }); }));

    static_assert(rtts::algorithm::constexpr_run<T, rtts::algorithm::n, 8, 5>(
        [](auto c) constexpr { return raze::algorithm::for_each_n[raze::options::fautovec][raze::options::aligned<64>](c.first(), c.count(), 
            [](auto& x) constexpr { if (x > T(2)) x += T(10); }, [](T& x) constexpr -> T& { return x; }); },
        [](auto c) constexpr { return std_for_each_n(c.first(), c.count(),
            [](auto& x) constexpr { if (x > T(2)) x += T(10); }, [](T& x) constexpr -> T& { return x; }); }));
};


RTTS_CASE("raze::algorithm::for_each_n views and edge cases") {
    namespace opt = raze::options;
    namespace views = std::views;

    // Iterators from views: take, drop, reverse, filter, transform, iota
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

    // n == 0, n == 1, single-pass input iterator, volatile, const, member projection
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