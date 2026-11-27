#include <tests/rts/rts.h>
#include <raze/algorithm/order/Rotate.h>

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

    struct rotate_item {
        int32_t key;
        int32_t value;
        bool operator==(const rotate_item&) const = default;
    };

    struct rotate_odd_item {
        uint8_t bytes[7];
        bool operator==(const rotate_odd_item&) const = default;
    };

    struct rotate_large_item {
        int32_t data[160];
        bool operator==(const rotate_large_item&) const = default;
    };

    template <class T>
    void check_rotate_variants(const std::vector<T>& input, size_t mid) {
        namespace opt = raze::options;
        const size_t n = input.size();
        auto exp = input;
        auto exp_res = std::ranges::rotate(exp, exp.begin() + mid);
        const auto exp_pos = exp_res.begin() - exp.begin();

        std::vector<T> v;
        auto check_range = [&](auto algo) {
            v = input;
            auto res = algo(v, v.begin() + mid);
            RTTS_EXPECT(res.begin() - v.begin() == exp_pos);
            RTTS_EXPECT(res.end() == v.end());
            RTTS_EXPECT(v == exp);
        };

        check_range(raze::algorithm::rotate);

        v = input;
        auto res = raze::algorithm::rotate(v.begin(), v.begin() + mid, v.end());
        RTTS_EXPECT(res.begin() - v.begin() == exp_pos);
        RTTS_EXPECT(res.end() == v.end());
        RTTS_EXPECT(v == exp);

        check_range(raze::algorithm::rotate[opt::fscalar]);
        check_range(raze::algorithm::rotate[opt::unroll<2>]);
        check_range(raze::algorithm::rotate[opt::fstatic]);

        res = raze::algorithm::rotate(v, v.begin() + (n - mid));
        RTTS_EXPECT(res.begin() == v.begin() + mid);
        RTTS_EXPECT(res.end() == v.end());
        RTTS_EXPECT(v == input);
    }

    template <class T>
    void rotate_boundary_sweep() {
        std::vector<size_t> sizes;
        for (size_t n = 0; n <= 140; ++n)
            sizes.push_back(n);

        for (size_t n : {
            size_t(256), size_t(384), size_t(511), size_t(512), size_t(513),
            size_t(600), size_t(768), size_t(1023), size_t(1024), size_t(1025),
            size_t(1535), size_t(1536), size_t(1600), size_t(2048) })
        {
            sizes.push_back(n);
        }

        for (size_t n : sizes) {
            std::vector<T> input(n);
            for (size_t i = 0; i < n; ++i)
                input[i] = static_cast<T>(i);

            std::vector<size_t> mids;
            if (n <= 32) {
                for (size_t m = 0; m <= n; ++m)
                    mids.push_back(m);
            }
            else {
                for (size_t m : {
                    size_t(0), size_t(1), size_t(2),
                    size_t(127), size_t(128), size_t(129),
                    size_t(511), size_t(512), size_t(513),
                    n / 4, n / 3, n / 2, (2 * n) / 3, (3 * n) / 4,
                    n - 2, n - 1, n })
                {
                    if (m <= n)
                        mids.push_back(m);
                }
                if (n > 513) {
                    mids.push_back(n - 513);
                    mids.push_back(n - 512);
                    mids.push_back(n - 511);
                }
                std::ranges::sort(mids);
                mids.erase(std::ranges::unique(mids).begin(), mids.end());
            }

            for (size_t mid : mids)
                check_rotate_variants(input, mid);
        }
    }

    template <class T>
    void rotate_fuzz(unsigned seed, size_t iters) {
        namespace opt = raze::options;
        std::mt19937 rng(seed);
        std::uniform_int_distribution<size_t> sz(0, 2500);

        for (size_t i = 0; i < iters; ++i) {
            const size_t n = sz(rng);
            auto input = rtts::random::vector<T>(n, seed + static_cast<unsigned>(i));

            const size_t mid = n == 0 ? 0 : rng() % (n + 1);
            auto exp = input;
            auto exp_res = std::ranges::rotate(exp, exp.begin() + mid);
            const auto exp_pos = exp_res.begin() - exp.begin();

            auto check_full = [&](auto fn) {
                auto v = input;
                auto res = fn(v);
                RTTS_EXPECT(res.begin() - v.begin() == exp_pos);
                RTTS_EXPECT(res.end() == v.end());
                RTTS_EXPECT(v == exp);
            };

            check_full([&](auto& v) { return raze::algorithm::rotate(v, v.begin() + mid); });
            check_full([&](auto& v) { return raze::algorithm::rotate[opt::fscalar](v.begin(), v.begin() + mid, v.end()); });
            check_full([&](auto& v) { return raze::algorithm::rotate[opt::fstatic](v, v.begin() + mid); });

            const size_t first = n == 0 ? 0 : rng() % (n + 1);
            const size_t last = first + (n == first ? 0 : rng() % (n - first + 1));
            const size_t sub_mid = first + (last == first ? 0 : rng() % (last - first + 1));

            auto v = input;
            exp = input;
            exp_res = std::ranges::rotate(exp.begin() + first, exp.begin() + sub_mid, exp.begin() + last);
            auto res = raze::algorithm::rotate(v.begin() + first, v.begin() + sub_mid, v.begin() + last);
            RTTS_EXPECT(res.begin() - v.begin() == exp_res.begin() - exp.begin());
            RTTS_EXPECT(res.end() == v.begin() + last);
            RTTS_EXPECT(v == exp);
        }
    }

    template <class Seq>
    void check_rotate_seq(const Seq& input, std::initializer_list<size_t> mids) {
        for (size_t mid : mids) {
            auto exp = input;
            auto exp_res = std::ranges::rotate(exp, std::ranges::next(exp.begin(), static_cast<std::ptrdiff_t>(mid)));
            const auto exp_pos = std::ranges::distance(exp.begin(), exp_res.begin());

            auto check = [&](auto fn) {
                auto v = input;
                auto res = fn(v, std::ranges::next(v.begin(), static_cast<std::ptrdiff_t>(mid)));
                RTTS_EXPECT(std::ranges::distance(v.begin(), res.begin()) == exp_pos);
                RTTS_EXPECT(res.end() == v.end());
                RTTS_EXPECT(v == exp);
            };

            check([](auto& v, auto m) { return raze::algorithm::rotate(v, m); });
            check([](auto& v, auto m) { return raze::algorithm::rotate[raze::options::fscalar](v.begin(), m, v.end()); });
        }
    }
}

RTTS_CASE_TPL("raze::algorithm::rotate", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    using namespace rtts::algorithm;
    namespace opt = raze::options;
    auto cfg = config::thorough();

    auto std_rotate_n = [](auto c) {
        auto mid = std::ranges::next(c.data.value.begin(), static_cast<std::ptrdiff_t>(c.count()));
        return std::ranges::rotate(c.data.value.begin(), mid, c.data.value.end());
    };

    auto check_iter = [&](auto algo) {
        run<n, T>(cfg,
            [algo](auto c) {
                auto mid = std::ranges::next(c.first(), static_cast<std::ptrdiff_t>(c.count()));
                return algo(c.first(), mid, c.last());
            },
            std_rotate_n);
    };

    auto check_range = [&](auto algo) {
        run<n, T>(cfg,
            [algo](auto c) {
                auto mid = std::ranges::next(c.first(), static_cast<std::ptrdiff_t>(c.count()));
                return algo(c.range(), mid);
            },
            std_rotate_n);
    };

    check_iter(raze::algorithm::rotate);
    check_range(raze::algorithm::rotate);
    check_iter(raze::algorithm::rotate[opt::fscalar]);
    check_range(raze::algorithm::rotate[opt::unroll<2>]);
    check_range(raze::algorithm::rotate[opt::fstatic]);
    check_iter(raze::algorithm::rotate[opt::fautovec]);
    check_range(raze::algorithm::rotate[opt::fautovec][opt::fstatic]);

    {
        auto acfg = config::compact();

        auto check_aligned = [&]<size_t Align>(auto algo) {
            run_aligned<Align, n, T>(acfg,
                [algo](auto c) {
                    auto mid = std::ranges::next(c.first(), static_cast<std::ptrdiff_t>(c.count()));
                    return algo(c.first(), mid, c.last());
                },
                std_rotate_n);

            run_aligned<Align, n, T>(acfg,
                [algo](auto c) {
                    auto mid = std::ranges::next(c.first(), static_cast<std::ptrdiff_t>(c.count()));
                    return algo(c.range(), mid);
                },
                std_rotate_n);
        };

        check_aligned.template operator()<16>(raze::algorithm::rotate[opt::aligned<16>]);
        check_aligned.template operator()<32>(raze::algorithm::rotate[opt::aligned<32>]);
        check_aligned.template operator()<32>(raze::algorithm::rotate[opt::aligned<32>][opt::unroll<2>]);
        check_aligned.template operator()<32>(raze::algorithm::rotate[opt::aligned<32>][opt::fstatic]);
        check_aligned.template operator()<64>(raze::algorithm::rotate[opt::aligned<64>]);
        check_aligned.template operator()<64>(raze::algorithm::rotate[opt::aligned<64>][opt::fautovec]);
        check_aligned.template operator()<64>(raze::algorithm::rotate[opt::aligned<64>][opt::fscalar]);
        check_aligned.template operator()<1>(raze::algorithm::rotate[opt::aligned<opt::unaligned>]);
        check_aligned.template operator()<1>(raze::algorithm::rotate[opt::aligned<64>][opt::aligned<opt::unaligned>]);
    }

    run<range, T>(cfg,
        [](auto c) {
            auto mid = std::ranges::next(c.first(), c.last());
            return raze::algorithm::rotate(c.first(), mid, c.last());
        },
        [](auto c) {
            return std::ranges::rotate(c.data.value.begin(), c.data.value.end(), c.data.value.end());
        });

    constexpr auto check_ce_range = []<size_t N, size_t M>(auto algo) constexpr {
        return constexpr_run<T, n, N, M>(
            [algo](auto c) constexpr {
                auto mid = std::ranges::next(c.first(), static_cast<std::ptrdiff_t>(c.count()));
                return algo(c.range(), mid);
            },
            [](auto c) constexpr {
                auto mid = std::ranges::next(c.data.value.begin(), static_cast<std::ptrdiff_t>(c.count()));
                return std::ranges::rotate(c.data.value.begin(), mid, c.data.value.end());
            });
    };

    static_assert(constexpr_run<T, range, 0>(
        [](auto c) constexpr { return raze::algorithm::rotate(c.first(), c.first(), c.last()); },
        [](auto c) constexpr { return std::ranges::rotate(c.data.value.begin(), c.data.value.begin(), c.data.value.end()); }));

    static_assert(constexpr_run<T, n, 5, 0>(
        [](auto c) constexpr {
            auto mid = std::ranges::next(c.first(), static_cast<std::ptrdiff_t>(c.count()));
            return raze::algorithm::rotate(c.first(), mid, c.last());
        },
        [](auto c) constexpr {
            auto mid = std::ranges::next(c.data.value.begin(), static_cast<std::ptrdiff_t>(c.count()));
            return std::ranges::rotate(c.data.value.begin(), mid, c.data.value.end());
        }));

    static_assert(check_ce_range.template operator()<5, 2>(raze::algorithm::rotate));
    static_assert(check_ce_range.template operator()<5, 2>(raze::algorithm::rotate[opt::fautovec]));
    static_assert(check_ce_range.template operator()<5, 2>(raze::algorithm::rotate[opt::aligned<64>]));

    static_assert([]() constexpr {
        std::array<T, 0> a0{};
        auto r0 = raze::algorithm::rotate(a0, a0.begin());

        std::array<T, 1> a1{ T(1) };
        auto r1 = raze::algorithm::rotate(a1, a1.begin());

        std::array<T, 4> a4{ T(1), T(2), T(3), T(4) };
        auto r4 = raze::algorithm::rotate(a4, a4.begin() + 1);

        std::array<T, 5> a5{ T(1), T(2), T(3), T(4), T(5) };
        auto r5 = raze::algorithm::rotate[opt::fscalar](a5, a5.begin() + 3);

        std::array<T, 4> a4e{ T(1), T(2), T(3), T(4) };
        auto r4e = raze::algorithm::rotate(a4e, a4e.end());

        std::array<T, 5> a5s{ T(1), T(2), T(3), T(4), T(5) };
        auto r5s = raze::algorithm::rotate(a5s.begin() + 1, a5s.begin() + 3, a5s.begin() + 4);

        return r0.begin() == a0.end() && r0.end() == a0.end()
            && r1.begin() == a1.end() && r1.end() == a1.end() && a1 == std::array<T, 1>{ T(1) }
            && r4.begin() == a4.begin() + 3 && r4.end() == a4.end() && a4 == std::array<T, 4>{ T(2), T(3), T(4), T(1) }
            && r5.begin() == a5.begin() + 2 && r5.end() == a5.end() && a5 == std::array<T, 5>{ T(4), T(5), T(1), T(2), T(3) }
            && r4e.begin() == a4e.begin() && r4e.end() == a4e.end() && a4e == std::array<T, 4>{ T(1), T(2), T(3), T(4) }
            && r5s.begin() == a5s.begin() + 2 && r5s.end() == a5s.begin() + 4 && a5s == std::array<T, 5>{ T(1), T(4), T(2), T(3), T(5) };
    }());
};

RTTS_CASE_TPL("raze::algorithm::rotate boundaries", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    rotate_boundary_sweep<T>();
};

RTTS_CASE_TPL("raze::algorithm::rotate fuzz", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    for (unsigned seed : fuzz_seeds)
        rotate_fuzz<T>(seed, 200);
};

RTTS_CASE("raze::algorithm::rotate structs") {
    {
        std::vector<rotate_item> input{
            { 1, 10 }, { 2, 20 }, { 3, 30 }, { 4, 40 }, { 5, 50 }
        };

        auto exp = input;
        std::ranges::rotate(exp, exp.begin() + 2);

        auto v = input;
        auto res = raze::algorithm::rotate(v, v.begin() + 2);
        RTTS_EXPECT(res.begin() == v.begin() + 3);
        RTTS_EXPECT(res.end() == v.end());
        RTTS_EXPECT(v == exp);

        v = input;
        res = raze::algorithm::rotate[raze::options::fscalar](v, v.begin() + 2);
        RTTS_EXPECT(res.begin() == v.begin() + 3);
        RTTS_EXPECT(res.end() == v.end());
        RTTS_EXPECT(v == exp);
    }

    {
        std::vector<rotate_odd_item> input(250);
        for (size_t i = 0; i < input.size(); ++i)
            for (size_t j = 0; j < 7; ++j)
                input[i].bytes[j] = static_cast<uint8_t>((i * 7 + j) & 0xFF);

        for (size_t mid : { size_t(50), size_t(75), size_t(100), size_t(175) }) {
            auto exp = input;
            std::ranges::rotate(exp, exp.begin() + mid);

            auto v = input;
            auto res = raze::algorithm::rotate(v, v.begin() + mid);
            RTTS_EXPECT(res.begin() == v.begin() + (input.size() - mid));
            RTTS_EXPECT(res.end() == v.end());
            RTTS_EXPECT(v == exp);
        }
    }

    {
        std::vector<rotate_large_item> input(7);
        for (size_t i = 0; i < input.size(); ++i)
            input[i].data[0] = static_cast<int32_t>(i + 1);

        for (size_t mid : { size_t(2), size_t(3), size_t(5) }) {
            auto exp = input;
            std::ranges::rotate(exp, exp.begin() + mid);

            auto v = input;
            auto res = raze::algorithm::rotate(v, v.begin() + mid);
            RTTS_EXPECT(res.begin() == v.begin() + (input.size() - mid));
            RTTS_EXPECT(res.end() == v.end());
            RTTS_EXPECT(v == exp);
        }
    }

    {
        std::vector<std::string> input{
            "alpha", "beta", "gamma", "delta", "epsilon"
        };

        auto exp = input;
        std::ranges::rotate(exp, exp.begin() + 2);

        auto v = input;
        auto res = raze::algorithm::rotate(v, v.begin() + 2);
        RTTS_EXPECT(res.begin() == v.begin() + 3);
        RTTS_EXPECT(res.end() == v.end());
        RTTS_EXPECT(v == exp);
    }
};

RTTS_CASE("raze::algorithm::rotate bidirectional") {
    check_rotate_seq(std::list<int>{ 1, 2, 3, 4, 5 }, { 0, 2, 3, 5 });
};

RTTS_CASE("raze::algorithm::rotate forward") {
    check_rotate_seq(std::forward_list<int>{ 1, 2, 3, 4, 5 }, { 0, 1, 2, 4, 5 });
};

RTTS_CASE("raze::algorithm::rotate sentinel") {
    {
        std::array<int, 5> input{ 1, 2, 3, 4, 5 };
        auto first = std::counted_iterator(input.begin(), 5);
        auto mid = std::counted_iterator(input.begin() + 2, 3);
        auto res = raze::algorithm::rotate(first, mid, std::default_sentinel);

        RTTS_EXPECT(res.begin().base() == input.begin() + 3);
        RTTS_EXPECT(res.end() == std::default_sentinel);
        RTTS_EXPECT((input == std::array<int, 5>{ 3, 4, 5, 1, 2 }));
    }

    {
        std::vector<uint8_t> v(1600);
        for (size_t i = 0; i < v.size(); ++i)
            v[i] = static_cast<uint8_t>(i & 0xFF);

        auto exp = v;
        std::ranges::rotate(exp, exp.begin() + 525);

        auto first = std::counted_iterator(v.begin(), static_cast<std::ptrdiff_t>(v.size()));
        auto mid = std::counted_iterator(v.begin() + 525, static_cast<std::ptrdiff_t>(v.size() - 525));
        auto res = raze::algorithm::rotate(first, mid, std::default_sentinel);

        RTTS_EXPECT(res.begin().base() == v.begin() + (v.size() - 525));
        RTTS_EXPECT(res.end() == std::default_sentinel);
        RTTS_EXPECT(v == exp);
    }
};

RTTS_CASE("raze::algorithm::rotate views and edge cases") {
    namespace opt = raze::options;
    namespace views = std::views;

    {
        std::vector<int> v1(35);
        std::iota(v1.begin(), v1.end(), 1);
        auto v2 = v1;

        auto dv1 = v1 | views::drop(3) | views::take(20);
        auto dv2 = v2 | views::drop(3) | views::take(20);
        raze::algorithm::rotate(dv1, dv1.begin() + 7);
        std::ranges::rotate(dv2, dv2.begin() + 7);
        RTTS_EXPECT(v1 == v2);

        auto rv1 = v1 | views::reverse;
        auto rv2 = v2 | views::reverse;
        raze::algorithm::rotate[opt::fstatic](rv1, rv1.begin() + 11);
        std::ranges::rotate(rv2, rv2.begin() + 11);
        RTTS_EXPECT(v1 == v2);

        auto is_even = [](int x) { return (x % 2) == 0; };
        auto fv1 = v1 | views::filter(is_even);
        auto fv2 = v2 | views::filter(is_even);
        raze::algorithm::rotate[opt::aligned<64>](fv1, std::ranges::next(fv1.begin(), 4));
        std::ranges::rotate(fv2, std::ranges::next(fv2.begin(), 4));
        RTTS_EXPECT(v1 == v2);

        auto nc1 = rtts::make_non_common(v1);
        auto nc2 = rtts::make_non_common(v2);
        raze::algorithm::rotate(nc1, nc1.begin() + 5);
        std::ranges::rotate(nc2, nc2.begin() + 5);
        RTTS_EXPECT(v1 == v2);
    }

    {
        std::vector<int> empty_v;
        auto er = raze::algorithm::rotate(empty_v, empty_v.begin());
        RTTS_EXPECT(er.begin() == empty_v.end() && er.end() == empty_v.end());

        std::vector<int> one_v = { 9 };
        auto or1 = raze::algorithm::rotate(one_v, one_v.begin());
        RTTS_EXPECT(or1.begin() == one_v.end() && one_v[0] == 9);

        std::vector<int> v = { 1, 2, 3, 4, 5 };
        auto mf = raze::algorithm::rotate(v, v.begin());
        RTTS_EXPECT(mf.begin() == v.end() && (v == std::vector<int>{ 1, 2, 3, 4, 5 }));
        auto ml = raze::algorithm::rotate(v, v.end());
        RTTS_EXPECT(ml.begin() == v.begin() && (v == std::vector<int>{ 1, 2, 3, 4, 5 }));

        volatile int vbuf[16];
        for (int i = 0; i < 16; ++i) vbuf[i] = i;
        raze::algorithm::rotate(vbuf, vbuf + 5, vbuf + 16);
        for (int i = 0; i < 16; ++i) RTTS_EXPECT(vbuf[i] == (i + 5) % 16);

        std::vector<int> tmp = { 1, 2, 3, 4 };
        auto mid = tmp.begin() + 2;
        auto rval = raze::algorithm::rotate(std::move(tmp), mid);
        static_assert(std::same_as<decltype(rval), std::ranges::dangling>);
    }
};
