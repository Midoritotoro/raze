#include <tests/rts/rts.h>
#include <raze/algorithm/find/FindLast.h>

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

    struct find_last_item {
        int32_t key;
        int32_t value;
        bool operator==(const find_last_item&) const = default;
    };

    template <std::forward_iterator It, std::sentinel_for<It> Sent, class T>
    constexpr std::ranges::subrange<It> ref_find_last(It first, Sent last, const T& value) {
#if defined(__cpp_lib_ranges_find_last) && __cpp_lib_ranges_find_last >= 202207L
        return std::ranges::find_last(std::move(first), std::move(last), value);
#else
        auto found = first;
        bool has_match = false;
        for (; first != last; ++first) {
            if (*first == value) {
                found = first;
                has_match = true;
            }
        }
        return { has_match ? found : first, first };
#endif
    }

    template <std::ranges::forward_range R, class T>
    constexpr std::ranges::borrowed_subrange_t<R> ref_find_last(R&& r, const T& value) {
        return ref_find_last(std::ranges::begin(r), std::ranges::end(r), value);
    }

    template <class SubA, class SubB>
    constexpr bool subrange_eq(const SubA& a, const SubB& b) {
        return a.begin() == b.begin() && a.end() == b.end();
    }

    template <class T>
    void check_find_last_variants(const std::vector<T>& input, T target) {
        auto exp = ref_find_last(input, target);
        const auto exp_off = exp.begin() - input.begin();
        const auto exp_end_off = exp.end() - input.begin();

        auto check = [&](auto sub) {
            RTTS_EXPECT(sub.begin() - input.begin() == exp_off);
            RTTS_EXPECT(sub.end() - input.begin() == exp_end_off);
        };

        check(raze::algorithm::find_last(input, target));
        check(raze::algorithm::find_last(input.begin(), input.end(), target));
        check(raze::algorithm::find_last[raze::options::fscalar](input, target));
        check(raze::algorithm::find_last[raze::options::unroll<2>](input, target));
        check(raze::algorithm::find_last[raze::options::unroll<4>](input, target));
        check(raze::algorithm::find_last[raze::options::fstatic](input, target));
    }

    template <class T>
    void find_last_boundary_sweep() {
        std::vector<size_t> sizes;
        for (size_t n = 0; n <= 140; ++n)
            sizes.push_back(n);
        for (size_t n : { size_t(255), size_t(256), size_t(257), size_t(511), size_t(512), size_t(513), size_t(1024) })
            sizes.push_back(n);

        for (size_t n : sizes) {
            std::vector<T> base(n);
            for (size_t i = 0; i < n; ++i)
                base[i] = static_cast<T>((i % 100) + 1);

            check_find_last_variants(base, T(0));

            if (n > 0) {
                std::vector<T> all_same(n, T(5));
                check_find_last_variants(all_same, T(5));

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
                    check_find_last_variants(input, T(0));

                    auto dup_input = input;
                    dup_input[0] = T(0);
                    check_find_last_variants(dup_input, T(0));
                }
            }
        }
    }

    template <class T>
    void find_last_fuzz(unsigned seed, size_t iters) {
        std::mt19937 rng(seed);
        std::uniform_int_distribution<size_t> sz(0, 512);

        for (size_t i = 0; i < iters; ++i) {
            const size_t n = sz(rng);
            auto input = rtts::random::vector<T>(n, seed + static_cast<unsigned>(i));
            const T target = n == 0 ? T(0) : input[rng() % n];

            auto exp = ref_find_last(input, target);

            RTTS_EXPECT(subrange_eq(raze::algorithm::find_last(input, target), exp));
            RTTS_EXPECT(subrange_eq(raze::algorithm::find_last[raze::options::fscalar](input.begin(), input.end(), target), exp));
            RTTS_EXPECT(subrange_eq(raze::algorithm::find_last[raze::options::unroll<4>](input, target), exp));
            RTTS_EXPECT(subrange_eq(raze::algorithm::find_last[raze::options::fstatic](input, target), exp));

            auto no_zero = input;
            for (auto& x : no_zero)
                if (x == T(0)) x = T(1);

            const std::ranges::subrange<typename std::vector<T>::iterator> miss_exp{ no_zero.end(), no_zero.end() };
            RTTS_EXPECT(subrange_eq(raze::algorithm::find_last(no_zero, T(0)), miss_exp));
            RTTS_EXPECT(subrange_eq(raze::algorithm::find_last[raze::options::fscalar](no_zero, T(0)), miss_exp));
            RTTS_EXPECT(subrange_eq(raze::algorithm::find_last[raze::options::fstatic](no_zero, T(0)), miss_exp));

            const size_t first = n == 0 ? 0 : rng() % (n + 1);
            const size_t last = first + (n == first ? 0 : rng() % (n - first + 1));
            auto sub_exp = ref_find_last(input.begin() + first, input.begin() + last, target);
            RTTS_EXPECT(subrange_eq(raze::algorithm::find_last(input.begin() + first, input.begin() + last, target), sub_exp));
        }
    }

    template <class T>
    void find_last_suite() {
        using namespace rtts::algorithm;
        namespace opt = raze::options;
        auto cfg = config::thorough();

        constexpr auto target_of = [](auto& c) {
            return *std::ranges::next(c.first(), static_cast<std::ptrdiff_t>(c.count()));
        };

        auto check_iter = [&](auto algo) {
            run<n, T>(cfg,
                [algo, target_of](auto c) {
                    auto res = algo(c.first(), c.last(), target_of(c));
                    RTTS_EXPECT(res.end() == std::ranges::next(c.first(), c.last()));
                    return res.begin();
                },
                [target_of](auto c) { return ref_find_last(c.first(), c.last(), target_of(c)).begin(); });
        };

        auto check_range = [&](auto algo) {
            run<n, T>(cfg,
                [algo, target_of](auto c) {
                    auto res = algo(c.range(), target_of(c));
                    RTTS_EXPECT(res.end() == std::ranges::next(c.first(), c.last()));
                    return res.begin();
                },
                [target_of](auto c) { return ref_find_last(c.range(), target_of(c)).begin(); });
        };

        check_iter(raze::algorithm::find_last);
        check_range(raze::algorithm::find_last);
        check_iter(raze::algorithm::find_last[opt::fscalar]);
        check_range(raze::algorithm::find_last[opt::unroll<2>]);
        check_range(raze::algorithm::find_last[opt::fstatic]);
        check_iter(raze::algorithm::find_last[opt::fautovec]);
        check_range(raze::algorithm::find_last[opt::fautovec][opt::fstatic]);

        {
            auto acfg = config::compact();

            auto check_aligned = [&]<size_t Align>(auto algo) {
                run_aligned<Align, n, T>(acfg,
                    [algo, target_of](auto c) { return algo(c.first(), c.last(), target_of(c)).begin(); },
                    [target_of](auto c) { return ref_find_last(c.first(), c.last(), target_of(c)).begin(); });

                run_aligned<Align, n, T>(acfg,
                    [algo, target_of](auto c) { return algo(c.range(), target_of(c)).begin(); },
                    [target_of](auto c) { return ref_find_last(c.range(), target_of(c)).begin(); });
            };

            check_aligned.template operator()<16>(raze::algorithm::find_last[opt::aligned<16>]);
            check_aligned.template operator()<32>(raze::algorithm::find_last[opt::aligned<32>]);
            check_aligned.template operator()<32>(raze::algorithm::find_last[opt::aligned<32>][opt::unroll<2>]);
            check_aligned.template operator()<32>(raze::algorithm::find_last[opt::aligned<32>][opt::fstatic]);
            check_aligned.template operator()<64>(raze::algorithm::find_last[opt::aligned<64>]);
            check_aligned.template operator()<64>(raze::algorithm::find_last[opt::aligned<64>][opt::fautovec]);
            check_aligned.template operator()<64>(raze::algorithm::find_last[opt::aligned<64>][opt::fscalar]);
            check_aligned.template operator()<1>(raze::algorithm::find_last[opt::aligned<opt::unaligned>]);
            check_aligned.template operator()<1>(raze::algorithm::find_last[opt::aligned<64>][opt::aligned<opt::unaligned>]);
        }

        static_assert(constexpr_run<T, range, 0>(
            [](auto c) { return raze::algorithm::find_last(c.first(), c.last(), T(1)).begin(); },
            [](auto c) { return ref_find_last(c.first(), c.last(), T(1)).begin(); }));

        constexpr auto check_ce = [target_of](auto algo) {
            return constexpr_run<T, n, 5, 3>(
                [algo, target_of](auto c) { return algo(c.range(), target_of(c)).begin(); },
                [target_of](auto c) { return ref_find_last(c.range(), target_of(c)).begin(); });
        };

        static_assert(check_ce(raze::algorithm::find_last));
        static_assert(check_ce(raze::algorithm::find_last[opt::fautovec]));
        static_assert(check_ce(raze::algorithm::find_last[opt::aligned<64>]));

        static_assert([]() constexpr {
            std::array<T, 0> a0{};
            std::array<T, 5> a5{ T(1), T(3), T(2), T(3), T(4) };
            std::array<T, 5> b5{ T(5), T(2), T(5), T(4), T(5) };
            std::array<T, 4> a4{ T(1), T(2), T(3), T(4) };
            auto r0 = raze::algorithm::find_last(a0, T(1));
            auto hit = raze::algorithm::find_last(a5, T(3));
            auto miss = raze::algorithm::find_last(a5, T(9));
            auto s_hit = raze::algorithm::find_last[opt::fscalar](b5, T(5));
            auto s_miss = raze::algorithm::find_last[opt::fscalar](b5, T(9));
            auto sub = raze::algorithm::find_last(a4.begin() + 1, a4.begin() + 3, T(4));
            return r0.begin() == a0.end() && r0.end() == a0.end() &&
                   hit.begin() == a5.begin() + 3 && hit.end() == a5.end() &&
                   miss.begin() == a5.end() && miss.end() == a5.end() &&
                   s_hit.begin() == b5.begin() + 4 && s_hit.end() == b5.end() &&
                   s_miss.begin() == b5.end() && s_miss.end() == b5.end() &&
                   sub.begin() == a4.begin() + 3 && sub.end() == a4.begin() + 3;
        }());
    }
}

RTTS_CASE_TPL("raze::algorithm::find_last", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    find_last_suite<T>();
};

RTTS_CASE_TPL("raze::algorithm::find_last boundaries", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    find_last_boundary_sweep<T>();
};

RTTS_CASE_TPL("raze::algorithm::find_last fuzz", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    for (unsigned seed : fuzz_seeds)
        find_last_fuzz<T>(seed, 200);
};

RTTS_CASE("raze::algorithm::find_last structs and projections") {
    const std::vector<find_last_item> input{
        { 1, 10 }, { 3, 30 }, { 3, 30 }, { 4, 10 }, { 5, 50 }
    };

    auto res = raze::algorithm::find_last(input, find_last_item{ 3, 30 });
    RTTS_EXPECT(res.begin() == input.begin() + 2 && res.end() == input.end());

    res = raze::algorithm::find_last(input, find_last_item{ 9, 90 });
    RTTS_EXPECT(res.begin() == input.end() && res.end() == input.end());

    res = raze::algorithm::find_last(input, 10, [](const auto& item) { return item.value; });
    RTTS_EXPECT(res.begin() == input.begin() + 3 && res.end() == input.end());

    res = raze::algorithm::find_last[raze::options::fscalar](input, 3, [](const auto& item) { return item.key; });
    RTTS_EXPECT(res.begin() == input.begin() + 2 && res.end() == input.end());

    const std::vector<std::string> words{ "alpha", "gamma", "beta", "gamma", "delta" };
    auto wres = raze::algorithm::find_last(words, std::string("gamma"));
    RTTS_EXPECT(wres.begin() == words.begin() + 3 && wres.end() == words.end());

    wres = raze::algorithm::find_last(words, "omega");
    RTTS_EXPECT(wres.begin() == words.end() && wres.end() == words.end());
};

RTTS_CASE("raze::algorithm::find_last sentinel") {
    for (size_t n : { size_t(1), size_t(3), size_t(16), size_t(64), size_t(300) }) {
        std::vector<int> v(n, 1);
        v[0] = 42;

        auto first = std::counted_iterator(v.begin(), static_cast<std::ptrdiff_t>(v.size()));
        auto tail_res = raze::algorithm::find_last(first, std::default_sentinel, 42);
        RTTS_EXPECT(tail_res.begin().base() == v.begin());
        RTTS_EXPECT(tail_res.end().base() == v.end() && tail_res.end() == std::default_sentinel);

        if (n > 1) {
            v[n - 1] = 42;
            auto last_res = raze::algorithm::find_last(first, std::default_sentinel, 42);
            RTTS_EXPECT(last_res.begin().base() == v.begin() + (n - 1));
            RTTS_EXPECT(last_res.end().base() == v.end() && last_res.end() == std::default_sentinel);
        }

        auto miss = raze::algorithm::find_last(first, std::default_sentinel, 777);
        RTTS_EXPECT(miss.begin() == std::default_sentinel && miss.end() == std::default_sentinel);
        RTTS_EXPECT(miss.begin().base() == v.end() && miss.end().base() == v.end());
    }
};


RTTS_CASE("raze::algorithm::find_last views and edge cases") {
    namespace opt = raze::options;
    namespace views = std::views;

    // Views: iota, take, drop, reverse, filter, transform, chained, non-common
    {
        auto iv = views::iota(1, 30);
        auto ri = raze::algorithm::find_last(iv, 17);
        auto si = std::ranges::find_last(iv, 17);
        RTTS_EXPECT(ri.begin() == si.begin() && ri.end() == si.end());

        const std::vector<int> src = { 1, 2, 3, 2, 4, 2, 5, 6, 2, 7 };
        auto dv = src | views::drop(1) | views::take(7);
        auto rd = raze::algorithm::find_last[opt::fstatic](dv, 2);
        auto sd = std::ranges::find_last(dv, 2);
        RTTS_EXPECT(rd.begin() == sd.begin() && rd.end() == sd.end());

        auto rv = src | views::reverse;
        auto rr = raze::algorithm::find_last(rv, 2);
        auto sr = std::ranges::find_last(rv, 2);
        RTTS_EXPECT(rr.begin() == sr.begin() && rr.end() == sr.end());

        auto chain = src | views::filter([](int x) { return x < 6; }) | views::transform([](int x) { return x * 2; });
        auto rc = raze::algorithm::find_last[opt::aligned<64>](chain, 4);
        auto sc = std::ranges::find_last(chain, 4);
        RTTS_EXPECT(rc.begin() == sc.begin() && rc.end() == sc.end());

        auto nc = rtts::make_non_common(src);
        auto rn = raze::algorithm::find_last(nc, 2);
        auto sn = std::ranges::find_last(nc, 2);
        RTTS_EXPECT(rn.begin() == sn.begin() && rn.end() == sn.end());
    }

    // Empty, 1-elem, volatile, rvalue dangling, member projection
    {
        std::vector<int> empty_v;
        auto er = raze::algorithm::find_last(empty_v, 1);
        RTTS_EXPECT(er.begin() == empty_v.end() && er.end() == empty_v.end());

        volatile int vbuf[16];
        for (int i = 0; i < 16; ++i) vbuf[i] = i % 5;
        auto vr = raze::algorithm::find_last(vbuf, vbuf + 16, 2);
        RTTS_EXPECT(vr.begin() == vbuf + 12 && vr.end() == vbuf + 16);

        auto rval = raze::algorithm::find_last(std::vector<int>{ 1, 2, 1 }, 1);
        static_assert(std::same_as<decltype(rval), std::ranges::dangling>);

        const std::vector<rtts::test_item> items = { { 5, 1 }, { 2, 2 }, { 5, 3 }, { 7, 4 } };
        auto ir = raze::algorithm::find_last(items, 5, &rtts::test_item::key);
        RTTS_EXPECT(ir.begin() == items.begin() + 2 && ir.begin()->payload == 3);
    }
};