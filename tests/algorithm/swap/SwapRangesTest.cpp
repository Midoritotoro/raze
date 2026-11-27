#include <tests/rts/rts.h>
#include <raze/algorithm/swap/Swap.h>
#include <bit>
#include <cstdint>

RTTS_CASE_TPL("raze::algorithm::swap_ranges", rtts::algorithm::all_types)
< class T > (rtts::type<T>) {
    using namespace rtts::algorithm;
    namespace opt = raze::options;
    auto cfg = config::thorough();

    auto check_iter = [&](auto algo) {
        run<range, T>(cfg,
            [algo](auto c) { return algo(c.first(), c.last(), c.first2(), c.last2()); },
            [](auto c) { return std::ranges::swap_ranges(c.first(), c.last(), c.first2(), c.last2()); });
    };

    auto check_range = [&](auto algo) {
        run<range, T>(cfg,
            [algo](auto c) { return algo(c.range(), c.range2()); },
            [](auto c) { return std::ranges::swap_ranges(c.range(), c.range2()); });
    };

    check_iter(raze::algorithm::swap_ranges);
    check_range(raze::algorithm::swap_ranges);
    check_iter(raze::algorithm::swap_ranges[opt::fscalar]);
    check_range(raze::algorithm::swap_ranges[opt::unroll<2>]);
    check_range(raze::algorithm::swap_ranges[opt::fstatic]);
    check_iter(raze::algorithm::swap_ranges[opt::fautovec]);
    check_range(raze::algorithm::swap_ranges[opt::fautovec][opt::fstatic]);

    {
        auto acfg = config::compact();

        auto check_aligned = [&]<size_t A1, size_t A2>(auto algo) {
            run_aligned<A1, A2, range, T>(acfg,
                [algo](auto c) { return algo(c.first(), c.last(), c.first2(), c.last2()); },
                [](auto c) { return std::ranges::swap_ranges(c.first(), c.last(), c.first2(), c.last2()); });

            run_aligned<A1, A2, range, T>(acfg,
                [algo](auto c) { return algo(c.range(), c.range2()); },
                [](auto c) { return std::ranges::swap_ranges(c.range(), c.range2()); });
        };

        check_aligned.template operator()<16, 16>(raze::algorithm::swap_ranges[opt::aligned<16>]);
        check_aligned.template operator()<32, 32>(raze::algorithm::swap_ranges[opt::aligned<32>]);
        check_aligned.template operator()<32, 32>(raze::algorithm::swap_ranges[opt::aligned<32>][opt::unroll<2>]);
        check_aligned.template operator()<64, 64>(raze::algorithm::swap_ranges[opt::aligned<64>]);
        check_aligned.template operator()<64, 64>(raze::algorithm::swap_ranges[opt::aligned<64>][opt::fautovec]);
        check_aligned.template operator()<64, 64>(raze::algorithm::swap_ranges[opt::aligned<64>][opt::fscalar]);
        check_aligned.template operator()<1, 1>(raze::algorithm::swap_ranges[opt::aligned<opt::unaligned>]);
        check_aligned.template operator()<1, 1>(raze::algorithm::swap_ranges[opt::aligned<64>][opt::aligned<opt::unaligned>]);
        check_aligned.template operator()<64, 1>(raze::algorithm::swap_ranges[opt::aligned<64, opt::unaligned>]);
        check_aligned.template operator()<1, 64>(raze::algorithm::swap_ranges[opt::aligned<opt::unaligned, 64>]);
        check_aligned.template operator()<32, 64>(raze::algorithm::swap_ranges[opt::aligned<32, 64>][opt::fstatic]);
        check_aligned.template operator()<64, 16>(raze::algorithm::swap_ranges[opt::aligned<64, 16>]);
    }

    static_assert(constexpr_run<T, range, 8>(
        [](auto c) constexpr { return raze::algorithm::swap_ranges(c.first(), c.last(), c.first2(), c.last2()); },
        [](auto c) constexpr { return std::ranges::swap_ranges(c.first(), c.last(), c.first2(), c.last2()); }));

    constexpr auto check_ce = [](auto algo) {
        return constexpr_run<T, range, 8>(
            [algo](auto c) constexpr { return algo(c.range(), c.range2()); },
            [](auto c) constexpr { return std::ranges::swap_ranges(c.range(), c.range2()); });
    };

    static_assert(check_ce(raze::algorithm::swap_ranges));
    static_assert(check_ce(raze::algorithm::swap_ranges[opt::fautovec]));
    static_assert(check_ce(raze::algorithm::swap_ranges[opt::aligned<64, opt::unaligned>]));
};

RTTS_CASE("raze::algorithm::swap_ranges mixed arithmetic types") {
    using namespace rtts::algorithm;
    namespace opt = raze::options;
    auto cfg = config::thorough();
    auto acfg = config::compact();

    auto check = [&]<class S, class D>(auto algo) {
        run<range, S, D>(cfg,
            [algo](auto c) { return algo(c.first(), c.last(), c.first2(), c.last2()); },
            [](auto c) { return std::ranges::swap_ranges(c.first(), c.last(), c.first2(), c.last2()); });
    };

    check.template operator()<int32_t, float>(raze::algorithm::swap_ranges);
    check.template operator()<float, int32_t>(raze::algorithm::swap_ranges);
    check.template operator()<int64_t, double>(raze::algorithm::swap_ranges);
    check.template operator()<double, int64_t>(raze::algorithm::swap_ranges);
    check.template operator()<int8_t, int16_t>(raze::algorithm::swap_ranges);
    check.template operator()<uint32_t, float>(raze::algorithm::swap_ranges);
    check.template operator()<int32_t, float>(raze::algorithm::swap_ranges[opt::fscalar]);
    check.template operator()<int32_t, float>(raze::algorithm::swap_ranges[opt::fautovec]);

    run_aligned_mixed<64, 1, range, int32_t, float>(acfg,
        [](auto c) { return raze::algorithm::swap_ranges[opt::aligned<64, opt::unaligned>](c.first(), c.last(), c.first2(), c.last2()); },
        [](auto c) { return std::ranges::swap_ranges(c.first(), c.last(), c.first2(), c.last2()); });

    run_aligned_mixed<64, 64, range, int64_t, double>(acfg,
        [](auto c) { return raze::algorithm::swap_ranges[opt::aligned<64>](c.range(), c.range2()); },
        [](auto c) { return std::ranges::swap_ranges(c.range(), c.range2()); });

    constexpr auto check_ce = []<class S, class D>() {
        return constexpr_run_mixed<S, D, range, 8>(
            [](auto c) constexpr { return raze::algorithm::swap_ranges(c.first(), c.last(), c.first2(), c.last2()); },
            [](auto c) constexpr { return std::ranges::swap_ranges(c.first(), c.last(), c.first2(), c.last2()); });
    };

    static_assert(check_ce.template operator()<int32_t, float>());
    static_assert(check_ce.template operator()<int64_t, double>());
};

struct tc_struct_4 {
    int32_t x;
    bool operator==(const tc_struct_4&) const = default;
};
struct tc_struct_8 {
    int64_t x;
    bool operator==(const tc_struct_8&) const = default;
};

RTTS_CASE("raze::algorithm::swap_ranges structs") {
    using namespace rtts::algorithm;
    namespace opt = raze::options;
    auto cfg = config::thorough();

    auto check = [&]<class S>(auto algo) {
        run<range, S>(cfg,
            [algo](auto c) { return algo(c.first(), c.last(), c.first2(), c.last2()); },
            [](auto c) { return std::ranges::swap_ranges(c.first(), c.last(), c.first2(), c.last2()); });
    };

    check.template operator()<tc_struct_4>(raze::algorithm::swap_ranges);
    check.template operator()<tc_struct_8>(raze::algorithm::swap_ranges);
    check.template operator()<tc_struct_4>(raze::algorithm::swap_ranges[opt::fscalar]);
    check.template operator()<tc_struct_4>(raze::algorithm::swap_ranges[opt::unroll<2>]);
    check.template operator()<tc_struct_4>(raze::algorithm::swap_ranges[opt::fautovec]);

    run_aligned<64, 1, range, tc_struct_4>(config::compact(),
        [](auto c) { return raze::algorithm::swap_ranges[opt::aligned<64, opt::unaligned>](c.range(), c.range2()); },
        [](auto c) { return std::ranges::swap_ranges(c.range(), c.range2()); });

    constexpr auto check_ce = []<class S>() {
        return constexpr_run<S, range, 8>(
            [](auto c) constexpr { return raze::algorithm::swap_ranges(c.first(), c.last(), c.first2(), c.last2()); },
            [](auto c) constexpr { return std::ranges::swap_ranges(c.first(), c.last(), c.first2(), c.last2()); });
    };

    static_assert(check_ce.template operator()<tc_struct_4>());
    static_assert(check_ce.template operator()<tc_struct_8>());
};


RTTS_CASE("raze::algorithm::swap_ranges views and edge cases") {
    namespace opt = raze::options;
    namespace views = std::views;

    // Writable views: take, drop, reverse, filter, non-common
    {
        std::vector<int> a1(24), b1(24);
        std::iota(a1.begin(), a1.end(), 1);
        std::iota(b1.begin(), b1.end(), 101);
        auto a2 = a1, b2 = b1;

        raze::algorithm::swap_ranges(a1 | views::drop(2) | views::take(10), b1 | views::reverse | views::take(10));
        std::ranges::swap_ranges(a2 | views::drop(2) | views::take(10), b2 | views::reverse | views::take(10));
        RTTS_EXPECT(a1 == a2 && b1 == b2);

        auto is_even = [](int x) { return (x % 2) == 0; };
        raze::algorithm::swap_ranges[opt::aligned<64, opt::unaligned>](a1 | views::filter(is_even), b1 | views::take(5));
        std::ranges::swap_ranges(a2 | views::filter(is_even), b2 | views::take(5));
        RTTS_EXPECT(a1 == a2 && b1 == b2);

        raze::algorithm::swap_ranges[opt::fstatic](rtts::make_non_common(a1), rtts::make_non_common(b1));
        std::ranges::swap_ranges(rtts::make_non_common(a2), rtts::make_non_common(b2));
        RTTS_EXPECT(a1 == a2 && b1 == b2);
    }

    // Mismatched lengths, empty, 1-elem, volatile, rvalue dangling
    {
        std::vector<int> s1 = { 1, 2, 3, 4, 5 }, l1 = { 10, 20, 30, 40, 50, 60, 70, 80, 90 };
        auto s2 = s1, l2 = l1;
        auto rr = raze::algorithm::swap_ranges(s1, l1);
        auto sr = std::ranges::swap_ranges(s2, l2);
        RTTS_EXPECT(s1 == s2 && l1 == l2);
        RTTS_EXPECT((rr.in1 - s1.begin()) == (sr.in1 - s2.begin()));
        RTTS_EXPECT((rr.in2 - l1.begin()) == (sr.in2 - l2.begin()));

        std::vector<int> empty_v;
        auto er = raze::algorithm::swap_ranges(empty_v, l1);
        RTTS_EXPECT(er.in1 == empty_v.end() && er.in2 == l1.begin());

        volatile int va[16], vb[16];
        for (int i = 0; i < 16; ++i) { va[i] = i; vb[i] = 100 + i; }
        raze::algorithm::swap_ranges(va, va + 16, vb, vb + 16);
        for (int i = 0; i < 16; ++i) RTTS_EXPECT(va[i] == 100 + i && vb[i] == i);

        auto rv = raze::algorithm::swap_ranges(std::vector<int>{ 1, 2 }, std::vector<int>{ 3, 4 });
        static_assert(std::same_as<decltype(rv.in1), std::ranges::dangling>);
        static_assert(std::same_as<decltype(rv.in2), std::ranges::dangling>);
    }
};