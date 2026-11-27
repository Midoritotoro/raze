#include <tests/rts/rts.h>
#include <raze/algorithm/find/Contains.h>

#include <algorithm>
#include <array>
#include <cstdint>
#include <forward_list>
#include <iterator>
#include <list>
#include <random>
#include <ranges>
#include <string>
#include <vector>

namespace {
    inline constexpr unsigned fuzz_seeds[] = { 42u, 0x9E3779B9u, 0xDEADBEEFu };

    struct contains_item {
        int32_t key;
        int32_t value;
        bool operator==(const contains_item&) const = default;
    };

    template <class T>
    void check_contains_variants(const std::vector<T>& input, T target) {
        const bool exp = std::ranges::contains(input, target);

        RTTS_EXPECT(raze::algorithm::contains(input, target) == exp);
        RTTS_EXPECT(raze::algorithm::contains(input.begin(), input.end(), target) == exp);
        RTTS_EXPECT(raze::algorithm::contains[raze::options::fscalar](input, target) == exp);
        RTTS_EXPECT(raze::algorithm::contains[raze::options::unroll<2>](input, target) == exp);
        RTTS_EXPECT(raze::algorithm::contains[raze::options::fstatic](input, target) == exp);
    }

    template <class T>
    void contains_boundary_sweep() {
        std::vector<size_t> sizes;
        for (size_t n = 0; n <= 140; ++n)
            sizes.push_back(n);
        for (size_t n : { size_t(255), size_t(256), size_t(257), size_t(511), size_t(512), size_t(513), size_t(1024) })
            sizes.push_back(n);

        for (size_t n : sizes) {
            std::vector<T> base(n);
            for (size_t i = 0; i < n; ++i)
                base[i] = static_cast<T>((i % 100) + 1);

            check_contains_variants(base, T(0));

            if (n > 0) {
                std::vector<size_t> positions;
                if (n <= 64) {
                    for (size_t p = 0; p < n; ++p)
                        positions.push_back(p);
                }
                else {
                    positions = { 0, 1, 15, 16, 31, 32, 63, 64, n / 2, n - 2, n - 1 };
                }

                for (size_t pos : positions) {
                    auto input = base;
                    input[pos] = T(0);
                    check_contains_variants(input, T(0));
                }
            }
        }
    }

    template <class T>
    void contains_fuzz(unsigned seed, size_t iters) {
        std::mt19937 rng(seed);
        std::uniform_int_distribution<size_t> sz(0, 512);

        for (size_t i = 0; i < iters; ++i) {
            const size_t n = sz(rng);
            auto input = rtts::random::vector<T>(n, seed + static_cast<unsigned>(i));
            const T target = n == 0 ? T(0) : input[rng() % n];

            const bool exp = std::ranges::contains(input, target);

            RTTS_EXPECT(raze::algorithm::contains(input, target) == exp);
            RTTS_EXPECT(raze::algorithm::contains[raze::options::fscalar](input.begin(), input.end(), target) == exp);
            RTTS_EXPECT(raze::algorithm::contains[raze::options::fstatic](input, target) == exp);

            auto no_zero = input;
            for (auto& x : no_zero)
                if (x == T(0)) x = T(1);

            RTTS_EXPECT(!raze::algorithm::contains(no_zero, T(0)));
            RTTS_EXPECT(!raze::algorithm::contains[raze::options::fscalar](no_zero, T(0)));
            RTTS_EXPECT(!raze::algorithm::contains[raze::options::fstatic](no_zero, T(0)));

            const size_t first = n == 0 ? 0 : rng() % (n + 1);
            const size_t last = first + (n == first ? 0 : rng() % (n - first + 1));
            const bool sub_exp = std::ranges::contains(input.begin() + first, input.begin() + last, target);
            RTTS_EXPECT(raze::algorithm::contains(input.begin() + first, input.begin() + last, target) == sub_exp);
        }
    }

    template <class T>
    void contains_suite() {
        using namespace rtts::algorithm;
        namespace opt = raze::options;
        auto cfg = config::thorough();

        constexpr auto target_of = [](auto& c) {
            return *std::ranges::next(c.first(), static_cast<std::ptrdiff_t>(c.count()));
        };

        auto check_iter = [&](auto algo) {
            run<n, T>(cfg,
                [algo, target_of](auto c) { return algo(c.first(), c.last(), target_of(c)); },
                [target_of](auto c) { return std::ranges::contains(c.first(), c.last(), target_of(c)); });
        };

        auto check_range = [&](auto algo) {
            run<n, T>(cfg,
                [algo, target_of](auto c) { return algo(c.range(), target_of(c)); },
                [target_of](auto c) { return std::ranges::contains(c.range(), target_of(c)); });
        };

        check_iter(raze::algorithm::contains);
        check_range(raze::algorithm::contains);
        check_iter(raze::algorithm::contains[opt::fscalar]);
        check_range(raze::algorithm::contains[opt::unroll<2>]);
        check_range(raze::algorithm::contains[opt::fstatic]);
        check_iter(raze::algorithm::contains[opt::fautovec]);
        check_range(raze::algorithm::contains[opt::fautovec][opt::fstatic]);

        {
            auto acfg = config::compact();

            auto check_aligned = [&]<size_t Align>(auto algo) {
                run_aligned<Align, n, T>(acfg,
                    [algo, target_of](auto c) { return algo(c.first(), c.last(), target_of(c)); },
                    [target_of](auto c) { return std::ranges::contains(c.first(), c.last(), target_of(c)); });

                run_aligned<Align, n, T>(acfg,
                    [algo, target_of](auto c) { return algo(c.range(), target_of(c)); },
                    [target_of](auto c) { return std::ranges::contains(c.range(), target_of(c)); });
            };

            check_aligned.template operator()<16>(raze::algorithm::contains[opt::aligned<16>]);
            check_aligned.template operator()<32>(raze::algorithm::contains[opt::aligned<32>]);
            check_aligned.template operator()<32>(raze::algorithm::contains[opt::aligned<32>][opt::unroll<2>]);
            check_aligned.template operator()<32>(raze::algorithm::contains[opt::aligned<32>][opt::fstatic]);
            check_aligned.template operator()<64>(raze::algorithm::contains[opt::aligned<64>]);
            check_aligned.template operator()<64>(raze::algorithm::contains[opt::aligned<64>][opt::fautovec]);
            check_aligned.template operator()<64>(raze::algorithm::contains[opt::aligned<64>][opt::fscalar]);
            check_aligned.template operator()<1>(raze::algorithm::contains[opt::aligned<opt::unaligned>]);
            check_aligned.template operator()<1>(raze::algorithm::contains[opt::aligned<64>][opt::aligned<opt::unaligned>]);
        }

        static_assert(constexpr_run<T, range, 0>(
            [](auto c) { return raze::algorithm::contains(c.first(), c.last(), T(1)); },
            [](auto c) { return std::ranges::contains(c.first(), c.last(), T(1)); }));

        constexpr auto check_ce = [target_of](auto algo) {
            return constexpr_run<T, n, 5, 3>(
                [algo, target_of](auto c) { return algo(c.range(), target_of(c)); },
                [target_of](auto c) { return std::ranges::contains(c.range(), target_of(c)); });
        };

        static_assert(check_ce(raze::algorithm::contains));
        static_assert(check_ce(raze::algorithm::contains[opt::fautovec]));
        static_assert(check_ce(raze::algorithm::contains[opt::aligned<64>]));

        static_assert([]() constexpr {
            std::array<T, 0> a0{};
            std::array<T, 4> a4{ T(1), T(2), T(3), T(4) };
            std::array<T, 5> a5{ T(1), T(2), T(3), T(4), T(5) };
            return !raze::algorithm::contains(a0, T(1)) &&
                    raze::algorithm::contains(a4, T(3)) &&
                   !raze::algorithm::contains(a4, T(9)) &&
                    raze::algorithm::contains[opt::fscalar](a5, T(5)) &&
                   !raze::algorithm::contains[opt::fscalar](a5, T(9)) &&
                   !raze::algorithm::contains(a4.begin() + 1, a4.begin() + 3, T(4)) &&
                    raze::algorithm::contains(a4.begin() + 1, a4.begin() + 3, T(2));
        }());
    }
}

RTTS_CASE_TPL("raze::algorithm::contains", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    contains_suite<T>();
};

RTTS_CASE_TPL("raze::algorithm::contains boundaries", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    contains_boundary_sweep<T>();
};

RTTS_CASE_TPL("raze::algorithm::contains fuzz", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    for (unsigned seed : fuzz_seeds)
        contains_fuzz<T>(seed, 200);
};

RTTS_CASE("raze::algorithm::contains structs and projections") {
    const std::vector<contains_item> input{
        { 1, 10 }, { 2, 20 }, { 3, 30 }, { 4, 40 }, { 5, 50 }
    };

    RTTS_EXPECT((raze::algorithm::contains(input, contains_item{ 3, 30 })));
    RTTS_EXPECT((!raze::algorithm::contains(input, contains_item{ 9, 90 })));

    RTTS_EXPECT(raze::algorithm::contains(input, 40, [](const auto& item) { return item.value; }));
    RTTS_EXPECT(!raze::algorithm::contains(input, 999, [](const auto& item) { return item.value; }));
    RTTS_EXPECT(raze::algorithm::contains[raze::options::fscalar](input, 2, [](const auto& item) { return item.key; }));

    const std::vector<std::string> words{ "alpha", "beta", "gamma", "delta" };
    RTTS_EXPECT(raze::algorithm::contains(words, std::string("gamma")));
    RTTS_EXPECT(!raze::algorithm::contains(words, std::string("omega")));
};

RTTS_CASE("raze::algorithm::contains sentinel") {
    {
        std::array<int, 3> empty_arr{ 42, 42, 42 };
        auto empty_first = std::counted_iterator(empty_arr.begin(), 0);
        RTTS_EXPECT(!raze::algorithm::contains(empty_first, std::default_sentinel, 42));
    }

    for (size_t n : { size_t(16), size_t(64), size_t(300) }) {
        std::vector<int> v(n, 1);
        auto first = std::counted_iterator(v.begin(), static_cast<std::ptrdiff_t>(v.size()));

        RTTS_EXPECT(!raze::algorithm::contains(first, std::default_sentinel, 42));

        v[0] = 42;
        RTTS_EXPECT(raze::algorithm::contains(first, std::default_sentinel, 42));

        v[0] = 1;
        v[n - 1] = 42;
        RTTS_EXPECT(raze::algorithm::contains(first, std::default_sentinel, 42));
    }
};


RTTS_CASE("raze::algorithm::contains views and edge cases") {
    namespace opt = raze::options;
    namespace views = std::views;

    // Views: iota, take, drop, reverse, filter, transform, chained, non-common
    {
        RTTS_EXPECT(raze::algorithm::contains(views::iota(1, 50), 25));
        RTTS_EXPECT(!raze::algorithm::contains(views::iota(1, 50), 75));

        const std::vector<int> src = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
        auto chain = src | views::filter([](int x) { return (x & 1) == 0; }) | views::transform([](int x) { return x * 5; }) | views::reverse;
        RTTS_EXPECT(raze::algorithm::contains[opt::aligned<64>](chain, 40));
        RTTS_EXPECT(!raze::algorithm::contains[opt::fautovec][opt::fstatic](chain, 15));

        RTTS_EXPECT(raze::algorithm::contains(rtts::make_non_common(src), 6));
    }

    // Empty, 1-elem, single-pass early exit, volatile, member projection
    {
        std::vector<int> empty_v;
        RTTS_EXPECT(!raze::algorithm::contains(empty_v, 1));

        const std::vector<int> data = { 10, 20, 30, 40, 50 };
        rtts::single_pass_input_range<int> spr(data);
        RTTS_EXPECT(raze::algorithm::contains(spr, 30));
        RTTS_EXPECT(spr.reads() == 2);

        volatile int vbuf[16];
        for (int i = 0; i < 16; ++i) vbuf[i] = i * 3;
        RTTS_EXPECT(raze::algorithm::contains(vbuf, vbuf + 16, 21));
        RTTS_EXPECT(!raze::algorithm::contains(vbuf, vbuf + 16, 22));

        const std::vector<rtts::test_item> items = { { 1, 10 }, { 7, 70 }, { 3, 30 } };
        RTTS_EXPECT(raze::algorithm::contains(items, 7, &rtts::test_item::key));
        RTTS_EXPECT(!raze::algorithm::contains(items, 9, &rtts::test_item::key));
    }
};