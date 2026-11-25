#include <tests/rts/rts.h>
#include <raze/algorithm/batch/ForEach.h>

RTTS_CASE_TPL("raze::algorithm::for_each", rtts::algorithm::all_types)
<class T> (rtts::type<T>) {
    auto cfg = rtts::algorithm::config::thorough();

    const T add_val = rtts::random::generator<T>(42)();
    auto op_scalar = [add_val](T& x) { x += add_val; };
    auto op_vector = [add_val](auto& x) { x += add_val; };
    auto op_branching = [add_val](auto& x) { if (x > add_val) x += add_val; };

    {
        rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
            [op_scalar](auto c) { return raze::algorithm::for_each(c.first(), c.last(), op_scalar); },
            [op_scalar](auto c) { return std::ranges::for_each(c.first(), c.last(), op_scalar); });

        rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
            [op_vector](auto c) { return raze::algorithm::for_each(c.first(), c.last(), op_vector); },
            [op_vector](auto c) { return std::ranges::for_each(c.first(), c.last(), op_vector); });

        rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
            [op_scalar](auto c) { return raze::algorithm::for_each(c.range(), op_scalar); },
            [op_scalar](auto c) { return std::ranges::for_each(c.range(), op_scalar); });

        rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
            [op_vector](auto c) { return raze::algorithm::for_each(c.range(), op_vector); },
            [op_vector](auto c) { return std::ranges::for_each(c.range(), op_vector); });
    }

    {
        rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
            [op_vector](auto c) { return raze::algorithm::for_each[raze::options::fscalar](c.first(), c.last(), op_vector); },
            [op_vector](auto c) { return std::ranges::for_each(c.first(), c.last(), op_vector); });

        rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
            [op_vector](auto c) { return raze::algorithm::for_each[raze::options::fscalar](c.range(), op_vector); },
            [op_vector](auto c) { return std::ranges::for_each(c.range(), op_vector); });

        rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
            [op_vector](auto c) { return raze::algorithm::for_each[raze::options::unroll<1>](c.range(), op_vector); },
            [op_vector](auto c) { return std::ranges::for_each(c.range(), op_vector); });

        rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
            [op_vector](auto c) { return raze::algorithm::for_each[raze::options::fstatic](c.range(), op_vector); },
            [op_vector](auto c) { return std::ranges::for_each(c.range(), op_vector); });

        rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
            [op_vector](auto c) { return raze::algorithm::for_each[raze::options::fscalar][raze::options::unroll<2>](c.range(), op_vector); },
            [op_vector](auto c) { return std::ranges::for_each(c.range(), op_vector); });

        rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
            [op_vector](auto c) { return raze::algorithm::for_each[raze::options::fstatic][raze::options::unroll<2>](c.range(), op_vector); },
            [op_vector](auto c) { return std::ranges::for_each(c.range(), op_vector); });

        rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
            [op_vector](auto c) { return raze::algorithm::for_each[raze::options::fautovec](c.first(), c.last(), op_vector); },
            [op_vector](auto c) { return std::ranges::for_each(c.first(), c.last(), op_vector); });

        rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
            [op_branching](auto c) { return raze::algorithm::for_each[raze::options::fautovec](c.range(), op_branching); },
            [op_branching](auto c) { return std::ranges::for_each(c.range(), op_branching); });

        rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
            [op_branching](auto c) { return raze::algorithm::for_each[raze::options::fautovec][raze::options::fstatic](c.range(), op_branching); },
            [op_branching](auto c) { return std::ranges::for_each(c.range(), op_branching); });
    }

    {
        auto acfg = rtts::algorithm::config::compact();

        rtts::algorithm::run_aligned<16, rtts::algorithm::range, T>(acfg,
            [op_vector](auto c) { return raze::algorithm::for_each[raze::options::aligned<16>](c.first(), c.last(), op_vector); },
            [op_vector](auto c) { return std::ranges::for_each(c.first(), c.last(), op_vector); });

        rtts::algorithm::run_aligned<32, rtts::algorithm::range, T>(acfg,
            [op_vector](auto c) { return raze::algorithm::for_each[raze::options::aligned<32>](c.range(), op_vector); },
            [op_vector](auto c) { return std::ranges::for_each(c.range(), op_vector); });

        rtts::algorithm::run_aligned<32, rtts::algorithm::range, T>(acfg,
            [op_vector](auto c) { return raze::algorithm::for_each[raze::options::aligned<32>][raze::options::unroll<2>](c.range(), op_vector); },
            [op_vector](auto c) { return std::ranges::for_each(c.range(), op_vector); });

        rtts::algorithm::run_aligned<32, rtts::algorithm::range, T>(acfg,
            [op_vector](auto c) { return raze::algorithm::for_each[raze::options::aligned<32>][raze::options::fstatic](c.range(), op_vector); },
            [op_vector](auto c) { return std::ranges::for_each(c.range(), op_vector); });

        rtts::algorithm::run_aligned<64, rtts::algorithm::range, T>(acfg,
            [op_vector](auto c) { return raze::algorithm::for_each[raze::options::aligned<64>](c.first(), c.last(), op_vector); },
            [op_vector](auto c) { return std::ranges::for_each(c.first(), c.last(), op_vector); });

        rtts::algorithm::run_aligned<64, rtts::algorithm::range, T>(acfg,
            [op_scalar](auto c) { return raze::algorithm::for_each[raze::options::aligned<64>](c.range(), op_scalar); },
            [op_scalar](auto c) { return std::ranges::for_each(c.range(), op_scalar); });

        rtts::algorithm::run_aligned<64, rtts::algorithm::range, T>(acfg,
            [op_branching](auto c) { return raze::algorithm::for_each[raze::options::aligned<64>][raze::options::fautovec](c.range(), op_branching); },
            [op_branching](auto c) { return std::ranges::for_each(c.range(), op_branching); });

        rtts::algorithm::run_aligned<64, rtts::algorithm::range, T>(acfg,
            [op_vector](auto c) { return raze::algorithm::for_each[raze::options::aligned<64>][raze::options::fscalar](c.range(), op_vector); },
            [op_vector](auto c) { return std::ranges::for_each(c.range(), op_vector); });

        rtts::algorithm::run_aligned<1, rtts::algorithm::range, T>(acfg,
            [op_vector](auto c) { return raze::algorithm::for_each[raze::options::aligned<raze::options::unaligned>](c.range(), op_vector); },
            [op_vector](auto c) { return std::ranges::for_each(c.range(), op_vector); });

        rtts::algorithm::run_aligned<1, rtts::algorithm::range, T>(acfg,
            [op_vector](auto c) { return raze::algorithm::for_each[raze::options::aligned<64>][raze::options::aligned<raze::options::unaligned>](c.range(), op_vector); },
            [op_vector](auto c) { return std::ranges::for_each(c.range(), op_vector); });
    }

    static_assert(rtts::algorithm::constexpr_run<T, rtts::algorithm::range, 8>(
        [](auto c) constexpr { return raze::algorithm::for_each(c.first(), c.last(), [](T& x) constexpr { x += T(1); }); },
        [](auto c) constexpr { return std::ranges::for_each(c.first(), c.last(), [](T& x) constexpr { x += T(1); }); }));

    static_assert(rtts::algorithm::constexpr_run<T, rtts::algorithm::range, 8>(
        [](auto c) constexpr { return raze::algorithm::for_each(c.range(), [](auto& x) constexpr { x += T(1); }); },
        [](auto c) constexpr { return std::ranges::for_each(c.range(), [](auto& x) constexpr { x += T(1); }); }));

    static_assert(rtts::algorithm::constexpr_run<T, rtts::algorithm::range, 8>(
        [](auto c) constexpr { return raze::algorithm::for_each[raze::options::fscalar](c.range(), [](auto& x) constexpr { x += T(1); }); },
        [](auto c) constexpr { return std::ranges::for_each(c.range(), [](auto& x) constexpr { x += T(1); }); }));

    static_assert(rtts::algorithm::constexpr_run<T, rtts::algorithm::range, 8>(
        [](auto c) constexpr { return raze::algorithm::for_each[raze::options::unroll<2>](c.range(), [](auto& x) constexpr { x += T(1); }); },
        [](auto c) constexpr { return std::ranges::for_each(c.range(), [](auto& x) constexpr { x += T(1); }); }));

    static_assert(rtts::algorithm::constexpr_run<T, rtts::algorithm::range, 8>(
        [](auto c) constexpr { return raze::algorithm::for_each[raze::options::fautovec](c.range(), [](auto& x) constexpr { if (x > T(2)) x += T(1); }); },
        [](auto c) constexpr { return std::ranges::for_each(c.range(), [](auto& x) constexpr { if (x > T(2)) x += T(1); }); }));

    static_assert(rtts::algorithm::constexpr_run<T, rtts::algorithm::range, 8>(
        [](auto c) constexpr { return raze::algorithm::for_each[raze::options::aligned<64>](c.range(), [](auto& x) constexpr { x += T(1); }); },
        [](auto c) constexpr { return std::ranges::for_each(c.range(), [](auto& x) constexpr { x += T(1); }); }));

    static_assert(rtts::algorithm::constexpr_run<T, rtts::algorithm::range, 8>(
        [](auto c) constexpr { return raze::algorithm::for_each[raze::options::aligned<raze::options::unaligned>][raze::options::fautovec](c.range(), [](auto& x) constexpr { if (x > T(2)) x += T(1); }); },
        [](auto c) constexpr { return std::ranges::for_each(c.range(), [](auto& x) constexpr { if (x > T(2)) x += T(1); }); }));
};

RTTS_CASE_TPL("raze::algorithm::for_each.projection", rtts::algorithm::all_types)
<class T> (rtts::type<T>) {
    auto cfg = rtts::algorithm::config::thorough();
    auto acfg = rtts::algorithm::config::compact();

    const T add_val = rtts::random::generator<T>(1000)();
    auto op = [add_val](auto& x) { x += add_val; };
    auto op_branching = [add_val](auto& x) { if (x > add_val) x += add_val; };
    auto proj = [](auto& x) -> auto& { return x; };

    rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
        [op, proj](auto c) { return raze::algorithm::for_each(c.first(), c.last(), op, proj); },
        [op, proj](auto c) { return std::ranges::for_each(c.first(), c.last(), op, proj); });

    rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
        [op, proj](auto c) { return raze::algorithm::for_each(c.range(), op, proj); },
        [op, proj](auto c) { return std::ranges::for_each(c.range(), op, proj); });

    rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
        [op, proj](auto c) { return raze::algorithm::for_each[raze::options::fscalar](c.range(), op, proj); },
        [op, proj](auto c) { return std::ranges::for_each(c.range(), op, proj); });

    rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
        [op_branching, proj](auto c) { return raze::algorithm::for_each[raze::options::fautovec](c.range(), op_branching, proj); },
        [op_branching, proj](auto c) { return std::ranges::for_each(c.range(), op_branching, proj); });

    rtts::algorithm::run_aligned<64, rtts::algorithm::range, T>(acfg,
        [op, proj](auto c) { return raze::algorithm::for_each[raze::options::aligned<64>](c.range(), op, proj); },
        [op, proj](auto c) { return std::ranges::for_each(c.range(), op, proj); });

    rtts::algorithm::run_aligned<1, rtts::algorithm::range, T>(acfg,
        [op, proj](auto c) { return raze::algorithm::for_each[raze::options::aligned<raze::options::unaligned>](c.range(), op, proj); },
        [op, proj](auto c) { return std::ranges::for_each(c.range(), op, proj); });

    static_assert(rtts::algorithm::constexpr_run<T, rtts::algorithm::range, 8>(
        [](auto c) constexpr { return raze::algorithm::for_each(c.first(), c.last(), [](T& x) constexpr { x += T(5); }, [](T& x) constexpr -> T& { return x; }); },
        [](auto c) constexpr { return std::ranges::for_each(c.first(), c.last(), [](T& x) constexpr { x += T(5); }, [](T& x) constexpr -> T& { return x; }); }));

    static_assert(rtts::algorithm::constexpr_run<T, rtts::algorithm::range, 8>(
        [](auto c) constexpr { return raze::algorithm::for_each[raze::options::fscalar](c.range(), [](T& x) constexpr { x += T(5); }, [](T& x) constexpr -> T& { return x; }); },
        [](auto c) constexpr { return std::ranges::for_each(c.range(), [](T& x) constexpr { x += T(5); }, [](T& x) constexpr -> T& { return x; }); }));

    static_assert(rtts::algorithm::constexpr_run<T, rtts::algorithm::range, 8>(
        [](auto c) constexpr { return raze::algorithm::for_each[raze::options::fautovec][raze::options::aligned<64>](c.range(), [](auto& x) constexpr { if (x > T(2)) x += T(5); }, [](T& x) constexpr -> T& { return x; }); },
        [](auto c) constexpr { return std::ranges::for_each(c.range(), [](auto& x) constexpr { if (x > T(2)) x += T(5); }, [](T& x) constexpr -> T& { return x; }); }));
};


RTTS_CASE("raze::algorithm::for_each views and edge cases") {
    namespace opt = raze::options;
    namespace views = std::views;

    {
        std::vector<int> v1(40);
        std::iota(v1.begin(), v1.end(), 1);
        auto v2 = v1;

        raze::algorithm::for_each(v1 | views::drop(5) | views::take(20), [](auto& x) { x *= 2; });
        std::ranges::for_each(v2 | views::drop(5) | views::take(20), [](auto& x) { x *= 2; });
        RTTS_EXPECT(v1 == v2);

        raze::algorithm::for_each[opt::fautovec][opt::fstatic](views::counted(v1.begin() + 3, 17), [](auto& x) {
            if (x > 10) x += 7;
        });
        std::ranges::for_each(views::counted(v2.begin() + 3, 17), [](auto& x) {
            if (x > 10) x += 7;
        });
        RTTS_EXPECT(v1 == v2);

        std::span<int> sp(v1.data(), v1.size());
        std::span<int> sp2(v2.data(), v2.size());
        raze::algorithm::for_each[opt::fstatic](sp, [](int& x) { x += 3; });
        std::ranges::for_each(sp2, [](int& x) { x += 3; });
        RTTS_EXPECT(v1 == v2);
    }

    {
        std::vector<int> v1(35);
        std::iota(v1.begin(), v1.end(), 0);
        auto v2 = v1;

        raze::algorithm::for_each(v1 | views::reverse, [](auto& x) { x += 5; });
        std::ranges::for_each(v2 | views::reverse, [](auto& x) { x += 5; });
        RTTS_EXPECT(v1 == v2);

        auto is_even = [](int x) { return (x % 2) == 0; };
        raze::algorithm::for_each[opt::aligned<64>](v1 | views::filter(is_even), [](int& x) { x *= 3; });
        std::ranges::for_each(v2 | views::filter(is_even), [](int& x) { x *= 3; });
        RTTS_EXPECT(v1 == v2);

        raze::algorithm::for_each[opt::fautovec][opt::fstatic](rtts::make_non_common(v1), [](auto& x) {
            if (x > 20) x -= 1;
        });
        std::ranges::for_each(rtts::make_non_common(v2), [](auto& x) {
            if (x > 20) x -= 1;
        });
        RTTS_EXPECT(v1 == v2);
    }

    {
        struct light_acc {
            int sum = 0;
            constexpr void operator()(int x) { sum += x; }
        };
        struct heavy_acc {
            int sum = 0;
            int pad[8]{};
            heavy_acc() = default;
            heavy_acc(const heavy_acc& o) : sum(o.sum) {}
            heavy_acc& operator=(const heavy_acc& o) { sum = o.sum; return *this; }
            ~heavy_acc() {}
            void operator()(int x) { sum += x; }
        };

        auto r_iota = raze::algorithm::for_each(views::iota(1, 21), light_acc{});
        auto s_iota = std::ranges::for_each(views::iota(1, 21), light_acc{});
        RTTS_EXPECT(r_iota.fun.sum == s_iota.fun.sum);

        const std::vector<int> src = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
        auto r_chain = raze::algorithm::for_each[opt::fscalar](
            src | views::filter([](int x) { return (x & 1) == 0; }) | views::transform([](int x) { return x * 10; }) | views::take(4),
            heavy_acc{});
        auto s_chain = std::ranges::for_each(
            src | views::filter([](int x) { return (x & 1) == 0; }) | views::transform([](int x) { return x * 10; }) | views::take(4),
            heavy_acc{});
        RTTS_EXPECT(r_chain.fun.sum == s_chain.fun.sum);

        auto r_const = raze::algorithm::for_each[opt::fautovec][opt::fstatic](src, light_acc{});
        auto s_const = std::ranges::for_each(src, light_acc{});
        RTTS_EXPECT(r_const.fun.sum == s_const.fun.sum);
    }

    {
        const std::vector<int> data = { 3, 1, 4, 1, 5, 9, 2, 6 };
        rtts::single_pass_input_range<int> spr(data);
        int sum = 0;
        raze::algorithm::for_each(spr, [&sum](int x) { sum += x; });
        RTTS_EXPECT(sum == 31);
        RTTS_EXPECT(spr.reads() == data.size());
    }

    {
        volatile int vbuf[16];
        for (int i = 0; i < 16; ++i) vbuf[i] = i + 1;
        raze::algorithm::for_each(vbuf, vbuf + 16, [](volatile int& x) { x = x * 2; });
        for (int i = 0; i < 16; ++i) RTTS_EXPECT(vbuf[i] == (i + 1) * 2);
    }

    {
        std::vector<int> empty_v;
        auto er = raze::algorithm::for_each(empty_v, [](auto& x) { x += 1; });
        RTTS_EXPECT(er.in == empty_v.end());

        std::vector<int> one_v = { 41 };
        auto or1 = raze::algorithm::for_each(one_v, [](auto& x) { x += 1; });
        RTTS_EXPECT(or1.in == one_v.end() && one_v[0] == 42);

        struct sum_fn {
            int total = 0;
            void operator()(int x) { total += x; }
        };
        auto rv = raze::algorithm::for_each(std::vector<int>{ 10, 20, 30 }, sum_fn{});
        static_assert(std::same_as<decltype(rv.in), std::ranges::dangling>);
        RTTS_EXPECT(rv.fun.total == 60);

        std::vector<rtts::test_item> items = { { 1, 10 }, { 2, 20 }, { 3, 30 } };
        raze::algorithm::for_each(items, [](int& k) { k *= 5; }, &rtts::test_item::key);
        RTTS_EXPECT(items[0].key == 5 && items[1].key == 10 && items[2].key == 15);
        RTTS_EXPECT(items[0].payload == 10 && items[2].payload == 30);
    }

    static_assert([]() constexpr {
        std::array<int, 6> a = { 1, 2, 3, 4, 5, 6 };
        raze::algorithm::for_each[opt::fautovec][opt::fstatic](
            a | views::drop(1) | views::take(4), [](auto& x) { x += 10; });
        int acc = 0;
        raze::algorithm::for_each(views::iota(1, 5), [&acc](int x) { acc += x; });
        return a[0] == 1 && a[1] == 12 && a[4] == 15 && a[5] == 6 && acc == 10;
    }());
};
