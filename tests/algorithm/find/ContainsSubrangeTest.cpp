#include <tests/rts/rts.h>
#include <raze/algorithm/find/ContainsSubrange.h>

#include <algorithm>
#include <array>
#include <cstdint>
#include <deque>
#include <forward_list>
#include <limits>
#include <list>
#include <random>
#include <ranges>
#include <string>
#include <string_view>
#include <vector>

namespace {
    inline constexpr unsigned fuzz_seeds[] = { 42u, 0x9E3779B9u, 0xDEADBEEFu };

    struct contains_subrange_item {
        int32_t key;
        int32_t value;
        bool operator==(const contains_subrange_item&) const = default;
    };

    template <class It1, class Sent1, class It2, class Sent2, class Pred = std::equal_to<>>
    constexpr bool ref_contains_subrange(It1 first1, Sent1 last1, It2 first2, Sent2 last2, Pred pred = {}) {
#if defined(__cpp_lib_ranges_contains) && __cpp_lib_ranges_contains >= 202207L
        return std::ranges::contains_subrange(std::move(first1), std::move(last1),
            std::move(first2), std::move(last2), std::move(pred));
#else
        return first2 == last2 || !std::ranges::search(
            std::move(first1), std::move(last1),
            std::move(first2), std::move(last2), std::move(pred)).empty();
#endif
    }

    template <class R1, class R2, class Pred = std::equal_to<>>
    constexpr bool ref_contains_subrange(const R1& r1, const R2& r2, Pred pred = {}) {
        return ref_contains_subrange(std::ranges::begin(r1), std::ranges::end(r1),
            std::ranges::begin(r2), std::ranges::end(r2), std::move(pred));
    }

    template <class T>
    void check_contains_subrange_variants(const std::vector<T>& haystack, const std::vector<T>& needle) {
        namespace opt = raze::options;
        const bool exp = ref_contains_subrange(haystack, needle);
        auto bin_pred = [](auto a, auto b) { return a == b; };
        auto eq_branching = [](auto a, auto b) { if (a == b) return true; return false; };

        RTTS_EXPECT(raze::algorithm::contains_subrange(haystack, needle) == exp);
        RTTS_EXPECT(raze::algorithm::contains_subrange(haystack.begin(), haystack.end(), needle.begin(), needle.end()) == exp);
        RTTS_EXPECT(raze::algorithm::contains_subrange[opt::fscalar](haystack, needle) == exp);
        RTTS_EXPECT(raze::algorithm::contains_subrange[opt::fstatic](haystack, needle) == exp);
        RTTS_EXPECT(raze::algorithm::contains_subrange[opt::fautovec](haystack, needle) == exp);
        RTTS_EXPECT(raze::algorithm::contains_subrange[opt::fautovec][opt::fstatic](haystack, needle) == exp);
        RTTS_EXPECT(raze::algorithm::contains_subrange[opt::unroll<1>](haystack, needle) == exp);
        RTTS_EXPECT(raze::algorithm::contains_subrange[opt::unroll<2>](haystack, needle) == exp);
        RTTS_EXPECT(raze::algorithm::contains_subrange[opt::unroll<4>](haystack, needle) == exp);

        RTTS_EXPECT(raze::algorithm::contains_subrange(haystack, needle, bin_pred) == exp);
        RTTS_EXPECT(raze::algorithm::contains_subrange[opt::fstatic](haystack, needle, bin_pred) == exp);
        RTTS_EXPECT(raze::algorithm::contains_subrange[opt::fscalar](haystack, needle, bin_pred) == exp);
        RTTS_EXPECT(raze::algorithm::contains_subrange[opt::fautovec][opt::fstatic](haystack, needle, eq_branching) == exp);
    }

    template <class T>
    void contains_subrange_boundary_sweep() {
        std::vector<size_t> hay_sizes;
        for (size_t h_len = 0; h_len <= 70; ++h_len)
            hay_sizes.push_back(h_len);
        for (size_t h_len : { size_t(127), size_t(128), size_t(129), size_t(255), size_t(256), size_t(257), size_t(512) })
            hay_sizes.push_back(h_len);

        for (size_t h_len : hay_sizes) {
            std::vector<T> haystack(h_len);
            for (size_t i = 0; i < h_len; ++i)
                haystack[i] = static_cast<T>((i % 97) + 1);

            // Empty needle -> ALWAYS true (even when haystack is empty!)
            check_contains_subrange_variants(haystack, std::vector<T>{});

            // Needle longer than haystack -> ALWAYS false
            std::vector<T> longer(h_len + 1, T(1));
            for (size_t i = 0; i < h_len; ++i)
                longer[i] = haystack[i];
            check_contains_subrange_variants(haystack, longer);

            if (h_len == 0)
                continue;

            check_contains_subrange_variants(haystack, haystack);

            auto bad = haystack;
            bad[h_len / 2] = T(0);
            check_contains_subrange_variants(haystack, bad);

            for (size_t p_len : { size_t(1), size_t(2), size_t(3), size_t(8), size_t(16), size_t(17), size_t(32), size_t(33), size_t(64) }) {
                if (p_len > h_len)
                    continue;
                const size_t pos = (h_len - p_len) / 2;
                std::vector<T> pat(haystack.begin() + static_cast<std::ptrdiff_t>(pos),
                                   haystack.begin() + static_cast<std::ptrdiff_t>(pos + p_len));
                check_contains_subrange_variants(haystack, pat);

                pat[p_len / 2] = T(0);
                check_contains_subrange_variants(haystack, pat);
            }
        }
    }

    template <class T>
    void contains_subrange_fuzz(unsigned seed, size_t iters) {
        namespace opt = raze::options;
        std::mt19937 rng(seed);
        std::uniform_int_distribution<size_t> h_dist(0, 300);
        std::uniform_int_distribution<size_t> p_dist(0, 80);

        for (size_t iter = 0; iter < iters; ++iter) {
            const size_t h_len = h_dist(rng);
            const size_t p_len = p_dist(rng);

            std::vector<T> h(h_len);
            std::vector<T> p(p_len);
            if (iter % 2 == 0) {
                for (auto& x : h) x = static_cast<T>(rng() % 3);
                for (auto& x : p) x = static_cast<T>(rng() % 3);
            }
            else {
                h = rtts::random::vector<T>(h_len, seed + static_cast<unsigned>(iter));
                p = rtts::random::vector<T>(p_len, seed + static_cast<unsigned>(iter) + 1000u);
            }

            if (p_len > 0 && p_len <= h_len && (rng() % 2 == 0)) {
                const size_t pos = rng() % (h_len - p_len + 1);
                for (size_t k = 0; k < p_len; ++k)
                    p[k] = h[pos + k];
                if (rng() % 4 == 0)
                    p[rng() % p_len] = static_cast<T>(p[0] + T(1));
            }

            const bool exp = ref_contains_subrange(h, p);
            RTTS_EXPECT(raze::algorithm::contains_subrange(h, p) == exp);
            RTTS_EXPECT(raze::algorithm::contains_subrange[opt::fscalar](h.begin(), h.end(), p.begin(), p.end()) == exp);
            RTTS_EXPECT(raze::algorithm::contains_subrange[opt::fstatic](h, p) == exp);
            RTTS_EXPECT(raze::algorithm::contains_subrange[opt::unroll<4>](h, p) == exp);
            RTTS_EXPECT(raze::algorithm::contains_subrange[opt::fautovec](h, p) == exp);
        }
    }

    template <class T>
    void contains_subrange_suite() {
        using namespace rtts::algorithm;
        namespace opt = raze::options;
        auto cfg = config::thorough();

        auto make_sub = [](auto& c) {
            auto p_beg = std::ranges::next(c.first(), static_cast<std::ptrdiff_t>(c.count()));
            auto p_end = std::ranges::next(p_beg, 1, c.last());
            return std::ranges::subrange(p_beg, p_end);
        };

        auto check_range = [&](auto algo) {
            run<n, T>(cfg,
                [algo, make_sub](auto c) { return algo(c.range(), make_sub(c)); },
                [make_sub](auto c) { return ref_contains_subrange(c.range(), make_sub(c)); });
        };

        run<n, T>(cfg,
            [make_sub](auto c) { auto sub = make_sub(c); return raze::algorithm::contains_subrange(c.first(), c.last(), sub.begin(), sub.end()); },
            [make_sub](auto c) { auto sub = make_sub(c); return ref_contains_subrange(c.first(), c.last(), sub.begin(), sub.end()); });

        check_range(raze::algorithm::contains_subrange);
        check_range(raze::algorithm::contains_subrange[opt::fscalar]);
        check_range(raze::algorithm::contains_subrange[opt::fstatic]);
        check_range(raze::algorithm::contains_subrange[opt::unroll<2>]);
        check_range(raze::algorithm::contains_subrange[opt::fautovec]);

        auto eq_branching = [](auto a, auto b) { if (a == b) return true; return false; };

        run<n, T>(cfg,
            [make_sub, eq_branching](auto c) { return raze::algorithm::contains_subrange[opt::fautovec][opt::fstatic](c.range(), make_sub(c), eq_branching); },
            [make_sub, eq_branching](auto c) { return ref_contains_subrange(c.range(), make_sub(c), eq_branching); });

        // Aligned tests
        {
            auto acfg = config::compact();

            auto check_aligned = [&]<size_t A1, size_t A2>(auto algo) {
                run_aligned<A1, A2, range, T>(acfg,
                    [algo](auto c) { return algo(c.first(), c.last(), c.first2(), c.last2()); },
                    [](auto c) { return ref_contains_subrange(c.first(), c.last(), c.first2(), c.last2()); });

                run_aligned<A1, A2, range, T>(acfg,
                    [algo](auto c) { return algo(c.range(), c.range2()); },
                    [](auto c) { return ref_contains_subrange(c.range(), c.range2()); });

                for (size_t sz : { size_t(0), size_t(1), size_t(16), size_t(32), size_t(64), size_t(128) }) {
                    rtts::aligned_buffer<T, A1> hay(sz);
                    for (size_t i = 0; i < sz; ++i)
                        hay[i] = static_cast<T>((i % 97) + 1);
                    const size_t p_len = sz / 3;
                    rtts::aligned_buffer<T, A2> pat(p_len);
                    for (size_t i = 0; i < p_len; ++i)
                        pat[i] = hay[sz / 2 + i];
                    RTTS_EXPECT(algo(hay, pat) == ref_contains_subrange(hay, pat));
                    if (p_len > 0) {
                        pat[p_len / 2] = T(0);
                        RTTS_EXPECT(algo(hay, pat) == ref_contains_subrange(hay, pat));
                    }
                }
            };

            check_aligned.template operator()<16, 16>(raze::algorithm::contains_subrange[opt::aligned<16>]);
            check_aligned.template operator()<32, 32>(raze::algorithm::contains_subrange[opt::aligned<32>][opt::unroll<2>]);
            check_aligned.template operator()<64, 64>(raze::algorithm::contains_subrange[opt::aligned<64>]);
            check_aligned.template operator()<64, 1>(raze::algorithm::contains_subrange[opt::aligned<64, opt::unaligned>]);
            check_aligned.template operator()<1, 64>(raze::algorithm::contains_subrange[opt::aligned<opt::unaligned, 64>]);
            check_aligned.template operator()<1, 1>(raze::algorithm::contains_subrange[opt::aligned<64>][opt::aligned<opt::unaligned>]);

            if constexpr (std::same_as<T, uint8_t> || std::same_as<T, int32_t> || std::same_as<T, float> || std::same_as<T, double>) {
                check_aligned.template operator()<32, 32>(raze::algorithm::contains_subrange[opt::aligned<32>]);
                check_aligned.template operator()<64, 64>(raze::algorithm::contains_subrange[opt::aligned<64>][opt::fautovec]);
                check_aligned.template operator()<64, 64>(raze::algorithm::contains_subrange[opt::aligned<64>][opt::fscalar]);
                check_aligned.template operator()<1, 1>(raze::algorithm::contains_subrange[opt::aligned<opt::unaligned>]);
                check_aligned.template operator()<32, 64>(raze::algorithm::contains_subrange[opt::aligned<32, 64>][opt::fstatic]);
                check_aligned.template operator()<64, 16>(raze::algorithm::contains_subrange[opt::aligned<64, 16>]);
            }
        }

        // Constexpr tests
        static_assert(constexpr_run<T, range, 0>(
            [](auto c) { return raze::algorithm::contains_subrange(c.range(), c.range()); },
            [](auto c) { return ref_contains_subrange(c.range(), c.range()); }));

        static_assert(constexpr_run<T, range, 5>(
            [](auto c) { return raze::algorithm::contains_subrange[opt::fautovec](c.range(), c.range2()); },
            [](auto c) { return ref_contains_subrange(c.range(), c.range2()); }));

        static_assert(constexpr_run<T, range, 5>(
            [](auto c) { return raze::algorithm::contains_subrange[opt::aligned<64, opt::unaligned>](c.range(), c.range2()); },
            [](auto c) { return ref_contains_subrange(c.range(), c.range2()); }));

        static_assert([]() constexpr {
            std::array<T, 0> empty{};
            std::array<T, 2> tiny{ T(1), T(2) };
            return raze::algorithm::contains_subrange(empty, empty) &&
                   raze::algorithm::contains_subrange(tiny, empty) &&
                   !raze::algorithm::contains_subrange(empty, tiny);
        }());

        static_assert([]() constexpr {
            std::array<T, 6> a{ T(1), T(2), T(3), T(4), T(5), T(6) };
            std::array<T, 3> mid{ T(3), T(4), T(5) };
            std::array<T, 3> bad{ T(3), T(9), T(5) };
            std::array<T, 7> too_long{ T(1), T(2), T(3), T(4), T(5), T(6), T(7) };
            return raze::algorithm::contains_subrange(a, mid) &&
                   !raze::algorithm::contains_subrange(a, bad) &&
                   !raze::algorithm::contains_subrange(a, too_long) &&
                   raze::algorithm::contains_subrange[opt::fscalar](a.begin(), a.end(), mid.begin(), mid.end()) &&
                   raze::algorithm::contains_subrange[opt::fstatic](a, a);
        }());
    }
}

RTTS_CASE_TPL("raze::algorithm::contains_subrange", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    contains_subrange_suite<T>();
};

RTTS_CASE_TPL("raze::algorithm::contains_subrange boundaries", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    contains_subrange_boundary_sweep<T>();
};

RTTS_CASE_TPL("raze::algorithm::contains_subrange fuzz", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    for (unsigned seed : fuzz_seeds)
        contains_subrange_fuzz<T>(seed, 150);
};

RTTS_CASE("raze::algorithm::contains_subrange structs, projections, rvalues, FP and predicates") {
    namespace opt = raze::options;

    // Rvalue ranges MUST work cleanly and return bool (NOT std::ranges::dangling!)
    {
        auto b1 = raze::algorithm::contains_subrange(std::vector<int32_t>{ 1, 2, 3, 4, 5 }, std::vector<int32_t>{ 3, 4 });
        auto b2 = raze::algorithm::contains_subrange(std::vector<int32_t>{ 1, 2, 3, 4, 5 }, std::vector<int32_t>{ 3, 9 });
        auto b3 = raze::algorithm::contains_subrange(std::vector<int32_t>{}, std::vector<int32_t>{});
        static_assert(std::same_as<decltype(b1), bool>);
        RTTS_EXPECT(b1);
        RTTS_EXPECT(!b2);
        RTTS_EXPECT(b3);
    }

    // Floating-point -0.0 / +0.0 / NaN
    {
        const double nan = std::numeric_limits<double>::quiet_NaN();
        std::vector<double> dh(40, 1.0);
        dh[20] = -0.0; dh[21] = -0.0; dh[22] = -0.0;
        std::vector<double> dp{ +0.0, +0.0, +0.0 };
        RTTS_EXPECT(raze::algorithm::contains_subrange(dh, dp));
        RTTS_EXPECT(raze::algorithm::contains_subrange[opt::fscalar](dh, dp));

        dh[21] = nan;
        std::vector<double> dnan{ -0.0, nan, -0.0 };
        RTTS_EXPECT(!raze::algorithm::contains_subrange(dh, dnan));
    }

    // Projections, structs, strings, custom predicates
    {
        const std::vector<contains_subrange_item> input{
            { 1, 10 }, { 2, 20 }, { 3, 30 }, { 4, 40 }
        };
        const std::vector<int32_t> keys{ 2, 3 };
        const std::vector<int32_t> bad_keys{ 2, 9 };

        RTTS_EXPECT(raze::algorithm::contains_subrange(input, keys, std::equal_to<>{}, &contains_subrange_item::key));
        RTTS_EXPECT(!raze::algorithm::contains_subrange(input, bad_keys, std::equal_to<>{}, &contains_subrange_item::key));

        const std::vector<contains_subrange_item> other{ { 9, 200 }, { 8, 300 } };
        RTTS_EXPECT(raze::algorithm::contains_subrange[opt::fscalar](
            input, other,
            [](int32_t a, int32_t b) { return a * 10 == b; },
            &contains_subrange_item::value, &contains_subrange_item::value));

        const std::string s = "hello world";
        RTTS_EXPECT(raze::algorithm::contains_subrange(s, std::string_view("lo wo")));
        RTTS_EXPECT(!raze::algorithm::contains_subrange(s, std::string_view("world!")));
    }
};

RTTS_CASE("raze::algorithm::contains_subrange views and edge cases") {
    namespace opt = raze::options;
    namespace views = std::views;

    // Views: iota, take, drop, reverse, filter, transform, non-common
    {
        auto iv = views::iota(1, 50);
        auto sub = views::iota(20, 30);
        RTTS_EXPECT(raze::algorithm::contains_subrange(iv, sub));
        RTTS_EXPECT(!raze::algorithm::contains_subrange(sub, iv));

        const std::vector<int> src = { 2, 4, 6, 8, 10, 12, 14, 16 };
        auto fv = views::iota(1, 20) | views::filter([](int x) { return (x & 1) == 0; });
        auto tv = views::iota(3, 7) | views::transform([](int x) { return x * 2; });
        RTTS_EXPECT((raze::algorithm::contains_subrange[opt::aligned<64, opt::unaligned>](fv, tv)));
        RTTS_EXPECT(raze::algorithm::contains_subrange[opt::fautovec][opt::fstatic](
            src, tv, [](auto x, auto y) { if (x == y) return true; return false; }));
        RTTS_EXPECT(raze::algorithm::contains_subrange(rtts::make_non_common(src), tv));
        RTTS_EXPECT(raze::algorithm::contains_subrange(src | views::reverse, std::vector<int>{ 12, 10, 8 }));
    }

    // Non-contiguous containers, empty, volatile, member projection
    {
        const std::list<int> lst{ 1, 2, 3, 4, 5 };
        const std::forward_list<int> flst{ 3, 4 };
        const std::deque<int> deq{ 2, 3, 4 };
        RTTS_EXPECT(raze::algorithm::contains_subrange(lst, flst));
        RTTS_EXPECT(raze::algorithm::contains_subrange(deq, flst));
        RTTS_EXPECT(!raze::algorithm::contains_subrange(flst, deq));

        volatile int vh[16], vp[4];
        for (int i = 0; i < 16; ++i) vh[i] = i;
        for (int i = 0; i < 4; ++i) vp[i] = 7 + i;
        RTTS_EXPECT(raze::algorithm::contains_subrange(vh, vh + 16, vp, vp + 4));
        vp[2] = 99;
        RTTS_EXPECT(!raze::algorithm::contains_subrange(vh, vh + 16, vp, vp + 4));

        const std::vector<rtts::test_item> i1 = { { 1, 10 }, { 2, 20 }, { 3, 30 }, { 4, 40 } };
        const std::vector<rtts::test_item> i2 = { { 2, 99 }, { 3, 88 } };
        RTTS_EXPECT(raze::algorithm::contains_subrange(i1, i2, std::equal_to<>{}, &rtts::test_item::key, &rtts::test_item::key));
    }
};

