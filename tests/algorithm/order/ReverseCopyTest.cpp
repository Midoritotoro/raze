#include <tests/rts/rts.h>
#include <raze/algorithm/order/ReverseCopy.h>

#include <algorithm>
#include <array>
#include <cstdint>
#include <iterator>
#include <list>
#include <random>
#include <ranges>
#include <vector>

namespace {
    inline constexpr unsigned fuzz_seeds[] = { 42u, 0x9E3779B9u, 0xDEADBEEFu };

    struct reverse_copy_item {
        int32_t key;
        int32_t value;
        bool operator==(const reverse_copy_item&) const = default;
    };

    template <class T>
    void reverse_copy_boundary_sweep() {
        namespace opt = raze::options;
        constexpr size_t MaxN = 140;
        for (size_t n = 0; n <= MaxN; ++n) {
            std::vector<T> input(n);
            for (size_t i = 0; i < n; ++i)
                input[i] = static_cast<T>(i);

            std::vector<T> exp(n);
            auto std_result = std::ranges::reverse_copy(input, exp.begin());
            RTTS_EXPECT(std_result.in == input.end());
            RTTS_EXPECT(std_result.out == exp.end());

            std::vector<T> out(n);
            auto check_range = [&](auto algo) {
                out.assign(n, T{});
                auto result = algo(input, out.begin());
                RTTS_EXPECT(result.in == input.end());
                RTTS_EXPECT(result.out == out.end());
                RTTS_EXPECT(out == exp);
            };

            auto result = raze::algorithm::reverse_copy(input.begin(), input.end(), out.begin());
            RTTS_EXPECT(result.in == input.end());
            RTTS_EXPECT(result.out == out.end());
            RTTS_EXPECT(out == exp);

            check_range(raze::algorithm::reverse_copy);
            check_range(raze::algorithm::reverse_copy[opt::fscalar]);
            check_range(raze::algorithm::reverse_copy[opt::unroll<2>]);
            check_range(raze::algorithm::reverse_copy[opt::fstatic]);
        }
    }

    template <class T>
    void reverse_copy_fuzz(unsigned seed, size_t iters) {
        std::mt19937 rng(seed);
        std::uniform_int_distribution<size_t> sz(0, 512);
        for (size_t i = 0; i < iters; ++i) {
            const size_t n = sz(rng);
            auto input = rtts::random::vector<T>(n, seed + static_cast<unsigned>(i));
            const auto original = input;

            const size_t first = n == 0 ? 0 : rng() % (n + 1);
            const size_t last = first + (n == first ? 0 : rng() % (n - first + 1));
            const size_t count = last - first;

            std::vector<T> exp(count + 2, T{});
            std::vector<T> out(count + 2, T{});

            auto std_result = std::ranges::reverse_copy(input.begin() + first, input.begin() + last, exp.begin() + 1);
            auto result = raze::algorithm::reverse_copy(input.begin() + first, input.begin() + last, out.begin() + 1);

            RTTS_EXPECT(result.in == std_result.in);
            RTTS_EXPECT(result.out - out.begin() == std_result.out - exp.begin());
            RTTS_EXPECT(out == exp);
            RTTS_EXPECT(input == original);

            out.assign(count + 2, T{});
            result = raze::algorithm::reverse_copy[raze::options::fscalar](input.begin() + first, input.begin() + last, out.begin() + 1);

            RTTS_EXPECT(result.in == input.begin() + last);
            RTTS_EXPECT(result.out == out.begin() + 1 + count);
            RTTS_EXPECT(out == exp);
            RTTS_EXPECT(input == original);
        }
    }
}

RTTS_CASE_TPL("raze::algorithm::reverse_copy", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    using namespace rtts::algorithm;
    namespace opt = raze::options;
    auto cfg = config::thorough();
    auto acfg = config::compact();

    run<range, T, rtts::algorithm::bidirectional_iterator>(cfg,
        [](auto c) { return raze::algorithm::reverse_copy[opt::fautovec](c.first(), c.last(), c.out()); },
        [](auto c) { return std::ranges::reverse_copy(c.first(), c.last(), c.out()); });

    run<range, T, rtts::algorithm::bidirectional_iterator>(cfg,
        [](auto c) { return raze::algorithm::reverse_copy[opt::fautovec][opt::fstatic](c.range(), c.out()); },
        [](auto c) { return std::ranges::reverse_copy(c.range(), c.out()); });

    {
        auto check_aligned = [&]<size_t A1, size_t AOut>(auto algo) {
            run_aligned<A1, AOut, range, T>(acfg,
                [algo](auto c) { return algo(c.first(), c.last(), c.out()); },
                [](auto c) { return std::ranges::reverse_copy(c.first(), c.last(), c.out()); });

            run_aligned<A1, AOut, range, T>(acfg,
                [algo](auto c) { return algo(c.range(), c.out()); },
                [](auto c) { return std::ranges::reverse_copy(c.range(), c.out()); });
        };

        check_aligned.template operator()<16, 16>(raze::algorithm::reverse_copy[opt::aligned<16>]);
        check_aligned.template operator()<32, 32>(raze::algorithm::reverse_copy[opt::aligned<32>]);
        check_aligned.template operator()<32, 32>(raze::algorithm::reverse_copy[opt::aligned<32>][opt::unroll<2>]);
        check_aligned.template operator()<64, 64>(raze::algorithm::reverse_copy[opt::aligned<64>]);
        check_aligned.template operator()<64, 64>(raze::algorithm::reverse_copy[opt::aligned<64>][opt::fautovec]);
        check_aligned.template operator()<64, 64>(raze::algorithm::reverse_copy[opt::aligned<64>][opt::fscalar]);
        check_aligned.template operator()<1, 1>(raze::algorithm::reverse_copy[opt::aligned<opt::unaligned>]);
        check_aligned.template operator()<1, 1>(raze::algorithm::reverse_copy[opt::aligned<64>][opt::aligned<opt::unaligned>]);
        check_aligned.template operator()<64, 1>(raze::algorithm::reverse_copy[opt::aligned<64, opt::unaligned>]);
        check_aligned.template operator()<1, 64>(raze::algorithm::reverse_copy[opt::aligned<opt::unaligned, 64>]);
        check_aligned.template operator()<32, 64>(raze::algorithm::reverse_copy[opt::aligned<32, 64>][opt::fstatic]);
        check_aligned.template operator()<64, 16>(raze::algorithm::reverse_copy[opt::aligned<64, 16>]);
    }

    constexpr auto check_ce = [](auto algo) constexpr {
        return constexpr_run<T, range, 8>(
            [algo](auto c) constexpr { return algo(c.range(), c.out()); },
            [](auto c) constexpr { return std::ranges::reverse_copy(c.range(), c.out()); });
    };

    static_assert(check_ce(raze::algorithm::reverse_copy[opt::fautovec]));
    static_assert(check_ce(raze::algorithm::reverse_copy[opt::aligned<64, opt::unaligned>]));

    static_assert([]() constexpr {
        std::array<T, 0> i0{}, o0{};
        auto r0 = raze::algorithm::reverse_copy(i0, o0.begin());

        std::array<T, 1> i1{ T(1) }, o1{};
        auto r1 = raze::algorithm::reverse_copy(i1, o1.begin());

        std::array<T, 4> i4{ T(1), T(2), T(3), T(4) }, o4{};
        auto r4 = raze::algorithm::reverse_copy(i4.begin(), i4.end(), o4.begin());

        std::array<T, 5> i5{ T(1), T(2), T(3), T(4), T(5) }, o5{};
        auto r5 = raze::algorithm::reverse_copy[opt::fscalar](i5, o5.begin());

        std::array<T, 4> i4s{ T(1), T(2), T(3), T(4) }, o4s{};
        auto r4s = raze::algorithm::reverse_copy(i4s.begin() + 1, i4s.begin() + 3, o4s.begin() + 1);

        return r0.in == i0.end() && r0.out == o0.end()
            && r1.in == i1.end() && r1.out == o1.end() && o1 == std::array<T, 1>{ T(1) }
            && r4.in == i4.end() && r4.out == o4.end() && o4 == std::array<T, 4>{ T(4), T(3), T(2), T(1) }
            && r5.in == i5.end() && r5.out == o5.end() && o5 == std::array<T, 5>{ T(5), T(4), T(3), T(2), T(1) }
            && r4s.in == i4s.begin() + 3 && r4s.out == o4s.begin() + 3 && o4s == std::array<T, 4>{ T(0), T(3), T(2), T(0) };
    }());
};

RTTS_CASE_TPL("raze::algorithm::reverse_copy boundaries", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    reverse_copy_boundary_sweep<T>();
};

RTTS_CASE_TPL("raze::algorithm::reverse_copy fuzz", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    for (unsigned seed : fuzz_seeds)
        reverse_copy_fuzz<T>(seed, 200);
};

RTTS_CASE("raze::algorithm::reverse_copy structs") {
    std::vector<reverse_copy_item> input{
        { 1, 10 }, { 2, 20 }, { 3, 30 }, { 4, 40 }, { 5, 50 }
    };
    const auto original = input;
    std::vector<reverse_copy_item> exp(input.size());
    std::vector<reverse_copy_item> out(input.size());

    std::ranges::reverse_copy(input, exp.begin());

    auto result = raze::algorithm::reverse_copy(input, out.begin());
    RTTS_EXPECT(result.in == input.end());
    RTTS_EXPECT(result.out == out.end());
    RTTS_EXPECT(out == exp);
    RTTS_EXPECT(input == original);

    out.assign(input.size(), {});
    result = raze::algorithm::reverse_copy[raze::options::fscalar](input, out.begin());
    RTTS_EXPECT(result.in == input.end());
    RTTS_EXPECT(result.out == out.end());
    RTTS_EXPECT(out == exp);
};

RTTS_CASE("raze::algorithm::reverse_copy bidirectional") {
    std::list<int> input{ 1, 2, 3, 4, 5 };
    std::vector<int> out;

    auto result = raze::algorithm::reverse_copy(input, std::back_inserter(out));
    RTTS_EXPECT(result.in == input.end());
    RTTS_EXPECT((out == std::vector<int>{ 5, 4, 3, 2, 1 }));

    out.clear();
    auto scalar_result = raze::algorithm::reverse_copy[raze::options::fscalar](
        input.begin(), input.end(), std::back_inserter(out));
    RTTS_EXPECT(scalar_result.in == input.end());
    RTTS_EXPECT((out == std::vector<int>{ 5, 4, 3, 2, 1 }));
};

RTTS_CASE("raze::algorithm::reverse_copy sentinel") {
    std::array<int, 5> input{ 1, 2, 3, 4, 5 };
    std::array<int, 3> out{};

    auto first = std::counted_iterator(input.begin() + 1, 3);
    auto result = raze::algorithm::reverse_copy(first, std::default_sentinel, out.begin());

    RTTS_EXPECT(result.in == std::default_sentinel);
    RTTS_EXPECT(result.out == out.end());
    RTTS_EXPECT((out == std::array<int, 3>{ 4, 3, 2 }));
};


RTTS_CASE("raze::algorithm::reverse_copy views and edge cases") {
    namespace opt = raze::options;
    namespace views = std::views;

    // Bidirectional views: iota, take, drop, reverse, filter, transform, non-common
    {
        const std::vector<int> src = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17 };

        std::vector<int> r1, s1;
        raze::algorithm::reverse_copy(views::iota(1, 20), std::back_inserter(r1));
        std::ranges::reverse_copy(views::iota(1, 20), std::back_inserter(s1));
        RTTS_EXPECT(r1 == s1);

        auto chain = src | views::filter([](int x) { return (x & 1) == 1; }) | views::transform([](int x) { return x * 10; });
        std::vector<int> r2, s2;
        raze::algorithm::reverse_copy[opt::aligned<64, opt::unaligned>](chain, std::back_inserter(r2));
        std::ranges::reverse_copy(chain, std::back_inserter(s2));
        RTTS_EXPECT(r2 == s2);

        std::vector<int> r3, s3;
        raze::algorithm::reverse_copy(src | views::reverse | views::take(8), std::back_inserter(r3));
        std::ranges::reverse_copy(src | views::reverse | views::take(8), std::back_inserter(s3));
        RTTS_EXPECT(r3 == s3);

        std::vector<int> nc1(src.size()), nc2(src.size());
        raze::algorithm::reverse_copy(rtts::make_non_common(src), nc1.begin());
        std::ranges::reverse_copy(rtts::make_non_common(src), nc2.begin());
        RTTS_EXPECT(nc1 == nc2);
    }

    // back_inserter, front_inserter, inserter, empty, 1-elem, volatile, rvalue dangling
    {
        const std::vector<int> src = { 1, 2, 3, 4, 5 };
        const std::vector<int> expected = { 5, 4, 3, 2, 1 };

        std::vector<int> b1, b2, b3, b4;
        raze::algorithm::reverse_copy(src, std::back_inserter(b1));
        raze::algorithm::reverse_copy[opt::fscalar](src, std::back_inserter(b2));
        raze::algorithm::reverse_copy[opt::fautovec][opt::fstatic](src, std::back_inserter(b3));
        alignas(64) const std::array<int, 5> src_aligned = { 1, 2, 3, 4, 5 };
        raze::algorithm::reverse_copy[opt::aligned<64, opt::unaligned>](src_aligned, std::back_inserter(b4));
        RTTS_EXPECT(b1 == expected && b2 == expected && b3 == expected && b4 == expected);

        std::deque<int> f1, f2;
        raze::algorithm::reverse_copy(src, std::front_inserter(f1));
        std::ranges::reverse_copy(src, std::front_inserter(f2));
        RTTS_EXPECT(f1 == f2);

        std::vector<int> i1 = { 0, 0 }, i2 = { 0, 0 };
        raze::algorithm::reverse_copy(src, std::inserter(i1, i1.begin() + 1));
        std::ranges::reverse_copy(src, std::inserter(i2, i2.begin() + 1));
        RTTS_EXPECT(i1 == i2);

        std::vector<int> empty_v, empty_out;
        raze::algorithm::reverse_copy(empty_v, std::back_inserter(empty_out));
        RTTS_EXPECT(empty_out.empty());

        volatile int vsrc[16], vdst[16];
        for (int i = 0; i < 16; ++i) vsrc[i] = i;
        raze::algorithm::reverse_copy(vsrc, vsrc + 16, vdst);
        for (int i = 0; i < 16; ++i) RTTS_EXPECT(vdst[i] == 15 - i);

        std::vector<int> rdst;
        auto rres = raze::algorithm::reverse_copy(std::vector<int>{ 1, 2, 3 }, std::back_inserter(rdst));
        static_assert(std::same_as<decltype(rres.in), std::ranges::dangling>);
        RTTS_EXPECT((rdst == std::vector<int>{ 3, 2, 1 }));
    }
};