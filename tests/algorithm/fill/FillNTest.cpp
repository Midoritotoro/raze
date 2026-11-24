#include <tests/rts/rts.h>
#include <raze/algorithm/fill/FillN.h>

template <class It, class Size, class T>
constexpr auto std_fill_n(It first, Size count, const T& value) {
    return std::ranges::fill_n(std::move(first), static_cast<std::iter_difference_t<It>>(count), value);
}

RTTS_CASE_TPL("raze::algorithm::fill_n", rtts::algorithm::all_types)
<class T> (rtts::type<T>) {
    auto cfg = rtts::algorithm::config::thorough();

    const T fill_val = rtts::random::generator<T>(42)();

    {
        rtts::algorithm::run<rtts::algorithm::n, T>(cfg,
            [fill_val](auto c) { return raze::algorithm::fill_n(c.first(), c.count(), fill_val); },
            [fill_val](auto c) { return std_fill_n(c.first(), c.count(), fill_val); });
    }

    {
        rtts::algorithm::run<rtts::algorithm::n, T>(cfg,
            [fill_val](auto c) { return raze::algorithm::fill_n[raze::options::fscalar](c.first(), c.count(), fill_val); },
            [fill_val](auto c) { return std_fill_n(c.first(), c.count(), fill_val); });

        rtts::algorithm::run<rtts::algorithm::n, T>(cfg,
            [fill_val](auto c) { return raze::algorithm::fill_n[raze::options::unroll<1>](c.first(), c.count(), fill_val); },
            [fill_val](auto c) { return std_fill_n(c.first(), c.count(), fill_val); });

        rtts::algorithm::run<rtts::algorithm::n, T>(cfg,
            [fill_val](auto c) { return raze::algorithm::fill_n[raze::options::fstatic](c.first(), c.count(), fill_val); },
            [fill_val](auto c) { return std_fill_n(c.first(), c.count(), fill_val); });

        rtts::algorithm::run<rtts::algorithm::n, T>(cfg,
            [fill_val](auto c) { return raze::algorithm::fill_n[raze::options::fscalar][raze::options::unroll<2>](c.first(), c.count(), fill_val); },
            [fill_val](auto c) { return std_fill_n(c.first(), c.count(), fill_val); });

        rtts::algorithm::run<rtts::algorithm::n, T>(cfg,
            [fill_val](auto c) { return raze::algorithm::fill_n[raze::options::fstatic][raze::options::unroll<2>](c.first(), c.count(), fill_val); },
            [fill_val](auto c) { return std_fill_n(c.first(), c.count(), fill_val); });

        rtts::algorithm::run<rtts::algorithm::n, T>(cfg,
            [fill_val](auto c) { return raze::algorithm::fill_n[raze::options::fautovec](c.first(), c.count(), fill_val); },
            [fill_val](auto c) { return std_fill_n(c.first(), c.count(), fill_val); });

        rtts::algorithm::run<rtts::algorithm::n, T>(cfg,
            [fill_val](auto c) { return raze::algorithm::fill_n[raze::options::fautovec][raze::options::fstatic](c.first(), c.count(), fill_val); },
            [fill_val](auto c) { return std_fill_n(c.first(), c.count(), fill_val); });
    }

    {
        namespace opt = raze::options;
        auto acfg = rtts::algorithm::config::compact();

        auto check_aligned = [&]<size_t Align>(auto algo) {
            rtts::algorithm::run_aligned<Align, rtts::algorithm::n, T>(acfg,
                [algo, fill_val](auto c) { return algo(c.first(), c.count(), fill_val); },
                [fill_val](auto c) { return std_fill_n(c.first(), c.count(), fill_val); });
        };

        check_aligned.template operator()<16>(raze::algorithm::fill_n[opt::aligned<16>]);
        check_aligned.template operator()<32>(raze::algorithm::fill_n[opt::aligned<32>]);
        check_aligned.template operator()<32>(raze::algorithm::fill_n[opt::aligned<32>][opt::unroll<2>]);
        check_aligned.template operator()<32>(raze::algorithm::fill_n[opt::aligned<32>][opt::fstatic]);
        check_aligned.template operator()<64>(raze::algorithm::fill_n[opt::aligned<64>]);
        check_aligned.template operator()<64>(raze::algorithm::fill_n[opt::aligned<64>][opt::fautovec]);
        check_aligned.template operator()<64>(raze::algorithm::fill_n[opt::aligned<64>][opt::fscalar]);
        check_aligned.template operator()<1>(raze::algorithm::fill_n[opt::aligned<opt::unaligned>]);
        check_aligned.template operator()<1>(raze::algorithm::fill_n[opt::aligned<64>][opt::aligned<opt::unaligned>]);
    }

    static_assert(rtts::algorithm::constexpr_run<T, rtts::algorithm::n, 8, 5>(
        [](auto c) constexpr { return raze::algorithm::fill_n(c.first(), c.count(), T(77)); },
        [](auto c) constexpr { return std_fill_n(c.first(), c.count(), T(77)); }));

    static_assert(rtts::algorithm::constexpr_run<T, rtts::algorithm::n, 8, 0>(
        [](auto c) constexpr { return raze::algorithm::fill_n(c.first(), c.count(), T(77)); },
        [](auto c) constexpr { return std_fill_n(c.first(), c.count(), T(77)); }));

    static_assert(rtts::algorithm::constexpr_run<T, rtts::algorithm::n, 8, 5>(
        [](auto c) constexpr { return raze::algorithm::fill_n[raze::options::fscalar](c.first(), c.count(), T(77)); },
        [](auto c) constexpr { return std_fill_n(c.first(), c.count(), T(77)); }));

    static_assert(rtts::algorithm::constexpr_run<T, rtts::algorithm::n, 8, 5>(
        [](auto c) constexpr { return raze::algorithm::fill_n[raze::options::unroll<2>](c.first(), c.count(), T(77)); },
        [](auto c) constexpr { return std_fill_n(c.first(), c.count(), T(77)); }));

    static_assert(rtts::algorithm::constexpr_run<T, rtts::algorithm::n, 8, 5>(
        [](auto c) constexpr { return raze::algorithm::fill_n[raze::options::fautovec](c.first(), c.count(), T(77)); },
        [](auto c) constexpr { return std_fill_n(c.first(), c.count(), T(77)); }));

    static_assert(rtts::algorithm::constexpr_run<T, rtts::algorithm::n, 8, 5>(
        [](auto c) constexpr { return raze::algorithm::fill_n[raze::options::aligned<64>](c.first(), c.count(), T(77)); },
        [](auto c) constexpr { return std_fill_n(c.first(), c.count(), T(77)); }));
};


RTTS_CASE("raze::algorithm::fill_n views and edge cases") {
    namespace opt = raze::options;
    namespace views = std::views;

    // Iterators from views: drop, reverse, filter
    {
        std::vector<int> v1(32, 0), v2(32, 0);

        auto dv1 = v1 | views::drop(4);
        auto dv2 = v2 | views::drop(4);
        raze::algorithm::fill_n[opt::fautovec][opt::fstatic](dv1.begin(), 16, 11);
        std::ranges::fill_n(dv2.begin(), 16, 11);
        RTTS_EXPECT(v1 == v2);

        auto rv1 = v1 | views::reverse;
        auto rv2 = v2 | views::reverse;
        raze::algorithm::fill_n(rv1.begin(), 6, 88);
        std::ranges::fill_n(rv2.begin(), 6, 88);
        RTTS_EXPECT(v1 == v2);

        auto fv1 = v1 | views::filter([](int x) { return x == 11; });
        auto fv2 = v2 | views::filter([](int x) { return x == 11; });
        raze::algorithm::fill_n[opt::aligned<64>](fv1.begin(), 5, 22);
        std::ranges::fill_n(fv2.begin(), 5, 22);
        RTTS_EXPECT(v1 == v2);
    }

    // Output iterators: back_inserter, front_inserter, inserter (n <= 0, n == 1, n == 17)
    {
        std::vector<int> b0, b1, b2, b3, b4, b5;
        raze::algorithm::fill_n(std::back_inserter(b0), 0, 42);
        raze::algorithm::fill_n(std::back_inserter(b0), -5, 42);
        RTTS_EXPECT(b0.empty());

        raze::algorithm::fill_n(std::back_inserter(b1), 1, 9);
        RTTS_EXPECT((b1 == std::vector<int>{ 9 }));

        raze::algorithm::fill_n(std::back_inserter(b2), 17, 7);
        raze::algorithm::fill_n[opt::fscalar](std::back_inserter(b3), 17, 7);
        raze::algorithm::fill_n[opt::fautovec][opt::fstatic](std::back_inserter(b4), 17, 7);
        raze::algorithm::fill_n[opt::aligned<64>](std::back_inserter(b5), 17, 7);
        const std::vector<int> expected(17, 7);
        RTTS_EXPECT(b2 == expected && b3 == expected && b4 == expected && b5 == expected);

        std::deque<int> deq;
        raze::algorithm::fill_n(std::front_inserter(deq), 4, 55);
        RTTS_EXPECT(deq == std::deque<int>(4, 55));

        std::vector<int> ins = { 1, 9 };
        raze::algorithm::fill_n(std::inserter(ins, ins.begin() + 1), 3, 5);
        RTTS_EXPECT((ins == std::vector<int>{ 1, 5, 5, 5, 9 }));
    }

    // Volatile buffers and non-pow2 large struct
    {
        volatile int vbuf[16];
        raze::algorithm::fill_n(vbuf, 16, 456);
        for (int i = 0; i < 16; ++i) RTTS_EXPECT(vbuf[i] == 456);

        volatile char vbuf8[32];
        raze::algorithm::fill_n(vbuf8, 32, char(90));
        for (int i = 0; i < 32; ++i) RTTS_EXPECT(vbuf8[i] == char(90));

        struct big_item {
            int data[5]{};
            bool operator==(const big_item&) const = default;
        };
        std::vector<big_item> bigs(6);
        big_item target{ { 9, 8, 7, 6, 5 } };
        raze::algorithm::fill_n(bigs.begin(), 4, target);
        for (size_t i = 0; i < 4; ++i) RTTS_EXPECT(bigs[i] == target);
        RTTS_EXPECT(bigs[4] == big_item{});
    }
};