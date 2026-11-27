#include <tests/rts/rts.h>
#include <raze/algorithm/find/FindIfNot.h>
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

    struct find_if_not_item {
        int32_t key;
        int32_t value;
        bool operator==(const find_if_not_item&) const = default;
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
    void check_find_if_not_variants(const std::vector<T>& input, T keep) {
        auto pred = [keep](auto x) { return x != keep; };
        auto scalar_pred = [keep](T x) { return x != keep; };

        auto exp_it = std::ranges::find_if_not(input, pred);
        const auto exp_off = exp_it - input.begin();

        RTTS_EXPECT(raze::algorithm::find_if_not(input, pred) - input.begin() == exp_off);
        RTTS_EXPECT(raze::algorithm::find_if_not(input.begin(), input.end(), pred) - input.begin() == exp_off);
        RTTS_EXPECT(raze::algorithm::find_if_not(input, scalar_pred) - input.begin() == exp_off);
        RTTS_EXPECT(raze::algorithm::find_if_not[raze::options::fscalar](input, pred) - input.begin() == exp_off);
        RTTS_EXPECT(raze::algorithm::find_if_not[raze::options::unroll<2>](input, pred) - input.begin() == exp_off);
        RTTS_EXPECT(raze::algorithm::find_if_not[raze::options::fstatic](input, pred) - input.begin() == exp_off);
    }

    template <class T>
    void check_find_if_not_vec_variants(const std::vector<T>& input, T keep) {
        auto vec_pred = raze::algorithm::equal_to(keep);

        auto exp_it = std::ranges::find_if_not(input, vec_pred);
        const auto exp_off = exp_it - input.begin();

        RTTS_EXPECT(raze::algorithm::find_if_not(input, vec_pred) - input.begin() == exp_off);
        RTTS_EXPECT(raze::algorithm::find_if_not(input.begin(), input.end(), vec_pred) - input.begin() == exp_off);
        RTTS_EXPECT(raze::algorithm::find_if_not[raze::options::fscalar](input, vec_pred) - input.begin() == exp_off);
        RTTS_EXPECT(raze::algorithm::find_if_not[raze::options::unroll<2>](input, vec_pred) - input.begin() == exp_off);
        RTTS_EXPECT(raze::algorithm::find_if_not[raze::options::fstatic](input, vec_pred) - input.begin() == exp_off);
    }

    template <class T>
    void find_if_not_boundary_sweep() {
        std::vector<size_t> sizes;
        for (size_t n = 0; n <= 140; ++n)
            sizes.push_back(n);
        for (size_t n : { size_t(255), size_t(256), size_t(257), size_t(511), size_t(512), size_t(513), size_t(1024) })
            sizes.push_back(n);

        for (size_t n : sizes) {
            std::vector<T> base(n);
            for (size_t i = 0; i < n; ++i)
                base[i] = static_cast<T>((i % 100) + 1);

            check_find_if_not_variants(base, T(0));

            std::vector<T> all_ones(n, T(1));
            check_find_if_not_vec_variants(all_ones, T(1));

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
                    check_find_if_not_variants(input, T(0));

                    auto vec_input = all_ones;
                    vec_input[pos] = T(0);
                    check_find_if_not_vec_variants(vec_input, T(1));
                }
            }
        }
    }

    template <class T>
    void find_if_not_fuzz(unsigned seed, size_t iters) {
        std::mt19937 rng(seed);
        std::uniform_int_distribution<size_t> sz(0, 512);

        for (size_t i = 0; i < iters; ++i) {
            const size_t n = sz(rng);
            auto input = rtts::random::vector<T>(n, seed + static_cast<unsigned>(i));
            const T keep = n == 0 ? T(0) : input[rng() % n];

            auto vec_pred = raze::algorithm::equal_to(keep);
            auto exp_it = std::ranges::find_if_not(input, vec_pred);

            RTTS_EXPECT(raze::algorithm::find_if_not(input, vec_pred) == exp_it);
            RTTS_EXPECT(raze::algorithm::find_if_not[raze::options::fscalar](input.begin(), input.end(), vec_pred) == exp_it);
            RTTS_EXPECT(raze::algorithm::find_if_not[raze::options::fstatic](input, vec_pred) == exp_it);

            std::vector<T> all_keep(n, keep);
            RTTS_EXPECT(raze::algorithm::find_if_not(all_keep, vec_pred) == all_keep.end());
            RTTS_EXPECT(raze::algorithm::find_if_not[raze::options::fscalar](all_keep, vec_pred) == all_keep.end());
            RTTS_EXPECT(raze::algorithm::find_if_not[raze::options::fstatic](all_keep, vec_pred) == all_keep.end());

            const size_t first = n == 0 ? 0 : rng() % (n + 1);
            const size_t last = first + (n == first ? 0 : rng() % (n - first + 1));
            auto sub_exp = std::ranges::find_if_not(input.begin() + first, input.begin() + last, vec_pred);
            RTTS_EXPECT(raze::algorithm::find_if_not(input.begin() + first, input.begin() + last, vec_pred) == sub_exp);
        }
    }

    template <class T>
    void find_if_not_suite() {
        using namespace rtts::algorithm;
        namespace opt = raze::options;
        auto cfg = config::thorough();

        constexpr auto target_of = [](auto& c) {
            return *std::ranges::next(c.first(), static_cast<std::ptrdiff_t>(c.count()));
        };

        auto check_iter = [&](auto algo) {
            run<n, T>(cfg,
                [algo, target_of](auto c) { return algo(c.first(), c.last(), not_equal_pred<T>{ target_of(c) }); },
                [target_of](auto c) { return std::ranges::find_if_not(c.first(), c.last(), not_equal_pred<T>{ target_of(c) }); });
        };

        auto check_range = [&](auto algo) {
            run<n, T>(cfg,
                [algo, target_of](auto c) { return algo(c.range(), not_equal_pred<T>{ target_of(c) }); },
                [target_of](auto c) { return std::ranges::find_if_not(c.range(), not_equal_pred<T>{ target_of(c) }); });
        };

        check_iter(raze::algorithm::find_if_not);
        check_range(raze::algorithm::find_if_not);
        check_iter(raze::algorithm::find_if_not[opt::fscalar]);
        check_range(raze::algorithm::find_if_not[opt::unroll<2>]);
        check_range(raze::algorithm::find_if_not[opt::fstatic]);
        check_iter(raze::algorithm::find_if_not[opt::fautovec]);

        run<range, T>(cfg,
            [](auto c) { return raze::algorithm::find_if_not(c.first(), c.last(), true_pred{}); },
            [](auto c) { return std::ranges::find_if_not(c.first(), c.last(), true_pred{}); });

        run<n, T>(cfg,
            [target_of](auto c) {
                T target = target_of(c);
                return raze::algorithm::find_if_not[opt::fautovec][opt::fstatic](c.range(), [target](auto x) { if (x != target) return true; return false; });
            },
            [target_of](auto c) {
                T target = target_of(c);
                return std::ranges::find_if_not(c.range(), [target](auto x) { if (x != target) return true; return false; });
            });

        {
            auto acfg = config::compact();

            auto check_aligned = [&]<size_t Align>(auto algo) {
                run_aligned<Align, n, T>(acfg,
                    [algo, target_of](auto c) { return algo(c.first(), c.last(), not_equal_pred<T>{ target_of(c) }); },
                    [target_of](auto c) { return std::ranges::find_if_not(c.first(), c.last(), not_equal_pred<T>{ target_of(c) }); });

                run_aligned<Align, n, T>(acfg,
                    [algo, target_of](auto c) { return algo(c.range(), not_equal_pred<T>{ target_of(c) }); },
                    [target_of](auto c) { return std::ranges::find_if_not(c.range(), not_equal_pred<T>{ target_of(c) }); });
            };

            check_aligned.template operator()<16>(raze::algorithm::find_if_not[opt::aligned<16>]);
            check_aligned.template operator()<32>(raze::algorithm::find_if_not[opt::aligned<32>]);
            check_aligned.template operator()<32>(raze::algorithm::find_if_not[opt::aligned<32>][opt::unroll<2>]);
            check_aligned.template operator()<32>(raze::algorithm::find_if_not[opt::aligned<32>][opt::fstatic]);
            check_aligned.template operator()<64>(raze::algorithm::find_if_not[opt::aligned<64>]);
            check_aligned.template operator()<64>(raze::algorithm::find_if_not[opt::aligned<64>][opt::fautovec]);
            check_aligned.template operator()<64>(raze::algorithm::find_if_not[opt::aligned<64>][opt::fscalar]);
            check_aligned.template operator()<1>(raze::algorithm::find_if_not[opt::aligned<opt::unaligned>]);
            check_aligned.template operator()<1>(raze::algorithm::find_if_not[opt::aligned<64>][opt::aligned<opt::unaligned>]);
        }

        static_assert(constexpr_run<T, range, 0>(
            [](auto c) { return raze::algorithm::find_if_not(c.first(), c.last(), false_pred{}); },
            [](auto c) { return std::ranges::find_if_not(c.first(), c.last(), false_pred{}); }));

        constexpr auto check_ce = [target_of](auto algo) {
            return constexpr_run<T, n, 5, 3>(
                [algo, target_of](auto c) { return algo(c.range(), not_equal_pred<T>{ target_of(c) }); },
                [target_of](auto c) { return std::ranges::find_if_not(c.range(), not_equal_pred<T>{ target_of(c) }); });
        };

        static_assert(check_ce(raze::algorithm::find_if_not));
        static_assert(check_ce(raze::algorithm::find_if_not[opt::aligned<64>]));

        static_assert(constexpr_run<T, n, 5, 3>(
            [target_of](auto c) {
                T target = target_of(c);
                return raze::algorithm::find_if_not[opt::fautovec](c.range(), [target](auto x) constexpr { if (x != target) return true; return false; });
            },
            [target_of](auto c) {
                T target = target_of(c);
                return std::ranges::find_if_not(c.range(), [target](auto x) constexpr { if (x != target) return true; return false; });
            }));

        static_assert([]() constexpr {
            std::array<T, 0> a0{};
            std::array<T, 4> a4{ T(1), T(1), T(3), T(1) };
            std::array<T, 4> b4{ T(1), T(1), T(1), T(4) };
            std::array<T, 5> a5{ T(1), T(2), T(3), T(4), T(5) };
            return raze::algorithm::find_if_not(a0, false_pred{}) == a0.end() &&
                   raze::algorithm::find_if_not(a4, eq_pred<T>{ T(1) }) == a4.begin() + 2 &&
                   raze::algorithm::find_if_not(a4, true_pred{}) == a4.end() &&
                   raze::algorithm::find_if_not[opt::fscalar](a5, [](auto x) { return x <= T(4); }) == a5.begin() + 4 &&
                   raze::algorithm::find_if_not(b4.begin() + 1, b4.begin() + 3, eq_pred<T>{ T(1) }) == b4.begin() + 3;
        }());
    }
}

RTTS_CASE_TPL("raze::algorithm::find_if_not", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    find_if_not_suite<T>();
};

RTTS_CASE_TPL("raze::algorithm::find_if_not boundaries", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    find_if_not_boundary_sweep<T>();
};

RTTS_CASE_TPL("raze::algorithm::find_if_not fuzz", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    for (unsigned seed : fuzz_seeds)
        find_if_not_fuzz<T>(seed, 200);
};

RTTS_CASE("raze::algorithm::find_if_not structs and projections") {
    const std::vector<find_if_not_item> input{
        { 1, 10 }, { 2, 10 }, { 3, 30 }, { 4, 10 }
    };

    auto it = raze::algorithm::find_if_not(
        input,
        [](int32_t v) { return v == 10; },
        [](const auto& item) { return item.value; });
    RTTS_EXPECT(it == input.begin() + 2);

    it = raze::algorithm::find_if_not[raze::options::fscalar](
        input,
        [](int32_t k) { return k < 4; },
        [](const auto& item) { return item.key; });
    RTTS_EXPECT(it == input.begin() + 3);

    const std::vector<std::string> words{ "aa", "bb", "gamma", "cc" };
    auto wit = raze::algorithm::find_if_not(words, [](const std::string& s) { return s.size() == 2; });
    RTTS_EXPECT(wit == words.begin() + 2);
};

RTTS_CASE("raze::algorithm::find_if_not sentinel") {
    for (size_t n : { size_t(16), size_t(64), size_t(300) }) {
        std::vector<int> v(n, 1);
        v[n - 1] = 42;

        auto first = std::counted_iterator(v.begin(), static_cast<std::ptrdiff_t>(v.size()));
        auto res = raze::algorithm::find_if_not(first, std::default_sentinel, raze::algorithm::equal_to(1));
        RTTS_EXPECT(res.base() == v.begin() + (n - 1));

        std::vector<int> all_ones(n, 1);
        auto miss_first = std::counted_iterator(all_ones.begin(), static_cast<std::ptrdiff_t>(all_ones.size()));
        auto miss = raze::algorithm::find_if_not(miss_first, std::default_sentinel, raze::algorithm::equal_to(1));
        RTTS_EXPECT(miss == std::default_sentinel);
    }
};

RTTS_CASE("raze::algorithm::find_if_not views and edge cases") {
    namespace opt = raze::options;
    namespace views = std::views;

    {
        auto pred_branch = [](auto x) { if (x < 20) return true; return false; };
        auto iv = views::iota(1, 40);
        RTTS_EXPECT(raze::algorithm::find_if_not[opt::fautovec][opt::fstatic](iv, pred_branch) == std::ranges::find_if_not(iv, pred_branch));

        const std::vector<int> src = { 2, 4, 6, 8, 9, 10, 12, 14 };
        auto dv = src | views::drop(1) | views::take(6);
        RTTS_EXPECT(raze::algorithm::find_if_not[opt::fautovec][opt::fstatic](dv, [](auto x) { if ((x & 1) == 0) return true; return false; }) ==
                    std::ranges::find_if_not(dv, [](int x) { return (x & 1) == 0; }));

        auto chain = src | views::reverse | views::filter([](int x) { return x > 4; }) | views::transform([](int x) { return x + 1; });
        RTTS_EXPECT(raze::algorithm::find_if_not[opt::aligned<64>](chain, [](int x) { return (x & 1) == 1; }) ==
                    std::ranges::find_if_not(chain, [](int x) { return (x & 1) == 1; }));

        auto nc = rtts::make_non_common(src);
        RTTS_EXPECT(raze::algorithm::find_if_not(nc, [](auto x) { return (x & 1) == 0; }) == std::ranges::find_if_not(nc, [](int x) { return (x & 1) == 0; }));
    }

    {
        std::vector<int> empty_v;
        RTTS_EXPECT(raze::algorithm::find_if_not(empty_v, [](auto x) { return x > 0; }) == empty_v.end());

        const std::vector<int> data = { 2, 4, 6, 7, 8 };
        rtts::single_pass_input_range<int> spr(data);
        auto it = raze::algorithm::find_if_not(spr, [](int x) { return (x & 1) == 0; });
        RTTS_EXPECT(it != spr.end() && *it == 7 && spr.reads() == 3);

        volatile int vbuf[16];
        for (int i = 0; i < 16; ++i) vbuf[i] = i;
        RTTS_EXPECT(raze::algorithm::find_if_not(vbuf, vbuf + 16, [](int x) { return x < 10; }) == vbuf + 10);

        auto rv = raze::algorithm::find_if_not(std::vector<int>{ 2, 4, 5 }, [](auto x) { return (x & 1) == 0; });
        static_assert(std::same_as<decltype(rv), std::ranges::dangling>);

        const std::vector<rtts::test_item> items = { { 2, 10 }, { 4, 20 }, { 5, 30 } };
        RTTS_EXPECT(raze::algorithm::find_if_not(items, [](int k) { return (k & 1) == 0; }, &rtts::test_item::key) == items.begin() + 2);
    }
};
