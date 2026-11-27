#include <tests/rts/rts.h>
#include <raze/algorithm/find/AllOf.h>
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

    struct all_of_item {
        int32_t key;
        int32_t value;
        bool operator==(const all_of_item&) const = default;
    };

    template <class T>
    struct equal_pred {
        T target;
        constexpr auto operator()(auto x) const { return x == target; }
    };

    template <class T>
    struct not_equal_pred {
        T target;
        constexpr auto operator()(auto x) const { return x != target; }
    };

    struct false_pred {
        constexpr bool operator()(auto) const { return false; }
    };

    struct true_pred {
        constexpr bool operator()(auto) const { return true; }
    };

    template <class T>
    void check_all_of_variants(const std::vector<T>& input, T expected_val) {
        namespace opt = raze::options;
        auto pred = [expected_val](auto x) { return x == expected_val; };
        auto scalar_pred = [expected_val](T x) { return x == expected_val; };
        auto vec_pred = raze::algorithm::equal_to(expected_val);

        const bool exp = std::ranges::all_of(input, pred);

        auto check_pred = [&](auto p) {
            RTTS_EXPECT(raze::algorithm::all_of(input, p) == exp);
            RTTS_EXPECT(raze::algorithm::all_of(input.begin(), input.end(), p) == exp);
            RTTS_EXPECT(raze::algorithm::all_of[opt::fscalar](input, p) == exp);
            RTTS_EXPECT(raze::algorithm::all_of[opt::unroll<2>](input, p) == exp);
            RTTS_EXPECT(raze::algorithm::all_of[opt::fstatic](input, p) == exp);
        };

        check_pred(pred);
        check_pred(vec_pred);
        RTTS_EXPECT(raze::algorithm::all_of(input, scalar_pred) == exp);
    }

    template <class T>
    void all_of_boundary_sweep() {
        std::vector<size_t> sizes;
        for (size_t n = 0; n <= 140; ++n)
            sizes.push_back(n);
        for (size_t n : { size_t(255), size_t(256), size_t(257), size_t(511), size_t(512), size_t(513), size_t(1024) })
            sizes.push_back(n);

        for (size_t n : sizes) {
            std::vector<T> base(n, T(1));

            check_all_of_variants(base, T(1));

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
                    check_all_of_variants(input, T(1));
                }
            }
        }
    }

    template <class T>
    void all_of_fuzz(unsigned seed, size_t iters) {
        std::mt19937 rng(seed);
        std::uniform_int_distribution<size_t> sz(0, 512);

        for (size_t i = 0; i < iters; ++i) {
            const size_t n = sz(rng);
            auto input = rtts::random::vector<T>(n, seed + static_cast<unsigned>(i));
            const T target = n == 0 ? T(0) : input[rng() % n];

            auto vec_pred = raze::algorithm::equal_to(target);
            const bool exp = std::ranges::all_of(input, vec_pred);

            RTTS_EXPECT(raze::algorithm::all_of(input, vec_pred) == exp);
            RTTS_EXPECT(raze::algorithm::all_of[raze::options::fscalar](input.begin(), input.end(), vec_pred) == exp);
            RTTS_EXPECT(raze::algorithm::all_of[raze::options::fstatic](input, vec_pred) == exp);

            std::vector<T> all_same(n, target);
            RTTS_EXPECT(raze::algorithm::all_of(all_same, vec_pred));
            RTTS_EXPECT(raze::algorithm::all_of[raze::options::fscalar](all_same, vec_pred));
            RTTS_EXPECT(raze::algorithm::all_of[raze::options::fstatic](all_same, vec_pred));

            const size_t first = n == 0 ? 0 : rng() % (n + 1);
            const size_t last = first + (n == first ? 0 : rng() % (n - first + 1));
            const bool sub_exp = std::ranges::all_of(input.begin() + first, input.begin() + last, vec_pred);
            RTTS_EXPECT(raze::algorithm::all_of(input.begin() + first, input.begin() + last, vec_pred) == sub_exp);
        }
    }

    template <class T>
    void all_of_suite() {
        using namespace rtts::algorithm;
        namespace opt = raze::options;
        auto cfg = config::thorough();

        constexpr auto target_of = [](auto& c) {
            return *std::ranges::next(c.first(), static_cast<std::ptrdiff_t>(c.count()));
        };

        auto check_iter = [&](auto algo) {
            run<n, T>(cfg,
                [algo, target_of](auto c) { return algo(c.first(), c.last(), not_equal_pred<T>{ target_of(c) }); },
                [target_of](auto c) { return std::ranges::all_of(c.first(), c.last(), not_equal_pred<T>{ target_of(c) }); });
        };

        auto check_range = [&](auto algo) {
            run<n, T>(cfg,
                [algo, target_of](auto c) { return algo(c.range(), not_equal_pred<T>{ target_of(c) }); },
                [target_of](auto c) { return std::ranges::all_of(c.range(), not_equal_pred<T>{ target_of(c) }); });
        };

        check_iter(raze::algorithm::all_of);
        check_range(raze::algorithm::all_of);
        check_iter(raze::algorithm::all_of[opt::fscalar]);
        check_range(raze::algorithm::all_of[opt::unroll<2>]);
        check_range(raze::algorithm::all_of[opt::fstatic]);
        check_iter(raze::algorithm::all_of[opt::fautovec]);

        run<range, T>(cfg,
            [](auto c) { return raze::algorithm::all_of(c.first(), c.last(), true_pred{}); },
            [](auto c) { return std::ranges::all_of(c.first(), c.last(), true_pred{}); });

        run<n, T>(cfg,
            [target_of](auto c) {
                T target = target_of(c);
                return raze::algorithm::all_of[opt::fautovec][opt::fstatic](c.range(), [target](auto x) { if (x != target) return true; return false; });
            },
            [target_of](auto c) {
                T target = target_of(c);
                return std::ranges::all_of(c.range(), [target](auto x) { if (x != target) return true; return false; });
            });

        {
            auto acfg = config::compact();

            auto check_aligned = [&]<size_t Align>(auto algo) {
                run_aligned<Align, n, T>(acfg,
                    [algo, target_of](auto c) { return algo(c.first(), c.last(), not_equal_pred<T>{ target_of(c) }); },
                    [target_of](auto c) { return std::ranges::all_of(c.first(), c.last(), not_equal_pred<T>{ target_of(c) }); });

                run_aligned<Align, range, T>(acfg,
                    [algo](auto c) { return algo(c.range(), true_pred{}); },
                    [](auto c) { return std::ranges::all_of(c.range(), true_pred{}); });
            };

            check_aligned.template operator()<16>(raze::algorithm::all_of[opt::aligned<16>]);
            check_aligned.template operator()<32>(raze::algorithm::all_of[opt::aligned<32>]);
            check_aligned.template operator()<32>(raze::algorithm::all_of[opt::aligned<32>][opt::unroll<2>]);
            check_aligned.template operator()<32>(raze::algorithm::all_of[opt::aligned<32>][opt::fstatic]);
            check_aligned.template operator()<64>(raze::algorithm::all_of[opt::aligned<64>]);
            check_aligned.template operator()<64>(raze::algorithm::all_of[opt::aligned<64>][opt::fautovec]);
            check_aligned.template operator()<64>(raze::algorithm::all_of[opt::aligned<64>][opt::fscalar]);
            check_aligned.template operator()<1>(raze::algorithm::all_of[opt::aligned<opt::unaligned>]);
            check_aligned.template operator()<1>(raze::algorithm::all_of[opt::aligned<64>][opt::aligned<opt::unaligned>]);
        }

        static_assert(constexpr_run<T, range, 0>(
            [](auto c) { return raze::algorithm::all_of(c.first(), c.last(), false_pred{}); },
            [](auto c) { return std::ranges::all_of(c.first(), c.last(), false_pred{}); }));

        constexpr auto check_ce = [target_of](auto algo) {
            return constexpr_run<T, n, 5, 3>(
                [algo, target_of](auto c) { return algo(c.range(), not_equal_pred<T>{ target_of(c) }); },
                [target_of](auto c) { return std::ranges::all_of(c.range(), not_equal_pred<T>{ target_of(c) }); });
        };

        static_assert(check_ce(raze::algorithm::all_of));
        static_assert(check_ce(raze::algorithm::all_of[opt::aligned<64>]));

        static_assert(constexpr_run<T, n, 5, 3>(
            [target_of](auto c) {
                T target = target_of(c);
                return raze::algorithm::all_of[opt::fautovec](c.range(), [target](auto x) constexpr { if (x != target) return true; return false; });
            },
            [target_of](auto c) {
                T target = target_of(c);
                return std::ranges::all_of(c.range(), [target](auto x) constexpr { if (x != target) return true; return false; });
            }));

        static_assert([]() constexpr {
            std::array<T, 0> a0{};
            std::array<T, 4> a4{ T(1), T(1), T(1), T(1) };
            std::array<T, 4> b4{ T(1), T(1), T(2), T(1) };
            std::array<T, 4> c4{ T(1), T(2), T(2), T(4) };
            std::array<T, 5> a5{ T(1), T(2), T(3), T(4), T(5) };
            return raze::algorithm::all_of(a0, false_pred{}) &&
                   raze::algorithm::all_of(a4, equal_pred<T>{ T(1) }) &&
                  !raze::algorithm::all_of(b4, equal_pred<T>{ T(1) }) &&
                   raze::algorithm::all_of[opt::fscalar](a5, [](auto x) { return x <= T(5); }) &&
                  !raze::algorithm::all_of[opt::fscalar](a5, [](auto x) { return x < T(5); }) &&
                   raze::algorithm::all_of(c4.begin() + 1, c4.begin() + 3, equal_pred<T>{ T(2) }) &&
                  !raze::algorithm::all_of(c4.begin() + 1, c4.end(), equal_pred<T>{ T(2) });
        }());
    }
}

RTTS_CASE_TPL("raze::algorithm::all_of", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    all_of_suite<T>();
};

RTTS_CASE_TPL("raze::algorithm::all_of boundaries", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    all_of_boundary_sweep<T>();
};

RTTS_CASE_TPL("raze::algorithm::all_of fuzz", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    for (unsigned seed : fuzz_seeds)
        all_of_fuzz<T>(seed, 200);
};

RTTS_CASE("raze::algorithm::all_of structs and projections") {
    const std::vector<all_of_item> input{
        { 1, 10 }, { 2, 10 }, { 3, 10 }, { 4, 10 }, { 5, 10 }
    };

    RTTS_EXPECT(raze::algorithm::all_of(
        input,
        [](int32_t v) { return v == 10; },
        [](const auto& item) { return item.value; }));

    RTTS_EXPECT(raze::algorithm::all_of[raze::options::fscalar](
        input,
        [](int32_t k) { return k > 0; },
        [](const auto& item) { return item.key; }));

    RTTS_EXPECT(!raze::algorithm::all_of(
        input,
        [](int32_t k) { return k < 5; },
        [](const auto& item) { return item.key; }));

    const std::vector<std::string> words{ "aa", "bb", "cc", "dd" };
    RTTS_EXPECT(raze::algorithm::all_of(words, [](const std::string& s) { return s.size() == 2; }));

    const std::vector<std::string> mixed{ "aa", "bbb", "cc" };
    RTTS_EXPECT(!raze::algorithm::all_of(mixed, [](const std::string& s) { return s.size() == 2; }));
};

RTTS_CASE("raze::algorithm::all_of sentinel") {
    {
        std::array<int, 3> empty_arr{ 1, 1, 1 };
        auto empty_first = std::counted_iterator(empty_arr.begin(), 0);
        RTTS_EXPECT(raze::algorithm::all_of(empty_first, std::default_sentinel, raze::algorithm::equal_to(42)));
    }

    for (size_t n : { size_t(16), size_t(64), size_t(300) }) {
        std::vector<int> v(n, 1);
        auto first = std::counted_iterator(v.begin(), static_cast<std::ptrdiff_t>(v.size()));

        RTTS_EXPECT(raze::algorithm::all_of(first, std::default_sentinel, raze::algorithm::equal_to(1)));

        v[0] = 42;
        RTTS_EXPECT(!raze::algorithm::all_of(first, std::default_sentinel, raze::algorithm::equal_to(1)));

        v[0] = 1;
        v[n - 1] = 42;
        RTTS_EXPECT(!raze::algorithm::all_of(first, std::default_sentinel, raze::algorithm::equal_to(1)));
    }
};


RTTS_CASE("raze::algorithm::all_of views and edge cases") {
    namespace opt = raze::options;
    namespace views = std::views;

    // Views: iota, take, drop, reverse, filter, transform, chained, non-common
    {
        RTTS_EXPECT(raze::algorithm::all_of(views::iota(1, 50), [](auto x) { return x > 0; }));
        RTTS_EXPECT(!raze::algorithm::all_of(views::iota(0, 50), [](auto x) { return x > 0; }));

        const std::vector<int> src = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
        auto evens = src | views::filter([](int x) { return (x & 1) == 0; });
        RTTS_EXPECT(raze::algorithm::all_of[opt::aligned<64>](evens, [](int x) { return (x & 1) == 0; }));

        auto doubled = src | views::transform([](int x) { return x * 2; }) | views::reverse;
        RTTS_EXPECT(raze::algorithm::all_of[opt::fautovec][opt::fstatic](doubled, [](auto x) { if ((x & 1) == 0) return true; return false; }));

        RTTS_EXPECT(raze::algorithm::all_of(rtts::make_non_common(src), [](auto x) { return x <= 10; }));
    }

    // Empty (vacuously true), 1-elem, single-pass early exit, volatile, member projection
    {
        std::vector<int> empty_v;
        RTTS_EXPECT(raze::algorithm::all_of(empty_v, [](auto x) { return x > 100; }));

        const std::vector<int> data = { 2, 4, 5, 8, 10 };
        rtts::single_pass_input_range<int> spr(data);
        RTTS_EXPECT(!raze::algorithm::all_of(spr, [](int x) { return (x & 1) == 0; }));
        RTTS_EXPECT(spr.reads() == 2);

        volatile int vbuf[16];
        for (int i = 0; i < 16; ++i) vbuf[i] = i + 1;
        RTTS_EXPECT(raze::algorithm::all_of(vbuf, vbuf + 16, [](int x) { return x > 0; }));

        const std::vector<rtts::test_item> items = { { 2, 1 }, { 4, 3 }, { 6, 5 } };
        RTTS_EXPECT(raze::algorithm::all_of(items, [](int k) { return (k & 1) == 0; }, &rtts::test_item::key));
        RTTS_EXPECT(!raze::algorithm::all_of(items, [](int p) { return (p & 1) == 0; }, &rtts::test_item::payload));
    }
};