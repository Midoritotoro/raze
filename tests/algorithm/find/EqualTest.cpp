#include <tests/rts/rts.h>
#include <raze/algorithm/find/Equal.h>

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

    struct equal_item {
        int32_t key;
        int32_t value;
        bool operator==(const equal_item&) const = default;
    };

    template <class T>
    void check_equal_variants(const std::vector<T>& a, const std::vector<T>& b) {
        const bool exp = std::ranges::equal(a, b);

        RTTS_EXPECT(raze::algorithm::equal(a, b) == exp);
        RTTS_EXPECT(raze::algorithm::equal(a.begin(), a.end(), b.begin(), b.end()) == exp);
        RTTS_EXPECT(raze::algorithm::equal[raze::options::fscalar](a, b) == exp);
        RTTS_EXPECT(raze::algorithm::equal[raze::options::unroll<2>](a, b) == exp);
        RTTS_EXPECT(raze::algorithm::equal[raze::options::fstatic](a, b) == exp);

        auto lambda_pred = [](T x, T y) { return x == y; };
        RTTS_EXPECT(raze::algorithm::equal(a, b, lambda_pred) == exp);
        RTTS_EXPECT(raze::algorithm::equal[raze::options::fscalar](a, b, lambda_pred) == exp);
    }

    template <class T>
    void equal_boundary_sweep() {
        std::vector<size_t> sizes;
        for (size_t n = 0; n <= 140; ++n)
            sizes.push_back(n);
        for (size_t n : { size_t(255), size_t(256), size_t(257), size_t(511), size_t(512), size_t(513), size_t(1024) })
            sizes.push_back(n);

        for (size_t n : sizes) {
            std::vector<T> a(n);
            for (size_t i = 0; i < n; ++i)
                a[i] = static_cast<T>((i % 100) + 1);

            auto b = a;
            check_equal_variants(a, b);

            if (n > 0) {
                std::vector<T> shorter(a.begin(), a.end() - 1);
                check_equal_variants(a, shorter);
                check_equal_variants(shorter, a);

                std::vector<size_t> positions;
                if (n <= 64) {
                    for (size_t p = 0; p < n; ++p)
                        positions.push_back(p);
                }
                else {
                    positions = { 0, 1, 15, 16, 31, 32, 63, 64, n / 2, n - 2, n - 1 };
                }

                for (size_t pos : positions) {
                    auto diff = a;
                    diff[pos] = T(0);
                    check_equal_variants(a, diff);
                }
            }
        }
    }

    template <class T>
    void equal_fuzz(unsigned seed, size_t iters) {
        std::mt19937 rng(seed);
        std::uniform_int_distribution<size_t> sz(0, 512);

        for (size_t i = 0; i < iters; ++i) {
            const size_t n = sz(rng);
            auto a = rtts::random::vector<T>(n, seed + static_cast<unsigned>(i));
            auto b = a;

            RTTS_EXPECT(raze::algorithm::equal(a, b));
            RTTS_EXPECT(raze::algorithm::equal[raze::options::fscalar](a.begin(), a.end(), b.begin(), b.end()));
            RTTS_EXPECT(raze::algorithm::equal[raze::options::fstatic](a, b));

            if (n > 0) {
                const size_t pos = rng() % n;
                b[pos] = static_cast<T>(b[pos] + T(1));
                const bool exp = std::ranges::equal(a, b);

                RTTS_EXPECT(raze::algorithm::equal(a, b) == exp);
                RTTS_EXPECT(raze::algorithm::equal[raze::options::fscalar](a, b) == exp);
                RTTS_EXPECT(raze::algorithm::equal[raze::options::fstatic](a, b) == exp);
            }

            const size_t first = n == 0 ? 0 : rng() % (n + 1);
            const size_t last = first + (n == first ? 0 : rng() % (n - first + 1));
            RTTS_EXPECT(raze::algorithm::equal(a.begin() + first, a.begin() + last, a.begin() + first, a.begin() + last));
        }
    }

    template <class T>
    void equal_suite() {
        using namespace rtts::algorithm;
        namespace opt = raze::options;
        auto cfg = config::thorough();

        auto check_self_iter = [&](auto algo) {
            run<range, T>(cfg,
                [algo](auto c) { return algo(c.first(), c.last(), c.first(), c.last()); },
                [](auto c) { return std::ranges::equal(c.first(), c.last(), c.first(), c.last()); });
        };

        auto check_self_range = [&](auto algo) {
            run<range, T>(cfg,
                [algo](auto c) { return algo(c.range(), c.range()); },
                [](auto c) { return std::ranges::equal(c.range(), c.range()); });
        };

        check_self_iter(raze::algorithm::equal);
        check_self_range(raze::algorithm::equal);
        check_self_iter(raze::algorithm::equal[opt::fscalar]);
        check_self_range(raze::algorithm::equal[opt::unroll<2>]);
        check_self_range(raze::algorithm::equal[opt::fstatic]);
        check_self_iter(raze::algorithm::equal[opt::fautovec]);

        run<range, T>(cfg,
            [](auto c) { return raze::algorithm::equal(c.first(), c.last(), c.first2(), c.last2()); },
            [](auto c) { return std::ranges::equal(c.first(), c.last(), c.first2(), c.last2()); });

        run<range, T>(cfg,
            [](auto c) { return raze::algorithm::equal(c.range(), c.range2()); },
            [](auto c) { return std::ranges::equal(c.range(), c.range2()); });

        auto eq_branching = [](auto a, auto b) { if (a == b) return true; return false; };

        run<range, T>(cfg,
            [eq_branching](auto c) { return raze::algorithm::equal[opt::fautovec][opt::fstatic](c.range(), c.range2(), eq_branching); },
            [eq_branching](auto c) { return std::ranges::equal(c.range(), c.range2(), eq_branching); });

        {
            auto acfg = config::compact();

            auto check_aligned = [&]<size_t A1, size_t A2>(auto algo) {
                run_aligned<A1, A2, range, T>(acfg,
                    [algo](auto c) { return algo(c.first(), c.last(), c.first2(), c.last2()); },
                    [](auto c) { return std::ranges::equal(c.first(), c.last(), c.first2(), c.last2()); });

                run_aligned<A1, A2, range, T>(acfg,
                    [algo](auto c) { return algo(c.range(), c.range2()); },
                    [](auto c) { return std::ranges::equal(c.range(), c.range2()); });

                for (size_t sz : { size_t(0), size_t(1), size_t(15), size_t(16), size_t(31), size_t(32), size_t(64), size_t(128) }) {
                    rtts::aligned_buffer<T, A1> a(sz);
                    rtts::aligned_buffer<T, A2> b(sz);
                    for (size_t i = 0; i < sz; ++i) { a[i] = static_cast<T>(i + 1); b[i] = a[i]; }
                    RTTS_EXPECT(algo(a, b));
                    if (sz > 0) {
                        b[sz - 1] = T(0);
                        RTTS_EXPECT(!algo(a, b));
                    }
                }
            };

            check_aligned.template operator()<16, 16>(raze::algorithm::equal[opt::aligned<16>]);
            check_aligned.template operator()<32, 32>(raze::algorithm::equal[opt::aligned<32>]);
            check_aligned.template operator()<32, 32>(raze::algorithm::equal[opt::aligned<32>][opt::unroll<2>]);
            check_aligned.template operator()<64, 64>(raze::algorithm::equal[opt::aligned<64>]);
            check_aligned.template operator()<64, 64>(raze::algorithm::equal[opt::aligned<64>][opt::fautovec]);
            check_aligned.template operator()<64, 64>(raze::algorithm::equal[opt::aligned<64>][opt::fscalar]);
            check_aligned.template operator()<1, 1>(raze::algorithm::equal[opt::aligned<opt::unaligned>]);
            check_aligned.template operator()<1, 1>(raze::algorithm::equal[opt::aligned<64>][opt::aligned<opt::unaligned>]);
            check_aligned.template operator()<64, 1>(raze::algorithm::equal[opt::aligned<64, opt::unaligned>]);
            check_aligned.template operator()<1, 64>(raze::algorithm::equal[opt::aligned<opt::unaligned, 64>]);
            check_aligned.template operator()<32, 64>(raze::algorithm::equal[opt::aligned<32, 64>][opt::fstatic]);
            check_aligned.template operator()<64, 16>(raze::algorithm::equal[opt::aligned<64, 16>]);
        }

        static_assert(constexpr_run<T, range, 0>(
            [](auto c) { return raze::algorithm::equal(c.first(), c.last(), c.first2(), c.last2()); },
            [](auto c) { return std::ranges::equal(c.first(), c.last(), c.first2(), c.last2()); }));

        constexpr auto check_ce_self = [](auto algo) {
            return constexpr_run<T, range, 5>(
                [algo](auto c) { return algo(c.range(), c.range()); },
                [](auto c) { return std::ranges::equal(c.range(), c.range()); });
        };

        constexpr auto check_ce_pair = [](auto algo) {
            return constexpr_run<T, range, 5>(
                [algo](auto c) { return algo(c.range(), c.range2()); },
                [](auto c) { return std::ranges::equal(c.range(), c.range2()); });
        };

        static_assert(check_ce_self(raze::algorithm::equal));
        static_assert(check_ce_self(raze::algorithm::equal[opt::fautovec]));
        static_assert(check_ce_pair(raze::algorithm::equal));
        static_assert(check_ce_pair(raze::algorithm::equal[opt::aligned<64, opt::unaligned>]));

        static_assert([]() constexpr {
            std::array<T, 0> e1{}, e2{};
            std::array<T, 2> t2{ T(1), T(2) };
            std::array<T, 4> a4{ T(1), T(2), T(3), T(4) };
            std::array<T, 4> b4{ T(1), T(2), T(3), T(4) };
            std::array<T, 4> c4{ T(1), T(2), T(9), T(4) };
            std::array<T, 5> a5{ T(1), T(2), T(3), T(4), T(5) };
            std::array<T, 5> b5{ T(1), T(2), T(3), T(4), T(5) };
            return raze::algorithm::equal(e1, e2) &&
                  !raze::algorithm::equal(e1, t2) &&
                  !raze::algorithm::equal(t2, e1) &&
                   raze::algorithm::equal(a4, b4) &&
                  !raze::algorithm::equal(a4, c4) &&
                   raze::algorithm::equal[opt::fscalar](a5, b5) &&
                  !raze::algorithm::equal[opt::fscalar](a5, a4) &&
                   raze::algorithm::equal(a5.begin(), a5.begin() + 4, a4.begin(), a4.end());
        }());
    }
}

RTTS_CASE_TPL("raze::algorithm::equal", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    equal_suite<T>();
};

RTTS_CASE_TPL("raze::algorithm::equal boundaries", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    equal_boundary_sweep<T>();
};

RTTS_CASE_TPL("raze::algorithm::equal fuzz", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    for (unsigned seed : fuzz_seeds)
        equal_fuzz<T>(seed, 200);
};

RTTS_CASE("raze::algorithm::equal structs, projections and mixed") {
    {
        const std::vector<equal_item> a{ { 1, 10 }, { 2, 20 }, { 3, 30 } };
        const std::vector<equal_item> b{ { 1, 10 }, { 2, 20 }, { 3, 30 } };
        const std::vector<equal_item> c{ { 1, 99 }, { 2, 20 }, { 3, 30 } };

        RTTS_EXPECT(raze::algorithm::equal(a, b));
        RTTS_EXPECT(!raze::algorithm::equal(a, c));

        auto key_proj = [](const auto& item) { return item.key; };
        auto val_proj = [](const auto& item) { return item.value; };
        RTTS_EXPECT(raze::algorithm::equal(a, c, std::equal_to<>{}, key_proj, key_proj));
        RTTS_EXPECT(!raze::algorithm::equal[raze::options::fscalar](a, c, std::equal_to<>{}, val_proj, val_proj));
    }

    {
        const std::vector<std::string> w1{ "alpha", "beta", "gamma" };
        const std::vector<std::string> w2{ "alpha", "beta", "gamma" };
        const std::vector<std::string> w3{ "alpha", "beta" };
        RTTS_EXPECT(raze::algorithm::equal(w1, w2));
        RTTS_EXPECT(!raze::algorithm::equal(w1, w3));
    }

    {
        const std::vector<int32_t> ints3{ 1, 2, 3 };
        const std::vector<double> dbls3{ 1.0, 2.0, 3.0 };
        const std::vector<int64_t> i64_3{ 1, 2, 3 };
        RTTS_EXPECT(raze::algorithm::equal(ints3, dbls3));
        RTTS_EXPECT(raze::algorithm::equal(ints3, i64_3));

        const std::vector<int32_t> ints2{ 1, 2 };  
        const std::vector<int64_t> i64_1{ 1 }; 
        RTTS_EXPECT(!raze::algorithm::equal(ints2, i64_1));
        RTTS_EXPECT(!raze::algorithm::equal(i64_1, ints2));
    }

    {
        const std::list<int> l2{ 1, 2 };
        const std::forward_list<int> fl2{ 1, 2 };
        const std::forward_list<int> fl3{ 1, 2, 3 };
        const std::vector<int> v3{ 1, 2, 3 };

        RTTS_EXPECT(raze::algorithm::equal(l2, fl2));
        RTTS_EXPECT(!raze::algorithm::equal(l2, fl3));
        RTTS_EXPECT(!raze::algorithm::equal(fl3, l2));
        RTTS_EXPECT(raze::algorithm::equal(v3, fl3));
        RTTS_EXPECT(!raze::algorithm::equal(v3, fl2));
        RTTS_EXPECT(!raze::algorithm::equal(fl2, v3));
    }
};

RTTS_CASE("raze::algorithm::equal sentinel") {
    {
        std::array<int, 3> buf{ 1, 2, 3 };
        auto e1 = std::counted_iterator(buf.begin(), 0);
        auto e2 = std::counted_iterator(buf.begin(), 0);
        auto n1 = std::counted_iterator(buf.begin(), 1);

        RTTS_EXPECT(raze::algorithm::equal(e1, std::default_sentinel, e2, std::default_sentinel));
        RTTS_EXPECT(!raze::algorithm::equal(e1, std::default_sentinel, n1, std::default_sentinel));
        RTTS_EXPECT(!raze::algorithm::equal(n1, std::default_sentinel, e1, std::default_sentinel));
    }

    for (size_t n : { size_t(16), size_t(64), size_t(300) }) {
        std::vector<int> a(n, 7);
        std::vector<int> b(n, 7);

        auto fa = std::counted_iterator(a.begin(), static_cast<std::ptrdiff_t>(a.size()));
        auto fb = std::counted_iterator(b.begin(), static_cast<std::ptrdiff_t>(b.size()));
        auto fb_short = std::counted_iterator(b.begin(), static_cast<std::ptrdiff_t>(b.size() - 1));

        RTTS_EXPECT(raze::algorithm::equal(fa, std::default_sentinel, fb, std::default_sentinel));
        RTTS_EXPECT(!raze::algorithm::equal(fa, std::default_sentinel, fb_short, std::default_sentinel));

        b[0] = 99;
        RTTS_EXPECT(!raze::algorithm::equal(fa, std::default_sentinel, fb, std::default_sentinel));

        b[0] = 7;
        b[n - 1] = 99;
        RTTS_EXPECT(!raze::algorithm::equal(fa, std::default_sentinel, fb, std::default_sentinel));
    }
};


RTTS_CASE("raze::algorithm::equal views and edge cases") {
    namespace opt = raze::options;
    namespace views = std::views;

    // Pairs of views: iota, take, drop, reverse, filter, transform, non-common
    {
        const std::vector<int> a = { 2, 4, 6, 8, 10, 12, 14, 16, 18, 20 };
        auto tv = views::iota(1, 11) | views::transform([](int x) { return x * 2; });
        RTTS_EXPECT(raze::algorithm::equal(a, tv));
        RTTS_EXPECT(raze::algorithm::equal[opt::fautovec][opt::fstatic](
            a, tv, [](auto x, auto y) { if (x == y) return true; return false; }));

        alignas(64) const std::array<int, 10> a_aligned = { 2, 4, 6, 8, 10, 12, 14, 16, 18, 20 };
        auto fv = views::iota(1, 21) | views::filter([](int x) { return (x & 1) == 0; });
        RTTS_EXPECT((raze::algorithm::equal[opt::aligned<64, opt::unaligned>](a_aligned, fv)));
        RTTS_EXPECT(!raze::algorithm::equal(a | views::reverse, fv));
        RTTS_EXPECT(raze::algorithm::equal(a | views::reverse, fv | views::reverse));
        RTTS_EXPECT(raze::algorithm::equal(rtts::make_non_common(a), fv));
    }

    // Mismatched lengths, empty, single-pass input ranges, volatile, member projections
    {
        const std::vector<int> s = { 1, 2, 3 };
        const std::vector<int> l = { 1, 2, 3, 4 };
        std::vector<int> empty_v;
        RTTS_EXPECT(!raze::algorithm::equal(s, l));
        RTTS_EXPECT(!raze::algorithm::equal(l, s));
        RTTS_EXPECT(raze::algorithm::equal(empty_v, empty_v));
        RTTS_EXPECT(!raze::algorithm::equal(empty_v, s));

        rtts::single_pass_input_range<int> sp1(s), sp2(s);
        RTTS_EXPECT(raze::algorithm::equal(sp1, sp2));
        RTTS_EXPECT(sp1.reads() == s.size() && sp2.reads() == s.size());

        rtts::single_pass_input_range<int> sp3(s), sp4(l);
        RTTS_EXPECT(!raze::algorithm::equal(sp3, sp4));

        volatile int va[16], vb[16];
        for (int i = 0; i < 16; ++i) { va[i] = i; vb[i] = i; }
        RTTS_EXPECT(raze::algorithm::equal(va, va + 16, vb, vb + 16));
        vb[11] = 999;
        RTTS_EXPECT(!raze::algorithm::equal(va, va + 16, vb, vb + 16));

        const std::vector<rtts::test_item> i1 = { { 1, 10 }, { 2, 20 }, { 3, 30 } };
        const std::vector<rtts::test_item> i2 = { { 1, 99 }, { 2, 88 }, { 3, 77 } };
        RTTS_EXPECT(raze::algorithm::equal(i1, i2, std::equal_to<>{}, &rtts::test_item::key, &rtts::test_item::key));
        RTTS_EXPECT(!raze::algorithm::equal(i1, i2, std::equal_to<>{}, &rtts::test_item::payload, &rtts::test_item::payload));
    }
};
