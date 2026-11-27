#include <tests/rts/rts.h>
#include <raze/algorithm/order/RotateCopy.h>

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

    struct rotate_copy_item {
        int32_t key;
        int32_t value;
        bool operator==(const rotate_copy_item&) const = default;
    };

    struct rotate_copy_odd_item {
        uint8_t bytes[7];
        bool operator==(const rotate_copy_odd_item&) const = default;
    };

    struct rotate_copy_large_item {
        int32_t data[160];
        bool operator==(const rotate_copy_large_item&) const = default;
    };

    template <class T>
    void check_rotate_copy_variants(const std::vector<T>& input, size_t mid) {
        namespace opt = raze::options;
        const size_t n = input.size();

        std::vector<T> exp(n);
        auto exp_res = std::ranges::rotate_copy(input, input.begin() + mid, exp.begin());
        RTTS_EXPECT(exp_res.in == input.end());
        RTTS_EXPECT(exp_res.out == exp.end());

        std::vector<T> out(n);
        auto check_range = [&](auto algo) {
            out.assign(n, T{});
            auto res = algo(input, input.begin() + mid, out.begin());
            RTTS_EXPECT(res.in == input.end());
            RTTS_EXPECT(res.out == out.end());
            RTTS_EXPECT(out == exp);
        };

        check_range(raze::algorithm::rotate_copy);

        out.assign(n, T{});
        auto res = raze::algorithm::rotate_copy(input.begin(), input.begin() + mid, input.end(), out.begin());
        RTTS_EXPECT(res.in == input.end());
        RTTS_EXPECT(res.out == out.end());
        RTTS_EXPECT(out == exp);

        check_range(raze::algorithm::rotate_copy[opt::fscalar]);
        check_range(raze::algorithm::rotate_copy[opt::unroll<2>]);
        check_range(raze::algorithm::rotate_copy[opt::fstatic]);
    }

    template <class T>
    void rotate_copy_boundary_sweep() {
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
                check_rotate_copy_variants(input, mid);
        }
    }

    template <class T>
    void rotate_copy_fuzz(unsigned seed, size_t iters) {
        namespace opt = raze::options;
        std::mt19937 rng(seed);
        std::uniform_int_distribution<size_t> sz(0, 2500);

        for (size_t i = 0; i < iters; ++i) {
            const size_t n = sz(rng);
            auto input = rtts::random::vector<T>(n, seed + static_cast<unsigned>(i));
            const auto original = input;

            const size_t mid = n == 0 ? 0 : rng() % (n + 1);
            std::vector<T> exp(n + 2, T{});
            std::vector<T> out(n + 2, T{});

            auto exp_res = std::ranges::rotate_copy(input, input.begin() + mid, exp.begin() + 1);
            const auto exp_out_pos = exp_res.out - exp.begin();

            auto check_full = [&](auto fn) {
                out.assign(n + 2, T{});
                auto res = fn(out.begin() + 1);
                RTTS_EXPECT(res.in == exp_res.in);
                RTTS_EXPECT(res.out - out.begin() == exp_out_pos);
                RTTS_EXPECT(out == exp);
                RTTS_EXPECT(input == original);
            };

            check_full([&](auto dst) { return raze::algorithm::rotate_copy(input, input.begin() + mid, dst); });
            check_full([&](auto dst) { return raze::algorithm::rotate_copy[opt::fscalar](input.begin(), input.begin() + mid, input.end(), dst); });
            check_full([&](auto dst) { return raze::algorithm::rotate_copy[opt::fstatic](input, input.begin() + mid, dst); });

            const size_t first = n == 0 ? 0 : rng() % (n + 1);
            const size_t last = first + (n == first ? 0 : rng() % (n - first + 1));
            const size_t sub_mid = first + (last == first ? 0 : rng() % (last - first + 1));
            const size_t count = last - first;

            exp.assign(count + 2, T{});
            out.assign(count + 2, T{});

            exp_res = std::ranges::rotate_copy(
                input.begin() + first, input.begin() + sub_mid, input.begin() + last, exp.begin() + 1);
            auto res = raze::algorithm::rotate_copy(
                input.begin() + first, input.begin() + sub_mid, input.begin() + last, out.begin() + 1);

            RTTS_EXPECT(res.in == exp_res.in);
            RTTS_EXPECT(res.out - out.begin() == exp_res.out - exp.begin());
            RTTS_EXPECT(out == exp);
            RTTS_EXPECT(input == original);
        }
    }
}

RTTS_CASE_TPL("raze::algorithm::rotate_copy", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    using namespace rtts::algorithm;
    namespace opt = raze::options;
    auto cfg = config::thorough();

    auto std_rotate_copy_n = [](auto c) {
        auto mid = std::ranges::next(c.data.value.begin(), static_cast<std::ptrdiff_t>(c.count()));
        return std::ranges::rotate_copy(c.data.value.begin(), mid, c.data.value.end(), c.out());
    };

    auto check_iter = [&](auto algo) {
        run<n, T>(cfg,
            [algo](auto c) {
                auto mid = std::ranges::next(c.first(), static_cast<std::ptrdiff_t>(c.count()));
                return algo(c.first(), mid, c.last(), c.out());
            },
            std_rotate_copy_n);
    };

    auto check_range = [&](auto algo) {
        run<n, T>(cfg,
            [algo](auto c) {
                auto mid = std::ranges::next(c.first(), static_cast<std::ptrdiff_t>(c.count()));
                return algo(c.range(), mid, c.out());
            },
            std_rotate_copy_n);
    };

    check_iter(raze::algorithm::rotate_copy);
    check_range(raze::algorithm::rotate_copy);
    check_iter(raze::algorithm::rotate_copy[opt::fscalar]);
    check_range(raze::algorithm::rotate_copy[opt::unroll<2>]);
    check_range(raze::algorithm::rotate_copy[opt::fstatic]);
    check_iter(raze::algorithm::rotate_copy[opt::fautovec]);
    check_range(raze::algorithm::rotate_copy[opt::fautovec][opt::fstatic]);

    {
        auto acfg = config::compact();

        auto check_aligned = [&]<size_t A1, size_t AOut>(auto algo) {
            run_aligned<A1, AOut, n, T>(acfg,
                [algo](auto c) {
                    auto mid = std::ranges::next(c.first(), static_cast<std::ptrdiff_t>(c.count()));
                    return algo(c.first(), mid, c.last(), c.out());
                },
                std_rotate_copy_n);

            run_aligned<A1, AOut, n, T>(acfg,
                [algo](auto c) {
                    auto mid = std::ranges::next(c.first(), static_cast<std::ptrdiff_t>(c.count()));
                    return algo(c.range(), mid, c.out());
                },
                std_rotate_copy_n);
        };

        check_aligned.template operator()<16, 16>(raze::algorithm::rotate_copy[opt::aligned<16>]);
        check_aligned.template operator()<32, 32>(raze::algorithm::rotate_copy[opt::aligned<32>]);
        check_aligned.template operator()<32, 32>(raze::algorithm::rotate_copy[opt::aligned<32>][opt::unroll<2>]);
        check_aligned.template operator()<64, 64>(raze::algorithm::rotate_copy[opt::aligned<64>]);
        check_aligned.template operator()<64, 64>(raze::algorithm::rotate_copy[opt::aligned<64>][opt::fautovec]);
        check_aligned.template operator()<64, 64>(raze::algorithm::rotate_copy[opt::aligned<64>][opt::fscalar]);
        check_aligned.template operator()<1, 1>(raze::algorithm::rotate_copy[opt::aligned<opt::unaligned>]);
        check_aligned.template operator()<1, 1>(raze::algorithm::rotate_copy[opt::aligned<64>][opt::aligned<opt::unaligned>]);
        check_aligned.template operator()<64, 1>(raze::algorithm::rotate_copy[opt::aligned<64, opt::unaligned>]);
        check_aligned.template operator()<1, 64>(raze::algorithm::rotate_copy[opt::aligned<opt::unaligned, 64>]);
        check_aligned.template operator()<32, 64>(raze::algorithm::rotate_copy[opt::aligned<32, 64>][opt::fstatic]);
        check_aligned.template operator()<64, 16>(raze::algorithm::rotate_copy[opt::aligned<64, 16>]);
    }

    run<range, T>(cfg,
        [](auto c) {
            auto mid = std::ranges::next(c.first(), c.last());
            return raze::algorithm::rotate_copy(c.first(), mid, c.last(), c.out());
        },
        [](auto c) {
            return std::ranges::rotate_copy(
                c.data.value.begin(), c.data.value.end(), c.data.value.end(), c.out());
        });

    constexpr auto check_ce_range = []<size_t N, size_t M>(auto algo) constexpr {
        return constexpr_run<T, n, N, M>(
            [algo](auto c) constexpr {
                auto mid = std::ranges::next(c.first(), static_cast<std::ptrdiff_t>(c.count()));
                return algo(c.range(), mid, c.out());
            },
            [](auto c) constexpr {
                auto mid = std::ranges::next(c.data.value.begin(), static_cast<std::ptrdiff_t>(c.count()));
                return std::ranges::rotate_copy(c.data.value.begin(), mid, c.data.value.end(), c.out());
            });
    };

    static_assert(constexpr_run<T, range, 0>(
        [](auto c) constexpr { return raze::algorithm::rotate_copy(c.first(), c.first(), c.last(), c.out()); },
        [](auto c) constexpr { return std::ranges::rotate_copy(c.data.value.begin(), c.data.value.begin(), c.data.value.end(), c.out()); }));

    static_assert(constexpr_run<T, n, 5, 0>(
        [](auto c) constexpr {
            auto mid = std::ranges::next(c.first(), static_cast<std::ptrdiff_t>(c.count()));
            return raze::algorithm::rotate_copy(c.first(), mid, c.last(), c.out());
        },
        [](auto c) constexpr {
            auto mid = std::ranges::next(c.data.value.begin(), static_cast<std::ptrdiff_t>(c.count()));
            return std::ranges::rotate_copy(c.data.value.begin(), mid, c.data.value.end(), c.out());
        }));

    static_assert(check_ce_range.template operator()<5, 2>(raze::algorithm::rotate_copy));
    static_assert(check_ce_range.template operator()<5, 2>(raze::algorithm::rotate_copy[opt::fautovec]));
    static_assert(check_ce_range.template operator()<5, 2>(raze::algorithm::rotate_copy[opt::aligned<64, opt::unaligned>]));

    static_assert([]() constexpr {
        std::array<T, 0> i0{}, o0{};
        auto r0 = raze::algorithm::rotate_copy(i0, i0.begin(), o0.begin());

        std::array<T, 1> i1{ T(1) }, o1{};
        auto r1 = raze::algorithm::rotate_copy(i1, i1.begin(), o1.begin());

        std::array<T, 4> i4{ T(1), T(2), T(3), T(4) }, o4{};
        auto r4 = raze::algorithm::rotate_copy(i4, i4.begin() + 1, o4.begin());

        std::array<T, 5> i5{ T(1), T(2), T(3), T(4), T(5) }, o5{};
        auto r5 = raze::algorithm::rotate_copy[opt::fscalar](i5, i5.begin() + 3, o5.begin());

        std::array<T, 4> i4e{ T(1), T(2), T(3), T(4) }, o4e{};
        auto r4e = raze::algorithm::rotate_copy(i4e, i4e.end(), o4e.begin());

        std::array<T, 5> i5s{ T(1), T(2), T(3), T(4), T(5) }, o5s{};
        auto r5s = raze::algorithm::rotate_copy(i5s.begin() + 1, i5s.begin() + 3, i5s.begin() + 4, o5s.begin() + 1);

        return r0.in == i0.end() && r0.out == o0.end()
            && r1.in == i1.end() && r1.out == o1.end() && o1 == std::array<T, 1>{ T(1) }
            && r4.in == i4.end() && r4.out == o4.end() && o4 == std::array<T, 4>{ T(2), T(3), T(4), T(1) }
            && r5.in == i5.end() && r5.out == o5.end() && o5 == std::array<T, 5>{ T(4), T(5), T(1), T(2), T(3) }
            && r4e.in == i4e.end() && r4e.out == o4e.end() && o4e == std::array<T, 4>{ T(1), T(2), T(3), T(4) }
            && r5s.in == i5s.begin() + 4 && r5s.out == o5s.begin() + 4 && o5s == std::array<T, 5>{ T(0), T(4), T(2), T(3), T(0) };
    }());
};

RTTS_CASE_TPL("raze::algorithm::rotate_copy boundaries", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    rotate_copy_boundary_sweep<T>();
};

RTTS_CASE_TPL("raze::algorithm::rotate_copy fuzz", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    for (unsigned seed : fuzz_seeds)
        rotate_copy_fuzz<T>(seed, 200);
};

RTTS_CASE("raze::algorithm::rotate_copy structs") {
    {
        std::vector<rotate_copy_item> input{
            { 1, 10 }, { 2, 20 }, { 3, 30 }, { 4, 40 }, { 5, 50 }
        };
        const auto original = input;

        std::vector<rotate_copy_item> exp(input.size());
        std::ranges::rotate_copy(input, input.begin() + 2, exp.begin());

        std::vector<rotate_copy_item> out(input.size());
        auto res = raze::algorithm::rotate_copy(input, input.begin() + 2, out.begin());
        RTTS_EXPECT(res.in == input.end());
        RTTS_EXPECT(res.out == out.end());
        RTTS_EXPECT(out == exp);
        RTTS_EXPECT(input == original);

        out.assign(input.size(), {});
        res = raze::algorithm::rotate_copy[raze::options::fscalar](input, input.begin() + 2, out.begin());
        RTTS_EXPECT(res.in == input.end());
        RTTS_EXPECT(res.out == out.end());
        RTTS_EXPECT(out == exp);
    }

    {
        std::vector<rotate_copy_odd_item> input(250);
        for (size_t i = 0; i < input.size(); ++i)
            for (size_t j = 0; j < 7; ++j)
                input[i].bytes[j] = static_cast<uint8_t>((i * 7 + j) & 0xFF);

        for (size_t mid : { size_t(0), size_t(50), size_t(75), size_t(100), size_t(250) }) {
            std::vector<rotate_copy_odd_item> exp(input.size());
            std::ranges::rotate_copy(input, input.begin() + mid, exp.begin());

            std::vector<rotate_copy_odd_item> out(input.size());
            auto res = raze::algorithm::rotate_copy(input, input.begin() + mid, out.begin());
            RTTS_EXPECT(res.in == input.end());
            RTTS_EXPECT(res.out == out.end());
            RTTS_EXPECT(out == exp);
        }
    }

    {
        std::vector<rotate_copy_large_item> input(7);
        for (size_t i = 0; i < input.size(); ++i)
            input[i].data[0] = static_cast<int32_t>(i + 1);

        for (size_t mid : { size_t(0), size_t(2), size_t(3), size_t(7) }) {
            std::vector<rotate_copy_large_item> exp(input.size());
            std::ranges::rotate_copy(input, input.begin() + mid, exp.begin());

            std::vector<rotate_copy_large_item> out(input.size());
            auto res = raze::algorithm::rotate_copy(input, input.begin() + mid, out.begin());
            RTTS_EXPECT(res.in == input.end());
            RTTS_EXPECT(res.out == out.end());
            RTTS_EXPECT(out == exp);
        }
    }

    {
        std::vector<std::string> input{
            "alpha", "beta", "gamma", "delta", "epsilon"
        };
        const auto original = input;

        std::vector<std::string> exp(input.size());
        std::ranges::rotate_copy(input, input.begin() + 2, exp.begin());

        std::vector<std::string> out(input.size());
        auto res = raze::algorithm::rotate_copy(input, input.begin() + 2, out.begin());
        RTTS_EXPECT(res.in == input.end());
        RTTS_EXPECT(res.out == out.end());
        RTTS_EXPECT(out == exp);
        RTTS_EXPECT(input == original);
    }

    {
        std::vector<int> input{ 1, 2, 3, 4, 5, 6, 7, 8, 9 };
        std::vector<double> exp(input.size());
        std::ranges::rotate_copy(input, input.begin() + 4, exp.begin());

        std::vector<double> out(input.size());
        auto res = raze::algorithm::rotate_copy(input, input.begin() + 4, out.begin());
        RTTS_EXPECT(res.in == input.end());
        RTTS_EXPECT(res.out == out.end());
        RTTS_EXPECT(out == exp);
    }
};

RTTS_CASE("raze::algorithm::rotate_copy bidirectional") {
    std::list<int> input{ 1, 2, 3, 4, 5 };

    for (size_t mid : { size_t(0), size_t(2), size_t(3), size_t(5) }) {
        std::vector<int> exp;
        std::ranges::rotate_copy(
            input, std::ranges::next(input.begin(), static_cast<std::ptrdiff_t>(mid)), std::back_inserter(exp));

        std::vector<int> out;
        auto res = raze::algorithm::rotate_copy(
            input, std::ranges::next(input.begin(), static_cast<std::ptrdiff_t>(mid)), std::back_inserter(out));
        RTTS_EXPECT(res.in == input.end());
        RTTS_EXPECT(out == exp);

        out.clear();
        auto scalar_res = raze::algorithm::rotate_copy[raze::options::fscalar](
            input.begin(),
            std::ranges::next(input.begin(), static_cast<std::ptrdiff_t>(mid)),
            input.end(),
            std::back_inserter(out));
        RTTS_EXPECT(scalar_res.in == input.end());
        RTTS_EXPECT(out == exp);
    }
};

RTTS_CASE("raze::algorithm::rotate_copy forward") {
    std::forward_list<int> input{ 1, 2, 3, 4, 5 };

    for (size_t mid : { size_t(0), size_t(1), size_t(2), size_t(4), size_t(5) }) {
        std::vector<int> exp;
        std::ranges::rotate_copy(
            input, std::ranges::next(input.begin(), static_cast<std::ptrdiff_t>(mid)), std::back_inserter(exp));

        std::vector<int> out;
        auto res = raze::algorithm::rotate_copy(
            input, std::ranges::next(input.begin(), static_cast<std::ptrdiff_t>(mid)), std::back_inserter(out));
        RTTS_EXPECT(res.in == input.end());
        RTTS_EXPECT(out == exp);

        std::vector<int> out_vec(5);
        auto scalar_res = raze::algorithm::rotate_copy[raze::options::fscalar](
            input.begin(),
            std::ranges::next(input.begin(), static_cast<std::ptrdiff_t>(mid)),
            input.end(),
            out_vec.begin());
        RTTS_EXPECT(scalar_res.in == input.end());
        RTTS_EXPECT(scalar_res.out == out_vec.end());
        RTTS_EXPECT(out_vec == exp);
    }
};

RTTS_CASE("raze::algorithm::rotate_copy sentinel") {
    {
        std::array<int, 5> input{ 1, 2, 3, 4, 5 };
        std::array<int, 3> out{};

        auto first = std::counted_iterator(input.begin() + 1, 3);
        auto mid = std::counted_iterator(input.begin() + 2, 2);
        auto res = raze::algorithm::rotate_copy(first, mid, std::default_sentinel, out.begin());

        RTTS_EXPECT(res.in == std::default_sentinel);
        RTTS_EXPECT(res.in.base() == input.begin() + 4);
        RTTS_EXPECT(res.out == out.end());
        RTTS_EXPECT((out == std::array<int, 3>{ 3, 4, 2 }));
    }

    {
        std::vector<uint8_t> input(1600);
        for (size_t i = 0; i < input.size(); ++i)
            input[i] = static_cast<uint8_t>(i & 0xFF);

        std::vector<uint8_t> exp(input.size());
        std::ranges::rotate_copy(input, input.begin() + 525, exp.begin());

        std::vector<uint8_t> out(input.size());
        auto first = std::counted_iterator(input.begin(), static_cast<std::ptrdiff_t>(input.size()));
        auto mid = std::counted_iterator(input.begin() + 525, static_cast<std::ptrdiff_t>(input.size() - 525));
        auto res = raze::algorithm::rotate_copy(first, mid, std::default_sentinel, out.begin());

        RTTS_EXPECT(res.in == std::default_sentinel);
        RTTS_EXPECT(res.in.base() == input.end());
        RTTS_EXPECT(res.out == out.end());
        RTTS_EXPECT(out == exp);
    }
};


RTTS_CASE("raze::algorithm::rotate_copy views and edge cases") {
    namespace opt = raze::options;
    namespace views = std::views;

    // Views: iota, take, drop, reverse, filter, transform, non-common
    {
        const std::vector<int> src = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };

        auto iv = views::iota(1, 20);
        std::vector<int> r1, s1;
        raze::algorithm::rotate_copy(iv, iv.begin() + 6, std::back_inserter(r1));
        std::ranges::rotate_copy(iv, iv.begin() + 6, std::back_inserter(s1));
        RTTS_EXPECT(r1 == s1);

        auto chain = src | views::filter([](int x) { return (x % 2) == 1; }) | views::transform([](int x) { return x * 10; });
        std::vector<int> r2, s2;
        raze::algorithm::rotate_copy[opt::aligned<64, opt::unaligned>](chain, std::ranges::next(chain.begin(), 3), std::back_inserter(r2));
        std::ranges::rotate_copy(chain, std::ranges::next(chain.begin(), 3), std::back_inserter(s2));
        RTTS_EXPECT(r2 == s2);

        auto rv = src | views::reverse;
        std::vector<int> r3, s3;
        raze::algorithm::rotate_copy(rv, rv.begin() + 4, std::back_inserter(r3));
        std::ranges::rotate_copy(rv, rv.begin() + 4, std::back_inserter(s3));
        RTTS_EXPECT(r3 == s3);

        auto nc = rtts::make_non_common(src);
        std::vector<int> nc1(src.size()), nc2(src.size());
        raze::algorithm::rotate_copy(nc, nc.begin() + 5, nc1.begin());
        std::ranges::rotate_copy(nc, nc.begin() + 5, nc2.begin());
        RTTS_EXPECT(nc1 == nc2);
    }

    // back_inserter, front_inserter, inserter, empty, middle == first/last, volatile, rvalue dangling
    {
        const std::vector<int> src = { 1, 2, 3, 4, 5 };
        const std::vector<int> expected = { 3, 4, 5, 1, 2 };

        std::vector<int> b1, b2, b3, b4;
        raze::algorithm::rotate_copy(src, src.begin() + 2, std::back_inserter(b1));
        raze::algorithm::rotate_copy[opt::fscalar](src, src.begin() + 2, std::back_inserter(b2));
        raze::algorithm::rotate_copy[opt::fautovec][opt::fstatic](src, src.begin() + 2, std::back_inserter(b3));
        alignas(64) const std::array<int, 5> src_aligned = { 1, 2, 3, 4, 5 };
        raze::algorithm::rotate_copy[opt::aligned<64, opt::unaligned>](src_aligned, src_aligned.begin() + 2, std::back_inserter(b4));
        RTTS_EXPECT(b1 == expected && b2 == expected && b3 == expected && b4 == expected);

        std::deque<int> f1, f2;
        raze::algorithm::rotate_copy(src, src.begin() + 2, std::front_inserter(f1));
        std::ranges::rotate_copy(src, src.begin() + 2, std::front_inserter(f2));
        RTTS_EXPECT(f1 == f2);

        std::vector<int> i1 = { 0, 0 }, i2 = { 0, 0 };
        raze::algorithm::rotate_copy(src, src.begin() + 2, std::inserter(i1, i1.begin() + 1));
        std::ranges::rotate_copy(src, src.begin() + 2, std::inserter(i2, i2.begin() + 1));
        RTTS_EXPECT(i1 == i2);

        std::vector<int> empty_v, empty_out;
        raze::algorithm::rotate_copy(empty_v, empty_v.begin(), std::back_inserter(empty_out));
        RTTS_EXPECT(empty_out.empty());

        std::vector<int> mf, ml;
        raze::algorithm::rotate_copy(src, src.begin(), std::back_inserter(mf));
        raze::algorithm::rotate_copy(src, src.end(), std::back_inserter(ml));
        RTTS_EXPECT(mf == src && ml == src);

        volatile int vsrc[16], vdst[16];
        for (int i = 0; i < 16; ++i) vsrc[i] = i;
        raze::algorithm::rotate_copy(vsrc, vsrc + 5, vsrc + 16, vdst);
        for (int i = 0; i < 16; ++i) RTTS_EXPECT(vdst[i] == (i + 5) % 16);

        std::vector<int> tmp = { 10, 20, 30, 40 };
        auto mid = tmp.begin() + 1;
        std::vector<int> rdst;
        auto rres = raze::algorithm::rotate_copy(std::move(tmp), mid, std::back_inserter(rdst));
        static_assert(std::same_as<decltype(rres.in), std::ranges::dangling>);
        RTTS_EXPECT((rdst == std::vector<int>{ 20, 30, 40, 10 }));
    }
};