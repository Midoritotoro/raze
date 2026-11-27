#include <tests/rts/rts.h>
#include <raze/algorithm/copy/CopyN.h>
#include <algorithm>
#include <array>
#include <bit>
#include <cstdint>
#include <iterator>
#include <vector>

RTTS_CASE_TPL("raze::algorithm::copy_n", rtts::algorithm::all_types)
< class T > (rtts::type<T>) {
    using namespace rtts::algorithm;
    namespace opt = raze::options;
    auto cfg = config::thorough();

    auto check = [&](auto algo) {
        run<range, T>(cfg,
            [algo](auto c) { return algo(c.first(), std::ranges::distance(c.first(), c.last()), c.out()); },
            [](auto c) { return std::ranges::copy_n(c.first(), std::ranges::distance(c.first(), c.last()), c.out()); });
    };

    check(raze::algorithm::copy_n);
    check(raze::algorithm::copy_n[opt::fscalar]);
    check(raze::algorithm::copy_n[opt::unroll<2>]);
    check(raze::algorithm::copy_n[opt::fstatic]);
    check(raze::algorithm::copy_n[opt::fautovec]);
    check(raze::algorithm::copy_n[opt::fautovec][opt::fstatic]);

    {
        auto acfg = config::compact();

        auto check_aligned = [&]<size_t A1, size_t AOut>(auto algo) {
            run_aligned<A1, AOut, range, T>(acfg,
                [algo](auto c) { return algo(c.first(), std::ranges::distance(c.first(), c.last()), c.out()); },
                [](auto c) { return std::ranges::copy_n(c.first(), std::ranges::distance(c.first(), c.last()), c.out()); });
        };

        check_aligned.template operator()<16, 16>(raze::algorithm::copy_n[opt::aligned<16>]);
        check_aligned.template operator()<32, 32>(raze::algorithm::copy_n[opt::aligned<32>]);
        check_aligned.template operator()<32, 32>(raze::algorithm::copy_n[opt::aligned<32>][opt::unroll<2>]);
        check_aligned.template operator()<64, 64>(raze::algorithm::copy_n[opt::aligned<64>]);
        check_aligned.template operator()<64, 64>(raze::algorithm::copy_n[opt::aligned<64>][opt::fautovec]);
        check_aligned.template operator()<64, 64>(raze::algorithm::copy_n[opt::aligned<64>][opt::fscalar]);
        check_aligned.template operator()<1, 1>(raze::algorithm::copy_n[opt::aligned<opt::unaligned>]);
        check_aligned.template operator()<1, 1>(raze::algorithm::copy_n[opt::aligned<64>][opt::aligned<opt::unaligned>]);
        check_aligned.template operator()<64, 1>(raze::algorithm::copy_n[opt::aligned<64, opt::unaligned>]);
        check_aligned.template operator()<1, 64>(raze::algorithm::copy_n[opt::aligned<opt::unaligned, 64>]);
        check_aligned.template operator()<32, 64>(raze::algorithm::copy_n[opt::aligned<32, 64>][opt::fstatic]);
        check_aligned.template operator()<64, 16>(raze::algorithm::copy_n[opt::aligned<64, 16>]);
    }

    constexpr auto check_ce = [](auto algo) {
        return constexpr_run<T, range, 8>(
            [algo](auto c) constexpr { return algo(c.first(), std::ranges::distance(c.first(), c.last()), c.out()); },
            [](auto c) constexpr { return std::ranges::copy_n(c.first(), std::ranges::distance(c.first(), c.last()), c.out()); });
    };

    static_assert(check_ce(raze::algorithm::copy_n));
    static_assert(check_ce(raze::algorithm::copy_n[opt::fautovec]));
    static_assert(check_ce(raze::algorithm::copy_n[opt::aligned<64, opt::unaligned>]));
};

RTTS_CASE("raze::algorithm::copy_n mixed arithmetic types") {
    using namespace rtts::algorithm;
    namespace opt = raze::options;
    auto cfg = config::thorough();

    auto check = [&]<class S, class D>(auto algo) {
        run<range, S, D>(cfg,
            [algo](auto c) { return algo(c.first(), std::ranges::distance(c.first(), c.last()), c.out2()); },
            [](auto c) { return std::ranges::copy_n(c.first(), std::ranges::distance(c.first(), c.last()), c.out2()); });
    };

    check.template operator()<int32_t, float>(raze::algorithm::copy_n);
    check.template operator()<uint32_t, float>(raze::algorithm::copy_n);
    check.template operator()<int64_t, double>(raze::algorithm::copy_n);
    check.template operator()<int8_t, int16_t>(raze::algorithm::copy_n);
    check.template operator()<int32_t, int64_t>(raze::algorithm::copy_n);
    check.template operator()<int16_t, int8_t>(raze::algorithm::copy_n);
    check.template operator()<int32_t, float>(raze::algorithm::copy_n[opt::fscalar]);
    check.template operator()<int32_t, float>(raze::algorithm::copy_n[opt::fautovec]);

    run_aligned_mixed<64, 1, range, int32_t, float>(config::compact(),
        [](auto c) { return raze::algorithm::copy_n[opt::aligned<64, opt::unaligned>](c.first(), std::ranges::distance(c.first(), c.last()), c.out2()); },
        [](auto c) { return std::ranges::copy_n(c.first(), std::ranges::distance(c.first(), c.last()), c.out2()); });

    constexpr auto check_ce = []<class S, class D>() {
        return constexpr_run_mixed<S, D, range, 8>(
            [](auto c) constexpr { return raze::algorithm::copy_n(c.first(), std::ranges::distance(c.first(), c.last()), c.out2()); },
            [](auto c) constexpr { return std::ranges::copy_n(c.first(), std::ranges::distance(c.first(), c.last()), c.out2()); });
    };

    static_assert(check_ce.template operator()<int32_t, float>());
    static_assert(check_ce.template operator()<int64_t, double>());
    static_assert(check_ce.template operator()<int8_t, int16_t>());
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

RTTS_CASE("raze::algorithm::copy_n structs") {
    using namespace rtts::algorithm;
    namespace opt = raze::options;
    auto cfg = config::thorough();

    auto check = [&]<class S>(auto algo) {
        run<range, S>(cfg,
            [algo](auto c) { return algo(c.first(), std::ranges::distance(c.first(), c.last()), c.out()); },
            [](auto c) { return std::ranges::copy_n(c.first(), std::ranges::distance(c.first(), c.last()), c.out()); });
    };

    check.template operator()<tc_struct_4>(raze::algorithm::copy_n);
    check.template operator()<tc_struct_8>(raze::algorithm::copy_n);
    check.template operator()<tc_struct_12>(raze::algorithm::copy_n);
    check.template operator()<tc_struct_4>(raze::algorithm::copy_n[opt::fscalar]);
    check.template operator()<tc_struct_4>(raze::algorithm::copy_n[opt::unroll<2>]);
    check.template operator()<tc_struct_4>(raze::algorithm::copy_n[opt::fautovec]);

    run_aligned<64, 1, range, tc_struct_4>(config::compact(),
        [](auto c) { return raze::algorithm::copy_n[opt::aligned<64, opt::unaligned>](c.first(), std::ranges::distance(c.first(), c.last()), c.out()); },
        [](auto c) { return std::ranges::copy_n(c.first(), std::ranges::distance(c.first(), c.last()), c.out()); });

    constexpr auto check_ce = []<class S>() {
        return constexpr_run<S, range, 8>(
            [](auto c) constexpr { return raze::algorithm::copy_n(c.first(), std::ranges::distance(c.first(), c.last()), c.out()); },
            [](auto c) constexpr { return std::ranges::copy_n(c.first(), std::ranges::distance(c.first(), c.last()), c.out()); });
    };

    static_assert(check_ce.template operator()<tc_struct_4>());
    static_assert(check_ce.template operator()<tc_struct_8>());
    static_assert(check_ce.template operator()<tc_struct_12>());
    static_assert(check_ce.template operator()<tc_nontrivial>());
};

RTTS_CASE("raze::algorithm::copy_n non trivially copyable") {
    for (const size_t total : { size_t(0), size_t(1), size_t(2), size_t(15), size_t(16), size_t(17), size_t(100) }) {
        std::vector<tc_nontrivial> src(total);
        for (size_t i = 0; i < total; ++i)
            src[i].x = static_cast<int32_t>(i * 7 + 1);

        for (const size_t n : { size_t(0), size_t(1), total / 2, total }) {
            if (n > total) continue;

            std::vector<tc_nontrivial> d1(n), d2(n);

            const auto r1 = raze::algorithm::copy_n(src.begin(),
                static_cast<std::ptrdiff_t>(n), d1.begin());
            const auto r2 = std::ranges::copy_n(src.begin(),
                static_cast<std::ptrdiff_t>(n), d2.begin());

            RTTS_EXPECT(d1 == d2);
            RTTS_EXPECT((r1.in - src.begin()) == (r2.in - src.begin()));
            RTTS_EXPECT((r1.out - d1.begin()) == (r2.out - d2.begin()));
        }
    }
};

RTTS_CASE("raze::algorithm::copy_n partial count (n < distance)") {
    for (const size_t total : { size_t(0), size_t(1), size_t(15), size_t(16), size_t(17),
                                size_t(31), size_t(32), size_t(33), size_t(100), size_t(1024) }) {
        for (const size_t n : { size_t(0), size_t(1), total / 2, total }) {
            if (n > total) continue;

            std::vector<int32_t> src(total);
            for (size_t i = 0; i < total; ++i)
                src[i] = static_cast<int32_t>(i * 7 + 1);

            constexpr size_t guard = 32;
            std::vector<int32_t> d1(n + guard, -1);
            auto d2 = d1;

            const auto r1 = raze::algorithm::copy_n(src.begin(),
                static_cast<std::ptrdiff_t>(n), d1.begin());
            const auto r2 = std::ranges::copy_n(src.begin(),
                static_cast<std::ptrdiff_t>(n), d2.begin());

            RTTS_EXPECT(d1 == d2);
            RTTS_EXPECT((r1.in - src.begin()) == (r2.in - src.begin()));
            RTTS_EXPECT((r1.out - d1.begin()) == (r2.out - d2.begin()));

            // Скопировано ровно n элементов, guard-зона не тронута.
            for (size_t i = 0; i < n; ++i) RTTS_EXPECT(d1[i] == src[i]);
            for (size_t i = n; i < n + guard; ++i) RTTS_EXPECT(d1[i] == -1);
        }
    }
};

RTTS_CASE("raze::algorithm::copy_n memcpy size boundaries") {
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
            for (const size_t dst_off : { size_t(0), size_t(1), size_t(7) }) {
                std::vector<uint8_t> src(n + src_off);
                for (size_t i = 0; i < src.size(); ++i)
                    src[i] = static_cast<uint8_t>(i * 131 + 7);

                std::vector<uint8_t> d1(n + dst_off + guard, 0xCC);
                auto d2 = d1;

                const auto first = src.begin() + static_cast<std::ptrdiff_t>(src_off);

                const auto r1 = raze::algorithm::copy_n(first,
                    static_cast<std::ptrdiff_t>(n),
                    d1.begin() + static_cast<std::ptrdiff_t>(dst_off));
                const auto r2 = std::ranges::copy_n(first,
                    static_cast<std::ptrdiff_t>(n),
                    d2.begin() + static_cast<std::ptrdiff_t>(dst_off));

                RTTS_EXPECT(d1 == d2);
                RTTS_EXPECT(r1.in == r2.in);
                RTTS_EXPECT((r1.out - d1.begin()) == (r2.out - d2.begin()));
            }
        }
    }
};

namespace {
template <class T, std::size_t N>
constexpr bool copy_n_static_array_matches() {
    std::array<T, N> src{};
    for (std::size_t i = 0; i < N; ++i) src[i] = static_cast<T>(i * 3 + 1);

    std::array<T, N> dst1{}, dst2{};
    const auto r1 = raze::algorithm::copy_n(src.begin(),
        static_cast<std::ptrdiff_t>(N), dst1.begin());
    const auto r2 = std::ranges::copy_n(src.begin(),
        static_cast<std::ptrdiff_t>(N), dst2.begin());

    bool ok = dst1 == dst2
        && (r1.in - src.begin()) == (r2.in - src.begin())
        && (r1.out - dst1.begin()) == (r2.out - dst2.begin());

    if constexpr (std::is_trivially_copyable_v<T>) ok = ok && dst1 == src;
    return ok;
}

template <class T, std::size_t N, std::size_t Count>
constexpr bool copy_n_static_array_partial_matches() {
    static_assert(Count <= N);

    std::array<T, N> src{};
    for (std::size_t i = 0; i < N; ++i) src[i] = static_cast<T>(i * 3 + 1);

    std::array<T, N> dst1{}, dst2{};
    const auto r1 = raze::algorithm::copy_n(src.begin(),
        static_cast<std::ptrdiff_t>(Count), dst1.begin());
    const auto r2 = std::ranges::copy_n(src.begin(),
        static_cast<std::ptrdiff_t>(Count), dst2.begin());

    bool ok = dst1 == dst2
        && (r1.in - src.begin()) == (r2.in - src.begin())
        && (r1.out - dst1.begin()) == (r2.out - dst2.begin())
        && static_cast<std::ptrdiff_t>(Count) == (r1.in - src.begin());

    if constexpr (std::is_trivially_copyable_v<T>) {
        for (std::size_t i = 0; i < Count; ++i) ok = ok && dst1[i] == src[i];
    }

    for (std::size_t i = Count; i < N; ++i) ok = ok && dst1[i] == T{};

    return ok;
}
}

static_assert(copy_n_static_array_matches<int32_t, 1>());
static_assert(copy_n_static_array_matches<int32_t, 7>());
static_assert(copy_n_static_array_matches<int8_t, 15>());
static_assert(copy_n_static_array_matches<int16_t, 33>());
static_assert(copy_n_static_array_matches<int64_t, 17>());
static_assert(copy_n_static_array_matches<uint8_t, 200>());
static_assert(copy_n_static_array_matches<tc_nontrivial, 5>());

static_assert(copy_n_static_array_partial_matches<int32_t, 16, 0>());
static_assert(copy_n_static_array_partial_matches<int32_t, 16, 1>());
static_assert(copy_n_static_array_partial_matches<int32_t, 16, 7>());
static_assert(copy_n_static_array_partial_matches<int32_t, 16, 16>());
static_assert(copy_n_static_array_partial_matches<int64_t, 33, 17>());
static_assert(copy_n_static_array_partial_matches<uint8_t, 200, 129>());
static_assert(copy_n_static_array_partial_matches<tc_nontrivial, 8, 3>());


RTTS_CASE("raze::algorithm::copy_n views and edge cases") {
    namespace opt = raze::options;
    namespace views = std::views;

    // Views iterators: iota, drop, reverse, filter, transform
    {
        const std::vector<int> src = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12 };

        std::vector<int> d1(6), d2(6);
        auto iv = views::iota(100, 200);
        raze::algorithm::copy_n(iv.begin(), 6, d1.begin());
        std::ranges::copy_n(iv.begin(), 6, d2.begin());
        RTTS_EXPECT(d1 == d2);

        auto rv = src | views::reverse;
        raze::algorithm::copy_n(rv.begin(), 6, d1.begin());
        std::ranges::copy_n(rv.begin(), 6, d2.begin());
        RTTS_EXPECT(d1 == d2);

        auto fv = src | views::filter([](int x) { return (x % 2) == 1; }) | views::transform([](int x) { return x * 5; });
        std::vector<int> f1, f2;
        raze::algorithm::copy_n(fv.begin(), 4, std::back_inserter(f1));
        std::ranges::copy_n(fv.begin(), 4, std::back_inserter(f2));
        RTTS_EXPECT(f1 == f2);
    }

    // back_inserter, front_inserter, inserter, n == 0, n == 1, single-pass, volatile
    {
        const std::vector<int> src = { 10, 20, 30, 40, 50 };
        std::vector<int> b0, b1, b2, b3, b4;
        raze::algorithm::copy_n(src.begin(), 0, std::back_inserter(b0));
        RTTS_EXPECT(b0.empty());

        raze::algorithm::copy_n(src.begin(), 1, std::back_inserter(b1));
        RTTS_EXPECT((b1 == std::vector<int>{ 10 }));

        raze::algorithm::copy_n[opt::fstatic](src.begin(), 5, std::back_inserter(b2));
        raze::algorithm::copy_n[opt::fautovec](src.begin(), 5, std::back_inserter(b3));
        alignas(64) const std::array<int, 5> src_aligned = { 10, 20, 30, 40, 50 };
        raze::algorithm::copy_n[opt::aligned<64, opt::unaligned>](src_aligned.begin(), 5, std::back_inserter(b4));
        RTTS_EXPECT(b2 == src && b3 == src && b4 == src);

        std::deque<int> deq1, deq2;
        raze::algorithm::copy_n(src.begin(), 4, std::front_inserter(deq1));
        std::ranges::copy_n(src.begin(), 4, std::front_inserter(deq2));
        RTTS_EXPECT(deq1 == deq2);

        std::vector<int> ins1 = { 1, 9 }, ins2 = { 1, 9 };
        raze::algorithm::copy_n(src.begin(), 3, std::inserter(ins1, ins1.begin() + 1));
        std::ranges::copy_n(src.begin(), 3, std::inserter(ins2, ins2.begin() + 1));
        RTTS_EXPECT(ins1 == ins2);

        rtts::single_pass_input_range<int> spr(src);
        std::vector<int> sp_out;
        raze::algorithm::copy_n(spr.begin(), 3, std::back_inserter(sp_out));
        RTTS_EXPECT((sp_out == std::vector<int>{ 10, 20, 30 }) && spr.reads() == 3);

        volatile int vsrc[16];
        volatile int vdst[16];
        for (int i = 0; i < 16; ++i) vsrc[i] = i + 5;
        raze::algorithm::copy_n(vsrc, 16, vdst);
        for (int i = 0; i < 16; ++i) RTTS_EXPECT(vdst[i] == i + 5);
    }
};