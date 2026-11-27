#include <tests/rts/rts.h>
#include <raze/algorithm/find/FindLastIfNot.h>
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

    struct find_last_if_not_item {
        int32_t key;
        int32_t value;
        bool operator==(const find_last_if_not_item&) const = default;
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

    template <std::forward_iterator It, std::sentinel_for<It> Sent, class Pred>
    constexpr std::ranges::subrange<It> ref_find_last_if_not(It first, Sent last, Pred pred) {
#if defined(__cpp_lib_ranges_find_last) && __cpp_lib_ranges_find_last >= 202207L
        return std::ranges::find_last_if_not(std::move(first), std::move(last), std::move(pred));
#else
        auto found = first;
        bool has_match = false;
        for (; first != last; ++first) {
            if (!pred(*first)) {
                found = first;
                has_match = true;
            }
        }
        return { has_match ? found : first, first };
#endif
    }

    template <std::ranges::forward_range R, class Pred>
    constexpr std::ranges::borrowed_subrange_t<R> ref_find_last_if_not(R&& r, Pred pred) {
        return ref_find_last_if_not(std::ranges::begin(r), std::ranges::end(r), std::move(pred));
    }

    template <class SubA, class SubB>
    constexpr bool subrange_eq(const SubA& a, const SubB& b) {
        return a.begin() == b.begin() && a.end() == b.end();
    }

    template <class T>
    void check_find_last_if_not_variants(const std::vector<T>& input, T keep) {
        auto pred = [keep](auto x) { return x != keep; };
        auto scalar_pred = [keep](T x) { return x != keep; };

        auto exp = ref_find_last_if_not(input, pred);
        const auto exp_off = exp.begin() - input.begin();
        const auto exp_end_off = exp.end() - input.begin();

        auto check = [&](auto sub) {
            RTTS_EXPECT(sub.begin() - input.begin() == exp_off);
            RTTS_EXPECT(sub.end() - input.begin() == exp_end_off);
        };

        check(raze::algorithm::find_last_if_not(input, pred));
        check(raze::algorithm::find_last_if_not(input.begin(), input.end(), pred));
        check(raze::algorithm::find_last_if_not(input, scalar_pred));
        check(raze::algorithm::find_last_if_not[raze::options::fscalar](input, pred));
        check(raze::algorithm::find_last_if_not[raze::options::unroll<2>](input, pred));
        check(raze::algorithm::find_last_if_not[raze::options::unroll<4>](input, pred));
        check(raze::algorithm::find_last_if_not[raze::options::fstatic](input, pred));
    }

    template <class T>
    void check_find_last_if_not_vec_variants(const std::vector<T>& input, T keep) {
        auto vec_pred = raze::algorithm::equal_to(keep);

        auto exp = ref_find_last_if_not(input, vec_pred);
        const auto exp_off = exp.begin() - input.begin();
        const auto exp_end_off = exp.end() - input.begin();

        auto check = [&](auto sub) {
            RTTS_EXPECT(sub.begin() - input.begin() == exp_off);
            RTTS_EXPECT(sub.end() - input.begin() == exp_end_off);
        };

        check(raze::algorithm::find_last_if_not(input, vec_pred));
        check(raze::algorithm::find_last_if_not(input.begin(), input.end(), vec_pred));
        check(raze::algorithm::find_last_if_not[raze::options::fscalar](input, vec_pred));
        check(raze::algorithm::find_last_if_not[raze::options::unroll<2>](input, vec_pred));
        check(raze::algorithm::find_last_if_not[raze::options::unroll<4>](input, vec_pred));
        check(raze::algorithm::find_last_if_not[raze::options::fstatic](input, vec_pred));
    }

    template <class T>
    void find_last_if_not_boundary_sweep() {
        std::vector<size_t> sizes;
        for (size_t n = 0; n <= 140; ++n)
            sizes.push_back(n);
        for (size_t n : { size_t(255), size_t(256), size_t(257), size_t(511), size_t(512), size_t(513), size_t(1024) })
            sizes.push_back(n);

        for (size_t n : sizes) {
            std::vector<T> base(n);
            for (size_t i = 0; i < n; ++i)
                base[i] = static_cast<T>((i % 100) + 1);

            check_find_last_if_not_variants(base, T(0));

            std::vector<T> all_ones(n, T(1));
            check_find_last_if_not_vec_variants(all_ones, T(1));

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
                    check_find_last_if_not_variants(input, T(0));

                    auto dup_input = input;
                    dup_input[0] = T(0);
                    check_find_last_if_not_variants(dup_input, T(0));

                    auto vec_input = all_ones;
                    vec_input[pos] = T(0);
                    check_find_last_if_not_vec_variants(vec_input, T(1));

                    auto dup_vec_input = vec_input;
                    dup_vec_input[0] = T(0);
                    check_find_last_if_not_vec_variants(dup_vec_input, T(1));
                }
            }
        }
    }

    template <class T>
    void find_last_if_not_fuzz(unsigned seed, size_t iters) {
        std::mt19937 rng(seed);
        std::uniform_int_distribution<size_t> sz(0, 512);

        for (size_t i = 0; i < iters; ++i) {
            const size_t n = sz(rng);
            auto input = rtts::random::vector<T>(n, seed + static_cast<unsigned>(i));
            const T keep = n == 0 ? T(0) : input[rng() % n];

            auto vec_pred = raze::algorithm::equal_to(keep);
            auto exp = ref_find_last_if_not(input, vec_pred);

            RTTS_EXPECT(subrange_eq(raze::algorithm::find_last_if_not(input, vec_pred), exp));
            RTTS_EXPECT(subrange_eq(raze::algorithm::find_last_if_not[raze::options::fscalar](input.begin(), input.end(), vec_pred), exp));
            RTTS_EXPECT(subrange_eq(raze::algorithm::find_last_if_not[raze::options::unroll<4>](input, vec_pred), exp));
            RTTS_EXPECT(subrange_eq(raze::algorithm::find_last_if_not[raze::options::fstatic](input, vec_pred), exp));

            std::vector<T> all_keep(n, keep);
            const std::ranges::subrange<typename std::vector<T>::iterator> miss_exp{ all_keep.end(), all_keep.end() };
            RTTS_EXPECT(subrange_eq(raze::algorithm::find_last_if_not(all_keep, vec_pred), miss_exp));
            RTTS_EXPECT(subrange_eq(raze::algorithm::find_last_if_not[raze::options::fscalar](all_keep, vec_pred), miss_exp));
            RTTS_EXPECT(subrange_eq(raze::algorithm::find_last_if_not[raze::options::fstatic](all_keep, vec_pred), miss_exp));

            const size_t first = n == 0 ? 0 : rng() % (n + 1);
            const size_t last = first + (n == first ? 0 : rng() % (n - first + 1));
            auto sub_exp = ref_find_last_if_not(input.begin() + first, input.begin() + last, vec_pred);
            RTTS_EXPECT(subrange_eq(raze::algorithm::find_last_if_not(input.begin() + first, input.begin() + last, vec_pred), sub_exp));
        }
    }

    template <class T>
    void find_last_if_not_suite() {
        using namespace rtts::algorithm;
        namespace opt = raze::options;
        auto cfg = config::thorough();

        constexpr auto target_of = [](auto& c) {
            return *std::ranges::next(c.first(), static_cast<std::ptrdiff_t>(c.count()));
        };

        auto check_iter = [&](auto algo) {
            run<n, T>(cfg,
                [algo, target_of](auto c) {
                    auto res = algo(c.first(), c.last(), not_equal_pred<T>{ target_of(c) });
                    RTTS_EXPECT(res.end() == std::ranges::next(c.first(), c.last()));
                    return res.begin();
                },
                [target_of](auto c) { return ref_find_last_if_not(c.first(), c.last(), not_equal_pred<T>{ target_of(c) }).begin(); });
        };

        auto check_range = [&](auto algo) {
            run<n, T>(cfg,
                [algo, target_of](auto c) {
                    auto res = algo(c.range(), not_equal_pred<T>{ target_of(c) });
                    RTTS_EXPECT(res.end() == std::ranges::next(c.first(), c.last()));
                    return res.begin();
                },
                [target_of](auto c) { return ref_find_last_if_not(c.range(), not_equal_pred<T>{ target_of(c) }).begin(); });
        };

        check_iter(raze::algorithm::find_last_if_not);
        check_range(raze::algorithm::find_last_if_not);
        check_iter(raze::algorithm::find_last_if_not[opt::fscalar]);
        check_range(raze::algorithm::find_last_if_not[opt::unroll<2>]);
        check_range(raze::algorithm::find_last_if_not[opt::fstatic]);
        check_iter(raze::algorithm::find_last_if_not[opt::fautovec]);

        run<range, T>(cfg,
            [](auto c) { return raze::algorithm::find_last_if_not(c.first(), c.last(), true_pred{}).begin(); },
            [](auto c) { return ref_find_last_if_not(c.first(), c.last(), true_pred{}).begin(); });

        run<n, T>(cfg,
            [target_of](auto c) {
                T target = target_of(c);
                return raze::algorithm::find_last_if_not[opt::fautovec][opt::fstatic](c.range(), [target](auto x) { if (x != target) return true; return false; }).begin();
            },
            [target_of](auto c) {
                T target = target_of(c);
                return ref_find_last_if_not(c.range(), [target](auto x) { if (x != target) return true; return false; }).begin();
            });

        {
            auto acfg = config::compact();

            auto check_aligned = [&]<size_t Align>(auto algo) {
                run_aligned<Align, n, T>(acfg,
                    [algo, target_of](auto c) { return algo(c.first(), c.last(), not_equal_pred<T>{ target_of(c) }).begin(); },
                    [target_of](auto c) { return ref_find_last_if_not(c.first(), c.last(), not_equal_pred<T>{ target_of(c) }).begin(); });

                run_aligned<Align, n, T>(acfg,
                    [algo, target_of](auto c) { return algo(c.range(), not_equal_pred<T>{ target_of(c) }).begin(); },
                    [target_of](auto c) { return ref_find_last_if_not(c.range(), not_equal_pred<T>{ target_of(c) }).begin(); });
            };

            check_aligned.template operator()<16>(raze::algorithm::find_last_if_not[opt::aligned<16>]);
            check_aligned.template operator()<32>(raze::algorithm::find_last_if_not[opt::aligned<32>]);
            check_aligned.template operator()<32>(raze::algorithm::find_last_if_not[opt::aligned<32>][opt::unroll<2>]);
            check_aligned.template operator()<32>(raze::algorithm::find_last_if_not[opt::aligned<32>][opt::fstatic]);
            check_aligned.template operator()<64>(raze::algorithm::find_last_if_not[opt::aligned<64>]);
            check_aligned.template operator()<64>(raze::algorithm::find_last_if_not[opt::aligned<64>][opt::fautovec]);
            check_aligned.template operator()<64>(raze::algorithm::find_last_if_not[opt::aligned<64>][opt::fscalar]);
            check_aligned.template operator()<1>(raze::algorithm::find_last_if_not[opt::aligned<opt::unaligned>]);
            check_aligned.template operator()<1>(raze::algorithm::find_last_if_not[opt::aligned<64>][opt::aligned<opt::unaligned>]);
        }

        static_assert(constexpr_run<T, range, 0>(
            [](auto c) { return raze::algorithm::find_last_if_not(c.first(), c.last(), false_pred{}).begin(); },
            [](auto c) { return ref_find_last_if_not(c.first(), c.last(), false_pred{}).begin(); }));

        constexpr auto check_ce = [target_of](auto algo) {
            return constexpr_run<T, n, 5, 3>(
                [algo, target_of](auto c) { return algo(c.range(), not_equal_pred<T>{ target_of(c) }).begin(); },
                [target_of](auto c) { return ref_find_last_if_not(c.range(), not_equal_pred<T>{ target_of(c) }).begin(); });
        };

        static_assert(check_ce(raze::algorithm::find_last_if_not));
        static_assert(check_ce(raze::algorithm::find_last_if_not[opt::aligned<64>]));

        static_assert(constexpr_run<T, n, 5, 3>(
            [target_of](auto c) {
                T target = target_of(c);
                return raze::algorithm::find_last_if_not[opt::fautovec](c.range(), [target](auto x) constexpr { if (x != target) return true; return false; }).begin();
            },
            [target_of](auto c) {
                T target = target_of(c);
                return ref_find_last_if_not(c.range(), [target](auto x) constexpr { if (x != target) return true; return false; }).begin();
            }));

        static_assert([]() constexpr {
            std::array<T, 0> a0{};
            std::array<T, 5> a5{ T(1), T(3), T(1), T(4), T(1) };
            std::array<T, 5> b5{ T(1), T(2), T(3), T(4), T(5) };
            std::array<T, 4> a4{ T(1), T(1), T(1), T(4) };
            auto r0 = raze::algorithm::find_last_if_not(a0, false_pred{});
            auto hit = raze::algorithm::find_last_if_not(a5, eq_pred<T>{ T(1) });
            auto miss = raze::algorithm::find_last_if_not(a5, true_pred{});
            auto r2 = raze::algorithm::find_last_if_not[opt::fscalar](b5, [](auto x) { return x <= T(4); });
            auto r3 = raze::algorithm::find_last_if_not(a4.begin() + 1, a4.begin() + 3, eq_pred<T>{ T(1) });
            return r0.begin() == a0.end() && r0.end() == a0.end() &&
                   hit.begin() == a5.begin() + 3 && hit.end() == a5.end() &&
                   miss.begin() == a5.end() && miss.end() == a5.end() &&
                   r2.begin() == b5.begin() + 4 && r2.end() == b5.end() &&
                   r3.begin() == a4.begin() + 3 && r3.end() == a4.begin() + 3;
        }());
    }
}

RTTS_CASE_TPL("raze::algorithm::find_last_if_not", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    find_last_if_not_suite<T>();
};

RTTS_CASE_TPL("raze::algorithm::find_last_if_not boundaries", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    find_last_if_not_boundary_sweep<T>();
};

RTTS_CASE_TPL("raze::algorithm::find_last_if_not fuzz", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    for (unsigned seed : fuzz_seeds)
        find_last_if_not_fuzz<T>(seed, 200);
};

RTTS_CASE("raze::algorithm::find_last_if_not structs and projections") {
    const std::vector<find_last_if_not_item> input{
        { 1, 20 }, { 2, 10 }, { 3, 30 }, { 4, 10 }
    };

    auto res = raze::algorithm::find_last_if_not(
        input,
        [](int32_t v) { return v == 10; },
        [](const auto& item) { return item.value; });
    RTTS_EXPECT(res.begin() == input.begin() + 2 && res.end() == input.end());

    res = raze::algorithm::find_last_if_not[raze::options::fscalar](
        input,
        [](int32_t k) { return k < 4; },
        [](const auto& item) { return item.key; });
    RTTS_EXPECT(res.begin() == input.begin() + 3 && res.end() == input.end());

    const std::vector<std::string> words{ "alpha", "bb", "gamma", "cc" };
    auto wres = raze::algorithm::find_last_if_not(words, [](const std::string& s) { return s.size() == 2; });
    RTTS_EXPECT(wres.begin() == words.begin() + 2 && wres.end() == words.end());
};

RTTS_CASE("raze::algorithm::find_last_if_not sentinel") {
    for (size_t n : { size_t(1), size_t(3), size_t(16), size_t(64), size_t(300) }) {
        std::vector<int> v(n, 1);
        v[0] = 42;

        auto first = std::counted_iterator(v.begin(), static_cast<std::ptrdiff_t>(v.size()));
        auto tail_res = raze::algorithm::find_last_if_not(first, std::default_sentinel, raze::algorithm::equal_to(1));
        RTTS_EXPECT(tail_res.begin().base() == v.begin());
        RTTS_EXPECT(tail_res.end().base() == v.end() && tail_res.end() == std::default_sentinel);

        if (n > 1) {
            v[n - 1] = 99;
            auto last_res = raze::algorithm::find_last_if_not(first, std::default_sentinel, raze::algorithm::equal_to(1));
            RTTS_EXPECT(last_res.begin().base() == v.begin() + (n - 1));
            RTTS_EXPECT(last_res.end().base() == v.end() && last_res.end() == std::default_sentinel);
        }

        std::vector<int> all_ones(n, 1);
        auto miss_first = std::counted_iterator(all_ones.begin(), static_cast<std::ptrdiff_t>(all_ones.size()));
        auto miss = raze::algorithm::find_last_if_not(miss_first, std::default_sentinel, raze::algorithm::equal_to(1));
        RTTS_EXPECT(miss.begin() == std::default_sentinel && miss.end() == std::default_sentinel);
    }
};

RTTS_CASE("raze::algorithm::find_last_if_not views and edge cases") {
    namespace opt = raze::options;
    namespace views = std::views;

    {
        auto pred_branch = [](auto x) { if (x > 20) return true; return false; };
        auto iv = views::iota(1, 30);
        auto ri = raze::algorithm::find_last_if_not[opt::fautovec][opt::fstatic](iv, pred_branch);
        auto si = std::ranges::find_last_if_not(iv, pred_branch);
        RTTS_EXPECT(ri.begin() == si.begin() && ri.end() == si.end());

        const std::vector<int> src = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
        auto chain = src | views::reverse | views::filter([](int x) { return x < 9; }) | views::transform([](int x) { return x * 2; });
        auto rc = raze::algorithm::find_last_if_not[opt::aligned<64>](chain, [](int x) { return x < 6; });
        auto sc = std::ranges::find_last_if_not(chain, [](int x) { return x < 6; });
        RTTS_EXPECT(rc.begin() == sc.begin() && rc.end() == sc.end());

        auto nc = rtts::make_non_common(src);
        auto rn = raze::algorithm::find_last_if_not(nc, [](auto x) { return x >= 8; });
        auto sn = std::ranges::find_last_if_not(nc, [](int x) { return x >= 8; });
        RTTS_EXPECT(rn.begin() == sn.begin() && rn.end() == sn.end());
    }

    {
        std::vector<int> empty_v;
        RTTS_EXPECT(raze::algorithm::find_last_if_not(empty_v, [](auto x) { return x > 0; }).empty());

        volatile int vbuf[16];
        for (int i = 0; i < 16; ++i) vbuf[i] = i;
        auto vr = raze::algorithm::find_last_if_not(vbuf, vbuf + 16, [](int x) { return x >= 12; });
        RTTS_EXPECT(vr.begin() == vbuf + 11);

        auto rval = raze::algorithm::find_last_if_not(std::vector<int>{ 1, 2, 3 }, [](auto x) { return x == 3; });
        static_assert(std::same_as<decltype(rval), std::ranges::dangling>);

        const std::vector<rtts::test_item> items = { { 1, 10 }, { 5, 50 }, { 6, 60 }, { 2, 20 } };
        auto ir = raze::algorithm::find_last_if_not(items, [](int k) { return k < 5; }, &rtts::test_item::key);
        RTTS_EXPECT(ir.begin() == items.begin() + 2 && ir.begin()->payload == 60);
    }
};
