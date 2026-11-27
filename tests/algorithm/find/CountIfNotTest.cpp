#include <tests/rts/rts.h>
#include <raze/algorithm/find/CountIfNot.h>
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

    struct count_if_not_item {
        int32_t key;
        int32_t value;
        bool operator==(const count_if_not_item&) const = default;
    };

    template <class T>
    struct not_equal_pred {
        T target;
        constexpr auto operator()(auto x) const { return x != target; }
    };

    template <class T>
    struct eq_pred {
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
    void check_count_if_not_variants(const std::vector<T>& input, T keep) {
        auto pred = [keep](auto x) { return x != keep; };
        auto scalar_pred = [keep](T x) { return x != keep; };

        const auto exp = std::ranges::count_if(input, raze::algorithm::make_not_fn(pred));

        RTTS_EXPECT(raze::algorithm::count_if_not(input, pred) == exp);
        RTTS_EXPECT(raze::algorithm::count_if_not(input.begin(), input.end(), pred) == exp);
        RTTS_EXPECT(raze::algorithm::count_if_not(input, scalar_pred) == exp);
        RTTS_EXPECT(raze::algorithm::count_if_not[raze::options::fscalar](input, pred) == exp);
        RTTS_EXPECT(raze::algorithm::count_if_not[raze::options::unroll<2>](input, pred) == exp);
        RTTS_EXPECT(raze::algorithm::count_if_not[raze::options::fstatic](input, pred) == exp);
    }

    template <class T>
    void check_count_if_not_vec_variants(const std::vector<T>& input, T keep) {
        auto vec_pred = raze::algorithm::equal_to(keep);
        const auto exp = std::ranges::count_if(input, raze::algorithm::make_not_fn(vec_pred));

        RTTS_EXPECT(raze::algorithm::count_if_not(input, vec_pred) == exp);
        RTTS_EXPECT(raze::algorithm::count_if_not(input.begin(), input.end(), vec_pred) == exp);
        RTTS_EXPECT(raze::algorithm::count_if_not[raze::options::fscalar](input, vec_pred) == exp);
        RTTS_EXPECT(raze::algorithm::count_if_not[raze::options::unroll<2>](input, vec_pred) == exp);
        RTTS_EXPECT(raze::algorithm::count_if_not[raze::options::fstatic](input, vec_pred) == exp);
    }

    template <class T>
    void count_if_not_boundary_sweep() {
        std::vector<size_t> sizes;
        for (size_t n = 0; n <= 140; ++n)
            sizes.push_back(n);
        for (size_t n : { size_t(255), size_t(256), size_t(257), size_t(511), size_t(512), size_t(513), size_t(1024) })
            sizes.push_back(n);

        for (size_t n : sizes) {
            std::vector<T> ones(n, T(1));
            check_count_if_not_variants(ones, T(0));
            check_count_if_not_vec_variants(ones, T(1));
            check_count_if_not_vec_variants(ones, T(0));

            std::vector<T> alt(n);
            for (size_t i = 0; i < n; ++i)
                alt[i] = static_cast<T>(i & 1 ? 0 : 1);
            check_count_if_not_variants(alt, T(0));
            check_count_if_not_vec_variants(alt, T(1));

            if (n > 0) {
                auto sparse = ones;
                sparse[0] = T(0);
                sparse[n - 1] = T(0);
                if (n > 16) sparse[16] = T(0);
                if (n > 31) sparse[31] = T(0);
                if (n > 64) sparse[64] = T(0);
                check_count_if_not_variants(sparse, T(0));
                check_count_if_not_vec_variants(sparse, T(1));
            }
        }
    }

    template <class T>
    void count_if_not_fuzz(unsigned seed, size_t iters) {
        std::mt19937 rng(seed);
        std::uniform_int_distribution<size_t> sz(0, 512);

        for (size_t i = 0; i < iters; ++i) {
            const size_t n = sz(rng);
            auto input = rtts::random::vector<T>(n, seed + static_cast<unsigned>(i));
            const T keep = n == 0 ? T(0) : input[rng() % n];

            auto vec_pred = raze::algorithm::equal_to(keep);
            const auto exp = std::ranges::count_if(input, raze::algorithm::make_not_fn(vec_pred));

            RTTS_EXPECT(raze::algorithm::count_if_not(input, vec_pred) == exp);
            RTTS_EXPECT(raze::algorithm::count_if_not[raze::options::fscalar](input.begin(), input.end(), vec_pred) == exp);
            RTTS_EXPECT(raze::algorithm::count_if_not[raze::options::fstatic](input, vec_pred) == exp);

            const size_t first = n == 0 ? 0 : rng() % (n + 1);
            const size_t last = first + (n == first ? 0 : rng() % (n - first + 1));
            const auto sub_exp = std::ranges::count_if(input.begin() + first, input.begin() + last, raze::algorithm::make_not_fn(vec_pred));
            RTTS_EXPECT(raze::algorithm::count_if_not(input.begin() + first, input.begin() + last, vec_pred) == sub_exp);
        }
    }

    template <class T>
    void count_if_not_suite() {
        using namespace rtts::algorithm;
        namespace opt = raze::options;
        auto cfg = config::thorough();

        constexpr auto target_of = [](auto& c) {
            return *std::ranges::next(c.first(), static_cast<std::ptrdiff_t>(c.count()));
        };

        auto check_iter = [&](auto algo) {
            run<n, T>(cfg,
                [algo, target_of](auto c) { return algo(c.first(), c.last(), not_equal_pred<T>{ target_of(c) }); },
                [target_of](auto c) { return std::ranges::count_if(c.first(), c.last(), raze::algorithm::make_not_fn(not_equal_pred<T>{ target_of(c) })); });
        };

        auto check_range = [&](auto algo) {
            run<n, T>(cfg,
                [algo, target_of](auto c) { return algo(c.range(), not_equal_pred<T>{ target_of(c) }); },
                [target_of](auto c) { return std::ranges::count_if(c.range(), raze::algorithm::make_not_fn(not_equal_pred<T>{ target_of(c) })); });
        };

        check_iter(raze::algorithm::count_if_not);
        check_range(raze::algorithm::count_if_not);
        check_iter(raze::algorithm::count_if_not[opt::fscalar]);
        check_range(raze::algorithm::count_if_not[opt::unroll<2>]);
        check_range(raze::algorithm::count_if_not[opt::fstatic]);
        check_iter(raze::algorithm::count_if_not[opt::fautovec]);

        run<range, T>(cfg,
            [](auto c) { return raze::algorithm::count_if_not(c.first(), c.last(), true_pred{}); },
            [](auto c) { return std::ranges::count_if(c.first(), c.last(), false_pred{}); });

        run<range, T>(cfg,
            [](auto c) { return raze::algorithm::count_if_not(c.first(), c.last(), false_pred{}); },
            [](auto c) { return std::ranges::count_if(c.first(), c.last(), true_pred{}); });

        run<n, T>(cfg,
            [target_of](auto c) {
                T target = target_of(c);
                return raze::algorithm::count_if_not[opt::fautovec][opt::fstatic](c.range(), [target](auto x) { if (x != target) return true; return false; });
            },
            [target_of](auto c) {
                T target = target_of(c);
                return std::ranges::count_if(c.range(), raze::algorithm::make_not_fn([target](auto x) { if (x != target) return true; return false; }));
            });

        {
            auto acfg = config::compact();

            auto check_aligned = [&]<size_t Align>(auto algo) {
                run_aligned<Align, n, T>(acfg,
                    [algo, target_of](auto c) { return algo(c.first(), c.last(), not_equal_pred<T>{ target_of(c) }); },
                    [target_of](auto c) { return std::ranges::count_if(c.first(), c.last(), raze::algorithm::make_not_fn(not_equal_pred<T>{ target_of(c) })); });

                run_aligned<Align, n, T>(acfg,
                    [algo, target_of](auto c) { return algo(c.range(), not_equal_pred<T>{ target_of(c) }); },
                    [target_of](auto c) { return std::ranges::count_if(c.range(), raze::algorithm::make_not_fn(not_equal_pred<T>{ target_of(c) })); });
            };

            check_aligned.template operator()<16>(raze::algorithm::count_if_not[opt::aligned<16>]);
            check_aligned.template operator()<32>(raze::algorithm::count_if_not[opt::aligned<32>]);
            check_aligned.template operator()<32>(raze::algorithm::count_if_not[opt::aligned<32>][opt::unroll<2>]);
            check_aligned.template operator()<32>(raze::algorithm::count_if_not[opt::aligned<32>][opt::fstatic]);
            check_aligned.template operator()<64>(raze::algorithm::count_if_not[opt::aligned<64>]);
            check_aligned.template operator()<64>(raze::algorithm::count_if_not[opt::aligned<64>][opt::fautovec]);
            check_aligned.template operator()<64>(raze::algorithm::count_if_not[opt::aligned<64>][opt::fscalar]);
            check_aligned.template operator()<1>(raze::algorithm::count_if_not[opt::aligned<opt::unaligned>]);
            check_aligned.template operator()<1>(raze::algorithm::count_if_not[opt::aligned<64>][opt::aligned<opt::unaligned>]);
        }

        static_assert(constexpr_run<T, range, 0>(
            [](auto c) { return raze::algorithm::count_if_not(c.first(), c.last(), false_pred{}); },
            [](auto c) { return std::ranges::count_if(c.first(), c.last(), true_pred{}); }));

        constexpr auto check_ce = [target_of](auto algo) {
            return constexpr_run<T, n, 5, 3>(
                [algo, target_of](auto c) { return algo(c.range(), not_equal_pred<T>{ target_of(c) }); },
                [target_of](auto c) { return std::ranges::count_if(c.range(), raze::algorithm::make_not_fn(not_equal_pred<T>{ target_of(c) })); });
        };

        static_assert(check_ce(raze::algorithm::count_if_not));
        static_assert(check_ce(raze::algorithm::count_if_not[opt::aligned<64>]));

        static_assert(constexpr_run<T, n, 5, 3>(
            [target_of](auto c) {
                T target = target_of(c);
                return raze::algorithm::count_if_not[opt::fautovec](c.range(), [target](auto x) constexpr { if (x != target) return true; return false; });
            },
            [target_of](auto c) {
                T target = target_of(c);
                return std::ranges::count_if(c.range(), raze::algorithm::make_not_fn([target](auto x) constexpr { if (x != target) return true; return false; }));
            }));

        static_assert([]() constexpr {
            std::array<T, 0> a0{};
            std::array<T, 4> a4{ T(1), T(2), T(1), T(4) };
            std::array<T, 5> a5{ T(1), T(2), T(3), T(4), T(5) };
            std::array<T, 5> b5{ T(1), T(2), T(1), T(4), T(1) };
            return raze::algorithm::count_if_not(a0, false_pred{}) == 0 &&
                   raze::algorithm::count_if_not(a4, eq_pred<T>{ T(1) }) == 2 &&
                   raze::algorithm::count_if_not(a4, true_pred{}) == 0 &&
                   raze::algorithm::count_if_not[opt::fscalar](a5, [](auto x) { return x <= T(2); }) == 3 &&
                   raze::algorithm::count_if_not(b5.begin() + 1, b5.begin() + 4, eq_pred<T>{ T(1) }) == 2;
        }());
    }
}

RTTS_CASE_TPL("raze::algorithm::count_if_not", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    count_if_not_suite<T>();
};

RTTS_CASE_TPL("raze::algorithm::count_if_not boundaries", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    count_if_not_boundary_sweep<T>();
};

RTTS_CASE_TPL("raze::algorithm::count_if_not fuzz", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    for (unsigned seed : fuzz_seeds)
        count_if_not_fuzz<T>(seed, 200);
};

RTTS_CASE("raze::algorithm::count_if_not structs and projections") {
    const std::vector<count_if_not_item> input{
        { 1, 10 }, { 2, 20 }, { 1, 30 }, { 4, 20 }, { 1, 50 }
    };

    RTTS_EXPECT(raze::algorithm::count_if_not(
        input,
        [](int32_t k) { return k != 1; },
        [](const auto& item) { return item.key; }) == 3);

    RTTS_EXPECT(raze::algorithm::count_if_not[raze::options::fscalar](
        input,
        [](int32_t v) { return v != 20; },
        [](const auto& item) { return item.value; }) == 2);

    RTTS_EXPECT(raze::algorithm::count_if_not(
        input,
        [](int32_t v) { return v > 0; },
        [](const auto& item) { return item.value; }) == 0);

    const std::vector<std::string> words{ "aa", "bbb", "cc", "dddd", "ee" };
    RTTS_EXPECT(raze::algorithm::count_if_not(words, [](const std::string& s) { return s.size() != 2; }) == 3);
};

RTTS_CASE("raze::algorithm::count_if_not sentinel") {
    {
        std::array<int, 3> empty_arr{ 1, 1, 1 };
        auto empty_first = std::counted_iterator(empty_arr.begin(), 0);
        RTTS_EXPECT(raze::algorithm::count_if_not(empty_first, std::default_sentinel, raze::algorithm::equal_to(0)) == 0);
    }

    for (size_t n : { size_t(16), size_t(64), size_t(300) }) {
        std::vector<int> v(n, 1);
        auto first = std::counted_iterator(v.begin(), static_cast<std::ptrdiff_t>(v.size()));

        RTTS_EXPECT(raze::algorithm::count_if_not(first, std::default_sentinel, raze::algorithm::equal_to(1)) == 0);
        RTTS_EXPECT(raze::algorithm::count_if_not(first, std::default_sentinel, raze::algorithm::equal_to(42)) == static_cast<std::ptrdiff_t>(n));

        v[0] = 42;
        v[n / 2] = 42;
        v[n - 1] = 42;
        RTTS_EXPECT(raze::algorithm::count_if_not(first, std::default_sentinel, raze::algorithm::equal_to(1)) == 3);
    }
};


RTTS_CASE("raze::algorithm::count_if_not views and edge cases") {
    namespace opt = raze::options;
    namespace views = std::views;

    // Views: iota, take, drop, reverse, filter, transform, chained, non-common
    {
        auto pred_branch = [](auto x) { if ((x & 1) == 0) return true; return false; };
        RTTS_EXPECT(raze::algorithm::count_if_not[opt::fautovec][opt::fstatic](views::iota(1, 21), pred_branch) == 10);

        const std::vector<int> src = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
        auto chain = src | views::filter([](int x) { return x > 2; }) | views::transform([](int x) { return x * 3; }) | views::reverse;
        RTTS_EXPECT(raze::algorithm::count_if_not[opt::aligned<64>](chain, [](int x) { return x > 18; }) == 4);

        RTTS_EXPECT(raze::algorithm::count_if_not(rtts::make_non_common(src), [](auto x) { return x > 3; }) == 3);
    }

    // Empty, 1-elem, single-pass, volatile, member projection
    {
        std::vector<int> empty_v;
        RTTS_EXPECT(raze::algorithm::count_if_not(empty_v, [](auto x) { return x > 0; }) == 0);

        const std::vector<int> data = { 1, 2, 3, 4, 5, 6 };
        rtts::single_pass_input_range<int> spr(data);
        RTTS_EXPECT(raze::algorithm::count_if_not(spr, [](int x) { return (x & 1) == 0; }) == 3 && spr.reads() == data.size());

        volatile int vbuf[16];
        for (int i = 0; i < 16; ++i) vbuf[i] = i;
        RTTS_EXPECT(raze::algorithm::count_if_not(vbuf, vbuf + 16, [](int x) { return x < 10; }) == 6);

        const std::vector<rtts::test_item> items = { { 1, 10 }, { 5, 50 }, { 6, 60 }, { 2, 20 } };
        RTTS_EXPECT(raze::algorithm::count_if_not(items, [](int k) { return k >= 5; }, &rtts::test_item::key) == 2);
    }
};