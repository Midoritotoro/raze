#include <tests/rts/rts.h>
#include <raze/algorithm/find/FindIf.h>
#include <src/raze/algorithm/EqualTo.h>

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

    struct find_if_item {
        int32_t key;
        int32_t value;
        bool operator==(const find_if_item&) const = default;
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
    void check_find_if_variants(const std::vector<T>& input, T target) {
        namespace opt = raze::options;
        auto pred = [target](auto x) { return x == target; };
        auto scalar_pred = [target](T x) { return x == target; };
        auto vec_pred = raze::algorithm::equal_to(target);

        const auto exp_off = std::ranges::find_if(input, pred) - input.begin();

        auto check_pred = [&](auto p) {
            RTTS_EXPECT(raze::algorithm::find_if(input, p) - input.begin() == exp_off);
            RTTS_EXPECT(raze::algorithm::find_if(input.begin(), input.end(), p) - input.begin() == exp_off);
            RTTS_EXPECT(raze::algorithm::find_if[opt::fscalar](input, p) - input.begin() == exp_off);
            RTTS_EXPECT(raze::algorithm::find_if[opt::unroll<2>](input, p) - input.begin() == exp_off);
            RTTS_EXPECT(raze::algorithm::find_if[opt::fstatic](input, p) - input.begin() == exp_off);
        };

        check_pred(pred);
        check_pred(vec_pred);
        RTTS_EXPECT(raze::algorithm::find_if(input, scalar_pred) - input.begin() == exp_off);
    }

    template <class T>
    void find_if_boundary_sweep() {
        std::vector<size_t> sizes;
        for (size_t n = 0; n <= 140; ++n)
            sizes.push_back(n);
        for (size_t n : { size_t(255), size_t(256), size_t(257), size_t(511), size_t(512), size_t(513), size_t(1024) })
            sizes.push_back(n);

        for (size_t n : sizes) {
            std::vector<T> base(n);
            for (size_t i = 0; i < n; ++i)
                base[i] = static_cast<T>((i % 100) + 1);

            check_find_if_variants(base, T(0));

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
                    check_find_if_variants(input, T(0));
                }
            }
        }
    }

    template <class T>
    void find_if_fuzz(unsigned seed, size_t iters) {
        std::mt19937 rng(seed);
        std::uniform_int_distribution<size_t> sz(0, 512);

        for (size_t i = 0; i < iters; ++i) {
            const size_t n = sz(rng);
            auto input = rtts::random::vector<T>(n, seed + static_cast<unsigned>(i));
            const T target = n == 0 ? T(0) : input[rng() % n];

            auto vec_pred = raze::algorithm::equal_to(target);
            auto exp_it = std::ranges::find_if(input, vec_pred);

            RTTS_EXPECT(raze::algorithm::find_if(input, vec_pred) == exp_it);
            RTTS_EXPECT(raze::algorithm::find_if[raze::options::fscalar](input.begin(), input.end(), vec_pred) == exp_it);
            RTTS_EXPECT(raze::algorithm::find_if[raze::options::fstatic](input, vec_pred) == exp_it);

            auto no_zero = input;
            for (auto& x : no_zero)
                if (x == T(0)) x = T(1);

            auto miss_pred = raze::algorithm::equal_to(T(0));
            RTTS_EXPECT(raze::algorithm::find_if(no_zero, miss_pred) == no_zero.end());
            RTTS_EXPECT(raze::algorithm::find_if[raze::options::fscalar](no_zero, miss_pred) == no_zero.end());
            RTTS_EXPECT(raze::algorithm::find_if[raze::options::fstatic](no_zero, miss_pred) == no_zero.end());

            const size_t first = n == 0 ? 0 : rng() % (n + 1);
            const size_t last = first + (n == first ? 0 : rng() % (n - first + 1));
            auto sub_exp = std::ranges::find_if(input.begin() + first, input.begin() + last, vec_pred);
            RTTS_EXPECT(raze::algorithm::find_if(input.begin() + first, input.begin() + last, vec_pred) == sub_exp);
        }
    }

    template <class T>
    void find_if_suite() {
        using namespace rtts::algorithm;
        namespace opt = raze::options;
        auto cfg = config::thorough();

        constexpr auto target_of = [](auto& c) {
            return *std::ranges::next(c.first(), static_cast<std::ptrdiff_t>(c.count()));
        };

        auto check_iter = [&](auto algo) {
            run<n, T>(cfg,
                [algo, target_of](auto c) { return algo(c.first(), c.last(), equal_pred<T>{ target_of(c) }); },
                [target_of](auto c) { return std::ranges::find_if(c.first(), c.last(), equal_pred<T>{ target_of(c) }); });
        };

        auto check_range = [&](auto algo) {
            run<n, T>(cfg,
                [algo, target_of](auto c) { return algo(c.range(), equal_pred<T>{ target_of(c) }); },
                [target_of](auto c) { return std::ranges::find_if(c.range(), equal_pred<T>{ target_of(c) }); });
        };

        check_iter(raze::algorithm::find_if);
        check_range(raze::algorithm::find_if);
        check_iter(raze::algorithm::find_if[opt::fscalar]);
        check_range(raze::algorithm::find_if[opt::unroll<2>]);
        check_range(raze::algorithm::find_if[opt::fstatic]);
        check_iter(raze::algorithm::find_if[opt::fautovec]);

        run<range, T>(cfg,
            [](auto c) { return raze::algorithm::find_if(c.first(), c.last(), false_pred{}); },
            [](auto c) { return std::ranges::find_if(c.first(), c.last(), false_pred{}); });

        run<n, T>(cfg,
            [target_of](auto c) {
                T target = target_of(c);
                return raze::algorithm::find_if[opt::fautovec][opt::fstatic](c.range(), [target](auto x) { if (x == target) return true; return false; });
            },
            [target_of](auto c) {
                T target = target_of(c);
                return std::ranges::find_if(c.range(), [target](auto x) { if (x == target) return true; return false; });
            });

        {
            auto acfg = config::compact();

            auto check_aligned = [&]<size_t Align>(auto algo) {
                run_aligned<Align, n, T>(acfg,
                    [algo, target_of](auto c) { return algo(c.first(), c.last(), equal_pred<T>{ target_of(c) }); },
                    [target_of](auto c) { return std::ranges::find_if(c.first(), c.last(), equal_pred<T>{ target_of(c) }); });

                run_aligned<Align, n, T>(acfg,
                    [algo, target_of](auto c) { return algo(c.range(), equal_pred<T>{ target_of(c) }); },
                    [target_of](auto c) { return std::ranges::find_if(c.range(), equal_pred<T>{ target_of(c) }); });
            };

            check_aligned.template operator()<16>(raze::algorithm::find_if[opt::aligned<16>]);
            check_aligned.template operator()<32>(raze::algorithm::find_if[opt::aligned<32>]);
            check_aligned.template operator()<32>(raze::algorithm::find_if[opt::aligned<32>][opt::unroll<2>]);
            check_aligned.template operator()<32>(raze::algorithm::find_if[opt::aligned<32>][opt::fstatic]);
            check_aligned.template operator()<64>(raze::algorithm::find_if[opt::aligned<64>]);
            check_aligned.template operator()<64>(raze::algorithm::find_if[opt::aligned<64>][opt::fautovec]);
            check_aligned.template operator()<64>(raze::algorithm::find_if[opt::aligned<64>][opt::fscalar]);
            check_aligned.template operator()<1>(raze::algorithm::find_if[opt::aligned<opt::unaligned>]);
            check_aligned.template operator()<1>(raze::algorithm::find_if[opt::aligned<64>][opt::aligned<opt::unaligned>]);
        }

        static_assert(constexpr_run<T, range, 0>(
            [](auto c) { return raze::algorithm::find_if(c.first(), c.last(), true_pred{}); },
            [](auto c) { return std::ranges::find_if(c.first(), c.last(), true_pred{}); }));

        constexpr auto check_ce = [target_of](auto algo) {
            return constexpr_run<T, n, 5, 3>(
                [algo, target_of](auto c) { return algo(c.range(), equal_pred<T>{ target_of(c) }); },
                [target_of](auto c) { return std::ranges::find_if(c.range(), equal_pred<T>{ target_of(c) }); });
        };

        static_assert(check_ce(raze::algorithm::find_if));
        static_assert(check_ce(raze::algorithm::find_if[opt::aligned<64>]));

        static_assert(constexpr_run<T, n, 5, 3>(
            [target_of](auto c) {
                T target = target_of(c);
                return raze::algorithm::find_if[opt::fautovec](c.range(), [target](auto x) constexpr { if (x == target) return true; return false; });
            },
            [target_of](auto c) {
                T target = target_of(c);
                return std::ranges::find_if(c.range(), [target](auto x) constexpr { if (x == target) return true; return false; });
            }));

        static_assert([]() constexpr {
            std::array<T, 0> a0{};
            std::array<T, 4> a4{ T(1), T(2), T(3), T(4) };
            std::array<T, 5> a5{ T(1), T(2), T(3), T(4), T(5) };
            return raze::algorithm::find_if(a0, true_pred{}) == a0.end() &&
                   raze::algorithm::find_if(a4, equal_pred<T>{ T(3) }) == a4.begin() + 2 &&
                   raze::algorithm::find_if[opt::fscalar](a5, [](auto x) { return x > T(4); }) == a5.begin() + 4 &&
                   raze::algorithm::find_if(a4.begin() + 1, a4.begin() + 3, equal_pred<T>{ T(4) }) == a4.begin() + 3;
        }());
    }
}

RTTS_CASE_TPL("raze::algorithm::find_if", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    find_if_suite<T>();
};

RTTS_CASE_TPL("raze::algorithm::find_if boundaries", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    find_if_boundary_sweep<T>();
};

RTTS_CASE_TPL("raze::algorithm::find_if fuzz", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    for (unsigned seed : fuzz_seeds)
        find_if_fuzz<T>(seed, 200);
};

RTTS_CASE("raze::algorithm::find_if structs and projections") {
    const std::vector<find_if_item> input{
        { 1, 10 }, { 2, 20 }, { 3, 30 }, { 4, 40 }, { 5, 50 }
    };

    auto it = raze::algorithm::find_if(
        input,
        [](int32_t k) { return k == 3; },
        [](const auto& item) { return item.key; });
    RTTS_EXPECT(it == input.begin() + 2);

    it = raze::algorithm::find_if[raze::options::fscalar](
        input,
        [](int32_t v) { return v == 50; },
        [](const auto& item) { return item.value; });
    RTTS_EXPECT(it == input.begin() + 4);

    it = raze::algorithm::find_if(
        input,
        [](int32_t v) { return v == 999; },
        [](const auto& item) { return item.value; });
    RTTS_EXPECT(it == input.end());

    const std::vector<std::string> words{ "alpha", "beta", "gamma", "delta" };
    auto wit = raze::algorithm::find_if(words, [](const std::string& s) { return s.size() == 5 && s[0] == 'g'; });
    RTTS_EXPECT(wit == words.begin() + 2);
};

RTTS_CASE("raze::algorithm::find_if sentinel") {
    for (size_t n : { size_t(16), size_t(64), size_t(300) }) {
        std::vector<int> v(n, 1);
        v[n - 1] = 42;

        auto first = std::counted_iterator(v.begin(), static_cast<std::ptrdiff_t>(v.size()));
        auto res = raze::algorithm::find_if(first, std::default_sentinel, raze::algorithm::equal_to(42));
        RTTS_EXPECT(res.base() == v.begin() + (n - 1));

        auto miss = raze::algorithm::find_if(first, std::default_sentinel, raze::algorithm::equal_to(777));
        RTTS_EXPECT(miss == std::default_sentinel);
    }
};

RTTS_CASE("raze::algorithm::find_if views and edge cases") {
    namespace opt = raze::options;
    namespace views = std::views;

    {
        auto pred_branch = [](auto x) { if (x > 25) return true; return false; };
        auto iv = views::iota(1, 50);
        RTTS_EXPECT(raze::algorithm::find_if[opt::fautovec][opt::fstatic](iv, pred_branch) == std::ranges::find_if(iv, pred_branch));

        const std::vector<int> src = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };
        auto dv = src | views::drop(2) | views::take(10);
        RTTS_EXPECT(raze::algorithm::find_if[opt::fautovec][opt::fstatic](dv, [](auto x) { if (x == 9) return true; return false; }) ==
                    std::ranges::find_if(dv, [](int x) { return x == 9; }));

        auto chain = src | views::reverse | views::filter([](int x) { return (x % 2) == 0; }) | views::transform([](int x) { return x * 10; });
        RTTS_EXPECT(raze::algorithm::find_if[opt::aligned<64>](chain, [](int x) { return x <= 60; }) ==
                    std::ranges::find_if(chain, [](int x) { return x <= 60; }));

        auto nc = rtts::make_non_common(src);
        RTTS_EXPECT(raze::algorithm::find_if(nc, [](auto x) { return x > 10; }) == std::ranges::find_if(nc, [](int x) { return x > 10; }));
    }

    {
        std::vector<int> empty_v;
        RTTS_EXPECT(raze::algorithm::find_if(empty_v, [](auto x) { return x > 0; }) == empty_v.end());

        const std::vector<int> data = { 1, 3, 5, 8, 9, 11 };
        rtts::single_pass_input_range<int> spr(data);
        auto it = raze::algorithm::find_if(spr, [](int x) { return (x % 2) == 0; });
        RTTS_EXPECT(it != spr.end() && *it == 8 && spr.reads() == 3);

        volatile int vbuf[16];
        for (int i = 0; i < 16; ++i) vbuf[i] = i;
        RTTS_EXPECT(raze::algorithm::find_if(vbuf, vbuf + 16, [](int x) { return x == 11; }) == vbuf + 11);

        auto rv = raze::algorithm::find_if(std::vector<int>{ 1, 2, 3 }, [](auto x) { return x > 1; });
        static_assert(std::same_as<decltype(rv), std::ranges::dangling>);

        const std::vector<rtts::test_item> items = { { 1, 10 }, { 5, 50 }, { 9, 90 } };
        RTTS_EXPECT(raze::algorithm::find_if(items, [](int k) { return k >= 5; }, &rtts::test_item::key) == items.begin() + 1);
    }
};
