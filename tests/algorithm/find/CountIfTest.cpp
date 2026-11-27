#include <tests/rts/rts.h>
#include <raze/algorithm/find/CountIf.h>
#include <src/raze/algorithm/EqualTo.h>

#include <algorithm>
#include <array>
#include <cstdint>
#include <iterator>
#include <random>
#include <ranges>
#include <string>
#include <vector>

namespace {
    inline constexpr unsigned fuzz_seeds[] = { 42u, 0x9E3779B9u, 0xDEADBEEFu };

    struct count_if_item {
        int32_t key;
        int32_t value;
        bool operator==(const count_if_item&) const = default;
    };

    template <class T>
    struct equal_pred {
        T target;
        constexpr auto operator()(auto x) const { return x == target; }
    };

    struct false_pred {
        constexpr bool operator()(auto) const { return false; }
    };

    struct true_pred {
        constexpr bool operator()(auto) const { return true; }
    };

    template <class T>
    void check_count_if_variants(const std::vector<T>& input, T target) {
        namespace opt = raze::options;
        auto pred = [target](auto x) { return x == target; };
        auto scalar_pred = [target](T x) { return x == target; };
        auto vec_pred = raze::algorithm::equal_to(target);

        const auto exp = std::ranges::count_if(input, pred);

        auto check_pred = [&](auto p) {
            RTTS_EXPECT(raze::algorithm::count_if(input, p) == exp);
            RTTS_EXPECT(raze::algorithm::count_if(input.begin(), input.end(), p) == exp);
            RTTS_EXPECT(raze::algorithm::count_if[opt::fscalar](input, p) == exp);
            RTTS_EXPECT(raze::algorithm::count_if[opt::unroll<2>](input, p) == exp);
            RTTS_EXPECT(raze::algorithm::count_if[opt::fstatic](input, p) == exp);
        };

        check_pred(pred);
        check_pred(vec_pred);
        RTTS_EXPECT(raze::algorithm::count_if(input, scalar_pred) == exp);
    }

    template <class T>
    void count_if_boundary_sweep() {
        std::vector<size_t> sizes;
        for (size_t n = 0; n <= 140; ++n)
            sizes.push_back(n);
        for (size_t n : { size_t(255), size_t(256), size_t(257), size_t(511), size_t(512), size_t(513), size_t(1024) })
            sizes.push_back(n);

        for (size_t n : sizes) {
            std::vector<T> ones(n, T(1));
            check_count_if_variants(ones, T(0));
            check_count_if_variants(ones, T(1));

            std::vector<T> alt(n);
            for (size_t i = 0; i < n; ++i)
                alt[i] = static_cast<T>(i & 1 ? 0 : 1);
            check_count_if_variants(alt, T(0));

            if (n > 0) {
                auto sparse = ones;
                sparse[0] = T(0);
                sparse[n - 1] = T(0);
                if (n > 16) sparse[16] = T(0);
                if (n > 31) sparse[31] = T(0);
                if (n > 64) sparse[64] = T(0);
                check_count_if_variants(sparse, T(0));
            }
        }
    }

    template <class T>
    void count_if_fuzz(unsigned seed, size_t iters) {
        std::mt19937 rng(seed);
        std::uniform_int_distribution<size_t> sz(0, 512);

        for (size_t i = 0; i < iters; ++i) {
            const size_t n = sz(rng);
            auto input = rtts::random::vector<T>(n, seed + static_cast<unsigned>(i));
            const T target = n == 0 ? T(0) : input[rng() % n];

            auto vec_pred = raze::algorithm::equal_to(target);
            const auto exp = std::ranges::count_if(input, vec_pred);

            RTTS_EXPECT(raze::algorithm::count_if(input, vec_pred) == exp);
            RTTS_EXPECT(raze::algorithm::count_if[raze::options::fscalar](input.begin(), input.end(), vec_pred) == exp);
            RTTS_EXPECT(raze::algorithm::count_if[raze::options::fstatic](input, vec_pred) == exp);

            const size_t first = n == 0 ? 0 : rng() % (n + 1);
            const size_t last = first + (n == first ? 0 : rng() % (n - first + 1));
            const auto sub_exp = std::ranges::count_if(input.begin() + first, input.begin() + last, vec_pred);
            RTTS_EXPECT(raze::algorithm::count_if(input.begin() + first, input.begin() + last, vec_pred) == sub_exp);
        }
    }

    template <class T>
    void count_if_suite() {
        using namespace rtts::algorithm;
        namespace opt = raze::options;
        auto cfg = config::thorough();

        constexpr auto target_of = [](auto& c) {
            return *std::ranges::next(c.first(), static_cast<std::ptrdiff_t>(c.count()));
        };

        auto check_iter = [&](auto algo) {
            run<n, T>(cfg,
                [algo, target_of](auto c) { return algo(c.first(), c.last(), equal_pred<T>{ target_of(c) }); },
                [target_of](auto c) { return std::ranges::count_if(c.first(), c.last(), equal_pred<T>{ target_of(c) }); });
        };

        auto check_range = [&](auto algo) {
            run<n, T>(cfg,
                [algo, target_of](auto c) { return algo(c.range(), equal_pred<T>{ target_of(c) }); },
                [target_of](auto c) { return std::ranges::count_if(c.range(), equal_pred<T>{ target_of(c) }); });
        };

        check_iter(raze::algorithm::count_if);
        check_range(raze::algorithm::count_if);
        check_iter(raze::algorithm::count_if[opt::fscalar]);
        check_range(raze::algorithm::count_if[opt::unroll<2>]);
        check_range(raze::algorithm::count_if[opt::fstatic]);
        check_iter(raze::algorithm::count_if[opt::fautovec]);

        run<range, T>(cfg,
            [](auto c) { return raze::algorithm::count_if(c.first(), c.last(), false_pred{}); },
            [](auto c) { return std::ranges::count_if(c.first(), c.last(), false_pred{}); });

        run<range, T>(cfg,
            [](auto c) { return raze::algorithm::count_if(c.first(), c.last(), true_pred{}); },
            [](auto c) { return std::ranges::count_if(c.first(), c.last(), true_pred{}); });

        run<n, T>(cfg,
            [target_of](auto c) {
                T target = target_of(c);
                return raze::algorithm::count_if[opt::fautovec][opt::fstatic](c.range(), [target](auto x) { if (x == target) return true; return false; });
            },
            [target_of](auto c) {
                T target = target_of(c);
                return std::ranges::count_if(c.range(), [target](auto x) { if (x == target) return true; return false; });
            });

        {
            auto acfg = config::compact();

            auto check_aligned = [&]<size_t Align>(auto algo) {
                run_aligned<Align, n, T>(acfg,
                    [algo, target_of](auto c) { return algo(c.first(), c.last(), equal_pred<T>{ target_of(c) }); },
                    [target_of](auto c) { return std::ranges::count_if(c.first(), c.last(), equal_pred<T>{ target_of(c) }); });

                run_aligned<Align, n, T>(acfg,
                    [algo, target_of](auto c) { return algo(c.range(), equal_pred<T>{ target_of(c) }); },
                    [target_of](auto c) { return std::ranges::count_if(c.range(), equal_pred<T>{ target_of(c) }); });
            };

            check_aligned.template operator()<16>(raze::algorithm::count_if[opt::aligned<16>]);
            check_aligned.template operator()<32>(raze::algorithm::count_if[opt::aligned<32>]);
            check_aligned.template operator()<32>(raze::algorithm::count_if[opt::aligned<32>][opt::unroll<2>]);
            check_aligned.template operator()<32>(raze::algorithm::count_if[opt::aligned<32>][opt::fstatic]);
            check_aligned.template operator()<64>(raze::algorithm::count_if[opt::aligned<64>]);
            check_aligned.template operator()<64>(raze::algorithm::count_if[opt::aligned<64>][opt::fautovec]);
            check_aligned.template operator()<64>(raze::algorithm::count_if[opt::aligned<64>][opt::fscalar]);
            check_aligned.template operator()<1>(raze::algorithm::count_if[opt::aligned<opt::unaligned>]);
            check_aligned.template operator()<1>(raze::algorithm::count_if[opt::aligned<64>][opt::aligned<opt::unaligned>]);
        }

        static_assert(constexpr_run<T, range, 0>(
            [](auto c) { return raze::algorithm::count_if(c.first(), c.last(), true_pred{}); },
            [](auto c) { return std::ranges::count_if(c.first(), c.last(), true_pred{}); }));

        constexpr auto check_ce = [target_of](auto algo) {
            return constexpr_run<T, n, 5, 3>(
                [algo, target_of](auto c) { return algo(c.range(), equal_pred<T>{ target_of(c) }); },
                [target_of](auto c) { return std::ranges::count_if(c.range(), equal_pred<T>{ target_of(c) }); });
        };

        static_assert(check_ce(raze::algorithm::count_if));
        static_assert(check_ce(raze::algorithm::count_if[opt::aligned<64>]));

        static_assert(constexpr_run<T, n, 5, 3>(
            [target_of](auto c) {
                T target = target_of(c);
                return raze::algorithm::count_if[opt::fautovec](c.range(), [target](auto x) constexpr { if (x == target) return true; return false; });
            },
            [target_of](auto c) {
                T target = target_of(c);
                return std::ranges::count_if(c.range(), [target](auto x) constexpr { if (x == target) return true; return false; });
            }));

        static_assert([]() constexpr {
            std::array<T, 0> a0{};
            std::array<T, 4> a4{ T(1), T(2), T(1), T(4) };
            std::array<T, 5> a5{ T(1), T(2), T(3), T(4), T(5) };
            std::array<T, 5> b5{ T(1), T(2), T(2), T(4), T(2) };
            return raze::algorithm::count_if(a0, true_pred{}) == 0 &&
                   raze::algorithm::count_if(a4, equal_pred<T>{ T(1) }) == 2 &&
                   raze::algorithm::count_if(a4, false_pred{}) == 0 &&
                   raze::algorithm::count_if[opt::fscalar](a5, [](auto x) { return x > T(2); }) == 3 &&
                   raze::algorithm::count_if(b5.begin() + 1, b5.begin() + 4, equal_pred<T>{ T(2) }) == 2;
        }());
    }
}

RTTS_CASE_TPL("raze::algorithm::count_if", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    count_if_suite<T>();
};

RTTS_CASE_TPL("raze::algorithm::count_if boundaries", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    count_if_boundary_sweep<T>();
};

RTTS_CASE_TPL("raze::algorithm::count_if fuzz", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    for (unsigned seed : fuzz_seeds)
        count_if_fuzz<T>(seed, 200);
};

RTTS_CASE("raze::algorithm::count_if structs and projections") {
    const std::vector<count_if_item> input{
        { 1, 10 }, { 2, 20 }, { 1, 30 }, { 4, 20 }, { 1, 50 }
    };

    RTTS_EXPECT(raze::algorithm::count_if(
        input,
        [](int32_t k) { return k == 1; },
        [](const auto& item) { return item.key; }) == 3);

    RTTS_EXPECT(raze::algorithm::count_if[raze::options::fscalar](
        input,
        [](int32_t v) { return v == 20; },
        [](const auto& item) { return item.value; }) == 2);

    RTTS_EXPECT(raze::algorithm::count_if(
        input,
        [](int32_t v) { return v == 999; },
        [](const auto& item) { return item.value; }) == 0);

    const std::vector<std::string> words{ "aa", "bbb", "cc", "dddd", "ee" };
    RTTS_EXPECT(raze::algorithm::count_if(words, [](const std::string& s) { return s.size() == 2; }) == 3);
};

RTTS_CASE("raze::algorithm::count_if sentinel") {
    {
        std::array<int, 3> empty_arr{ 1, 1, 1 };
        auto empty_first = std::counted_iterator(empty_arr.begin(), 0);
        RTTS_EXPECT(raze::algorithm::count_if(empty_first, std::default_sentinel, raze::algorithm::equal_to(1)) == 0);
    }

    for (size_t n : { size_t(16), size_t(64), size_t(300) }) {
        std::vector<int> v(n, 1);
        auto first = std::counted_iterator(v.begin(), static_cast<std::ptrdiff_t>(v.size()));

        RTTS_EXPECT(raze::algorithm::count_if(first, std::default_sentinel, raze::algorithm::equal_to(42)) == 0);
        RTTS_EXPECT(raze::algorithm::count_if(first, std::default_sentinel, raze::algorithm::equal_to(1)) == static_cast<std::ptrdiff_t>(n));

        v[0] = 42;
        v[n / 2] = 42;
        v[n - 1] = 42;
        RTTS_EXPECT(raze::algorithm::count_if(first, std::default_sentinel, raze::algorithm::equal_to(42)) == 3);
    }
};


RTTS_CASE("raze::algorithm::count_if views and edge cases") {
    namespace opt = raze::options;
    namespace views = std::views;

    // Views: iota, take, drop, reverse, filter, transform, chained, non-common
    {
        auto pred_branch = [](auto x) { if ((x % 3) == 0) return true; return false; };
        RTTS_EXPECT(raze::algorithm::count_if[opt::fautovec][opt::fstatic](views::iota(1, 31), pred_branch) ==
                    std::ranges::count_if(views::iota(1, 31), pred_branch));

        const std::vector<int> src = { -3, -2, -1, 0, 1, 2, 3, 4, 5, 6, 7, 8 };
        auto chain = src | views::filter([](int x) { return x > 0; }) | views::transform([](int x) { return x * 2; }) | views::reverse;
        RTTS_EXPECT(raze::algorithm::count_if[opt::aligned<64>](chain, [](int x) { return x >= 10; }) ==
                    std::ranges::count_if(chain, [](int x) { return x >= 10; }));

        RTTS_EXPECT(raze::algorithm::count_if(rtts::make_non_common(src), [](auto x) { return x < 0; }) == 3);
    }

    // Empty, 1-elem, single-pass, volatile, member projection
    {
        std::vector<int> empty_v;
        RTTS_EXPECT(raze::algorithm::count_if(empty_v, [](auto x) { return x > 0; }) == 0);

        const std::vector<int> data = { 1, 2, 3, 4, 5, 6 };
        rtts::single_pass_input_range<int> spr(data);
        RTTS_EXPECT(raze::algorithm::count_if(spr, [](int x) { return (x & 1) == 0; }) == 3 && spr.reads() == data.size());

        volatile int vbuf[16];
        for (int i = 0; i < 16; ++i) vbuf[i] = i;
        RTTS_EXPECT(raze::algorithm::count_if(vbuf, vbuf + 16, [](int x) { return x >= 10; }) == 6);

        const std::vector<rtts::test_item> items = { { 1, 10 }, { 5, 50 }, { 6, 60 }, { 2, 20 } };
        RTTS_EXPECT(raze::algorithm::count_if(items, [](int k) { return k >= 5; }, &rtts::test_item::key) == 2);
    }
};