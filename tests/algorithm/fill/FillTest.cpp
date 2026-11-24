#include <tests/rts/rts.h>
#include <raze/algorithm/fill/Fill.h>

RTTS_CASE_TPL("raze::algorithm::fill", rtts::algorithm::all_types)
<class T> (rtts::type<T>) {
    auto cfg = rtts::algorithm::config::thorough();

    const T fill_val = rtts::random::generator<T>(42)();

    {
        rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
            [fill_val](auto c) { return raze::algorithm::fill(c.first(), c.last(), fill_val); },
            [fill_val](auto c) { return std::ranges::fill(c.first(), c.last(), fill_val); });

        rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
            [fill_val](auto c) { return raze::algorithm::fill(c.range(), fill_val); },
            [fill_val](auto c) { return std::ranges::fill(c.range(), fill_val); });
    }

    {
        rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
            [fill_val](auto c) { return raze::algorithm::fill[raze::options::fscalar](c.first(), c.last(), fill_val); },
            [fill_val](auto c) { return std::ranges::fill(c.first(), c.last(), fill_val); });

        rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
            [fill_val](auto c) { return raze::algorithm::fill[raze::options::fscalar](c.range(), fill_val); },
            [fill_val](auto c) { return std::ranges::fill(c.range(), fill_val); });

        rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
            [fill_val](auto c) { return raze::algorithm::fill[raze::options::unroll<1>](c.range(), fill_val); },
            [fill_val](auto c) { return std::ranges::fill(c.range(), fill_val); });

        rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
            [fill_val](auto c) { return raze::algorithm::fill[raze::options::fstatic](c.range(), fill_val); },
            [fill_val](auto c) { return std::ranges::fill(c.range(), fill_val); });

        rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
            [fill_val](auto c) { return raze::algorithm::fill[raze::options::fscalar][raze::options::unroll<2>](c.range(), fill_val); },
            [fill_val](auto c) { return std::ranges::fill(c.range(), fill_val); });

        rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
            [fill_val](auto c) { return raze::algorithm::fill[raze::options::fstatic][raze::options::unroll<2>](c.range(), fill_val); },
            [fill_val](auto c) { return std::ranges::fill(c.range(), fill_val); });

        rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
            [fill_val](auto c) { return raze::algorithm::fill[raze::options::fautovec](c.first(), c.last(), fill_val); },
            [fill_val](auto c) { return std::ranges::fill(c.first(), c.last(), fill_val); });

        rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
            [fill_val](auto c) { return raze::algorithm::fill[raze::options::fautovec][raze::options::fstatic](c.range(), fill_val); },
            [fill_val](auto c) { return std::ranges::fill(c.range(), fill_val); });
    }

    {
        namespace opt = raze::options;
        auto acfg = rtts::algorithm::config::compact();

        auto check_aligned = [&]<size_t Align>(auto algo) {
            rtts::algorithm::run_aligned<Align, rtts::algorithm::range, T>(acfg,
                [algo, fill_val](auto c) { return algo(c.first(), c.last(), fill_val); },
                [fill_val](auto c) { return std::ranges::fill(c.first(), c.last(), fill_val); });

            rtts::algorithm::run_aligned<Align, rtts::algorithm::range, T>(acfg,
                [algo, fill_val](auto c) { return algo(c.range(), fill_val); },
                [fill_val](auto c) { return std::ranges::fill(c.range(), fill_val); });
        };

        check_aligned.template operator()<16>(raze::algorithm::fill[opt::aligned<16>]);
        check_aligned.template operator()<32>(raze::algorithm::fill[opt::aligned<32>]);
        check_aligned.template operator()<32>(raze::algorithm::fill[opt::aligned<32>][opt::unroll<2>]);
        check_aligned.template operator()<32>(raze::algorithm::fill[opt::aligned<32>][opt::fstatic]);
        check_aligned.template operator()<64>(raze::algorithm::fill[opt::aligned<64>]);
        check_aligned.template operator()<64>(raze::algorithm::fill[opt::aligned<64>][opt::fautovec]);
        check_aligned.template operator()<64>(raze::algorithm::fill[opt::aligned<64>][opt::fscalar]);
        check_aligned.template operator()<1>(raze::algorithm::fill[opt::aligned<opt::unaligned>]);
        check_aligned.template operator()<1>(raze::algorithm::fill[opt::aligned<64>][opt::aligned<opt::unaligned>]);
    }

    static_assert(rtts::algorithm::constexpr_run<T, rtts::algorithm::range, 8>(
        [](auto c) constexpr { return raze::algorithm::fill(c.first(), c.last(), T(99)); },
        [](auto c) constexpr { return std::ranges::fill(c.first(), c.last(), T(99)); }));

    static_assert(rtts::algorithm::constexpr_run<T, rtts::algorithm::range, 8>(
        [](auto c) constexpr { return raze::algorithm::fill(c.range(), T(99)); },
        [](auto c) constexpr { return std::ranges::fill(c.range(), T(99)); }));

    static_assert(rtts::algorithm::constexpr_run<T, rtts::algorithm::range, 8>(
        [](auto c) constexpr { return raze::algorithm::fill[raze::options::fscalar](c.range(), T(99)); },
        [](auto c) constexpr { return std::ranges::fill(c.range(), T(99)); }));

    static_assert(rtts::algorithm::constexpr_run<T, rtts::algorithm::range, 8>(
        [](auto c) constexpr { return raze::algorithm::fill[raze::options::unroll<2>](c.range(), T(99)); },
        [](auto c) constexpr { return std::ranges::fill(c.range(), T(99)); }));

    static_assert(rtts::algorithm::constexpr_run<T, rtts::algorithm::range, 8>(
        [](auto c) constexpr { return raze::algorithm::fill[raze::options::fautovec](c.range(), T(99)); },
        [](auto c) constexpr { return std::ranges::fill(c.range(), T(99)); }));

    static_assert(rtts::algorithm::constexpr_run<T, rtts::algorithm::range, 8>(
        [](auto c) constexpr { return raze::algorithm::fill[raze::options::aligned<64>](c.range(), T(99)); },
        [](auto c) constexpr { return std::ranges::fill(c.range(), T(99)); }));
};


RTTS_CASE("raze::algorithm::fill views and edge cases") {
    namespace opt = raze::options;
    namespace views = std::views;

    // Writable views: take, drop, reverse, filter, counted, span, non-common
    {
        std::vector<int> v1(40, 0), v2(40, 0);

        raze::algorithm::fill(v1 | views::drop(5) | views::take(20), 7);
        std::ranges::fill(v2 | views::drop(5) | views::take(20), 7);
        RTTS_EXPECT(v1 == v2);

        raze::algorithm::fill[opt::fautovec][opt::fstatic](views::counted(v1.begin() + 2, 10), 13);
        std::ranges::fill(views::counted(v2.begin() + 2, 10), 13);
        RTTS_EXPECT(v1 == v2);

        raze::algorithm::fill(v1 | views::reverse | views::take(8), 99);
        std::ranges::fill(v2 | views::reverse | views::take(8), 99);
        RTTS_EXPECT(v1 == v2);

        auto is_odd_val = [](int x) { return (x & 1) != 0; };
        raze::algorithm::fill[opt::aligned<64>](v1 | views::filter(is_odd_val), 42);
        std::ranges::fill(v2 | views::filter(is_odd_val), 42);
        RTTS_EXPECT(v1 == v2);

        raze::algorithm::fill[opt::fstatic](rtts::make_non_common(v1), 55);
        std::ranges::fill(rtts::make_non_common(v2), 55);
        RTTS_EXPECT(v1 == v2);
    }

    // Pure output iterators via counted_iterator + back_inserter / front_inserter
    {
        std::vector<int> dst;
        raze::algorithm::fill(std::counted_iterator(std::back_inserter(dst), 10), std::default_sentinel, 77);
        RTTS_EXPECT(dst == std::vector<int>(10, 77));

        std::deque<int> deq;
        raze::algorithm::fill[opt::fscalar](std::counted_iterator(std::front_inserter(deq), 5), std::default_sentinel, 33);
        RTTS_EXPECT(deq == std::deque<int>(5, 33));
    }

    // Volatile buffers (both 4B int and 1B char to verify memset/SIMD bypass)
    {
        volatile int vbuf[16];
        raze::algorithm::fill(vbuf, vbuf + 16, 123);
        for (int i = 0; i < 16; ++i) RTTS_EXPECT(vbuf[i] == 123);

        volatile char vbuf8[32];
        raze::algorithm::fill(vbuf8, vbuf8 + 32, char(65));
        for (int i = 0; i < 32; ++i) RTTS_EXPECT(vbuf8[i] == char(65));
    }

    // Empty, 1-elem, rvalue dangling, non-pow2 large struct
    {
        std::vector<int> empty_v;
        RTTS_EXPECT(raze::algorithm::fill(empty_v, 1) == empty_v.end());

        std::vector<int> one_v = { 0 };
        RTTS_EXPECT(raze::algorithm::fill(one_v, 9) == one_v.end() && one_v[0] == 9);

        auto rv = raze::algorithm::fill(std::vector<int>(4), 5);
        static_assert(std::same_as<decltype(rv), std::ranges::dangling>);

        struct big_item {
            int data[5]{};
            bool operator==(const big_item&) const = default;
        };
        std::vector<big_item> bigs(7);
        big_item target{ { 1, 2, 3, 4, 5 } };
        raze::algorithm::fill(bigs, target);
        for (const auto& b : bigs) RTTS_EXPECT(b == target);
    }
};