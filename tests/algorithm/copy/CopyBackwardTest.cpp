#include <tests/rts/rts.h>
#include <raze/algorithm/copy/CopyBackward.h>
#include <algorithm>
#include <array>
#include <bit>
#include <cstdint>
#include <iterator>
#include <vector>

RTTS_CASE_TPL("raze::algorithm::copy_backward", rtts::algorithm::all_types)
< class T > (rtts::type<T>) {
    using namespace rtts::algorithm;
    namespace opt = raze::options;
    auto cfg = config::thorough();

    auto check_iter = [&](auto algo) {
        run<range, T, bidirectional_iterator>(cfg,
            [algo](auto c) { return algo(c.first(), c.last(), c.out() + std::ranges::distance(c.first(), c.last())); },
            [](auto c) { return std::ranges::copy_backward(c.first(), c.last(), c.out() + std::ranges::distance(c.first(), c.last())); });
    };

    auto check_range = [&](auto algo) {
        run<range, T, bidirectional_iterator>(cfg,
            [algo](auto c) { return algo(c.range(), c.out() + std::ranges::distance(c.range())); },
            [](auto c) { return std::ranges::copy_backward(c.range(), c.out() + std::ranges::distance(c.range())); });
    };

    check_iter(raze::algorithm::copy_backward);
    check_range(raze::algorithm::copy_backward);
    check_iter(raze::algorithm::copy_backward[opt::fscalar]);
    check_range(raze::algorithm::copy_backward[opt::unroll<2>]);
    check_range(raze::algorithm::copy_backward[opt::fstatic]);
    check_iter(raze::algorithm::copy_backward[opt::fautovec]);
    check_range(raze::algorithm::copy_backward[opt::fautovec][opt::fstatic]);

    {
        auto acfg = config::compact();

        auto check_aligned = [&]<size_t A1, size_t AOut>(auto algo) {
            run_aligned<A1, AOut, range, T>(acfg,
                [algo](auto c) { return algo(c.first(), c.last(), c.out() + std::ranges::distance(c.first(), c.last())); },
                [](auto c) { return std::ranges::copy_backward(c.first(), c.last(), c.out() + std::ranges::distance(c.first(), c.last())); });

            run_aligned<A1, AOut, range, T>(acfg,
                [algo](auto c) { return algo(c.range(), c.out() + std::ranges::distance(c.range())); },
                [](auto c) { return std::ranges::copy_backward(c.range(), c.out() + std::ranges::distance(c.range())); });
        };

        check_aligned.template operator()<16, 16>(raze::algorithm::copy_backward[opt::aligned<16>]);
        check_aligned.template operator()<32, 32>(raze::algorithm::copy_backward[opt::aligned<32>]);
        check_aligned.template operator()<32, 32>(raze::algorithm::copy_backward[opt::aligned<32>][opt::unroll<2>]);
        check_aligned.template operator()<64, 64>(raze::algorithm::copy_backward[opt::aligned<64>]);
        check_aligned.template operator()<64, 64>(raze::algorithm::copy_backward[opt::aligned<64>][opt::fautovec]);
        check_aligned.template operator()<64, 64>(raze::algorithm::copy_backward[opt::aligned<64>][opt::fscalar]);
        check_aligned.template operator()<1, 1>(raze::algorithm::copy_backward[opt::aligned<opt::unaligned>]);
        check_aligned.template operator()<1, 1>(raze::algorithm::copy_backward[opt::aligned<64>][opt::aligned<opt::unaligned>]);
        check_aligned.template operator()<64, 1>(raze::algorithm::copy_backward[opt::aligned<64, opt::unaligned>]);
        check_aligned.template operator()<1, 64>(raze::algorithm::copy_backward[opt::aligned<opt::unaligned, 64>]);
        check_aligned.template operator()<32, 64>(raze::algorithm::copy_backward[opt::aligned<32, 64>][opt::fstatic]);
        check_aligned.template operator()<64, 16>(raze::algorithm::copy_backward[opt::aligned<64, 16>]);
    }

    static_assert(constexpr_run<T, range, 8>(
        [](auto c) constexpr { return raze::algorithm::copy_backward(c.first(), c.last(), c.out() + std::ranges::distance(c.first(), c.last())); },
        [](auto c) constexpr { return std::ranges::copy_backward(c.first(), c.last(), c.out() + std::ranges::distance(c.first(), c.last())); }));

    constexpr auto check_ce = [](auto algo) {
        return constexpr_run<T, range, 8>(
            [algo](auto c) constexpr { return algo(c.range(), c.out() + std::ranges::distance(c.range())); },
            [](auto c) constexpr { return std::ranges::copy_backward(c.range(), c.out() + std::ranges::distance(c.range())); });
    };

    static_assert(check_ce(raze::algorithm::copy_backward));
    static_assert(check_ce(raze::algorithm::copy_backward[opt::fautovec]));
    static_assert(check_ce(raze::algorithm::copy_backward[opt::aligned<64, opt::unaligned>]));
};

RTTS_CASE("raze::algorithm::copy_backward mixed arithmetic types") {
    using namespace rtts::algorithm;
    namespace opt = raze::options;
    auto cfg = config::thorough();

    auto check_iter = [&]<class S, class D>(auto algo) {
        run<range, S, D, bidirectional_iterator>(cfg,
            [algo](auto c) { return algo(c.first(), c.last(), c.out2() + std::ranges::distance(c.first(), c.last())); },
            [](auto c) { return std::ranges::copy_backward(c.first(), c.last(), c.out2() + std::ranges::distance(c.first(), c.last())); });
    };

    check_iter.template operator()<int32_t, float>(raze::algorithm::copy_backward);
    check_iter.template operator()<uint32_t, float>(raze::algorithm::copy_backward);
    check_iter.template operator()<int64_t, double>(raze::algorithm::copy_backward);
    check_iter.template operator()<int8_t, int16_t>(raze::algorithm::copy_backward);
    check_iter.template operator()<int16_t, int8_t>(raze::algorithm::copy_backward);
    check_iter.template operator()<int32_t, float>(raze::algorithm::copy_backward[opt::fscalar]);
    check_iter.template operator()<int32_t, float>(raze::algorithm::copy_backward[opt::fautovec]);

    run<range, int32_t, int64_t, bidirectional_iterator>(cfg,
        [](auto c) { return raze::algorithm::copy_backward(c.range(), c.out2() + std::ranges::distance(c.range())); },
        [](auto c) { return std::ranges::copy_backward(c.range(), c.out2() + std::ranges::distance(c.range())); });

    run_aligned_mixed<64, 1, range, int32_t, float>(config::compact(),
        [](auto c) { return raze::algorithm::copy_backward[opt::aligned<64, opt::unaligned>](c.first(), c.last(), c.out2() + std::ranges::distance(c.first(), c.last())); },
        [](auto c) { return std::ranges::copy_backward(c.first(), c.last(), c.out2() + std::ranges::distance(c.first(), c.last())); });

    constexpr auto check_ce_iter = []<class S, class D>() {
        return constexpr_run_mixed<S, D, range, 8>(
            [](auto c) constexpr { return raze::algorithm::copy_backward(c.first(), c.last(), c.out2() + std::ranges::distance(c.first(), c.last())); },
            [](auto c) constexpr { return std::ranges::copy_backward(c.first(), c.last(), c.out2() + std::ranges::distance(c.first(), c.last())); });
    };

    static_assert(check_ce_iter.template operator()<int32_t, float>());
    static_assert(check_ce_iter.template operator()<int64_t, double>());
    static_assert(constexpr_run_mixed<int8_t, int16_t, range, 8>(
        [](auto c) constexpr { return raze::algorithm::copy_backward(c.range(), c.out2() + std::ranges::distance(c.range())); },
        [](auto c) constexpr { return std::ranges::copy_backward(c.range(), c.out2() + std::ranges::distance(c.range())); }));
};

struct tc_struct_4 {
    int32_t x;
    bool operator==(const tc_struct_4&) const = default;
};
struct tc_struct_8 {
    int64_t x;
    bool operator==(const tc_struct_8&) const = default;
};
struct tc_struct_12 {
    int32_t a = 0, b = 0, c = 0;
    bool operator==(const tc_struct_12&) const = default;
};

struct tc_nontrivial {
    int32_t x = 0;
    int32_t copies = 0;

    constexpr tc_nontrivial() = default;
    constexpr tc_nontrivial(int32_t v) : x(v) {}
    constexpr tc_nontrivial(const tc_nontrivial&) = default;
    constexpr tc_nontrivial& operator=(const tc_nontrivial& o) {
        x = o.x;
        copies = o.copies + 1;
        return *this;
    }
    bool operator==(const tc_nontrivial&) const = default;
};

RTTS_CASE("raze::algorithm::copy_backward structs") {
    using namespace rtts::algorithm;
    namespace opt = raze::options;
    auto cfg = config::thorough();

    auto check_iter = [&]<class S>(auto algo) {
        run<range, S, bidirectional_iterator>(cfg,
            [algo](auto c) { return algo(c.first(), c.last(), c.out() + std::ranges::distance(c.first(), c.last())); },
            [](auto c) { return std::ranges::copy_backward(c.first(), c.last(), c.out() + std::ranges::distance(c.first(), c.last())); });
    };

    check_iter.template operator()<tc_struct_4>(raze::algorithm::copy_backward);
    check_iter.template operator()<tc_struct_8>(raze::algorithm::copy_backward);
    check_iter.template operator()<tc_struct_12>(raze::algorithm::copy_backward);
    check_iter.template operator()<tc_struct_4>(raze::algorithm::copy_backward[opt::fscalar]);
    check_iter.template operator()<tc_struct_4>(raze::algorithm::copy_backward[opt::unroll<2>]);
    check_iter.template operator()<tc_struct_4>(raze::algorithm::copy_backward[opt::fautovec]);

    run<range, tc_struct_12, bidirectional_iterator>(cfg,
        [](auto c) { return raze::algorithm::copy_backward(c.range(), c.out() + std::ranges::distance(c.range())); },
        [](auto c) { return std::ranges::copy_backward(c.range(), c.out() + std::ranges::distance(c.range())); });

    run_aligned<64, 1, range, tc_struct_4>(config::compact(),
        [](auto c) { return raze::algorithm::copy_backward[opt::aligned<64, opt::unaligned>](c.range(), c.out() + std::ranges::distance(c.range())); },
        [](auto c) { return std::ranges::copy_backward(c.range(), c.out() + std::ranges::distance(c.range())); });

    constexpr auto check_ce = []<class S>() {
        return constexpr_run<S, range, 8>(
            [](auto c) constexpr { return raze::algorithm::copy_backward(c.first(), c.last(), c.out() + std::ranges::distance(c.first(), c.last())); },
            [](auto c) constexpr { return std::ranges::copy_backward(c.first(), c.last(), c.out() + std::ranges::distance(c.first(), c.last())); });
    };

    static_assert(check_ce.template operator()<tc_struct_4>());
    static_assert(check_ce.template operator()<tc_struct_8>());
    static_assert(check_ce.template operator()<tc_nontrivial>());
    static_assert(constexpr_run<tc_struct_12, range, 8>(
        [](auto c) constexpr { return raze::algorithm::copy_backward(c.range(), c.out() + std::ranges::distance(c.range())); },
        [](auto c) constexpr { return std::ranges::copy_backward(c.range(), c.out() + std::ranges::distance(c.range())); }));
};

RTTS_CASE("raze::algorithm::copy_backward non trivially copyable") {
    for (const size_t n : { size_t(0), size_t(1), size_t(2), size_t(15), size_t(16), size_t(17), size_t(100) }) {
        std::vector<tc_nontrivial> src(n);
        for (size_t i = 0; i < n; ++i)
            src[i].x = static_cast<int32_t>(i * 7 + 1);

        std::vector<tc_nontrivial> d1(n), d2(n);

        const auto r1 = raze::algorithm::copy_backward(src.begin(), src.end(), d1.end());
        const auto r2 = std::ranges::copy_backward(src.begin(), src.end(), d2.end());

        RTTS_EXPECT(d1 == d2);
        RTTS_EXPECT((r1.in - src.begin()) == (r2.in - src.begin()));
        RTTS_EXPECT((r1.out - d1.begin()) == (r2.out - d2.begin()));

        std::vector<tc_nontrivial> d3(n), d4(n);
        const auto r3 = raze::algorithm::copy_backward(src, d3.end());
        const auto r4 = std::ranges::copy_backward(src, d4.end());

        RTTS_EXPECT(d3 == d4);
        RTTS_EXPECT((r3.out - d3.begin()) == (r4.out - d4.begin()));
    }
};

RTTS_CASE("raze::algorithm::copy_backward overlap shift-right") {
    for (const size_t n : { size_t(0), size_t(1), size_t(2), size_t(3), size_t(5),
        size_t(7), size_t(8), size_t(9), size_t(15),
        size_t(16), size_t(17), size_t(31), size_t(32),
        size_t(33), size_t(63), size_t(64), size_t(65),
        size_t(100), size_t(1000) }) {
        for (const size_t shift : { size_t(0), size_t(1), size_t(2), size_t(3), size_t(7) }) {
            if (shift > n) continue;

            std::vector<int32_t> v1(n);
            for (size_t i = 0; i < n; ++i) v1[i] = static_cast<int32_t>(i);
            auto v2 = v1;

            const auto k = static_cast<std::ptrdiff_t>(n - shift);

            raze::algorithm::copy_backward(v1.begin(), v1.begin() + k, v1.end());
            std::ranges::copy_backward(v2.begin(), v2.begin() + k, v2.end());

            RTTS_EXPECT(v1 == v2);

            for (size_t i = 0; i < n; ++i) {
                if (i < shift)
                    RTTS_EXPECT(v1[i] == static_cast<int32_t>(i));
                else
                    RTTS_EXPECT(v1[i] == static_cast<int32_t>(i - shift));
            }
        }
    }
};

RTTS_CASE("raze::algorithm::copy_backward overlap with nontrivial type") {
    for (const size_t n : { size_t(1), size_t(3), size_t(7), size_t(15),
        size_t(16), size_t(17), size_t(64) }) {
        std::vector<tc_nontrivial> v1(n);
        for (size_t i = 0; i < n; ++i) v1[i].x = static_cast<int32_t>(i);
        auto v2 = v1;

        const auto k = static_cast<std::ptrdiff_t>(n - 1);
        if (k <= 0) continue;

        raze::algorithm::copy_backward(v1.begin(), v1.begin() + k, v1.end());
        std::ranges::copy_backward(v2.begin(), v2.begin() + k, v2.end());

        RTTS_EXPECT(v1 == v2);
    }
};

RTTS_CASE("raze::algorithm::copy_backward memcpy size boundaries") {
    const size_t ns[] = {
        0, 1, 2, 3, 4, 7, 8, 9, 15, 16, 17, 31, 32, 33, 63, 64, 65,
        127, 128, 129, 255, 256, 257, 511, 512, 513,
        1023, 1024, 1025, 2047, 2048, 2049, 4095, 4096, 4097, 8191, 8192, 8193,
        65536, 65537,
        3u * 1024 * 1024 - 1, 3u * 1024 * 1024, 3u * 1024 * 1024 + 1, 3u * 1024 * 1024 + 129
    };
    constexpr size_t guard = 64;

    for (const size_t n : ns) {
        for (const size_t src_off : { size_t(0), size_t(3) }) {
            for (const size_t dst_prefix : { size_t(0), size_t(1), size_t(7) }) {
                std::vector<uint8_t> src(n + src_off);
                for (size_t i = 0; i < src.size(); ++i)
                    src[i] = static_cast<uint8_t>(i * 131 + 7);

                std::vector<uint8_t> d1(dst_prefix + n + guard, 0xCC);
                auto d2 = d1;

                const auto first = src.begin() + static_cast<std::ptrdiff_t>(src_off);
                const auto last = src.end();

                const auto out_end_offset = static_cast<std::ptrdiff_t>(dst_prefix + n);

                const auto r1 = raze::algorithm::copy_backward(
                    first, last, d1.begin() + out_end_offset);
                const auto r2 = std::ranges::copy_backward(
                    first, last, d2.begin() + out_end_offset);

                RTTS_EXPECT(d1 == d2);
                RTTS_EXPECT((r1.in - src.begin()) == (r2.in - src.begin()));
                RTTS_EXPECT((r1.out - d1.begin()) == (r2.out - d2.begin()));

                for (size_t i = 0; i < dst_prefix; ++i)
                    RTTS_EXPECT(d1[i] == 0xCC);

                for (size_t i = dst_prefix + n; i < d1.size(); ++i)
                    RTTS_EXPECT(d1[i] == 0xCC);
            }
        }
    }
};

namespace {
    template <class T, std::size_t N>
    constexpr bool copy_backward_static_array_matches() {
        std::array<T, N> src{};
        for (std::size_t i = 0; i < N; ++i) src[i] = static_cast<T>(i * 3 + 1);

        std::array<T, N> dst1{}, dst2{};
        const auto r1 = raze::algorithm::copy_backward(src.begin(), src.end(), dst1.end());
        const auto r2 = std::ranges::copy_backward(src.begin(), src.end(), dst2.end());

        bool ok = dst1 == dst2
            && (r1.in - src.begin()) == (r2.in - src.begin())
            && (r1.out - dst1.begin()) == (r2.out - dst2.begin());

        if constexpr (std::is_trivially_copyable_v<T>) ok = ok && dst1 == src;
        return ok;
    }

    template <class T, std::size_t N, std::size_t Shift>
    constexpr bool copy_backward_static_array_shift_right_matches() {
        static_assert(Shift < N);

        std::array<T, N> src{};
        for (std::size_t i = 0; i < N; ++i) src[i] = static_cast<T>(i * 3 + 1);

        std::array<T, N> dst1{}, dst2{};

        const auto k = static_cast<std::ptrdiff_t>(N - Shift);
        const auto r1 = raze::algorithm::copy_backward(src.begin(), src.begin() + k, dst1.end());
        const auto r2 = std::ranges::copy_backward(src.begin(), src.begin() + k, dst2.end());

        bool ok = dst1 == dst2
            && (r1.in - src.begin()) == (r2.in - src.begin())
            && (r1.out - dst1.begin()) == (r2.out - dst2.begin());

        for (std::size_t i = 0; i < Shift; ++i) ok = ok && dst1[i] == T{};
        if constexpr (std::is_trivially_copyable_v<T>) {
            for (std::size_t i = 0; i < N - Shift; ++i)
                ok = ok && dst1[i + Shift] == src[i];
        }

        return ok;
    }
}

static_assert(copy_backward_static_array_matches<int32_t, 1>());
static_assert(copy_backward_static_array_matches<int32_t, 7>());
static_assert(copy_backward_static_array_matches<int8_t, 15>());
static_assert(copy_backward_static_array_matches<int16_t, 33>());
static_assert(copy_backward_static_array_matches<int64_t, 17>());
static_assert(copy_backward_static_array_matches<uint8_t, 200>());
static_assert(copy_backward_static_array_matches<tc_nontrivial, 5>());

static_assert(copy_backward_static_array_shift_right_matches<int32_t, 8, 1>());
static_assert(copy_backward_static_array_shift_right_matches<int32_t, 8, 3>());
static_assert(copy_backward_static_array_shift_right_matches<int32_t, 16, 7>());
static_assert(copy_backward_static_array_shift_right_matches<int64_t, 33, 17>());
static_assert(copy_backward_static_array_shift_right_matches<uint8_t, 200, 129>());
static_assert(copy_backward_static_array_shift_right_matches<tc_nontrivial, 8, 3>());


RTTS_CASE("raze::algorithm::copy_backward views and edge cases") {
    namespace opt = raze::options;
    namespace views = std::views;

    {
        const std::vector<int> src = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16 };

        std::vector<int> d1(8), d2(8);
        raze::algorithm::copy_backward(views::iota(1, 9), d1.end());
        std::ranges::copy_backward(views::iota(1, 9), d2.end());
        RTTS_EXPECT(d1 == d2);

        raze::algorithm::copy_backward[opt::fstatic](src | views::drop(4) | views::take(8), d1.end());
        std::ranges::copy_backward(src | views::drop(4) | views::take(8), d2.end());
        RTTS_EXPECT(d1 == d2);

        raze::algorithm::copy_backward(src | views::reverse | views::take(8), d1.end());
        std::ranges::copy_backward(src | views::reverse | views::take(8), d2.end());
        RTTS_EXPECT(d1 == d2);

        auto fv = src | views::filter([](int x) { return (x % 2) == 0; }) | views::transform([](int x) { return x * 2; });
        std::vector<int> f1(8), f2(8);
        raze::algorithm::copy_backward[opt::aligned<64>](fv, f1.end());
        std::ranges::copy_backward(fv, f2.end());
        RTTS_EXPECT(f1 == f2);

        std::vector<int> nc1(src.size()), nc2(src.size());
        raze::algorithm::copy_backward(rtts::make_non_common(src), nc1.end());
        std::ranges::copy_backward(rtts::make_non_common(src), nc2.end());
        RTTS_EXPECT(nc1 == nc2);
    }

    {
        const std::vector<int> src = { 5, 6, 7, 8 };
        std::list<int> l1(4), l2(4);
        raze::algorithm::copy_backward(src, l1.end());
        std::ranges::copy_backward(src, l2.end());
        RTTS_EXPECT(l1 == l2);

        volatile int vsrc[16];
        volatile int vdst[16];
        for (int i = 0; i < 16; ++i) vsrc[i] = i * 4;
        raze::algorithm::copy_backward(vsrc, vsrc + 16, vdst + 16);
        for (int i = 0; i < 16; ++i) RTTS_EXPECT(vdst[i] == i * 4);

        std::vector<int> empty_v, dst_empty;
        auto er = raze::algorithm::copy_backward(empty_v, dst_empty.end());
        RTTS_EXPECT(er.in == empty_v.end() && er.out == dst_empty.end());

        std::vector<int> rdst(3);
        auto rres = raze::algorithm::copy_backward(std::vector<int>{ 7, 8, 9 }, rdst.end());
        static_assert(std::same_as<decltype(rres.in), std::ranges::dangling>);
        RTTS_EXPECT((rdst == std::vector<int>{ 7, 8, 9 }));
    }
};