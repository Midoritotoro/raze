#include <tests/rts/rts.h>
#include <raze/algorithm/find/EndsWith.h>

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

    struct ends_with_item {
        int32_t key;
        int32_t value;
        bool operator==(const ends_with_item&) const = default;
    };

    template <class It1, class Sent1, class It2, class Sent2, class Pred = std::equal_to<>>
    constexpr bool ref_ends_with(It1 first1, Sent1 sent1, It2 first2, Sent2 sent2, Pred pred = {}) {
#if defined(__cpp_lib_ranges_starts_with) && __cpp_lib_ranges_starts_with >= 202207L
        return std::ranges::ends_with(std::move(first1), std::move(sent1),
            std::move(first2), std::move(sent2), std::move(pred));
#else
        const auto size1 = std::ranges::distance(first1, sent1);
        const auto size2 = std::ranges::distance(first2, sent2);
        if (size1 < size2)
            return false;
        std::ranges::advance(first1, size1 - size2);
        return std::ranges::equal(std::move(first1), std::move(sent1),
            std::move(first2), std::move(sent2), std::move(pred));
#endif
    }

    template <class R1, class R2, class Pred = std::equal_to<>>
    constexpr bool ref_ends_with(const R1& r1, const R2& r2, Pred pred = {}) {
        return ref_ends_with(std::ranges::begin(r1), std::ranges::end(r1),
            std::ranges::begin(r2), std::ranges::end(r2), std::move(pred));
    }

    template <class T>
    void check_ends_with_variants(const std::vector<T>& haystack, const std::vector<T>& needle) {
        const bool expected = ref_ends_with(haystack, needle);
        auto bin_pred = [](auto a, auto b) { return a == b; };

        RTTS_EXPECT(raze::algorithm::ends_with(haystack, needle) == expected);
        RTTS_EXPECT(raze::algorithm::ends_with(haystack.begin(), haystack.end(), needle.begin(), needle.end()) == expected);
        RTTS_EXPECT(raze::algorithm::ends_with[raze::options::fscalar](haystack, needle) == expected);
        RTTS_EXPECT(raze::algorithm::ends_with[raze::options::unroll<2>](haystack, needle) == expected);
        RTTS_EXPECT(raze::algorithm::ends_with[raze::options::unroll<4>](haystack, needle) == expected);
        RTTS_EXPECT(raze::algorithm::ends_with[raze::options::fstatic](haystack, needle) == expected);

        RTTS_EXPECT(raze::algorithm::ends_with(haystack, needle, bin_pred) == expected);
        RTTS_EXPECT(raze::algorithm::ends_with[raze::options::fscalar](haystack, needle, bin_pred) == expected);
        RTTS_EXPECT(raze::algorithm::ends_with[raze::options::fstatic](haystack, needle, bin_pred) == expected);
    }

    template <class T>
    void ends_with_boundary_sweep() {
        std::vector<size_t> sizes;
        for (size_t n = 0; n <= 140; ++n)
            sizes.push_back(n);
        for (size_t n : { size_t(255), size_t(256), size_t(257), size_t(511), size_t(512), size_t(513), size_t(1024) })
            sizes.push_back(n);

        for (size_t n : sizes) {
            std::vector<T> haystack(n);
            for (size_t i = 0; i < n; ++i)
                haystack[i] = static_cast<T>((i % 100) + 1);

            check_ends_with_variants(haystack, std::vector<T>{});

            std::vector<T> longer(n + 1, T(1));
            for (size_t i = 0; i < n; ++i)
                longer[i + 1] = haystack[i];
            check_ends_with_variants(haystack, longer);

            if (n > 0) {
                check_ends_with_variants(haystack, haystack);

                std::vector<size_t> positions;
                if (n <= 64) {
                    for (size_t p = 0; p < n; ++p)
                        positions.push_back(p);
                }
                else {
                    positions = { 0, 1, 15, 16, 31, 32, 63, 64, n / 2, n - 2, n - 1 };
                }

                for (size_t pos : positions) {
                    auto broken = haystack;
                    broken[pos] = T(0);
                    check_ends_with_variants(haystack, broken);
                }
            }
        }

        for (size_t n : { size_t(65), size_t(128), size_t(300) }) {
            std::vector<T> haystack(n);
            for (size_t i = 0; i < n; ++i)
                haystack[i] = static_cast<T>((i % 100) + 1);

            for (size_t m : { size_t(1), size_t(3), size_t(15), size_t(16), size_t(17),
                              size_t(31), size_t(32), size_t(33), size_t(64), n - 1 })
            {
                std::vector<T> suffix(haystack.end() - m, haystack.end());
                check_ends_with_variants(haystack, suffix);

                auto bad_first = suffix;
                bad_first[0] = T(0);
                check_ends_with_variants(haystack, bad_first);

                auto bad_last = suffix;
                bad_last[m - 1] = T(0);
                check_ends_with_variants(haystack, bad_last);

                std::vector<T> prefix_only(haystack.begin(), haystack.begin() + m);
                check_ends_with_variants(haystack, prefix_only);
            }
        }
    }

    template <class T>
    void ends_with_fuzz(unsigned seed, size_t iters) {
        std::mt19937 rng(seed);
        std::uniform_int_distribution<size_t> sz(0, 512);

        for (size_t i = 0; i < iters; ++i) {
            const size_t n1 = sz(rng);
            const size_t n2 = sz(rng);
            auto h = rtts::random::vector<T>(n1, seed + static_cast<unsigned>(i));
            auto needle = (n2 <= n1 && (rng() % 2 == 0))
                ? std::vector<T>(h.end() - n2, h.end())
                : rtts::random::vector<T>(n2, seed + static_cast<unsigned>(i) + 1000u);

            if (n2 > 0 && n2 <= n1 && (rng() % 4 == 0))
                needle[rng() % n2] = static_cast<T>(needle[0] + T(1));

            const bool exp = ref_ends_with(h, needle);
            RTTS_EXPECT(raze::algorithm::ends_with(h, needle) == exp);
            RTTS_EXPECT(raze::algorithm::ends_with[raze::options::fscalar](h.begin(), h.end(), needle.begin(), needle.end()) == exp);
            RTTS_EXPECT(raze::algorithm::ends_with[raze::options::unroll<4>](h, needle) == exp);
            RTTS_EXPECT(raze::algorithm::ends_with[raze::options::fstatic](h, needle) == exp);
        }
    }

    template <class T>
    void ends_with_suite() {
        using namespace rtts::algorithm;
        namespace opt = raze::options;
        auto cfg = config::thorough();

        constexpr auto suf_of = [](auto& c) {
            return std::ranges::subrange(std::ranges::next(c.first(), static_cast<std::ptrdiff_t>(c.count())), c.last());
        };

        auto check_iter = [&](auto algo) {
            run<n, T>(cfg,
                [algo, suf_of](auto c) { auto s = suf_of(c); return algo(c.first(), c.last(), s.begin(), s.end()); },
                [suf_of](auto c) { auto s = suf_of(c); return ref_ends_with(c.first(), c.last(), s.begin(), s.end()); });
        };

        auto check_range = [&](auto algo) {
            run<n, T>(cfg,
                [algo, suf_of](auto c) { return algo(c.range(), suf_of(c)); },
                [suf_of](auto c) { return ref_ends_with(c.range(), suf_of(c)); });
        };

        check_iter(raze::algorithm::ends_with);
        check_range(raze::algorithm::ends_with);
        check_iter(raze::algorithm::ends_with[opt::fscalar]);
        check_range(raze::algorithm::ends_with[opt::unroll<2>]);
        check_range(raze::algorithm::ends_with[opt::fstatic]);
        check_iter(raze::algorithm::ends_with[opt::fautovec]);

        auto eq_branching = [](auto a, auto b) { if (a == b) return true; return false; };

        run<n, T>(cfg,
            [suf_of, eq_branching](auto c) { return raze::algorithm::ends_with[opt::fautovec][opt::fstatic](c.range(), suf_of(c), eq_branching); },
            [suf_of, eq_branching](auto c) { return ref_ends_with(c.range(), suf_of(c), eq_branching); });

        {
            auto acfg = config::compact();

            auto check_aligned = [&]<size_t A1, size_t A2>(auto algo) {
                run_aligned<A1, A2, range, T>(acfg,
                    [algo](auto c) { return algo(c.first(), c.last(), c.first2(), c.last2()); },
                    [](auto c) { return ref_ends_with(c.first(), c.last(), c.first2(), c.last2()); });

                run_aligned<A1, A2, range, T>(acfg,
                    [algo](auto c) { return algo(c.range(), c.range2()); },
                    [](auto c) { return ref_ends_with(c.range(), c.range2()); });

                for (size_t sz : { size_t(0), size_t(1), size_t(15), size_t(16), size_t(31), size_t(32), size_t(64), size_t(128) }) {
                    rtts::aligned_buffer<T, A1> hay(sz);
                    for (size_t i = 0; i < sz; ++i) hay[i] = static_cast<T>((i % 100) + 1);
                    const size_t suf_len = sz / 2;
                    rtts::aligned_buffer<T, A2> suf(suf_len);
                    for (size_t i = 0; i < suf_len; ++i) suf[i] = hay[sz - suf_len + i];
                    RTTS_EXPECT(algo(hay, suf) == ref_ends_with(hay, suf));
                    if (suf_len > 0) {
                        suf[0] = T(0);
                        RTTS_EXPECT(algo(hay, suf) == ref_ends_with(hay, suf));
                    }
                }
            };

            check_aligned.template operator()<16, 16>(raze::algorithm::ends_with[opt::aligned<16>]);
            check_aligned.template operator()<32, 32>(raze::algorithm::ends_with[opt::aligned<32>]);
            check_aligned.template operator()<32, 32>(raze::algorithm::ends_with[opt::aligned<32>][opt::unroll<2>]);
            check_aligned.template operator()<64, 64>(raze::algorithm::ends_with[opt::aligned<64>]);
            check_aligned.template operator()<64, 64>(raze::algorithm::ends_with[opt::aligned<64>][opt::fautovec]);
            check_aligned.template operator()<64, 64>(raze::algorithm::ends_with[opt::aligned<64>][opt::fscalar]);
            check_aligned.template operator()<1, 1>(raze::algorithm::ends_with[opt::aligned<opt::unaligned>]);
            check_aligned.template operator()<1, 1>(raze::algorithm::ends_with[opt::aligned<64>][opt::aligned<opt::unaligned>]);
            check_aligned.template operator()<64, 1>(raze::algorithm::ends_with[opt::aligned<64, opt::unaligned>]);
            check_aligned.template operator()<1, 64>(raze::algorithm::ends_with[opt::aligned<opt::unaligned, 64>]);
            check_aligned.template operator()<32, 64>(raze::algorithm::ends_with[opt::aligned<32, 64>][opt::fstatic]);
            check_aligned.template operator()<64, 16>(raze::algorithm::ends_with[opt::aligned<64, 16>]);
        }

        static_assert(constexpr_run<T, range, 0>(
            [](auto c) { return raze::algorithm::ends_with(c.range(), c.range()); },
            [](auto c) { return ref_ends_with(c.range(), c.range()); }));

        static_assert(constexpr_run<T, n, 5, 3>(
            [suf_of](auto c) { return raze::algorithm::ends_with(c.range(), suf_of(c)); },
            [suf_of](auto c) { return ref_ends_with(c.range(), suf_of(c)); }));

        static_assert(constexpr_run<T, range, 5>(
            [](auto c) { return raze::algorithm::ends_with[opt::fautovec](c.range(), c.range()); },
            [](auto c) { return ref_ends_with(c.range(), c.range()); }));

        static_assert(constexpr_run<T, range, 5>(
            [](auto c) { return raze::algorithm::ends_with[opt::aligned<64, opt::unaligned>](c.range(), c.range2()); },
            [](auto c) { return ref_ends_with(c.range(), c.range2()); }));

        static_assert([]() constexpr {
            std::array<T, 0> empty{};
            std::array<T, 2> tiny{ T(1), T(2) };
            std::array<T, 5> a{ T(1), T(2), T(3), T(4), T(5) };
            std::array<T, 3> s{ T(3), T(4), T(5) };
            std::array<T, 3> bad{ T(3), T(9), T(5) };
            std::array<T, 6> too_long{ T(0), T(1), T(2), T(3), T(4), T(5) };
            return raze::algorithm::ends_with(empty, empty) &&
                   raze::algorithm::ends_with(tiny, empty) &&
                  !raze::algorithm::ends_with(empty, tiny) &&
                   raze::algorithm::ends_with(a, s) &&
                  !raze::algorithm::ends_with(a, bad) &&
                  !raze::algorithm::ends_with(a, too_long) &&
                   raze::algorithm::ends_with[opt::fscalar](a.begin(), a.end(), s.begin(), s.end());
        }());
    }
}

RTTS_CASE_TPL("raze::algorithm::ends_with", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    ends_with_suite<T>();
};

RTTS_CASE_TPL("raze::algorithm::ends_with boundaries", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    ends_with_boundary_sweep<T>();
};

RTTS_CASE_TPL("raze::algorithm::ends_with fuzz", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    for (unsigned seed : fuzz_seeds)
        ends_with_fuzz<T>(seed, 200);
};

RTTS_CASE("raze::algorithm::ends_with structs, projections, rvalues and predicates") {
    const std::vector<ends_with_item> input{
        { 1, 10 }, { 2, 20 }, { 3, 30 }, { 4, 40 }
    };
    const std::vector<int32_t> keys{ 2, 3, 4 };
    const std::vector<int32_t> bad_keys{ 2, 9, 4 };

    RTTS_EXPECT(raze::algorithm::ends_with(
        input, keys, std::equal_to<>{},
        [](const auto& item) { return item.key; }));

    RTTS_EXPECT(!raze::algorithm::ends_with(
        input, bad_keys, std::equal_to<>{},
        [](const auto& item) { return item.key; }));

    const std::vector<ends_with_item> other{ { 8, 300 }, { 9, 400 } };
    RTTS_EXPECT(raze::algorithm::ends_with[raze::options::fscalar](
        input, other,
        [](int32_t a, int32_t b) { return a * 10 == b; },
        [](const auto& x) { return x.value; },
        [](const auto& y) { return y.value; }));

    const std::vector<int32_t> nums{ 1, 2, 3, 4, 5, 6 };
    RTTS_EXPECT(raze::algorithm::ends_with(nums, std::vector<int32_t>{ 4, 5, 6 }));
    RTTS_EXPECT(!raze::algorithm::ends_with(nums, std::vector<int32_t>{ 1, 2, 3 }));
    RTTS_EXPECT(raze::algorithm::ends_with(std::vector<int32_t>{ 1, 2, 3, 4 }, std::vector<int32_t>{ 3, 4 }));

    const std::string s = "hello world";
    RTTS_EXPECT(raze::algorithm::ends_with(s, std::string_view("world")));
    RTTS_EXPECT(!raze::algorithm::ends_with(s, std::string_view("hello")));
};

RTTS_CASE("raze::algorithm::ends_with views and edge cases") {
    namespace opt = raze::options;
    namespace views = std::views;

    {
        auto iv = views::iota(1, 50);
        auto suf = views::iota(40, 50);
        RTTS_EXPECT(raze::algorithm::ends_with(iv, suf));
        RTTS_EXPECT(!raze::algorithm::ends_with(suf, iv));

        const std::vector<int> src = { 2, 4, 6, 8, 10, 12, 14, 16, 18, 20 };
        auto fv = views::iota(1, 21) | views::filter([](int x) { return (x & 1) == 0; });
        auto tv = views::iota(7, 11) | views::transform([](int x) { return x * 2; });
        RTTS_EXPECT((raze::algorithm::ends_with[opt::aligned<64, opt::unaligned>](fv, tv)));
        RTTS_EXPECT(raze::algorithm::ends_with[opt::fautovec][opt::fstatic](
            src, tv, [](auto x, auto y) { if (x == y) return true; return false; }));
        RTTS_EXPECT(raze::algorithm::ends_with(rtts::make_non_common(src), tv));
    }

    {
        const std::vector<int> h = { 1, 2, 3, 4, 5 };
        std::vector<int> empty_v;
        RTTS_EXPECT(raze::algorithm::ends_with(h, empty_v));
        RTTS_EXPECT(raze::algorithm::ends_with(empty_v, empty_v));
        RTTS_EXPECT(!raze::algorithm::ends_with(empty_v, h));

        rtts::single_pass_input_range<int> sp1(h);
        const std::vector<int> s = { 3, 4, 5 };
        rtts::single_pass_input_range<int> sp2(s);
        RTTS_EXPECT(raze::algorithm::ends_with(views::counted(sp1.begin(), 5), views::counted(sp2.begin(), 3)));

        volatile int vh[16], vs[4];
        for (int i = 0; i < 16; ++i) vh[i] = i;
        for (int i = 0; i < 4; ++i) vs[i] = 12 + i;
        RTTS_EXPECT(raze::algorithm::ends_with(vh, vh + 16, vs, vs + 4));
        vs[0] = 99;
        RTTS_EXPECT(!raze::algorithm::ends_with(vh, vh + 16, vs, vs + 4));

        const std::vector<rtts::test_item> i1 = { { 1, 10 }, { 2, 20 }, { 3, 30 } };
        const std::vector<rtts::test_item> i2 = { { 2, 99 }, { 3, 88 } };
        RTTS_EXPECT(raze::algorithm::ends_with(i1, i2, std::equal_to<>{}, &rtts::test_item::key, &rtts::test_item::key));
    }
};
