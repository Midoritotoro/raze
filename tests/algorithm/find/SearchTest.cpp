#include <tests/rts/rts.h>
#include <raze/algorithm/find/Search.h>

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

    struct search_item {
        int32_t key;
        int32_t value;
        bool operator==(const search_item&) const = default;
    };

    template <class T>
    void check_search_variants(const std::vector<T>& haystack, const std::vector<T>& needle) {
        namespace opt = raze::options;
        const auto exp = std::ranges::search(haystack, needle);
        auto check = [&](auto res) {
            RTTS_EXPECT(res.begin() == exp.begin() && res.end() == exp.end());
        };

        auto bin_pred = [](auto a, auto b) { return a == b; };
        auto eq_branching = [](auto a, auto b) { if (a == b) return true; return false; };

        check(raze::algorithm::search(haystack, needle));
        check(raze::algorithm::search(haystack.begin(), haystack.end(), needle.begin(), needle.end()));
        check(raze::algorithm::search[opt::fscalar](haystack, needle));
        check(raze::algorithm::search[opt::fstatic](haystack, needle));
        check(raze::algorithm::search[opt::fautovec](haystack, needle));
        check(raze::algorithm::search[opt::fautovec][opt::fstatic](haystack, needle));
        check(raze::algorithm::search[opt::unroll<1>](haystack, needle));
        check(raze::algorithm::search[opt::unroll<2>](haystack, needle));
        check(raze::algorithm::search[opt::unroll<4>](haystack, needle));

        check(raze::algorithm::search(haystack, needle, bin_pred));
        check(raze::algorithm::search[opt::fstatic](haystack, needle, bin_pred));
        check(raze::algorithm::search[opt::fscalar](haystack, needle, bin_pred));
        check(raze::algorithm::search[opt::fautovec][opt::fstatic](haystack, needle, eq_branching));
    }

    template <class T>
    void search_boundary_sweep() {
        std::vector<size_t> hay_sizes;
        for (size_t h_len = 0; h_len <= 70; ++h_len)
            hay_sizes.push_back(h_len);
        for (size_t h_len : { size_t(127), size_t(128), size_t(129), size_t(255), size_t(256), size_t(257), size_t(512) })
            hay_sizes.push_back(h_len);

        for (size_t h_len : hay_sizes) {
            std::vector<T> haystack(h_len);
            for (size_t i = 0; i < h_len; ++i)
                haystack[i] = static_cast<T>((i % 97) + 1);

            // Empty needle -> {begin, begin}
            check_search_variants(haystack, std::vector<T>{});

            // Needle longer than haystack -> {end, end}
            std::vector<T> longer(h_len + 1, T(1));
            for (size_t i = 0; i < h_len; ++i)
                longer[i] = haystack[i];
            check_search_variants(haystack, longer);

            if (h_len == 0)
                continue;

            // Exact full-range match (including huge H == N -> exercises verify_tail_long)
            check_search_variants(haystack, haystack);

            // Full-range mismatch at first, middle, and last
            {
                auto bad = haystack;
                bad[h_len / 2] = T(0);
                check_search_variants(haystack, bad);
                bad = haystack;
                bad[h_len - 1] = T(0);
                check_search_variants(haystack, bad);
            }

            // Representative needle lengths and match offsets
            std::vector<size_t> needle_lens = { 1, 2, 3, 4, 7, 8, 15, 16, 17, 31, 32, 33, 64, 65 };
            for (size_t p_len : needle_lens) {
                if (p_len > h_len)
                    continue;

                const size_t max_pos = h_len - p_len;
                std::vector<size_t> positions;
                positions.reserve(12);
                positions.push_back(0);
                positions.push_back(max_pos);
                if (max_pos >= 1) positions.push_back(1);
                if (max_pos >= 15) positions.push_back(15);
                if (max_pos >= 16) positions.push_back(16);
                if (max_pos >= 31) positions.push_back(31);
                if (max_pos >= 32) positions.push_back(32);
                if (max_pos >= 63) positions.push_back(63);
                if (max_pos >= 64) positions.push_back(64);
                if (max_pos > 2) positions.push_back(max_pos / 2);

                std::sort(positions.begin(), positions.end());
                positions.erase(std::unique(positions.begin(), positions.end()), positions.end());

                for (size_t pos : positions) {
                    std::vector<T> pat(haystack.begin() + static_cast<std::ptrdiff_t>(pos),
                                       haystack.begin() + static_cast<std::ptrdiff_t>(pos + p_len));
                    check_search_variants(haystack, pat);
                }

                // Missing pattern with matching endpoints
                std::vector<T> miss(p_len, T(0));
                miss[0] = haystack[0];
                miss[p_len - 1] = haystack[p_len - 1];
                check_search_variants(haystack, miss);
            }
        }
    }

    template <class T>
    void search_decoys_and_overlaps() {
        // Dense false-positive first+last hits + multiple/overlapping real matches
        for (size_t h_len : { size_t(16), size_t(33), size_t(65), size_t(131), size_t(260) }) {
            // Overlapping runs of identical elements
            std::vector<T> all_same(h_len, T(7));
            for (size_t p_len : { size_t(1), size_t(2), size_t(4), size_t(17), size_t(65) }) {
                if (p_len <= h_len)
                    check_search_variants(all_same, std::vector<T>(p_len, T(7)));
            }

            // Periodic pattern with overlapping occurrences: {1, 2, 1, 2, 1, ...}
            std::vector<T> periodic(h_len);
            for (size_t i = 0; i < h_len; ++i)
                periodic[i] = (i % 2 == 0) ? T(1) : T(2);
            for (size_t p_len : { size_t(2), size_t(3), size_t(5), size_t(11), size_t(35) }) {
                if (p_len <= h_len) {
                    std::vector<T> pat(periodic.begin() + 1, periodic.begin() + 1 + static_cast<std::ptrdiff_t>(p_len));
                    check_search_variants(periodic, pat);
                }
            }

            // False-positive endpoints: haystack filled with T(5), real match placed late with unique middle
            for (size_t p_len : { size_t(3), size_t(4), size_t(10), size_t(19), size_t(35), size_t(67) }) {
                if (p_len + 4 > h_len)
                    continue;
                std::vector<T> h(h_len, T(5));
                std::vector<T> pat(p_len, T(9));
                pat[0] = T(5);
                pat[p_len - 1] = T(5);

                // Place decoy (matches everything except one middle byte) earlier than real match
                const size_t real_pos = h_len - p_len - 1;
                if (real_pos >= p_len + 1) {
                    for (size_t k = 0; k < p_len; ++k)
                        h[1 + k] = pat[k];
                    h[1 + p_len / 2] = T(4);
                }
                for (size_t k = 0; k < p_len; ++k)
                    h[real_pos + k] = pat[k];

                check_search_variants(h, pat);

                // Also test when NO real match exists despite 100% endpoint hits
                h[real_pos + p_len / 2] = T(4);
                check_search_variants(h, pat);
            }
        }
    }

    template <class T>
    void search_fuzz(unsigned seed, size_t iters) {
        namespace opt = raze::options;
        std::mt19937 rng(seed);
        std::uniform_int_distribution<size_t> h_dist(0, 300);
        std::uniform_int_distribution<size_t> p_dist(0, 80);

        for (size_t iter = 0; iter < iters; ++iter) {
            const size_t h_len = h_dist(rng);
            const size_t p_len = p_dist(rng);
            const bool small_alpha = (iter % 2 == 0);

            std::vector<T> h(h_len);
            std::vector<T> p(p_len);

            if (small_alpha) {
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

            const auto exp = std::ranges::search(h, p);
            auto eq_res = [&](auto r) { return r.begin() == exp.begin() && r.end() == exp.end(); };

            RTTS_EXPECT(eq_res(raze::algorithm::search(h, p)));
            RTTS_EXPECT(eq_res(raze::algorithm::search[opt::fscalar](h.begin(), h.end(), p.begin(), p.end())));
            RTTS_EXPECT(eq_res(raze::algorithm::search[opt::fstatic](h, p)));
            RTTS_EXPECT(eq_res(raze::algorithm::search[opt::unroll<2>](h, p)));
            RTTS_EXPECT(eq_res(raze::algorithm::search[opt::unroll<4>](h, p)));
            RTTS_EXPECT(eq_res(raze::algorithm::search[opt::fautovec](h, p)));
        }
    }

    template <class T>
    void search_suite() {
        using namespace rtts::algorithm;
        namespace opt = raze::options;
        auto cfg = config::thorough();

        auto make_sub = [](auto& c) {
            auto p_beg = std::ranges::next(c.first(), static_cast<std::ptrdiff_t>(c.count()));
            auto p_end = std::ranges::next(p_beg, 1, c.last());
            return std::ranges::subrange(p_beg, p_end);
        };

        run<n, T>(cfg,
            [make_sub](auto c) {
                auto sub = make_sub(c);
                return raze::algorithm::search(c.first(), c.last(), sub.begin(), sub.end());
            },
            [make_sub](auto c) {
                auto sub = make_sub(c);
                return std::ranges::search(c.first(), c.last(), sub.begin(), sub.end());
            });

        run<n, T>(cfg,
            [make_sub](auto c) { return raze::algorithm::search(c.range(), make_sub(c)); },
            [make_sub](auto c) { return std::ranges::search(c.range(), make_sub(c)); });

        run<n, T>(cfg,
            [make_sub](auto c) { return raze::algorithm::search[opt::fscalar](c.range(), make_sub(c)); },
            [make_sub](auto c) { return std::ranges::search(c.range(), make_sub(c)); });

        run<n, T>(cfg,
            [make_sub](auto c) { return raze::algorithm::search[opt::fstatic](c.range(), make_sub(c)); },
            [make_sub](auto c) { return std::ranges::search(c.range(), make_sub(c)); });

        run<n, T>(cfg,
            [make_sub](auto c) { return raze::algorithm::search[opt::unroll<2>](c.range(), make_sub(c)); },
            [make_sub](auto c) { return std::ranges::search(c.range(), make_sub(c)); });

        auto eq_branching = [](auto a, auto b) { if (a == b) return true; return false; };

        run<n, T>(cfg,
            [make_sub](auto c) { return raze::algorithm::search[opt::fautovec](c.range(), make_sub(c)); },
            [make_sub](auto c) { return std::ranges::search(c.range(), make_sub(c)); });

        run<n, T>(cfg,
            [make_sub, eq_branching](auto c) {
                return raze::algorithm::search[opt::fautovec][opt::fstatic](c.range(), make_sub(c), eq_branching);
            },
            [make_sub, eq_branching](auto c) {
                return std::ranges::search(c.range(), make_sub(c), eq_branching);
            });

        // Aligned tests
        {
            auto acfg = config::compact();

            auto check_aligned = [&]<size_t A1, size_t A2>(auto algo) {
                run_aligned<A1, A2, range, T>(acfg,
                    [algo](auto c) { return algo(c.first(), c.last(), c.first2(), c.last2()); },
                    [](auto c) { return std::ranges::search(c.first(), c.last(), c.first2(), c.last2()); });

                run_aligned<A1, A2, range, T>(acfg,
                    [algo](auto c) { return algo(c.range(), c.range2()); },
                    [](auto c) { return std::ranges::search(c.range(), c.range2()); });

                for (size_t sz : { size_t(0), size_t(1), size_t(15), size_t(16), size_t(31), size_t(32), size_t(64), size_t(130) }) {
                    rtts::aligned_buffer<T, A1> hay(sz);
                    for (size_t i = 0; i < sz; ++i)
                        hay[i] = static_cast<T>((i % 97) + 1);

                    for (size_t p_len : { size_t(0), size_t(1), size_t(2), size_t(5), size_t(17), size_t(33) }) {
                        if (p_len > sz)
                            continue;
                        rtts::aligned_buffer<T, A2> pat(p_len);
                        const size_t pos = (sz - p_len) / 2;
                        for (size_t i = 0; i < p_len; ++i)
                            pat[i] = hay[pos + i];

                        auto r = algo(hay, pat);
                        auto exp = std::ranges::search(hay, pat);
                        RTTS_EXPECT(r.begin() == exp.begin() && r.end() == exp.end());

                        if (p_len > 0) {
                            pat[p_len / 2] = T(0);
                            auto r2 = algo(hay, pat);
                            auto exp2 = std::ranges::search(hay, pat);
                            RTTS_EXPECT(r2.begin() == exp2.begin() && r2.end() == exp2.end());
                        }
                    }
                }
            };

            check_aligned.template operator()<16, 16>(raze::algorithm::search[opt::aligned<16>]);
            check_aligned.template operator()<32, 32>(raze::algorithm::search[opt::aligned<32>][opt::unroll<2>]);
            check_aligned.template operator()<64, 64>(raze::algorithm::search[opt::aligned<64>]);
            check_aligned.template operator()<64, 1>(raze::algorithm::search[opt::aligned<64, opt::unaligned>]);
            check_aligned.template operator()<1, 64>(raze::algorithm::search[opt::aligned<opt::unaligned, 64>]);
            check_aligned.template operator()<1, 1>(raze::algorithm::search[opt::aligned<64>][opt::aligned<opt::unaligned>]);

            if constexpr (std::same_as<T, uint8_t> || std::same_as<T, int32_t> || std::same_as<T, float> || std::same_as<T, double>) {
                check_aligned.template operator()<32, 32>(raze::algorithm::search[opt::aligned<32>]);
                check_aligned.template operator()<64, 64>(raze::algorithm::search[opt::aligned<64>][opt::fautovec]);
                check_aligned.template operator()<64, 64>(raze::algorithm::search[opt::aligned<64>][opt::fscalar]);
                check_aligned.template operator()<1, 1>(raze::algorithm::search[opt::aligned<opt::unaligned>]);
                check_aligned.template operator()<32, 64>(raze::algorithm::search[opt::aligned<32, 64>][opt::fstatic]);
                check_aligned.template operator()<64, 16>(raze::algorithm::search[opt::aligned<64, 16>]);
            }
        }

        // Constexpr tests
        static_assert(constexpr_run<T, range, 0>(
            [](auto c) { return raze::algorithm::search(c.range(), c.range()); },
            [](auto c) { return std::ranges::search(c.range(), c.range()); }));

        static_assert(constexpr_run<T, range, 5>(
            [](auto c) { return raze::algorithm::search[opt::fautovec](c.range(), c.range2()); },
            [](auto c) { return std::ranges::search(c.range(), c.range2()); }));

        static_assert(constexpr_run<T, range, 5>(
            [](auto c) { return raze::algorithm::search[opt::aligned<64, opt::unaligned>](c.range(), c.range2()); },
            [](auto c) { return std::ranges::search(c.range(), c.range2()); }));

        static_assert([]() constexpr {
            std::array<T, 0> empty{};
            std::array<T, 2> tiny{ T(1), T(2) };
            auto r0 = raze::algorithm::search(empty, empty);
            auto r1 = raze::algorithm::search(tiny, empty);
            auto r2 = raze::algorithm::search(empty, tiny);
            return r0.begin() == empty.begin() && r0.end() == empty.begin() &&
                   r1.begin() == tiny.begin() && r1.end() == tiny.begin() &&
                   r2.begin() == empty.end() && r2.end() == empty.end();
        }());

        static_assert([]() constexpr {
            std::array<T, 8> a{ T(1), T(2), T(1), T(2), T(1), T(2), T(3), T(4) };
            std::array<T, 3> p{ T(1), T(2), T(1) };
            std::array<T, 2> tail{ T(3), T(4) };
            std::array<T, 3> miss{ T(1), T(9), T(1) };
            std::array<T, 9> too_long{ T(1), T(2), T(1), T(2), T(1), T(2), T(3), T(4), T(5) };

            auto hit1 = raze::algorithm::search(a, p);
            auto hit2 = raze::algorithm::search[opt::fscalar](a.begin(), a.end(), tail.begin(), tail.end());
            auto hit3 = raze::algorithm::search[opt::fstatic](a, a);
            auto m1 = raze::algorithm::search(a, miss);
            auto m2 = raze::algorithm::search(a, too_long);
            return hit1.begin() == a.begin() && hit1.end() == a.begin() + 3 &&
                   hit2.begin() == a.begin() + 6 && hit2.end() == a.end() &&
                   hit3.begin() == a.begin() && hit3.end() == a.end() &&
                   m1.begin() == a.end() && m1.end() == a.end() &&
                   m2.begin() == a.end() && m2.end() == a.end();
        }());
    }
}

RTTS_CASE_TPL("raze::algorithm::search", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    search_suite<T>();
};

RTTS_CASE_TPL("raze::algorithm::search boundaries and decoys", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    search_boundary_sweep<T>();
    search_decoys_and_overlaps<T>();
};

RTTS_CASE_TPL("raze::algorithm::search fuzz", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    for (unsigned seed : fuzz_seeds)
        search_fuzz<T>(seed, 150);
};

RTTS_CASE("raze::algorithm::search floating point -0.0 and NaN") {
    namespace opt = raze::options;

    auto check_fp = []<class F>() {
        const F nan = std::numeric_limits<F>::quiet_NaN();
        const F pos0 = F(+0.0);
        const F neg0 = F(-0.0);

        // -0.0 == +0.0 across first, middle, and last positions (SIMD & tail)
        for (size_t sz : { size_t(4), size_t(36), size_t(70) }) {
            std::vector<F> hay(sz, F(1.0));
            const size_t pos = sz - 4;
            hay[pos + 0] = neg0;
            hay[pos + 1] = neg0;
            hay[pos + 2] = neg0;
            hay[pos + 3] = neg0;
            std::vector<F> pat = { pos0, pos0, pos0, pos0 };

            auto exp = std::ranges::search(hay, pat);
            auto r1 = raze::algorithm::search(hay, pat);
            auto r2 = raze::algorithm::search[opt::fscalar](hay, pat);
            auto r3 = raze::algorithm::search[opt::fstatic](hay, pat);
            auto r4 = raze::algorithm::search[opt::unroll<2>](hay, pat);
            RTTS_EXPECT(r1.begin() == exp.begin() && r1.end() == exp.end());
            RTTS_EXPECT(r2.begin() == exp.begin() && r2.end() == exp.end());
            RTTS_EXPECT(r3.begin() == exp.begin() && r3.end() == exp.end());
            RTTS_EXPECT(r4.begin() == exp.begin() && r4.end() == exp.end());

            // Long middle containing -0.0 vs +0.0 (exercises SIMD verify_middle & verify_tail_long)
            if (sz >= 36) {
                std::vector<F> long_h(sz, neg0);
                std::vector<F> long_p(sz, pos0);
                auto lr1 = raze::algorithm::search(long_h, long_p);
                RTTS_EXPECT(lr1.begin() == long_h.begin() && lr1.end() == long_h.end());
            }

            // NaN != NaN in first, middle, and last
            std::vector<F> nan_h(sz, F(2.0));
            nan_h[pos + 0] = F(3.0);
            nan_h[pos + 1] = nan;
            nan_h[pos + 2] = F(4.0);
            nan_h[pos + 3] = F(5.0);
            std::vector<F> nan_p = { F(3.0), nan, F(4.0), F(5.0) };

            RTTS_EXPECT(raze::algorithm::search(nan_h, nan_p).begin() == nan_h.end());
            RTTS_EXPECT(raze::algorithm::search[opt::fscalar](nan_h, nan_p).begin() == nan_h.end());
            RTTS_EXPECT(raze::algorithm::search[opt::fstatic](nan_h, nan_p).begin() == nan_h.end());

            std::vector<F> nan1 = { nan };
            RTTS_EXPECT(raze::algorithm::search(nan_h, nan1).begin() == nan_h.end());
        }
    };

    check_fp.template operator()<float>();
    check_fp.template operator()<double>();
};

RTTS_CASE("raze::algorithm::search predicates, projections, containers and structs") {
    namespace opt = raze::options;

    // Custom vectorizable predicates (std::less, std::not_equal_to, bitwise lambda)
    {
        std::vector<int32_t> hay(100);
        for (size_t i = 0; i < hay.size(); ++i) hay[i] = static_cast<int32_t>(i);
        const std::vector<int32_t> pat{ 50, 51, 52, 53 };

        auto check_pred = [&](auto pred) {
            auto exp = std::ranges::search(hay, pat, pred);
            auto r1 = raze::algorithm::search(hay, pat, pred);
            auto r2 = raze::algorithm::search[opt::fscalar](hay, pat, pred);
            auto r3 = raze::algorithm::search[opt::fstatic](hay, pat, pred);
            auto r4 = raze::algorithm::search[opt::unroll<2>](hay, pat, pred);
            RTTS_EXPECT(r1.begin() == exp.begin() && r1.end() == exp.end());
            RTTS_EXPECT(r2.begin() == exp.begin() && r2.end() == exp.end());
            RTTS_EXPECT(r3.begin() == exp.begin() && r3.end() == exp.end());
            RTTS_EXPECT(r4.begin() == exp.begin() && r4.end() == exp.end());
        };

        check_pred(std::less<>{});
        check_pred(std::greater<>{});
        check_pred(std::not_equal_to<>{});
        check_pred([](auto a, auto b) { return (a & 7) == (b & 7); });
    }

    // Projections & structs
    {
        const std::vector<search_item> items{
            { 1, 10 }, { 2, 20 }, { 3, 30 }, { 2, 40 }, { 3, 50 }, { 4, 60 }
        };
        const std::vector<int32_t> keys{ 2, 3 };
        auto res = raze::algorithm::search(items, keys, std::equal_to<>{}, &search_item::key);
        RTTS_EXPECT(res.begin() == items.begin() + 1 && res.end() == items.begin() + 3);

        const std::vector<search_item> pat{ { 9, 200 }, { 8, 300 } };
        auto res2 = raze::algorithm::search(
            items, pat,
            [](int32_t a, int32_t b) { return a * 10 == b; },
            &search_item::value, &search_item::value);
        RTTS_EXPECT(res2.begin() == items.begin() + 1 && res2.end() == items.begin() + 3);

        const std::string s = "abracadabra";
        auto sr = raze::algorithm::search(s, std::string_view("abra"));
        RTTS_EXPECT(sr.begin() == s.begin() && sr.end() == s.begin() + 4);
        auto sr2 = raze::algorithm::search(s, std::string_view("cada"));
        RTTS_EXPECT(sr2.begin() == s.begin() + 4 && sr2.end() == s.begin() + 8);
    }

    // Non-contiguous & mixed containers (deque, list, forward_list, vector+list, vector+forward_list)
    {
        const std::vector<int> vec{ 1, 2, 3, 4, 2, 3, 5 };
        const std::deque<int> deq(vec.begin(), vec.end());
        const std::list<int> lst(vec.begin(), vec.end());
        const std::forward_list<int> flst(vec.begin(), vec.end());

        const std::list<int> pat_lst{ 2, 3, 5 };
        const std::forward_list<int> pat_flst{ 2, 3, 5 };
        const std::vector<int> pat_vec{ 2, 3, 5 };

        auto check_c = [](const auto& h, const auto& p) {
            auto r = raze::algorithm::search(h, p);
            auto e = std::ranges::search(h, p);
            RTTS_EXPECT(r.begin() == e.begin() && r.end() == e.end());
        };

        check_c(deq, pat_vec);
        check_c(deq, pat_lst);
        check_c(lst, pat_lst);
        check_c(lst, pat_vec);
        check_c(flst, pat_flst);
        check_c(flst, pat_vec);
        check_c(vec, pat_lst);
        check_c(vec, pat_flst);
    }

    // Static std::array constexpr_sized_source runtime path
    {
        std::array<int, 64> h{};
        for (size_t i = 0; i < h.size(); ++i) h[i] = static_cast<int>(i);
        std::array<int, 4> p{ 41, 42, 43, 44 };
        auto r = raze::algorithm::search(h, p);
        RTTS_EXPECT(r.begin() == h.begin() + 41 && r.end() == h.begin() + 45);
    }
};

RTTS_CASE("raze::algorithm::search views and edge cases") {
    namespace opt = raze::options;
    namespace views = std::views;

    // Views: iota, take, drop, reverse, filter, transform, non-common
    {
        auto iv = views::iota(0, 64);
        auto pv = views::iota(30, 35);
        auto r1 = raze::algorithm::search(iv, pv);
        auto s1 = std::ranges::search(iv, pv);
        RTTS_EXPECT(r1.begin() == s1.begin() && r1.end() == s1.end());

        const std::vector<int> src = { 1, 2, 3, 2, 3, 4, 2, 3, 4, 5, 6 };
        auto dv = src | views::drop(1) | views::take(8);
        std::vector<int> pat = { 2, 3, 4 };
        auto r2 = raze::algorithm::search[opt::fstatic](dv, pat);
        auto s2 = std::ranges::search(dv, pat);
        RTTS_EXPECT(r2.begin() == s2.begin() && r2.end() == s2.end());

        auto rv = src | views::reverse;
        std::vector<int> rpat = { 4, 3, 2 };
        auto r3 = raze::algorithm::search(rv, rpat);
        auto s3 = std::ranges::search(rv, rpat);
        RTTS_EXPECT(r3.begin() == s3.begin() && r3.end() == s3.end());

        auto fv = src | views::filter([](int x) { return x != 1; });
        auto tv = views::iota(2, 5) | views::transform([](int x) { return x; });
        auto r4 = raze::algorithm::search[opt::aligned<64, opt::unaligned>](fv, tv);
        auto s4 = std::ranges::search(fv, tv);
        RTTS_EXPECT(r4.begin() == s4.begin() && r4.end() == s4.end());

        auto nc_h = rtts::make_non_common(src);
        auto nc_p = rtts::make_non_common(pat);
        auto r5 = raze::algorithm::search[opt::fautovec][opt::fstatic](
            nc_h, nc_p, [](auto a, auto b) { if (a == b) return true; return false; });
        auto s5 = std::ranges::search(nc_h, nc_p);
        RTTS_EXPECT(r5.begin() == s5.begin() && r5.end() == s5.end());

        // Non-common miss
        std::vector<int> miss_pat = { 9, 9, 9 };
        auto nc_m = rtts::make_non_common(miss_pat);
        auto r6 = raze::algorithm::search(nc_h, nc_m);
        auto s6 = std::ranges::search(nc_h, nc_m);
        RTTS_EXPECT(r6.begin() == s6.begin() && r6.end() == s6.end());
    }

    // Volatile, rvalue dangling, member projection
    {
        volatile int vh[20], vp[4];
        for (int i = 0; i < 20; ++i) vh[i] = i;
        for (int i = 0; i < 4; ++i) vp[i] = 10 + i;
        auto vr = raze::algorithm::search(vh, vh + 20, vp, vp + 4);
        RTTS_EXPECT(vr.begin() == vh + 10 && vr.end() == vh + 14);
        vp[2] = 99;
        auto vm = raze::algorithm::search(vh, vh + 20, vp, vp + 4);
        RTTS_EXPECT(vm.begin() == vh + 20 && vm.end() == vh + 20);

        auto rval = raze::algorithm::search(std::vector<int>{ 1, 2, 3, 4 }, std::vector<int>{ 2, 3 });
        static_assert(std::same_as<decltype(rval), std::ranges::dangling>);

        const std::vector<rtts::test_item> i1 = { { 1, 10 }, { 2, 20 }, { 3, 30 }, { 2, 40 }, { 3, 50 } };
        const std::vector<rtts::test_item> i2 = { { 2, 99 }, { 3, 88 } };
        auto ir = raze::algorithm::search(i1, i2, std::equal_to<>{}, &rtts::test_item::key, &rtts::test_item::key);
        RTTS_EXPECT(ir.begin() == i1.begin() + 1 && ir.end() == i1.begin() + 3);
    }
};

