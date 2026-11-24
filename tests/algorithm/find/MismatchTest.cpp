#include <tests/rts/rts.h>
#include <raze/algorithm/find/Mismatch.h>

#include <algorithm>
#include <array>
#include <cstdint>
#include <functional>
#include <iterator>
#include <random>
#include <ranges>
#include <string>
#include <vector>

namespace {
    inline constexpr unsigned fuzz_seeds[] = { 42u, 0x9E3779B9u, 0xDEADBEEFu };

    struct mismatch_item {
        int32_t key;
        int32_t value;
        bool operator==(const mismatch_item&) const = default;
    };

    template <class T>
    void check_mismatch_variants(const std::vector<T>& a, const std::vector<T>& b) {
        auto exp = std::ranges::mismatch(a, b);
        const auto exp_off1 = exp.in1 - a.begin();
        const auto exp_off2 = exp.in2 - b.begin();

        auto r = raze::algorithm::mismatch(a, b);
        RTTS_EXPECT(r.in1 - a.begin() == exp_off1);
        RTTS_EXPECT(r.in2 - b.begin() == exp_off2);

        r = raze::algorithm::mismatch(a.begin(), a.end(), b.begin(), b.end());
        RTTS_EXPECT(r.in1 - a.begin() == exp_off1);
        RTTS_EXPECT(r.in2 - b.begin() == exp_off2);

        r = raze::algorithm::mismatch[raze::options::fscalar](a, b);
        RTTS_EXPECT(r.in1 - a.begin() == exp_off1);
        RTTS_EXPECT(r.in2 - b.begin() == exp_off2);

        r = raze::algorithm::mismatch[raze::options::unroll<2>](a, b);
        RTTS_EXPECT(r.in1 - a.begin() == exp_off1);
        RTTS_EXPECT(r.in2 - b.begin() == exp_off2);

        r = raze::algorithm::mismatch[raze::options::fstatic](a, b);
        RTTS_EXPECT(r.in1 - a.begin() == exp_off1);
        RTTS_EXPECT(r.in2 - b.begin() == exp_off2);

        auto lambda_pred = [](T x, T y) { return x == y; };
        r = raze::algorithm::mismatch(a, b, lambda_pred);
        RTTS_EXPECT(r.in1 - a.begin() == exp_off1);
        RTTS_EXPECT(r.in2 - b.begin() == exp_off2);

        r = raze::algorithm::mismatch[raze::options::fscalar](a, b, lambda_pred);
        RTTS_EXPECT(r.in1 - a.begin() == exp_off1);
        RTTS_EXPECT(r.in2 - b.begin() == exp_off2);
    }

    template <class T>
    void mismatch_boundary_sweep() {
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
            check_mismatch_variants(a, b);

            if (n > 0) {
                std::vector<T> shorter(a.begin(), a.end() - 1);
                check_mismatch_variants(a, shorter);
                check_mismatch_variants(shorter, a);

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
                    check_mismatch_variants(a, diff);
                }
            }
        }
    }

    template <class T>
    void mismatch_fuzz(unsigned seed, size_t iters) {
        std::mt19937 rng(seed);
        std::uniform_int_distribution<size_t> sz(0, 512);

        for (size_t i = 0; i < iters; ++i) {
            const size_t n1 = sz(rng);
            const size_t n2 = sz(rng);
            auto a = rtts::random::vector<T>(n1, seed + static_cast<unsigned>(i));
            auto b = rtts::random::vector<T>(n2, seed + 100000u + static_cast<unsigned>(i));

            const size_t common = (n1 == 0 || n2 == 0) ? 0 : rng() % (std::min(n1, n2) + 1);
            for (size_t j = 0; j < common; ++j)
                b[j] = a[j];

            auto exp = std::ranges::mismatch(a, b);
            auto r1 = raze::algorithm::mismatch(a, b);
            auto r2 = raze::algorithm::mismatch[raze::options::fscalar](a.begin(), a.end(), b.begin(), b.end());
            auto r3 = raze::algorithm::mismatch[raze::options::fstatic](a, b);

            RTTS_EXPECT(r1.in1 == exp.in1 && r1.in2 == exp.in2);
            RTTS_EXPECT(r2.in1 == exp.in1 && r2.in2 == exp.in2);
            RTTS_EXPECT(r3.in1 == exp.in1 && r3.in2 == exp.in2);

            const size_t f1 = n1 == 0 ? 0 : rng() % (n1 + 1);
            const size_t l1 = f1 + (n1 == f1 ? 0 : rng() % (n1 - f1 + 1));
            const size_t f2 = n2 == 0 ? 0 : rng() % (n2 + 1);
            const size_t l2 = f2 + (n2 == f2 ? 0 : rng() % (n2 - f2 + 1));

            auto sub_exp = std::ranges::mismatch(a.begin() + f1, a.begin() + l1, b.begin() + f2, b.begin() + l2);
            auto sub_r = raze::algorithm::mismatch(a.begin() + f1, a.begin() + l1, b.begin() + f2, b.begin() + l2);
            RTTS_EXPECT(sub_r.in1 == sub_exp.in1 && sub_r.in2 == sub_exp.in2);
        }
    }

    template <class T>
    void mismatch_suite() {
        using namespace rtts::algorithm;
        auto cfg = config::thorough();

        run<range, T>(cfg,
            [](auto c) { return raze::algorithm::mismatch(c.first(), c.last(), c.first2(), c.last2()); },
            [](auto c) { return std::ranges::mismatch(c.first(), c.last(), c.first2(), c.last2()); });

        run<range, T>(cfg,
            [](auto c) { return raze::algorithm::mismatch(c.range(), c.range2()); },
            [](auto c) { return std::ranges::mismatch(c.range(), c.range2()); });

        run<range, T>(cfg,
            [](auto c) { return raze::algorithm::mismatch[raze::options::fscalar](c.first(), c.last(), c.first2(), c.last2()); },
            [](auto c) { return std::ranges::mismatch(c.first(), c.last(), c.first2(), c.last2()); });

        run<range, T>(cfg,
            [](auto c) { return raze::algorithm::mismatch[raze::options::unroll<2>](c.range(), c.range2()); },
            [](auto c) { return std::ranges::mismatch(c.range(), c.range2()); });

        run<range, T>(cfg,
            [](auto c) { return raze::algorithm::mismatch[raze::options::fstatic](c.range(), c.range2()); },
            [](auto c) { return std::ranges::mismatch(c.range(), c.range2()); });

        auto eq_branching = [](auto a, auto b) { if (a == b) return true; return false; };

        run<range, T>(cfg,
            [](auto c) { return raze::algorithm::mismatch[raze::options::fautovec](c.first(), c.last(), c.first2(), c.last2()); },
            [](auto c) { return std::ranges::mismatch(c.first(), c.last(), c.first2(), c.last2()); });

        run<range, T>(cfg,
            [eq_branching](auto c) { return raze::algorithm::mismatch[raze::options::fautovec][raze::options::fstatic](c.range(), c.range2(), eq_branching); },
            [eq_branching](auto c) { return std::ranges::mismatch(c.range(), c.range2(), eq_branching); });

        {
            namespace opt = raze::options;
            auto acfg = config::compact();

            auto check_aligned = [&]<size_t A1, size_t A2>(auto algo) {
                run_aligned<A1, A2, range, T>(acfg,
                    [algo](auto c) { return algo(c.first(), c.last(), c.first2(), c.last2()); },
                    [](auto c) { return std::ranges::mismatch(c.first(), c.last(), c.first2(), c.last2()); });

                run_aligned<A1, A2, range, T>(acfg,
                    [algo](auto c) { return algo(c.range(), c.range2()); },
                    [](auto c) { return std::ranges::mismatch(c.range(), c.range2()); });

                for (size_t sz : { size_t(0), size_t(1), size_t(15), size_t(16), size_t(31), size_t(32), size_t(64), size_t(128) }) {
                    rtts::aligned_buffer<T, A1> a(sz);
                    rtts::aligned_buffer<T, A2> b(sz);
                    for (size_t i = 0; i < sz; ++i) { a[i] = static_cast<T>(i + 1); b[i] = a[i]; }
                    auto r_eq = algo(a, b);
                    RTTS_EXPECT(r_eq.in1 == a.end() && r_eq.in2 == b.end());
                    if (sz > 0) {
                        b[sz - 1] = T(0);
                        auto r_diff = algo(a, b);
                        RTTS_EXPECT(r_diff.in1 == a.begin() + (sz - 1) && r_diff.in2 == b.begin() + (sz - 1));
                    }
                }
            };

            check_aligned.template operator()<16, 16>(raze::algorithm::mismatch[opt::aligned<16>]);
            check_aligned.template operator()<32, 32>(raze::algorithm::mismatch[opt::aligned<32>]);
            check_aligned.template operator()<32, 32>(raze::algorithm::mismatch[opt::aligned<32>][opt::unroll<2>]);
            check_aligned.template operator()<64, 64>(raze::algorithm::mismatch[opt::aligned<64>]);
            check_aligned.template operator()<64, 64>(raze::algorithm::mismatch[opt::aligned<64>][opt::fautovec]);
            check_aligned.template operator()<64, 64>(raze::algorithm::mismatch[opt::aligned<64>][opt::fscalar]);
            check_aligned.template operator()<1, 1>(raze::algorithm::mismatch[opt::aligned<opt::unaligned>]);
            check_aligned.template operator()<1, 1>(raze::algorithm::mismatch[opt::aligned<64>][opt::aligned<opt::unaligned>]);
            check_aligned.template operator()<64, 1>(raze::algorithm::mismatch[opt::aligned<64, opt::unaligned>]);
            check_aligned.template operator()<1, 64>(raze::algorithm::mismatch[opt::aligned<opt::unaligned, 64>]);
            check_aligned.template operator()<32, 64>(raze::algorithm::mismatch[opt::aligned<32, 64>][opt::fstatic]);
            check_aligned.template operator()<64, 16>(raze::algorithm::mismatch[opt::aligned<64, 16>]);
        }

        static_assert(constexpr_run<T, range, 0>(
            [](auto c) { return raze::algorithm::mismatch(c.first(), c.last(), c.first2(), c.last2()); },
            [](auto c) { return std::ranges::mismatch(c.first(), c.last(), c.first2(), c.last2()); }));

        static_assert(constexpr_run<T, range, 5>(
            [](auto c) { return raze::algorithm::mismatch(c.range(), c.range2()); },
            [](auto c) { return std::ranges::mismatch(c.range(), c.range2()); }));

        static_assert(constexpr_run<T, range, 5>(
            [](auto c) { return raze::algorithm::mismatch[raze::options::fautovec](c.range(), c.range2()); },
            [](auto c) { return std::ranges::mismatch(c.range(), c.range2()); }));

        static_assert(constexpr_run<T, range, 5>(
            [](auto c) { return raze::algorithm::mismatch[raze::options::aligned<64, raze::options::unaligned>](c.range(), c.range2()); },
            [](auto c) { return std::ranges::mismatch(c.range(), c.range2()); }));

        static_assert([]() constexpr {
            std::array<T, 0> a{};
            std::array<T, 0> b{};
            std::array<T, 3> c{ T(1), T(2), T(3) };

            auto r0 = raze::algorithm::mismatch(a, b);
            auto r1 = raze::algorithm::mismatch(a, c);
            auto r2 = raze::algorithm::mismatch(c, a);

            return r0.in1 == a.end() && r0.in2 == b.end() &&
                   r1.in1 == a.end() && r1.in2 == c.begin() &&
                   r2.in1 == c.begin() && r2.in2 == a.end();
            }());

        static_assert([]() constexpr {
            std::array<T, 4> a{ T(1), T(2), T(3), T(4) };
            std::array<T, 4> b{ T(1), T(2), T(3), T(4) };
            std::array<T, 4> c{ T(1), T(2), T(9), T(4) };

            auto r0 = raze::algorithm::mismatch(a, b);
            auto r1 = raze::algorithm::mismatch(a, c);

            return r0.in1 == a.end() && r0.in2 == b.end() &&
                   r1.in1 == a.begin() + 2 && r1.in2 == c.begin() + 2;
            }());

        static_assert([]() constexpr {
            std::array<T, 5> a{ T(1), T(2), T(3), T(4), T(5) };
            std::array<T, 3> b{ T(1), T(2), T(3) };

            auto r0 = raze::algorithm::mismatch[raze::options::fscalar](a, b);
            auto r1 = raze::algorithm::mismatch(b, a);

            return r0.in1 == a.begin() + 3 && r0.in2 == b.end() &&
                   r1.in1 == b.end() && r1.in2 == a.begin() + 3;
            }());
    }
}

RTTS_CASE_TPL("raze::algorithm::mismatch", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    mismatch_suite<T>();
};

RTTS_CASE_TPL("raze::algorithm::mismatch boundaries", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    mismatch_boundary_sweep<T>();
};

RTTS_CASE_TPL("raze::algorithm::mismatch fuzz", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    for (unsigned seed : fuzz_seeds)
        mismatch_fuzz<T>(seed, 200);
};

RTTS_CASE("raze::algorithm::mismatch structs, projections and mixed") {
    {
        const std::vector<mismatch_item> a{ { 1, 10 }, { 2, 20 }, { 3, 30 } };
        const std::vector<mismatch_item> b{ { 1, 10 }, { 2, 99 }, { 3, 30 } };

        auto r0 = raze::algorithm::mismatch(a, b);
        RTTS_EXPECT(r0.in1 == a.begin() + 1 && r0.in2 == b.begin() + 1);

        auto key_proj = [](const auto& item) { return item.key; };
        auto r1 = raze::algorithm::mismatch(a, b, std::equal_to<>{}, key_proj, key_proj);
        RTTS_EXPECT(r1.in1 == a.end() && r1.in2 == b.end());

        auto val_proj = [](const auto& item) { return item.value; };
        auto r2 = raze::algorithm::mismatch[raze::options::fscalar](a, b, std::equal_to<>{}, val_proj, val_proj);
        RTTS_EXPECT(r2.in1 == a.begin() + 1 && r2.in2 == b.begin() + 1);
    }

    {
        const std::vector<std::string> w1{ "alpha", "beta", "gamma" };
        const std::vector<std::string> w2{ "alpha", "beta", "delta" };
        const std::vector<std::string> w3{ "alpha", "beta" };

        auto r1 = raze::algorithm::mismatch(w1, w2);
        RTTS_EXPECT(r1.in1 == w1.begin() + 2 && r1.in2 == w2.begin() + 2);

        auto r2 = raze::algorithm::mismatch(w1, w3);
        RTTS_EXPECT(r2.in1 == w1.begin() + 2 && r2.in2 == w3.end());
    }

    {
        const std::vector<int32_t> ints3{ 1, 2, 9 };
        const std::vector<double> dbls3{ 1.0, 2.0, 3.0 };
        auto r1 = raze::algorithm::mismatch(ints3, dbls3);
        RTTS_EXPECT(r1.in1 == ints3.begin() + 2 && r1.in2 == dbls3.begin() + 2);

        const std::vector<int32_t> ints2{ 1, 2 };  
        const std::vector<int64_t> i64_1{ 1 }; 
        auto r2 = raze::algorithm::mismatch(ints2, i64_1);
        RTTS_EXPECT(r2.in1 == ints2.begin() + 1 && r2.in2 == i64_1.end());
    }

    {
        std::array<int, 100> arr{};
        arr.fill(7);
        const std::vector<int> short_vec{ 7, 7, 7 };

        auto r1 = raze::algorithm::mismatch(arr, short_vec);
        RTTS_EXPECT(r1.in1 == arr.begin() + 3 && r1.in2 == short_vec.end());

        auto r2 = raze::algorithm::mismatch(short_vec, arr);
        RTTS_EXPECT(r2.in1 == short_vec.end() && r2.in2 == arr.begin() + 3);
    }
};

RTTS_CASE("raze::algorithm::mismatch sentinel") {
    {
        std::array<int, 3> buf{ 1, 2, 3 };
        auto e1 = std::counted_iterator(buf.begin(), 0);
        auto e2 = std::counted_iterator(buf.begin(), 0);
        auto n1 = std::counted_iterator(buf.begin(), 1);

        auto r0 = raze::algorithm::mismatch(e1, std::default_sentinel, e2, std::default_sentinel);
        RTTS_EXPECT(r0.in1 == std::default_sentinel && r0.in2 == std::default_sentinel);

        auto r1 = raze::algorithm::mismatch(e1, std::default_sentinel, n1, std::default_sentinel);
        RTTS_EXPECT(r1.in1 == std::default_sentinel && r1.in2.base() == buf.begin());
    }

    for (size_t n : { size_t(16), size_t(64), size_t(300) }) {
        std::vector<int> a(n, 7);
        std::vector<int> b(n, 7);

        auto fa = std::counted_iterator(a.begin(), static_cast<std::ptrdiff_t>(a.size()));
        auto fb = std::counted_iterator(b.begin(), static_cast<std::ptrdiff_t>(b.size()));
        auto fb_short = std::counted_iterator(b.begin(), static_cast<std::ptrdiff_t>(b.size() - 3));

        auto r_eq = raze::algorithm::mismatch(fa, std::default_sentinel, fb, std::default_sentinel);
        RTTS_EXPECT(r_eq.in1 == std::default_sentinel && r_eq.in2 == std::default_sentinel);

        auto r_short = raze::algorithm::mismatch(fa, std::default_sentinel, fb_short, std::default_sentinel);
        RTTS_EXPECT(r_short.in1.base() == a.begin() + (n - 3) && r_short.in2 == std::default_sentinel);

        b[0] = 99;
        auto r_head = raze::algorithm::mismatch(fa, std::default_sentinel, fb, std::default_sentinel);
        RTTS_EXPECT(r_head.in1.base() == a.begin() && r_head.in2.base() == b.begin());

        b[0] = 7;
        b[n - 1] = 99;
        auto r_tail = raze::algorithm::mismatch(fa, std::default_sentinel, fb, std::default_sentinel);
        RTTS_EXPECT(r_tail.in1.base() == a.begin() + (n - 1) && r_tail.in2.base() == b.begin() + (n - 1));
    }
};


RTTS_CASE("raze::algorithm::mismatch views and edge cases") {
    namespace opt = raze::options;
    namespace views = std::views;

    // Pairs of views: iota, take, drop, reverse, filter, transform, non-common
    {
        const std::vector<int> a = { 2, 4, 6, 8, 99, 12, 14 };
        auto tv = views::iota(1, 10) | views::transform([](int x) { return x * 2; });
        auto rm = raze::algorithm::mismatch(a, tv);
        auto sm = std::ranges::mismatch(a, tv);
        RTTS_EXPECT(rm.in1 == sm.in1 && rm.in2 == sm.in2);

        auto fv = views::iota(1, 20) | views::filter([](int x) { return (x & 1) == 0; });
        auto rf = raze::algorithm::mismatch[opt::aligned<64, opt::unaligned>](a, fv);
        auto sf = std::ranges::mismatch(a, fv);
        RTTS_EXPECT(rf.in1 == sf.in1 && rf.in2 == sf.in2);

        auto nc = rtts::make_non_common(a);
        auto rn = raze::algorithm::mismatch[opt::fautovec][opt::fstatic](
            nc, tv, [](auto x, auto y) { if (x == y) return true; return false; });
        auto sn = std::ranges::mismatch(nc, tv);
        RTTS_EXPECT(rn.in1 == sn.in1 && rn.in2 == sn.in2);
    }

    // Mismatched lengths, empty, single-pass early exit, volatile, rvalue dangling, member projections
    {
        const std::vector<int> s = { 1, 2, 3 };
        const std::vector<int> l = { 1, 2, 3, 4, 5 };
        auto rl = raze::algorithm::mismatch(s, l);
        RTTS_EXPECT(rl.in1 == s.end() && rl.in2 == l.begin() + 3);

        std::vector<int> empty_v;
        auto re = raze::algorithm::mismatch(empty_v, l);
        RTTS_EXPECT(re.in1 == empty_v.end() && re.in2 == l.begin());

        const std::vector<int> d1 = { 1, 2, 9, 4, 5 };
        rtts::single_pass_input_range<int> sp1(d1), sp2(l);
        auto rsp = raze::algorithm::mismatch(sp1, sp2);
        RTTS_EXPECT(rsp.in1 != sp1.end() && *rsp.in1 == 9 && sp1.reads() == 2);

        volatile int va[16], vb[16];
        for (int i = 0; i < 16; ++i) { va[i] = i; vb[i] = i; }
        vb[7] = 99;
        auto vr = raze::algorithm::mismatch(va, va + 16, vb, vb + 16);
        RTTS_EXPECT(vr.in1 == va + 7 && vr.in2 == vb + 7);

        auto rval = raze::algorithm::mismatch(std::vector<int>{ 1, 2 }, std::vector<int>{ 1, 3 });
        static_assert(std::same_as<decltype(rval.in1), std::ranges::dangling>);
        static_assert(std::same_as<decltype(rval.in2), std::ranges::dangling>);

        const std::vector<rtts::test_item> i1 = { { 1, 10 }, { 2, 20 }, { 9, 30 } };
        const std::vector<rtts::test_item> i2 = { { 1, 99 }, { 2, 88 }, { 3, 77 } };
        auto ip = raze::algorithm::mismatch(i1, i2, std::equal_to<>{}, &rtts::test_item::key, &rtts::test_item::key);
        RTTS_EXPECT(ip.in1 == i1.begin() + 2 && ip.in2 == i2.begin() + 2);
    }
};