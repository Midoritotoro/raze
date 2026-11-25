#include <tests/rts/rts.h>
#include <raze/algorithm/copy/Copy.h>
#include <algorithm>
#include <array>
#include <bit>
#include <cstdint>
#include <vector>

RTTS_CASE_TPL("raze::algorithm::copy", rtts::algorithm::all_types)
< class T > (rtts::type<T>) {
    auto cfg = rtts::algorithm::config::thorough();

    {
        rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
            [](auto c) { return raze::algorithm::copy(c.first(), c.last(), c.out()); },
            [](auto c) { return std::ranges::copy(c.first(), c.last(), c.out()); });

        rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
            [](auto c) { return raze::algorithm::copy(c.range(), c.out()); },
            [](auto c) { return std::ranges::copy(c.range(), c.out()); });
    }

    {
        rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
            [](auto c) { return raze::algorithm::copy[raze::options::fscalar](c.first(), c.last(), c.out()); },
            [](auto c) { return std::ranges::copy(c.first(), c.last(), c.out()); });

        rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
            [](auto c) { return raze::algorithm::copy[raze::options::unroll<2>](c.range(), c.out()); },
            [](auto c) { return std::ranges::copy(c.range(), c.out()); });

        rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
            [](auto c) { return raze::algorithm::copy[raze::options::fstatic](c.range(), c.out()); },
            [](auto c) { return std::ranges::copy(c.range(), c.out()); });

        rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
            [](auto c) { return raze::algorithm::copy[raze::options::fautovec](c.first(), c.last(), c.out()); },
            [](auto c) { return std::ranges::copy(c.first(), c.last(), c.out()); });

        rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
            [](auto c) { return raze::algorithm::copy[raze::options::fautovec][raze::options::fstatic](c.range(), c.out()); },
            [](auto c) { return std::ranges::copy(c.range(), c.out()); });
    }

    {
        namespace opt = raze::options;
        auto acfg = rtts::algorithm::config::compact();

        auto check_aligned = [&]<size_t A1, size_t AOut>(auto algo) {
            rtts::algorithm::run_aligned<A1, AOut, rtts::algorithm::range, T>(acfg,
                [algo](auto c) { return algo(c.first(), c.last(), c.out()); },
                [](auto c) { return std::ranges::copy(c.first(), c.last(), c.out()); });

            rtts::algorithm::run_aligned<A1, AOut, rtts::algorithm::range, T>(acfg,
                [algo](auto c) { return algo(c.range(), c.out()); },
                [](auto c) { return std::ranges::copy(c.range(), c.out()); });
        };

        check_aligned.template operator()<16, 16>(raze::algorithm::copy[opt::aligned<16>]);
        check_aligned.template operator()<32, 32>(raze::algorithm::copy[opt::aligned<32>]);
        check_aligned.template operator()<32, 32>(raze::algorithm::copy[opt::aligned<32>][opt::unroll<2>]);
        check_aligned.template operator()<64, 64>(raze::algorithm::copy[opt::aligned<64>]);
        check_aligned.template operator()<64, 64>(raze::algorithm::copy[opt::aligned<64>][opt::fautovec]);
        check_aligned.template operator()<64, 64>(raze::algorithm::copy[opt::aligned<64>][opt::fscalar]);
        check_aligned.template operator()<1, 1>(raze::algorithm::copy[opt::aligned<opt::unaligned>]);
        check_aligned.template operator()<1, 1>(raze::algorithm::copy[opt::aligned<64>][opt::aligned<opt::unaligned>]);
        check_aligned.template operator()<64, 1>(raze::algorithm::copy[opt::aligned<64, opt::unaligned>]);
        check_aligned.template operator()<1, 64>(raze::algorithm::copy[opt::aligned<opt::unaligned, 64>]);
        check_aligned.template operator()<32, 64>(raze::algorithm::copy[opt::aligned<32, 64>][opt::fstatic]);
        check_aligned.template operator()<64, 16>(raze::algorithm::copy[opt::aligned<64, 16>]);
    }

    static_assert(rtts::algorithm::constexpr_run<T, rtts::algorithm::range, 8>(
        [](auto c) constexpr { return raze::algorithm::copy(c.first(), c.last(), c.out()); },
        [](auto c) constexpr { return std::ranges::copy(c.first(), c.last(), c.out()); }));

    static_assert(rtts::algorithm::constexpr_run<T, rtts::algorithm::range, 8>(
        [](auto c) constexpr { return raze::algorithm::copy(c.range(), c.out()); },
        [](auto c) constexpr { return std::ranges::copy(c.range(), c.out()); }));

    static_assert(rtts::algorithm::constexpr_run<T, rtts::algorithm::range, 8>(
        [](auto c) constexpr { return raze::algorithm::copy[raze::options::fautovec](c.range(), c.out()); },
        [](auto c) constexpr { return std::ranges::copy(c.range(), c.out()); }));

    static_assert(rtts::algorithm::constexpr_run<T, rtts::algorithm::range, 8>(
        [](auto c) constexpr { return raze::algorithm::copy[raze::options::aligned<64, raze::options::unaligned>](c.range(), c.out()); },
        [](auto c) constexpr { return std::ranges::copy(c.range(), c.out()); }));
};

RTTS_CASE("raze::algorithm::copy mixed arithmetic types") {
    auto cfg = rtts::algorithm::config::thorough();

    {
        rtts::algorithm::run<rtts::algorithm::range, int32_t, float>(cfg,
            [](auto c) { return raze::algorithm::copy(c.first(), c.last(), c.out2()); },
            [](auto c) { return std::ranges::copy(c.first(), c.last(), c.out2()); });

        rtts::algorithm::run<rtts::algorithm::range, uint32_t, float>(cfg,
            [](auto c) { return raze::algorithm::copy(c.first(), c.last(), c.out2()); },
            [](auto c) { return std::ranges::copy(c.first(), c.last(), c.out2()); });
    }

    {
        rtts::algorithm::run<rtts::algorithm::range, int64_t, double>(cfg,
            [](auto c) { return raze::algorithm::copy(c.first(), c.last(), c.out2()); },
            [](auto c) { return std::ranges::copy(c.first(), c.last(), c.out2()); });
    }

    {
        rtts::algorithm::run<rtts::algorithm::range, int8_t, int16_t>(cfg,
            [](auto c) { return raze::algorithm::copy(c.first(), c.last(), c.out2()); },
            [](auto c) { return std::ranges::copy(c.first(), c.last(), c.out2()); });

        rtts::algorithm::run<rtts::algorithm::range, int32_t, int64_t>(cfg,
            [](auto c) { return raze::algorithm::copy(c.range(), c.out2()); },
            [](auto c) { return std::ranges::copy(c.range(), c.out2()); });

        rtts::algorithm::run<rtts::algorithm::range, int16_t, int8_t>(cfg,
            [](auto c) { return raze::algorithm::copy(c.first(), c.last(), c.out2()); },
            [](auto c) { return std::ranges::copy(c.first(), c.last(), c.out2()); });
    }

    {
        rtts::algorithm::run<rtts::algorithm::range, int32_t, float>(cfg,
            [](auto c) { return raze::algorithm::copy[raze::options::fscalar](c.first(), c.last(), c.out2()); },
            [](auto c) { return std::ranges::copy(c.first(), c.last(), c.out2()); });

        rtts::algorithm::run<rtts::algorithm::range, int32_t, float>(cfg,
            [](auto c) { return raze::algorithm::copy[raze::options::fautovec](c.first(), c.last(), c.out2()); },
            [](auto c) { return std::ranges::copy(c.first(), c.last(), c.out2()); });

        auto acfg = rtts::algorithm::config::compact();
        rtts::algorithm::run_aligned_mixed<64, 1, rtts::algorithm::range, int32_t, float>(acfg,
            [](auto c) { return raze::algorithm::copy[raze::options::aligned<64, raze::options::unaligned>](c.first(), c.last(), c.out2()); },
            [](auto c) { return std::ranges::copy(c.first(), c.last(), c.out2()); });

        rtts::algorithm::run_aligned_mixed<64, 64, rtts::algorithm::range, int16_t, int8_t>(acfg,
            [](auto c) { return raze::algorithm::copy[raze::options::aligned<64>](c.range(), c.out2()); },
            [](auto c) { return std::ranges::copy(c.range(), c.out2()); });
    }

    static_assert(rtts::algorithm::constexpr_run_mixed<int32_t, float, rtts::algorithm::range, 8>(
        [](auto c) constexpr { return raze::algorithm::copy(c.first(), c.last(), c.out2()); },
        [](auto c) constexpr { return std::ranges::copy(c.first(), c.last(), c.out2()); }));

    static_assert(rtts::algorithm::constexpr_run_mixed<int64_t, double, rtts::algorithm::range, 8>(
        [](auto c) constexpr { return raze::algorithm::copy(c.first(), c.last(), c.out2()); },
        [](auto c) constexpr { return std::ranges::copy(c.first(), c.last(), c.out2()); }));

    static_assert(rtts::algorithm::constexpr_run_mixed<int8_t, int16_t, rtts::algorithm::range, 8>(
        [](auto c) constexpr { return raze::algorithm::copy(c.range(), c.out2()); },
        [](auto c) constexpr { return std::ranges::copy(c.range(), c.out2()); }));
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

RTTS_CASE("raze::algorithm::copy structs") {
    auto cfg = rtts::algorithm::config::thorough();

    {
        rtts::algorithm::run<rtts::algorithm::range, tc_struct_4>(cfg,
            [](auto c) { return raze::algorithm::copy(c.first(), c.last(), c.out()); },
            [](auto c) { return std::ranges::copy(c.first(), c.last(), c.out()); });

        rtts::algorithm::run<rtts::algorithm::range, tc_struct_8>(cfg,
            [](auto c) { return raze::algorithm::copy(c.first(), c.last(), c.out()); },
            [](auto c) { return std::ranges::copy(c.first(), c.last(), c.out()); });

        rtts::algorithm::run<rtts::algorithm::range, tc_struct_12>(cfg,
            [](auto c) { return raze::algorithm::copy(c.first(), c.last(), c.out()); },
            [](auto c) { return std::ranges::copy(c.first(), c.last(), c.out()); });

        rtts::algorithm::run<rtts::algorithm::range, tc_struct_12>(cfg,
            [](auto c) { return raze::algorithm::copy(c.range(), c.out()); },
            [](auto c) { return std::ranges::copy(c.range(), c.out()); });
    }

    {
        rtts::algorithm::run<rtts::algorithm::range, tc_struct_4>(cfg,
            [](auto c) { return raze::algorithm::copy[raze::options::fscalar](c.first(), c.last(), c.out()); },
            [](auto c) { return std::ranges::copy(c.first(), c.last(), c.out()); });

        rtts::algorithm::run<rtts::algorithm::range, tc_struct_4>(cfg,
            [](auto c) { return raze::algorithm::copy[raze::options::unroll<2>](c.first(), c.last(), c.out()); },
            [](auto c) { return std::ranges::copy(c.first(), c.last(), c.out()); });

        rtts::algorithm::run<rtts::algorithm::range, tc_struct_4>(cfg,
            [](auto c) { return raze::algorithm::copy[raze::options::fautovec](c.first(), c.last(), c.out()); },
            [](auto c) { return std::ranges::copy(c.first(), c.last(), c.out()); });

        rtts::algorithm::run_aligned<64, 1, rtts::algorithm::range, tc_struct_4>(rtts::algorithm::config::compact(),
            [](auto c) { return raze::algorithm::copy[raze::options::aligned<64, raze::options::unaligned>](c.range(), c.out()); },
            [](auto c) { return std::ranges::copy(c.range(), c.out()); });
    }

    static_assert(rtts::algorithm::constexpr_run<tc_struct_4, rtts::algorithm::range, 8>(
        [](auto c) constexpr { return raze::algorithm::copy(c.first(), c.last(), c.out()); },
        [](auto c) constexpr { return std::ranges::copy(c.first(), c.last(), c.out()); }));

    static_assert(rtts::algorithm::constexpr_run<tc_struct_8, rtts::algorithm::range, 8>(
        [](auto c) constexpr { return raze::algorithm::copy(c.first(), c.last(), c.out()); },
        [](auto c) constexpr { return std::ranges::copy(c.first(), c.last(), c.out()); }));

    static_assert(rtts::algorithm::constexpr_run<tc_struct_12, rtts::algorithm::range, 8>(
        [](auto c) constexpr { return raze::algorithm::copy(c.range(), c.out()); },
        [](auto c) constexpr { return std::ranges::copy(c.range(), c.out()); }));

    static_assert(rtts::algorithm::constexpr_run<tc_nontrivial, rtts::algorithm::range, 8>(
        [](auto c) constexpr { return raze::algorithm::copy(c.first(), c.last(), c.out()); },
        [](auto c) constexpr { return std::ranges::copy(c.first(), c.last(), c.out()); }));
};

RTTS_CASE("raze::algorithm::copy non trivially copyable") {
    for (const size_t n : { size_t(0), size_t(1), size_t(2), size_t(15), size_t(16), size_t(17), size_t(100) }) {
        std::vector<tc_nontrivial> src(n);
        for (size_t i = 0; i < n; ++i)
            src[i].x = static_cast<int32_t>(i * 7 + 1);

        std::vector<tc_nontrivial> d1(n), d2(n);

        const auto r1 = raze::algorithm::copy(src.begin(), src.end(), d1.begin());
        const auto r2 = std::ranges::copy(src.begin(), src.end(), d2.begin());

        RTTS_EXPECT(d1 == d2);
        RTTS_EXPECT((r1.in - src.begin()) == (r2.in - src.begin()));
        RTTS_EXPECT((r1.out - d1.begin()) == (r2.out - d2.begin()));

        std::vector<tc_nontrivial> d3(n), d4(n);
        const auto r3 = raze::algorithm::copy(src, d3.begin());
        const auto r4 = std::ranges::copy(src, d4.begin());

        RTTS_EXPECT(d3 == d4);
        RTTS_EXPECT((r3.out - d3.begin()) == (r4.out - d4.begin()));
    }
};

RTTS_CASE("raze::algorithm::copy memcpy size boundaries") {
    const size_t sizes[] = {
        0, 1, 2, 3, 4, 7, 8, 9, 15, 16, 17, 31, 32, 33, 63, 64, 65,
        127, 128, 129, 255, 256, 257, 511, 512, 513,
        1023, 1024, 1025, 2047, 2048, 2049, 4095, 4096, 4097, 8191, 8192, 8193,
        65536, 65537,
        3u * 1024 * 1024 - 1, 3u * 1024 * 1024, 3u * 1024 * 1024 + 1, 3u * 1024 * 1024 + 129
    };
    constexpr size_t guard = 64;

    for (const size_t n : sizes) {
        for (const size_t src_off : { size_t(0), size_t(3) }) {
            for (const size_t dst_off : { size_t(0), size_t(1), size_t(7) }) {
                std::vector<uint8_t> src(n + src_off);
                for (size_t i = 0; i < src.size(); ++i)
                    src[i] = static_cast<uint8_t>(i * 131 + 7);

                std::vector<uint8_t> d1(n + dst_off + guard, 0xCC);
                auto d2 = d1;

                const auto first = src.begin() + static_cast<std::ptrdiff_t>(src_off);
                const auto last = src.end();

                const auto r1 = raze::algorithm::copy(first, last, d1.begin() + static_cast<std::ptrdiff_t>(dst_off));
                const auto r2 = std::ranges::copy(first, last, d2.begin() + static_cast<std::ptrdiff_t>(dst_off));

                RTTS_EXPECT(d1 == d2);
                RTTS_EXPECT(r1.in == r2.in);
                RTTS_EXPECT((r1.out - d1.begin()) == (r2.out - d2.begin()));
            }
        }
    }
};


RTTS_CASE("raze::algorithm::copy views and edge cases") {
    namespace opt = raze::options;
    namespace views = std::views;

    // Views: iota, take, drop, reverse, filter, transform, counted, chained, non-common
    {
        const std::vector<int> src = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20 };

        std::vector<int> d1(10), d2(10);
        raze::algorithm::copy(views::iota(10, 20), d1.begin());
        std::ranges::copy(views::iota(10, 20), d2.begin());
        RTTS_EXPECT(d1 == d2);

        raze::algorithm::copy[opt::fstatic](src | views::drop(3) | views::take(10), d1.begin());
        std::ranges::copy(src | views::drop(3) | views::take(10), d2.begin());
        RTTS_EXPECT(d1 == d2);

        std::vector<int> rev1(src.size()), rev2(src.size());
        raze::algorithm::copy(src | views::reverse, rev1.begin());
        std::ranges::copy(src | views::reverse, rev2.begin());
        RTTS_EXPECT(rev1 == rev2);

        auto chain = src
            | views::filter([](int x) { return (x % 2) == 0; })
            | views::transform([](int x) { return x * 3; })
            | views::take(6);
        std::vector<int> c1, c2;
        raze::algorithm::copy[opt::aligned<64>](chain, std::back_inserter(c1));
        std::ranges::copy(chain, std::back_inserter(c2));
        RTTS_EXPECT(c1 == c2);

        std::vector<int> nc1(src.size()), nc2(src.size());
        raze::algorithm::copy(rtts::make_non_common(src), nc1.begin());
        std::ranges::copy(rtts::make_non_common(src), nc2.begin());
        RTTS_EXPECT(nc1 == nc2);
    }

    // Output iterators: back_inserter, front_inserter, inserter (with options & aligned)
    {
        const std::vector<int> src(45, 7);
        std::vector<int> b1, b2, b3, b4, b5;
        raze::algorithm::copy(src, std::back_inserter(b1));
        raze::algorithm::copy[opt::fscalar](src, std::back_inserter(b2));
        raze::algorithm::copy[opt::fstatic](src, std::back_inserter(b3));
        raze::algorithm::copy[opt::fautovec](src, std::back_inserter(b4));
        alignas(64) std::array<int, 45> src_aligned;
        src_aligned.fill(7);
        raze::algorithm::copy[opt::aligned<64, opt::unaligned>](src_aligned, std::back_inserter(b5));
        RTTS_EXPECT(b1 == src && b2 == src && b3 == src && b4 == src && b5 == src);

        const std::vector<int> seq = { 1, 2, 3, 4, 5 };
        std::deque<int> f1, f2;
        raze::algorithm::copy(seq, std::front_inserter(f1));
        std::ranges::copy(seq, std::front_inserter(f2));
        RTTS_EXPECT(f1 == f2);

        std::vector<int> i1 = { 100, 200 }, i2 = { 100, 200 };
        raze::algorithm::copy(seq, std::inserter(i1, i1.begin() + 1));
        std::ranges::copy(seq, std::inserter(i2, i2.begin() + 1));
        RTTS_EXPECT(i1 == i2);

        std::vector<int> empty_src, empty_dst;
        raze::algorithm::copy(empty_src, std::back_inserter(empty_dst));
        RTTS_EXPECT(empty_dst.empty());
    }

    // Single-pass input range -> back_inserter and contiguous dst
    {
        const std::vector<int> data = { 9, 8, 7, 6, 5, 4 };
        rtts::single_pass_input_range<int> sp1(data);
        std::vector<int> out1;
        raze::algorithm::copy(sp1, std::back_inserter(out1));
        RTTS_EXPECT(out1 == data && sp1.reads() == data.size());

        rtts::single_pass_input_range<int> sp2(data);
        std::vector<int> out2(data.size());
        raze::algorithm::copy(sp2, out2.begin());
        RTTS_EXPECT(out2 == data && sp2.reads() == data.size());
    }

    // Volatile src/dst, non-borrowed rvalue (dangling), constexpr views
    {
        volatile int vsrc[16];
        volatile int vdst[16];
        int plain[16]{};
        for (int i = 0; i < 16; ++i) vsrc[i] = i * 3 + 1;

        raze::algorithm::copy(vsrc, vsrc + 16, plain);
        raze::algorithm::copy(plain, plain + 16, vdst);
        for (int i = 0; i < 16; ++i) {
            RTTS_EXPECT(plain[i] == i * 3 + 1);
            RTTS_EXPECT(vdst[i] == i * 3 + 1);
        }

        std::vector<int> rdst(3);
        auto rres = raze::algorithm::copy(std::vector<int>{ 11, 22, 33 }, rdst.begin());
        static_assert(std::same_as<decltype(rres.in), std::ranges::dangling>);
        RTTS_EXPECT((rdst == std::vector<int>{ 11, 22, 33 }));
    }

    static_assert([]() constexpr {
        std::array<int, 5> dst{};
        raze::algorithm::copy(views::iota(1, 6), dst.begin());
        return dst[0] == 1 && dst[4] == 5;
    }());
};
