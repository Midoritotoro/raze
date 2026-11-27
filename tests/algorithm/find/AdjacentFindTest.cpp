#include <tests/rts/rts.h>
#include <raze/algorithm/find/AdjacentFind.h>

#include <algorithm>
#include <array>
#include <cstdint>
#include <forward_list>
#include <functional>
#include <iterator>
#include <list>
#include <random>
#include <ranges>
#include <string>
#include <vector>

namespace {
    inline constexpr unsigned fuzz_seeds[] = { 42u, 0x9E3779B9u, 0xDEADBEEFu };

    struct adjacent_find_item {
        int32_t key;
        int32_t value;
        bool operator==(const adjacent_find_item&) const = default;
    };

    struct abs_diff_one_pred {
        constexpr bool operator()(int32_t a, int32_t b) const {
            return (a - b) == 1 || (b - a) == 1;
        }
    };

    template <class T>
    void check_adjacent_find_variants(const std::vector<T>& input) {
        auto exp_it = std::ranges::adjacent_find(input);
        const auto exp_off = exp_it - input.begin();

        RTTS_EXPECT(raze::algorithm::adjacent_find(input) - input.begin() == exp_off);
        RTTS_EXPECT(raze::algorithm::adjacent_find(input.begin(), input.end()) - input.begin() == exp_off);
        RTTS_EXPECT(raze::algorithm::adjacent_find[raze::options::fscalar](input) - input.begin() == exp_off);
        RTTS_EXPECT(raze::algorithm::adjacent_find[raze::options::unroll<2>](input) - input.begin() == exp_off);
        RTTS_EXPECT(raze::algorithm::adjacent_find[raze::options::fstatic](input) - input.begin() == exp_off);

        auto lambda_pred = [](T a, T b) { return a == b; };
        RTTS_EXPECT(raze::algorithm::adjacent_find(input, lambda_pred) - input.begin() == exp_off);
        RTTS_EXPECT(raze::algorithm::adjacent_find[raze::options::fscalar](input, lambda_pred) - input.begin() == exp_off);

        auto exp_gt = std::ranges::adjacent_find(input, std::greater<>{});
        const auto exp_gt_off = exp_gt - input.begin();
        RTTS_EXPECT(raze::algorithm::adjacent_find(input, std::greater<>{}) - input.begin() == exp_gt_off);
        RTTS_EXPECT(raze::algorithm::adjacent_find[raze::options::fscalar](input, std::greater<>{}) - input.begin() == exp_gt_off);
    }

    template <class T>
    void adjacent_find_boundary_sweep() {
        std::vector<size_t> sizes;
        for (size_t n = 0; n <= 140; ++n)
            sizes.push_back(n);
        for (size_t n : { size_t(255), size_t(256), size_t(257), size_t(258),
                          size_t(511), size_t(512), size_t(513), size_t(514), size_t(1024) })
        {
            sizes.push_back(n);
        }

        for (size_t n : sizes) {
            std::vector<T> base(n);
            for (size_t i = 0; i < n; ++i)
                base[i] = static_cast<T>((i % 100) + 1);

            check_adjacent_find_variants(base);

            if (n >= 2) {
                std::vector<size_t> positions;
                if (n <= 64) {
                    for (size_t p = 0; p + 1 < n; ++p)
                        positions.push_back(p);
                }
                else {
                    for (size_t p : {
                        size_t(0), size_t(1), size_t(14), size_t(15), size_t(16), size_t(17),
                        size_t(30), size_t(31), size_t(32), size_t(33),
                        size_t(62), size_t(63), size_t(64), size_t(65),
                        n / 2, n - 3, n - 2 })
                    {
                        if (p + 1 < n)
                            positions.push_back(p);
                    }
                }

                for (size_t pos : positions) {
                    auto input = base;
                    input[pos + 1] = input[pos];
                    check_adjacent_find_variants(input);
                }
            }
        }
    }

    template <class T>
    void adjacent_find_fuzz(unsigned seed, size_t iters) {
        std::mt19937 rng(seed);
        std::uniform_int_distribution<size_t> sz(0, 512);

        for (size_t i = 0; i < iters; ++i) {
            const size_t n = sz(rng);
            auto input = rtts::random::vector<T>(n, seed + static_cast<unsigned>(i));

            auto exp_it = std::ranges::adjacent_find(input);

            RTTS_EXPECT(raze::algorithm::adjacent_find(input) == exp_it);
            RTTS_EXPECT(raze::algorithm::adjacent_find[raze::options::fscalar](input.begin(), input.end()) == exp_it);
            RTTS_EXPECT(raze::algorithm::adjacent_find[raze::options::fstatic](input) == exp_it);

            if (n >= 2) {
                auto with_dup = input;
                const size_t pos = rng() % (n - 1);
                with_dup[pos + 1] = with_dup[pos];

                auto dup_exp = std::ranges::adjacent_find(with_dup);
                RTTS_EXPECT(raze::algorithm::adjacent_find(with_dup) == dup_exp);
                RTTS_EXPECT(raze::algorithm::adjacent_find[raze::options::fscalar](with_dup) == dup_exp);
                RTTS_EXPECT(raze::algorithm::adjacent_find[raze::options::fstatic](with_dup) == dup_exp);
            }

            const size_t first = n == 0 ? 0 : rng() % (n + 1);
            const size_t last = first + (n == first ? 0 : rng() % (n - first + 1));
            auto sub_exp = std::ranges::adjacent_find(input.begin() + first, input.begin() + last);
            RTTS_EXPECT(raze::algorithm::adjacent_find(input.begin() + first, input.begin() + last) == sub_exp);
        }
    }

    template <class T>
    void adjacent_find_suite() {
        using namespace rtts::algorithm;
        namespace opt = raze::options;
        auto cfg = config::thorough();

        auto check_iter = [&](auto algo) {
            run<range, T>(cfg,
                [algo](auto c) { return algo(c.first(), c.last()); },
                [](auto c) { return std::ranges::adjacent_find(c.first(), c.last()); });
        };

        auto check_range = [&](auto algo) {
            run<range, T>(cfg,
                [algo](auto c) { return algo(c.range()); },
                [](auto c) { return std::ranges::adjacent_find(c.range()); });
        };

        check_iter(raze::algorithm::adjacent_find);
        check_range(raze::algorithm::adjacent_find);
        check_iter(raze::algorithm::adjacent_find[opt::fscalar]);
        check_range(raze::algorithm::adjacent_find[opt::unroll<2>]);
        check_range(raze::algorithm::adjacent_find[opt::fstatic]);
        check_iter(raze::algorithm::adjacent_find[opt::fautovec]);

        run<range, T>(cfg,
            [](auto c) { return raze::algorithm::adjacent_find(c.first(), c.last(), std::greater<>{}); },
            [](auto c) { return std::ranges::adjacent_find(c.first(), c.last(), std::greater<>{}); });

        auto eq_branching = [](auto a, auto b) { if (a == b) return true; return false; };

        run<range, T>(cfg,
            [eq_branching](auto c) { return raze::algorithm::adjacent_find[opt::fautovec](c.range(), eq_branching); },
            [eq_branching](auto c) { return std::ranges::adjacent_find(c.range(), eq_branching); });

        run<range, T>(cfg,
            [eq_branching](auto c) { return raze::algorithm::adjacent_find[opt::fautovec][opt::fstatic](c.range(), eq_branching); },
            [eq_branching](auto c) { return std::ranges::adjacent_find(c.range(), eq_branching); });

        {
            auto acfg = config::compact();

            auto check_aligned = [&]<size_t Align>(auto algo) {
                run_aligned<Align, range, T>(acfg,
                    [algo](auto c) { return algo(c.first(), c.last()); },
                    [](auto c) { return std::ranges::adjacent_find(c.first(), c.last()); });

                run_aligned<Align, range, T>(acfg,
                    [algo](auto c) { return algo(c.range()); },
                    [](auto c) { return std::ranges::adjacent_find(c.range()); });
            };

            check_aligned.template operator()<16>(raze::algorithm::adjacent_find[opt::aligned<16>]);
            check_aligned.template operator()<32>(raze::algorithm::adjacent_find[opt::aligned<32>]);
            check_aligned.template operator()<32>(raze::algorithm::adjacent_find[opt::aligned<32>][opt::unroll<2>]);
            check_aligned.template operator()<32>(raze::algorithm::adjacent_find[opt::aligned<32>][opt::fstatic]);
            check_aligned.template operator()<64>(raze::algorithm::adjacent_find[opt::aligned<64>]);
            check_aligned.template operator()<64>(raze::algorithm::adjacent_find[opt::aligned<64>][opt::fautovec]);
            check_aligned.template operator()<64>(raze::algorithm::adjacent_find[opt::aligned<64>][opt::fscalar]);
            check_aligned.template operator()<1>(raze::algorithm::adjacent_find[opt::aligned<opt::unaligned>]);
            check_aligned.template operator()<1>(raze::algorithm::adjacent_find[opt::aligned<64>][opt::aligned<opt::unaligned>]);
        }

        static_assert(constexpr_run<T, range, 0>(
            [](auto c) { return raze::algorithm::adjacent_find(c.first(), c.last()); },
            [](auto c) { return std::ranges::adjacent_find(c.first(), c.last()); }));

        static_assert(constexpr_run<T, range, 1>(
            [](auto c) { return raze::algorithm::adjacent_find(c.first(), c.last()); },
            [](auto c) { return std::ranges::adjacent_find(c.first(), c.last()); }));

        constexpr auto check_ce = [](auto algo) {
            return constexpr_run<T, range, 5>(
                [algo](auto c) { return algo(c.range()); },
                [](auto c) { return std::ranges::adjacent_find(c.range()); });
        };

        static_assert(check_ce(raze::algorithm::adjacent_find));
        static_assert(check_ce(raze::algorithm::adjacent_find[opt::fautovec]));
        static_assert(check_ce(raze::algorithm::adjacent_find[opt::aligned<64>]));

        static_assert([]() constexpr {
            std::array<T, 0> a0{};
            std::array<T, 1> a1{ T(7) };
            std::array<T, 2> a2{ T(3), T(3) };
            std::array<T, 2> b2{ T(3), T(4) };
            std::array<T, 5> a5{ T(1), T(2), T(3), T(3), T(5) };
            std::array<T, 5> b5{ T(1), T(2), T(3), T(4), T(4) };
            return raze::algorithm::adjacent_find(a0) == a0.end() &&
                   raze::algorithm::adjacent_find(a1) == a1.end() &&
                   raze::algorithm::adjacent_find(a2) == a2.begin() &&
                   raze::algorithm::adjacent_find(b2) == b2.end() &&
                   raze::algorithm::adjacent_find(a5) == a5.begin() + 2 &&
                   raze::algorithm::adjacent_find[opt::fscalar](a5) == a5.begin() + 2 &&
                   raze::algorithm::adjacent_find(b5.begin(), b5.begin() + 4) == b5.begin() + 4 &&
                   raze::algorithm::adjacent_find(b5.begin() + 1, b5.end()) == b5.begin() + 3;
        }());
    }
}

RTTS_CASE_TPL("raze::algorithm::adjacent_find", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    adjacent_find_suite<T>();
};

RTTS_CASE_TPL("raze::algorithm::adjacent_find boundaries", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    adjacent_find_boundary_sweep<T>();
};

RTTS_CASE_TPL("raze::algorithm::adjacent_find fuzz", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    for (unsigned seed : fuzz_seeds)
        adjacent_find_fuzz<T>(seed, 200);
};

RTTS_CASE("raze::algorithm::adjacent_find structs and projections") {
    {
        const std::vector<adjacent_find_item> input{
            { 1, 10 }, { 2, 20 }, { 3, 20 }, { 4, 40 }, { 4, 50 }
        };

        RTTS_EXPECT((raze::algorithm::adjacent_find(input) == input.end()));

        auto it = raze::algorithm::adjacent_find(
            input, std::equal_to<>{}, [](const auto& item) { return item.value; });
        RTTS_EXPECT(it == input.begin() + 1);

        it = raze::algorithm::adjacent_find[raze::options::fscalar](
            input, std::equal_to<>{}, [](const auto& item) { return item.key; });
        RTTS_EXPECT(it == input.begin() + 3);
    }

    {
        const std::vector<int32_t> nums{ 10, 20, 21, 30 };
        auto it = raze::algorithm::adjacent_find(nums, abs_diff_one_pred{});
        RTTS_EXPECT(it == nums.begin() + 1);
    }

    {
        const std::vector<std::string> words{ "alpha", "beta", "beta", "gamma" };
        RTTS_EXPECT(raze::algorithm::adjacent_find(words) == words.begin() + 1);

        const std::vector<std::string> unique_words{ "alpha", "beta", "gamma" };
        RTTS_EXPECT(raze::algorithm::adjacent_find(unique_words) == unique_words.end());
    }
};

RTTS_CASE("raze::algorithm::adjacent_find sentinel") {
    for (size_t n : { size_t(0), size_t(1) }) {
        std::array<int, 2> buf{ 7, 7 };
        auto first = std::counted_iterator(buf.begin(), static_cast<std::ptrdiff_t>(n));
        RTTS_EXPECT(raze::algorithm::adjacent_find(first, std::default_sentinel) == std::default_sentinel);
    }

    for (size_t n : { size_t(16), size_t(17), size_t(18), size_t(64), size_t(65), size_t(300) }) {
        std::vector<int> v(n);
        for (size_t i = 0; i < n; ++i)
            v[i] = static_cast<int>(i);

        auto first = std::counted_iterator(v.begin(), static_cast<std::ptrdiff_t>(v.size()));
        RTTS_EXPECT(raze::algorithm::adjacent_find(first, std::default_sentinel) == std::default_sentinel);

        v[n - 1] = v[n - 2];
        auto res = raze::algorithm::adjacent_find(first, std::default_sentinel);
        RTTS_EXPECT(res.base() == v.begin() + (n - 2));
    }
};

RTTS_CASE("raze::algorithm::adjacent_find views and edge cases") {
    namespace opt = raze::options;
    namespace views = std::views;

    {
        auto iv = views::iota(1, 30);
        RTTS_EXPECT(raze::algorithm::adjacent_find(iv) == iv.end());

        const std::vector<int> src = { 1, 2, 3, 4, 4, 5, 6, 7, 7, 8 };
        auto dv = src | views::drop(1) | views::take(6);
        RTTS_EXPECT(raze::algorithm::adjacent_find[opt::fstatic](dv) == std::ranges::adjacent_find(dv));

        auto rv = src | views::reverse;
        RTTS_EXPECT(raze::algorithm::adjacent_find(rv) == std::ranges::adjacent_find(rv));

        auto tv = views::iota(0, 20) | views::transform([](int x) { return x / 3; });
        RTTS_EXPECT(raze::algorithm::adjacent_find[opt::aligned<64>](tv) == std::ranges::adjacent_find(tv));

        auto fv = src | views::filter([](int x) { return x >= 4; });
        RTTS_EXPECT(raze::algorithm::adjacent_find[opt::fautovec][opt::fstatic](
            fv, [](auto a, auto b) { if (a == b) return true; return false; }) == std::ranges::adjacent_find(fv));

        auto nc = rtts::make_non_common(src);
        RTTS_EXPECT(raze::algorithm::adjacent_find(nc) == std::ranges::adjacent_find(nc));
    }

    {
        std::vector<int> empty_v;
        RTTS_EXPECT(raze::algorithm::adjacent_find(empty_v) == empty_v.end());

        std::vector<int> one_v = { 5 };
        RTTS_EXPECT(raze::algorithm::adjacent_find(one_v) == one_v.end());

        volatile int vbuf[16];
        for (int i = 0; i < 16; ++i) vbuf[i] = i;
        vbuf[10] = 9;
        RTTS_EXPECT(raze::algorithm::adjacent_find(vbuf, vbuf + 16) == vbuf + 9);

        auto rval = raze::algorithm::adjacent_find(std::vector<int>{ 1, 2, 2, 3 });
        static_assert(std::same_as<decltype(rval), std::ranges::dangling>);

        const std::vector<rtts::test_item> items = { { 1, 10 }, { 2, 20 }, { 2, 99 }, { 3, 30 } };
        RTTS_EXPECT(raze::algorithm::adjacent_find(items, std::equal_to<>{}, &rtts::test_item::key) == items.begin() + 1);
    }
};
