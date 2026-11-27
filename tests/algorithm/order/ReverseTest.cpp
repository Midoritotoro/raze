#include <tests/rts/rts.h>
#include <raze/algorithm/order/Reverse.h>

#include <algorithm>
#include <array>
#include <iterator>
#include <list>
#include <random>
#include <ranges>
#include <vector>

namespace {
    inline constexpr unsigned fuzz_seeds[] = { 42u, 0x9E3779B9u, 0xDEADBEEFu };

    struct reverse_item {
        int32_t key;
        int32_t value;
        bool operator==(const reverse_item&) const = default;
    };

    template <class T>
    void reverse_boundary_sweep() {
        namespace opt = raze::options;
        constexpr size_t MaxN = 140;
        for (size_t n = 0; n <= MaxN; ++n) {
            std::vector<T> input(n);
            for (size_t i = 0; i < n; ++i)
                input[i] = static_cast<T>(i);

            auto exp = input;
            std::ranges::reverse(exp);

            std::vector<T> v;
            auto check_range = [&](auto algo) {
                v = input;
                auto it = algo(v);
                RTTS_EXPECT(it == v.end());
                RTTS_EXPECT(v == exp);
            };

            check_range(raze::algorithm::reverse);

            v = input;
            auto it = raze::algorithm::reverse(v.begin(), v.end());
            RTTS_EXPECT(it == v.end());
            RTTS_EXPECT(v == exp);

            check_range(raze::algorithm::reverse[opt::fscalar]);
            check_range(raze::algorithm::reverse[opt::unroll<2>]);
            check_range(raze::algorithm::reverse[opt::fstatic]);

            it = raze::algorithm::reverse(v);
            RTTS_EXPECT(it == v.end());
            RTTS_EXPECT(v == input);
        }
    }

    template <class T>
    void reverse_fuzz(unsigned seed, size_t iters) {
        std::mt19937 rng(seed);
        std::uniform_int_distribution<size_t> sz(0, 512);
        for (size_t i = 0; i < iters; ++i) {
            const size_t n = sz(rng);
            auto input = rtts::random::vector<T>(n, seed + static_cast<unsigned>(i));
            auto exp = input;
            std::ranges::reverse(exp);

            auto v = input;
            auto it = raze::algorithm::reverse(v);
            RTTS_EXPECT(it == v.end());
            RTTS_EXPECT(v == exp);

            v = input;
            it = raze::algorithm::reverse[raze::options::fscalar](v.begin(), v.end());
            RTTS_EXPECT(it == v.end());
            RTTS_EXPECT(v == exp);

            const size_t first = n == 0 ? 0 : rng() % (n + 1);
            const size_t last = first + (n == first ? 0 : rng() % (n - first + 1));
            v = input;
            exp = input;
            std::ranges::reverse(exp.begin() + first, exp.begin() + last);
            it = raze::algorithm::reverse(v.begin() + first, v.begin() + last);
            RTTS_EXPECT(it == v.begin() + last);
            RTTS_EXPECT(v == exp);
        }
    }
}

RTTS_CASE_TPL("raze::algorithm::reverse", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    using namespace rtts::algorithm;
    namespace opt = raze::options;
    auto cfg = config::thorough();

    auto check_iter = [&](auto algo) {
        run<range, T, rtts::algorithm::bidirectional_iterator>(cfg,
            [algo](auto c) { return std::ranges::distance(c.first(), algo(c.first(), c.last())); },
            [](auto c) { return std::ranges::distance(c.first(), std::ranges::reverse(c.first(), c.last())); });
    };

    auto check_range = [&](auto algo) {
        run<range, T, rtts::algorithm::bidirectional_iterator>(cfg,
            [algo](auto c) { return std::ranges::distance(c.first(), algo(c.range())); },
            [](auto c) { return std::ranges::distance(c.first(), std::ranges::reverse(c.range())); });
    };

    check_iter(raze::algorithm::reverse);
    check_range(raze::algorithm::reverse);
    check_iter(raze::algorithm::reverse[opt::fscalar]);
    check_range(raze::algorithm::reverse[opt::unroll<2>]);
    check_range(raze::algorithm::reverse[opt::fstatic]);
    check_iter(raze::algorithm::reverse[opt::fautovec]);
    check_range(raze::algorithm::reverse[opt::fautovec][opt::fstatic]);

    {
        auto acfg = config::compact();

        auto check_aligned = [&]<size_t Align>(auto algo) {
            run_aligned<Align, range, T>(acfg,
                [algo](auto c) { return std::ranges::distance(c.first(), algo(c.first(), c.last())); },
                [](auto c) { return std::ranges::distance(c.first(), std::ranges::reverse(c.first(), c.last())); });

            run_aligned<Align, range, T>(acfg,
                [algo](auto c) { return std::ranges::distance(c.first(), algo(c.range())); },
                [](auto c) { return std::ranges::distance(c.first(), std::ranges::reverse(c.range())); });
        };

        check_aligned.template operator()<16>(raze::algorithm::reverse[opt::aligned<16>]);
        check_aligned.template operator()<32>(raze::algorithm::reverse[opt::aligned<32>]);
        check_aligned.template operator()<32>(raze::algorithm::reverse[opt::aligned<32>][opt::unroll<2>]);
        check_aligned.template operator()<32>(raze::algorithm::reverse[opt::aligned<32>][opt::fstatic]);
        check_aligned.template operator()<64>(raze::algorithm::reverse[opt::aligned<64>]);
        check_aligned.template operator()<64>(raze::algorithm::reverse[opt::aligned<64>][opt::fautovec]);
        check_aligned.template operator()<64>(raze::algorithm::reverse[opt::aligned<64>][opt::fscalar]);
        check_aligned.template operator()<1>(raze::algorithm::reverse[opt::aligned<opt::unaligned>]);
        check_aligned.template operator()<1>(raze::algorithm::reverse[opt::aligned<64>][opt::aligned<opt::unaligned>]);
    }

    constexpr auto check_ce = [](auto algo) constexpr {
        return constexpr_run<T, range, 8>(
            [algo](auto c) constexpr { return std::ranges::distance(c.first(), algo(c.range())); },
            [](auto c) constexpr { return std::ranges::distance(c.first(), std::ranges::reverse(c.range())); });
    };

    static_assert(check_ce(raze::algorithm::reverse[opt::fautovec]));
    static_assert(check_ce(raze::algorithm::reverse[opt::aligned<64>]));

    static_assert([]() constexpr {
        std::array<T, 0> a0{};
        std::array<T, 1> a1{ T(1) };
        std::array<T, 4> a4{ T(1), T(2), T(3), T(4) };
        std::array<T, 5> a5{ T(1), T(2), T(3), T(4), T(5) };
        std::array<T, 4> a4s{ T(1), T(2), T(3), T(4) };
        return raze::algorithm::reverse(a0) == a0.end()
            && raze::algorithm::reverse(a1) == a1.end() && a1 == std::array<T, 1>{ T(1) }
            && raze::algorithm::reverse(a4) == a4.end() && a4 == std::array<T, 4>{ T(4), T(3), T(2), T(1) }
            && raze::algorithm::reverse[opt::fscalar](a5) == a5.end() && a5 == std::array<T, 5>{ T(5), T(4), T(3), T(2), T(1) }
            && raze::algorithm::reverse(a4s.begin() + 1, a4s.begin() + 3) == a4s.begin() + 3 && a4s == std::array<T, 4>{ T(1), T(3), T(2), T(4) };
    }());
};

RTTS_CASE_TPL("raze::algorithm::reverse boundaries", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    reverse_boundary_sweep<T>();
};

RTTS_CASE_TPL("raze::algorithm::reverse fuzz", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    for (unsigned seed : fuzz_seeds)
        reverse_fuzz<T>(seed, 200);
};

RTTS_CASE("raze::algorithm::reverse structs") {
    std::vector<reverse_item> input{
        { 1, 10 }, { 2, 20 }, { 3, 30 }, { 4, 40 }, { 5, 50 }
    };

    auto exp = input;
    std::ranges::reverse(exp);

    auto v = input;
    auto it = raze::algorithm::reverse(v);
    RTTS_EXPECT(it == v.end());
    RTTS_EXPECT(v == exp);

    v = input;
    it = raze::algorithm::reverse[raze::options::fscalar](v);
    RTTS_EXPECT(it == v.end());
    RTTS_EXPECT(v == exp);
};

RTTS_CASE("raze::algorithm::reverse bidirectional") {
    std::list<int> input{ 1, 2, 3, 4, 5 };
    auto exp = input;
    std::ranges::reverse(exp);

    auto v = input;
    auto it = raze::algorithm::reverse(v);
    RTTS_EXPECT(it == v.end());
    RTTS_EXPECT(v == exp);

    v = input;
    it = raze::algorithm::reverse[raze::options::fscalar](v.begin(), v.end());
    RTTS_EXPECT(it == v.end());
    RTTS_EXPECT(v == exp);
};


RTTS_CASE("raze::algorithm::reverse views and edge cases") {
    namespace opt = raze::options;
    namespace views = std::views;

    // Writable views: take, drop, reverse, filter, non-common
    {
        std::vector<int> v1(35);
        std::iota(v1.begin(), v1.end(), 1);
        auto v2 = v1;

        raze::algorithm::reverse(v1 | views::drop(4) | views::take(21));
        std::ranges::reverse(v2 | views::drop(4) | views::take(21));
        RTTS_EXPECT(v1 == v2);

        raze::algorithm::reverse[opt::fstatic](v1 | views::reverse | views::take(10));
        std::ranges::reverse(v2 | views::reverse | views::take(10));
        RTTS_EXPECT(v1 == v2);

        auto is_even = [](int x) { return (x % 2) == 0; };
        raze::algorithm::reverse[opt::aligned<64>](v1 | views::filter(is_even));
        std::ranges::reverse(v2 | views::filter(is_even));
        RTTS_EXPECT(v1 == v2);

        raze::algorithm::reverse(rtts::make_non_common(v1));
        std::ranges::reverse(rtts::make_non_common(v2));
        RTTS_EXPECT(v1 == v2);
    }

    // Empty, 1-elem, 2-elem, volatile, rvalue dangling
    {
        std::vector<int> empty_v;
        RTTS_EXPECT(raze::algorithm::reverse(empty_v) == empty_v.end());

        std::vector<int> one_v = { 7 };
        RTTS_EXPECT(raze::algorithm::reverse(one_v) == one_v.end() && one_v[0] == 7);

        std::vector<int> two_v = { 1, 2 };
        raze::algorithm::reverse(two_v);
        RTTS_EXPECT((two_v == std::vector<int>{ 2, 1 }));

        volatile int vbuf[17];
        for (int i = 0; i < 17; ++i) vbuf[i] = i;
        raze::algorithm::reverse(vbuf, vbuf + 17);
        for (int i = 0; i < 17; ++i) RTTS_EXPECT(vbuf[i] == 16 - i);

        auto rv = raze::algorithm::reverse(std::vector<int>{ 1, 2, 3 });
        static_assert(std::same_as<decltype(rv), std::ranges::dangling>);
    }
};